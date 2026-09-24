/**
 * @file bsp_rtc.c
 * @brief PCF85063A Real-Time Clock BSP Driver Implementation
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics
 * - Microcontroller: Espressif Systems ESP32-S3
 * - Original PCF85063A Component: Espressif / Waveshare (Apache-2.0)
 * - BSP Unification & Enhancements: Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include "esp_log.h"
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp/bsp_i2c.h"
#include "bsp/bsp_rtc.h"

#define BSP_RTC_I2C_ADDRESS     0x51

#define BSP_RTC_REG_CONTROL_1   0x00
#define BSP_RTC_REG_CONTROL_2   0x01
#define BSP_RTC_REG_OFFSET      0x02
#define BSP_RTC_REG_RAM_BYTE    0x03
#define BSP_RTC_REG_TIME        0x04
#define BSP_RTC_REG_ALARM_SEC   0x0B
#define BSP_RTC_REG_TIMER_VAL   0x10
#define BSP_RTC_REG_TIMER_MOD   0x11

/* Control_1 bitmasks */
#define CTRL1_EXT_TEST          (1 << 7)
#define CTRL1_STOP              (1 << 5)
#define CTRL1_SR                (1 << 4)
#define CTRL1_CIE               (1 << 2)
#define CTRL1_12_24             (1 << 1)
#define CTRL1_CAP_SEL_12_5PF    (1 << 0)
#define CTRL1_SW_RESET_CMD      0x58

/* Control_2 bitmasks */
#define CTRL2_AIE               (1 << 7)
#define CTRL2_AF                (1 << 6)
#define CTRL2_MI                (1 << 5)
#define CTRL2_HMI               (1 << 4)
#define CTRL2_TF                (1 << 3)
#define CTRL2_COF_OFF           0x07

/* Timer Mode bitmasks */
#define TIMER_MODE_TCF_1HZ      (2 << 3)
#define TIMER_MODE_TE           (1 << 2)
#define TIMER_MODE_TIE          (1 << 1)
#define TIMER_MODE_TI_TP        (1 << 0)

static const char *TAG = "bsp_rtc";
static i2c_master_dev_handle_t s_rtc_handle = NULL;

static inline uint8_t bcd_to_binary(uint8_t val) { 
    return (uint8_t)((val & 0x0F) + ((val >> 4) * 10)); 
}

static inline uint8_t binary_to_bcd(uint8_t val) { 
    return (uint8_t)(((val / 10) << 4) | (val % 10)); 
}

static bool datetime_is_valid(const bsp_rtc_datetime_t *datetime)
{
    return datetime != NULL &&
           datetime->year    >= 2000 && datetime->year   <= 2099 &&
           datetime->month   >= 1    && datetime->month  <= 12   &&
           datetime->day     >= 1    && datetime->day    <= 31   &&
           datetime->weekday <= 6    &&
           datetime->hour    <= 23   && datetime->minute <= 59   &&
           datetime->second  <= 59;
}

esp_err_t bsp_rtc_deinit(void)
{
    if (s_rtc_handle != NULL) {
        i2c_master_bus_rm_device(s_rtc_handle);
        s_rtc_handle = NULL;
    }
    return ESP_OK;
}

esp_err_t bsp_rtc_software_reset(void)
{
    if (s_rtc_handle == NULL) {
        esp_err_t ret = bsp_rtc_init();
        if (ret != ESP_OK) return ret;
    }
    uint8_t reset_cmd = CTRL1_SW_RESET_CMD;
    return bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_CONTROL_1, &reset_cmd, 1);
}

esp_err_t bsp_rtc_init(void)
{
    if (s_rtc_handle != NULL) return ESP_OK;

    // 1. Configure the RTC INT GPIO5 as open-drain input with pullup
    gpio_config_t int_conf = {
        .pin_bit_mask = (1ULL << BSP_GPIO_RTC_INT),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&int_conf);

    // 2. Register PCF85063A on shared I2C bus
    const i2c_device_config_t device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = BSP_RTC_I2C_ADDRESS,
        .scl_speed_hz    = 100000, // 100kHz standard mode for shared bus safety
    };

    esp_err_t ret = bsp_i2c_add_device(&device_config, &s_rtc_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register PCF85063A on I2C bus: %s", esp_err_to_name(ret));
        return ret;
    }

    // 3. Fast probe test (3 retries with short 20ms delays)
    bool ready = false;
    uint8_t test_reg = 0;
    for (int retry = 0; retry < 3; retry++) {
        if (bsp_i2c_read_reg(s_rtc_handle, BSP_RTC_REG_CONTROL_1, &test_reg, 1) == ESP_OK) {
            ready = true;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    if (!ready) {
        // Return without deinit so late-starting background tasks can communicate once the rail powers up
        return ESP_ERR_NOT_FOUND;
    }

    // 4. Configure Control_1 (12.5pF Cap, STOP=0, 24h mode) & Control_2 (CLKOUT disabled)
    const uint8_t init_ctrl[2] = { CTRL1_CAP_SEL_12_5PF, CTRL2_COF_OFF };
    bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_CONTROL_1, init_ctrl, sizeof(init_ctrl));

    return ESP_OK;
}

esp_err_t bsp_rtc_is_running(bool *is_running)
{
    if (is_running == NULL) return ESP_ERR_INVALID_ARG;
    if (s_rtc_handle == NULL) {
        esp_err_t ret = bsp_rtc_init();
        if (ret != ESP_OK) return ret;
    }

    uint8_t sec_reg = 0;
    esp_err_t ret = bsp_i2c_read_reg(s_rtc_handle, BSP_RTC_REG_TIME, &sec_reg, 1);
    if (ret != ESP_OK) return ret;

    // Bit 7 is OS (Oscillator Stop): 1 = stopped, 0 = integrity guaranteed
    *is_running = ((sec_reg & 0x80) == 0);
    return ESP_OK;
}

esp_err_t bsp_rtc_get_datetime(bsp_rtc_datetime_t *datetime)
{
    if (datetime == NULL) return ESP_ERR_INVALID_ARG;
    if (s_rtc_handle == NULL) {
        esp_err_t ret = bsp_rtc_init();
        if (ret != ESP_OK) return ret;
    }

    // Must read all 7 date/time registers in one atomic burst to prevent carry errors
    uint8_t regs[7] = {0};
    esp_err_t ret = bsp_i2c_read_reg(s_rtc_handle, BSP_RTC_REG_TIME, regs, sizeof(regs));
    if (ret != ESP_OK) return ret;

    if (regs[0] & 0x80) {
        ESP_LOGW(TAG, "Time integrity lost: OS flag is set");
    }

    datetime->second  = bcd_to_binary(regs[0] & 0x7F);
    datetime->minute  = bcd_to_binary(regs[1] & 0x7F);
    datetime->hour    = bcd_to_binary(regs[2] & 0x3F);
    datetime->day     = bcd_to_binary(regs[3] & 0x3F);
    datetime->weekday = bcd_to_binary(regs[4] & 0x07);
    datetime->month   = bcd_to_binary(regs[5] & 0x1F);
    datetime->year    = (uint16_t)(2000 + bcd_to_binary(regs[6]));

    return ESP_OK;
}

esp_err_t bsp_rtc_set_datetime(const bsp_rtc_datetime_t *datetime)
{
    if (!datetime_is_valid(datetime)) return ESP_ERR_INVALID_ARG;
    if (s_rtc_handle == NULL) {
        esp_err_t ret = bsp_rtc_init();
        if (ret != ESP_OK) return ret;
    }

    // Stop prescaler briefly to write clean time values (Section 8.2.1.2)
    uint8_t stop_cmd = CTRL1_STOP | CTRL1_CAP_SEL_12_5PF;
    bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_CONTROL_1, &stop_cmd, 1);

    // Writing 0 to bit 7 of seconds register clears the OS flag
    const uint8_t regs[7] = {
        (uint8_t)(binary_to_bcd(datetime->second) & 0x7F),
        binary_to_bcd(datetime->minute),
        binary_to_bcd(datetime->hour),
        binary_to_bcd(datetime->day),
        binary_to_bcd(datetime->weekday),
        binary_to_bcd(datetime->month),
        binary_to_bcd((uint8_t)(datetime->year - 2000)),
    };
    esp_err_t ret = bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_TIME, regs, sizeof(regs));

    // Release prescaler to restart time counter
    uint8_t start_cmd = CTRL1_CAP_SEL_12_5PF;
    bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_CONTROL_1, &start_cmd, 1);

    return ret;
}

esp_err_t bsp_rtc_set_offset(int8_t offset, bsp_rtc_offset_mode_t mode)
{
    if (offset < -64 || offset > 63) return ESP_ERR_INVALID_ARG;
    if (s_rtc_handle == NULL) {
        esp_err_t ret = bsp_rtc_init();
        if (ret != ESP_OK) return ret;
    }

    uint8_t val = (uint8_t)offset & 0x7F;
    if (mode == BSP_RTC_OFFSET_MODE_4_MIN) {
        val |= 0x80;
    }
    return bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_OFFSET, &val, 1);
}

esp_err_t bsp_rtc_set_alarm(const bsp_rtc_alarm_t *alarm)
{
    if (alarm == NULL) return ESP_ERR_INVALID_ARG;
    if (s_rtc_handle == NULL) {
        esp_err_t ret = bsp_rtc_init();
        if (ret != ESP_OK) return ret;
    }

    // Bit 7 = 0 enables matching; Bit 7 = 1 disables matching (ignored)
    uint8_t alarm_regs[5] = {
        (alarm->second  >= 0) ? (binary_to_bcd((uint8_t)alarm->second)  & 0x7F) : 0x80,
        (alarm->minute  >= 0) ? (binary_to_bcd((uint8_t)alarm->minute)  & 0x7F) : 0x80,
        (alarm->hour    >= 0) ? (binary_to_bcd((uint8_t)alarm->hour)    & 0x3F) : 0x80,
        (alarm->day     >= 0) ? (binary_to_bcd((uint8_t)alarm->day)     & 0x3F) : 0x80,
        (alarm->weekday >= 0) ? (binary_to_bcd((uint8_t)alarm->weekday) & 0x07) : 0x80,
    };

    esp_err_t ret = bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_ALARM_SEC, alarm_regs, sizeof(alarm_regs));
    if (ret != ESP_OK) return ret;

    // Read-modify-write Control_2: enable AIE (bit 7), clear AF (bit 6 = 0), keep COF disabled
    uint8_t ctrl2 = 0;
    bsp_i2c_read_reg(s_rtc_handle, BSP_RTC_REG_CONTROL_2, &ctrl2, 1);
    ctrl2 = (ctrl2 & ~CTRL2_AF) | CTRL2_AIE | CTRL2_COF_OFF;
    return bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_CONTROL_2, &ctrl2, 1);
}

esp_err_t bsp_rtc_clear_alarm(void)
{
    if (s_rtc_handle == NULL) return ESP_OK;

    uint8_t disabled[5] = {0x80, 0x80, 0x80, 0x80, 0x80};
    bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_ALARM_SEC, disabled, sizeof(disabled));

    uint8_t ctrl2 = 0;
    bsp_i2c_read_reg(s_rtc_handle, BSP_RTC_REG_CONTROL_2, &ctrl2, 1);
    ctrl2 = (ctrl2 & ~(CTRL2_AIE | CTRL2_AF)) | CTRL2_COF_OFF;
    return bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_CONTROL_2, &ctrl2, 1);
}

esp_err_t bsp_rtc_set_countdown_timer(uint8_t seconds)
{
    if (seconds == 0) return ESP_ERR_INVALID_ARG;
    if (s_rtc_handle == NULL) {
        esp_err_t ret = bsp_rtc_init();
        if (ret != ESP_OK) return ret;
    }

    // 1. Disable timer before reloading value
    uint8_t mode = 0x00;
    bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_TIMER_MOD, &mode, 1);

    // 2. Set timer countdown value
    bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_TIMER_VAL, &seconds, 1);

    // 3. Configure 1Hz source, enable timer (TE=1), enable INT (TIE=1), level mode (TI_TP=0)
    mode = TIMER_MODE_TCF_1HZ | TIMER_MODE_TE | TIMER_MODE_TIE;
    return bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_TIMER_MOD, &mode, 1);
}

esp_err_t bsp_rtc_clear_countdown_timer(void)
{
    if (s_rtc_handle == NULL) return ESP_OK;
    uint8_t mode = 0x00;
    return bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_TIMER_MOD, &mode, 1);
}

esp_err_t bsp_rtc_get_and_clear_interrupts(bool *alarm_flag, bool *timer_flag)
{
    if (s_rtc_handle == NULL) return ESP_ERR_INVALID_STATE;

    uint8_t ctrl2 = 0;
    esp_err_t ret = bsp_i2c_read_reg(s_rtc_handle, BSP_RTC_REG_CONTROL_2, &ctrl2, 1);
    if (ret != ESP_OK) return ret;

    if (alarm_flag) *alarm_flag = (ctrl2 & CTRL2_AF) != 0;
    if (timer_flag) *timer_flag = (ctrl2 & CTRL2_TF) != 0;

    // Clear AF and TF flags by writing 0 while preserving AIE and COF
    uint8_t clear_ctrl2 = (ctrl2 & ~(CTRL2_AF | CTRL2_TF)) | CTRL2_COF_OFF;
    return bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_CONTROL_2, &clear_ctrl2, 1);
}

esp_err_t bsp_rtc_enable_wakeup(bool deep_sleep)
{
    gpio_pullup_en((gpio_num_t)BSP_GPIO_RTC_INT);
    gpio_pulldown_dis((gpio_num_t)BSP_GPIO_RTC_INT);

    if (deep_sleep) {
        return esp_sleep_enable_ext0_wakeup((gpio_num_t)BSP_GPIO_RTC_INT, 0);
    } else {
        gpio_wakeup_enable((gpio_num_t)BSP_GPIO_RTC_INT, GPIO_INTR_LOW_LEVEL);
        return esp_sleep_enable_gpio_wakeup();
    }
}

/*
void app_main(void)
{
    // 1. Initialize shared I2C master bus
    ESP_ERROR_CHECK(bsp_i2c_init());

    // 2. Initialize RTC
    ESP_ERROR_CHECK(bsp_rtc_init());

    // 3. Check if time was lost (e.g., initial battery install or power failure)
    bool is_running = false;
    bsp_rtc_is_running(&is_running);

    if (!is_running) {
        ESP_LOGW(TAG, "RTC power-loss detected. Programming default datetime...");
        bsp_rtc_datetime_t init_time = {
            .year    = 2026,
            .month   = 9,
            .day     = 23,
            .weekday = 3, // Wednesday
            .hour    = 12,
            .minute  = 0,
            .second  = 0,
        };
        ESP_ERROR_CHECK(bsp_rtc_set_datetime(&init_time));
    }

    // 4. Check wake-up reason
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    if (cause == ESP_SLEEP_WAKEUP_EXT0) {
        bool alarm_hit = false, timer_hit = false;
        bsp_rtc_get_and_clear_interrupts(&alarm_hit, &timer_hit);
        ESP_LOGI(TAG, "Woke up from RTC interrupt! (Alarm: %d, Timer: %d)", alarm_hit, timer_hit);
    }

    // 5. Read current timestamp
    bsp_rtc_datetime_t now;
    if (bsp_rtc_get_datetime(&now) == ESP_OK) {
        ESP_LOGI(TAG, "Current Time: %04d-%02d-%02d %02d:%02d:%02d (Day: %d)",
                 now.year, now.month, now.day, now.hour, now.minute, now.second, now.weekday);
    }

    // 6. Set a 30-second countdown timer for next deep sleep
    ESP_LOGI(TAG, "Arming 30s countdown timer and entering deep sleep...");
    bsp_rtc_set_countdown_timer(30);
    bsp_rtc_enable_wakeup(true);

    vTaskDelay(pdMS_TO_TICKS(100)); // Allow logs to flush
    esp_deep_sleep_start();
}
*/