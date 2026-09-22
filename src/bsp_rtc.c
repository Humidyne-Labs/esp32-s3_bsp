/**
 * @file bsp_rtc.c
 * @brief rtc controller lib
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics (https://www.waveshare.com)
 * - Microcontroller: Espressif Systems ESP32-S3 (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 * Original License-Identifier: Apache License
 * Adapted from: https://components.espressif.com/components/waveshare/pcf85063a/versions/2.1.0/readme
 */

#include <string.h>
#include "esp_log.h"
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp/bsp_i2c.h"
#include "bsp/bsp_rtc.h"

#define BSP_RTC_I2C_ADDRESS   0x51
#define BSP_RTC_REG_CONTROL_1 0x00
#define BSP_RTC_REG_CONTROL_2 0x01
#define BSP_RTC_REG_TIME      0x04
#define BSP_RTC_REG_ALARM_SEC 0x0B
#define BSP_RTC_REG_TIMER_VAL 0x10
#define BSP_RTC_REG_TIMER_MOD 0x11

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

esp_err_t bsp_rtc_init(void)
{
    if (s_rtc_handle != NULL) return ESP_OK;

    // 1. Configure the RTC INT GPIO5 as input with internal pullup
    gpio_config_t int_conf = {
        .pin_bit_mask = (1ULL << BSP_GPIO_RTC_INT),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,   // PCF85063A INT pin is open-drain
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&int_conf);

    // 2. Register PCF85063A on shared I2C bus at 100kHz standard mode
    const i2c_device_config_t device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = BSP_RTC_I2C_ADDRESS,
        .scl_speed_hz    = 100000,
    };

    esp_err_t ret = bsp_i2c_add_device(&device_config, &s_rtc_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register PCF85063A on I2C bus: %s", esp_err_to_name(ret));
        return ret;
    }

    /*
    // 3. Fast probe with short retry to allow oscillator stabilization
    bool device_ready = false;
    for (int retry = 0; retry < 3; retry++) {
        uint8_t test_reg = 0;
        if (bsp_i2c_read_reg(s_rtc_handle, BSP_RTC_REG_CONTROL_1, &test_reg, 1) == ESP_OK) {
            device_ready = true;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(250)); // Wait 250ms before retrying
    }

    if (!device_ready) {
        ESP_LOGW(TAG, "PCF85063A not responding on I2C (check power rail)");
        bsp_rtc_deinit();
        return ESP_ERR_NOT_FOUND;
    }
    */
    
    // 4. Configure Low-Power Operational Settings:
    // Control_1 (0x00):
    //   - STOP = 0 (Clock running)
    //   - 12_24 = 0 (24-hour mode)
    //   - CAP_SEL = 1 (12.5 pF load capacitance for stable crystal oscillation)
    //   -> Value: 0x01
    // Control_2 (0x01):
    //   - COF[2:0] = 111 (Disable CLKOUT output to save power & reduce noise)
    //   - Clear all flags (AF=0, TF=0)
    //   -> Value: 0x07
    const uint8_t init_ctrl[2] = {0x01, 0x07};
    ret = bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_CONTROL_1, init_ctrl, sizeof(init_ctrl));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write PCF85063A configuration: %s", esp_err_to_name(ret));
        bsp_rtc_deinit();
        return ret;
    }

    ESP_LOGI(TAG, "PCF85063A RTC initialized (12.5pF Cap, CLKOUT disabled, INT: GPIO %d)", BSP_GPIO_RTC_INT);
    return ESP_OK;
}

/*
esp_err_t bsp_rtc_reset(void)
{
    if (s_rtc_handle == NULL) {
        esp_err_t ret = bsp_rtc_init();
        if (ret != ESP_OK) return ret;
    }

    // Software Reset: Write 0x58 to Control_1 (Section 8.2.1.3)
    uint8_t reset_cmd = 0x58;
    esp_err_t ret = bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_CONTROL_1, &reset_cmd, 1);
    vTaskDelay(pdMS_TO_TICKS(10));

    // Reapply 12.5pF capacitance & disable CLKOUT
    const uint8_t init_ctrl[2] = {0x01, 0x07};
    bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_CONTROL_1, init_ctrl, sizeof(init_ctrl));
    return ret;
}
*/

esp_err_t bsp_rtc_get_datetime(bsp_rtc_datetime_t *datetime)
{
    if (datetime == NULL) return ESP_ERR_INVALID_ARG;
    if (s_rtc_handle == NULL) {
        esp_err_t ret = bsp_rtc_init();
        if (ret != ESP_OK) return ret;
    }

    uint8_t registers[7] = {0};
    esp_err_t ret = bsp_i2c_read_reg(s_rtc_handle, BSP_RTC_REG_TIME, registers, sizeof(registers));
    if (ret != ESP_OK) return ret;

    // Check OS (Oscillator Stop) flag (Bit 7 of seconds register)
    if (registers[0] & 0x80) {
        ESP_LOGW(TAG, "RTC oscillator stopped / battery power loss detected");
    }

    datetime->second  = bcd_to_binary(registers[0] & 0x7F);
    datetime->minute  = bcd_to_binary(registers[1] & 0x7F);
    datetime->hour    = bcd_to_binary(registers[2] & 0x3F);
    datetime->day     = bcd_to_binary(registers[3] & 0x3F);
    datetime->weekday = bcd_to_binary(registers[4] & 0x07);
    datetime->month   = bcd_to_binary(registers[5] & 0x1F);
    datetime->year    = (uint16_t)(2000 + bcd_to_binary(registers[6]));
    return ESP_OK;
}

esp_err_t bsp_rtc_set_datetime(const bsp_rtc_datetime_t *datetime)
{
    if (!datetime_is_valid(datetime)) return ESP_ERR_INVALID_ARG;
    if (s_rtc_handle == NULL) {
        esp_err_t ret = bsp_rtc_init();
        if (ret != ESP_OK) return ret;
    }

    const uint8_t registers[7] = {
        (uint8_t)(binary_to_bcd(datetime->second) & 0x7F), // Clear OS flag
        binary_to_bcd(datetime->minute),
        binary_to_bcd(datetime->hour),
        binary_to_bcd(datetime->day),
        binary_to_bcd(datetime->weekday),
        binary_to_bcd(datetime->month),
        binary_to_bcd((uint8_t)(datetime->year - 2000)),
    };
    return bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_TIME, registers, sizeof(registers));
}

esp_err_t bsp_rtc_set_alarm(const bsp_rtc_alarm_t *alarm)
{
    if (alarm == NULL) return ESP_ERR_INVALID_ARG;
    if (s_rtc_handle == NULL) {
        esp_err_t ret = bsp_rtc_init();
        if (ret != ESP_OK) return ret;
    }

    // Datasheet Table 29-33: Bit 7 = 0 enables matching, 1 disables (ignores) matching
    uint8_t alarm_regs[5] = {
        (alarm->second  >= 0) ? (binary_to_bcd((uint8_t)alarm->second)  & 0x7F) : 0x80,
        (alarm->minute  >= 0) ? (binary_to_bcd((uint8_t)alarm->minute)  & 0x7F) : 0x80,
        (alarm->hour    >= 0) ? (binary_to_bcd((uint8_t)alarm->hour)    & 0x3F) : 0x80,
        (alarm->day     >= 0) ? (binary_to_bcd((uint8_t)alarm->day)     & 0x3F) : 0x80,
        (alarm->weekday >= 0) ? (binary_to_bcd((uint8_t)alarm->weekday) & 0x07) : 0x80,
    };

    esp_err_t ret = bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_ALARM_SEC, alarm_regs, sizeof(alarm_regs));
    if (ret != ESP_OK) return ret;

    // Enable Alarm Interrupt (AIE = Bit 7), clear AF (Bit 6 = 0)
    uint8_t ctrl2 = 0x80;
    return bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_CONTROL_2, &ctrl2, 1);
}

esp_err_t bsp_rtc_clear_alarm(void)
{
    if (s_rtc_handle == NULL) return ESP_OK;

    uint8_t disabled[5] = {0x80, 0x80, 0x80, 0x80, 0x80};
    bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_ALARM_SEC, disabled, sizeof(disabled));

    uint8_t ctrl2 = 0x07; // CLKOUT off, interrupts disabled
    return bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_CONTROL_2, &ctrl2, 1);
}

esp_err_t bsp_rtc_set_countdown_timer(uint8_t seconds)
{
    if (seconds == 0) return ESP_ERR_INVALID_ARG;
    if (s_rtc_handle == NULL) {
        esp_err_t ret = bsp_rtc_init();
        if (ret != ESP_OK) return ret;
    }

    // 1. Disable timer before setting new countdown value (Section 8.6.3)
    uint8_t mode = 0x00;
    bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_TIMER_MOD, &mode, 1);

    // 2. Set countdown value
    bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_TIMER_VAL, &seconds, 1);

    // 3. Mode: 1Hz clock (TCF=10), Timer Enable (TE=1), INT Enable (TIE=1), Level Mode (TI_TP=0)
    // TCF=10 -> 0x10, TE=1 -> 0x04, TIE=1 -> 0x02, TI_TP=0 -> 0x00 => 0x16
    mode = 0x16;
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

    if (alarm_flag) *alarm_flag = (ctrl2 & 0x40) != 0; // AF (Bit 6)
    if (timer_flag) *timer_flag = (ctrl2 & 0x08) != 0; // TF (Bit 3)

    // Clear AF and TF flags while maintaining AIE/TIE and CLKOUT disabled
    uint8_t clear_ctrl2 = (ctrl2 & ~(0x40 | 0x08)) | 0x07;
    return bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_CONTROL_2, &clear_ctrl2, 1);
}

esp_err_t bsp_rtc_enable_wakeup(bool deep_sleep)
{
    // Ensure GPIO5 has pull-up enabled during sleep
    gpio_pullup_en((gpio_num_t)BSP_GPIO_RTC_INT);
    gpio_pulldown_dis((gpio_num_t)BSP_GPIO_RTC_INT);

    if (deep_sleep) {
        // Deep sleep EXT0 wakeup: wakes when GPIO5 is pulled LOW (0)
        return esp_sleep_enable_ext0_wakeup((gpio_num_t)BSP_GPIO_RTC_INT, 0);
    } else {
        // Light sleep GPIO wakeup
        gpio_wakeup_enable((gpio_num_t)BSP_GPIO_RTC_INT, GPIO_INTR_LOW_LEVEL);
        return esp_sleep_enable_gpio_wakeup();
    }
}