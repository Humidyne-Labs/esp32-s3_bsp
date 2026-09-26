/**
 * @file bsp_rtc.c
 * @brief PCF85063A Real-Time Clock BSP Driver Implementation
 * 
 * Hardware Target:
 *  - Device: NXP Semiconductors PCF85063A (I2C Address: 0x51)
 *  - Quartz crystal: 32.768 kHz with 12.5pF internal load capacitance
 *  - Interrupt Line: GPIO 5 (BSP_PIN_RTC_INT, Active Low)
 * 
 * Register Map:
 *  - 0x00: Control_1
 *  - 0x01: Control_2
 *  - 0x02: Offset
 *  - 0x03: RAM_byte
 *  - 0x04: Seconds (BCD 00-59, bit 7 = OS)
 *  - 0x05: Minutes (BCD 00-59)
 *  - 0x06: Hours   (BCD 00-23)
 *  - 0x07: Days    (BCD 01-31)
 *  - 0x08: Weekdays(0-6)
 *  - 0x09: Months  (BCD 01-12)
 *  - 0x0A: Years   (BCD 00-99, offset from 2000)
 *  - 0x0B: Alarm_seconds
 *  - 0x0C: Alarm_minutes
 *  - 0x0D: Alarm_hours
 *  - 0x0E: Alarm_days
 *  - 0x0F: Alarm_weekdays
 *  - 0x10: Timer_value
 *  - 0x11: Timer_mode
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics
 * - Microcontroller: Espressif Systems ESP32-S3
 * - Original PCF85063A Component: Espressif / Waveshare (Apache-2.0)
 * - BSP Unification & Enhancements: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp/bsp_i2c.h"
#include "bsp/bsp_rtc.h"
#include "bsp/pinout.h"
#include "bsp/bsp.h"

static const char *TAG = "bsp_rtc";

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

static inline uint8_t bcd_to_binary(uint8_t val)
{
    return (uint8_t)((val & 0x0F) + ((val >> 4) * 10));
}

static inline uint8_t binary_to_bcd(uint8_t val)
{
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

esp_err_t bsp_rtc_init(void)
{
    // 1. Ensure master IO configuration is applied (RTC_INT on GPIO 5)
    bsp_init_io();

    // 2. Ensure I2C bus is initialized
    esp_err_t ret = bsp_i2c_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C bus: %s", esp_err_to_name(ret));
        return ret;
    }

    // 3. Fast probe test (3 retries with short 20ms delays)
    bool ready = false;
    uint8_t test_reg = 0;
    for (int retry = 0; retry < 3; retry++) {
        if (bsp_i2c_read_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_CONTROL_1, &test_reg, 1) == ESP_OK) {
            ready = true;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    if (!ready) {
        ESP_LOGW(TAG, "PCF85063A not responding on I2C address 0x%02X", BSP_I2C_ADDR_PCF85063);
        return ESP_ERR_NOT_FOUND;
    }

    // 4. Configure Control_1 (12.5pF Cap, STOP=0, 24h mode) & Control_2 (CLKOUT disabled)
    const uint8_t init_ctrl[2] = { CTRL1_CAP_SEL_12_5PF, CTRL2_COF_OFF };
    ret = bsp_i2c_write_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_CONTROL_1, init_ctrl, sizeof(init_ctrl));

    // 5. Clear countdown timer, alarm, and interrupt flags so RTC_INT (GPIO 5) releases to HIGH
    bsp_rtc_clear_countdown_timer();
    bsp_rtc_clear_alarm();
    bsp_rtc_get_and_clear_interrupts(NULL, NULL);

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "PCF85063A RTC initialized (12.5pF crystal, 24h mode, INT pin GPIO %d)", BSP_PIN_RTC_INT);
    }
    return ret;
}

esp_err_t bsp_rtc_deinit(void)
{
    return ESP_OK;
}

esp_err_t bsp_rtc_software_reset(void)
{
    uint8_t reset_cmd = CTRL1_SW_RESET_CMD;
    return bsp_i2c_write_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_CONTROL_1, &reset_cmd, 1);
}

esp_err_t bsp_rtc_is_running(bool *is_running)
{
    if (is_running == NULL) return ESP_ERR_INVALID_ARG;

    uint8_t sec_reg = 0;
    esp_err_t ret = bsp_i2c_read_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_TIME, &sec_reg, 1);
    if (ret != ESP_OK) return ret;

    // Bit 7 is OS (Oscillator Stop): 1 = stopped / power loss, 0 = integrity guaranteed
    *is_running = ((sec_reg & 0x80) == 0);
    return ESP_OK;
}

esp_err_t bsp_rtc_get_datetime(bsp_rtc_datetime_t *datetime)
{
    if (datetime == NULL) return ESP_ERR_INVALID_ARG;

    // Read all 7 date/time registers in one atomic burst to prevent carry/rollover errors
    uint8_t regs[7] = {0};
    esp_err_t ret = bsp_i2c_read_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_TIME, regs, sizeof(regs));
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

    // Stop prescaler briefly to write clean time values (Section 8.2.1.2)
    uint8_t stop_cmd = CTRL1_STOP | CTRL1_CAP_SEL_12_5PF;
    bsp_i2c_write_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_CONTROL_1, &stop_cmd, 1);

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
    esp_err_t ret = bsp_i2c_write_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_TIME, regs, sizeof(regs));

    // Release prescaler to restart time counter
    uint8_t start_cmd = CTRL1_CAP_SEL_12_5PF;
    bsp_i2c_write_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_CONTROL_1, &start_cmd, 1);

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "RTC Set Time -> %04d-%02d-%02d %02d:%02d:%02d (Day: %d)",
                 datetime->year, datetime->month, datetime->day,
                 datetime->hour, datetime->minute, datetime->second, datetime->weekday);
    }
    return ret;
}

esp_err_t bsp_rtc_set_offset(int8_t offset, bsp_rtc_offset_mode_t mode)
{
    if (offset < -64 || offset > 63) return ESP_ERR_INVALID_ARG;

    uint8_t val = (uint8_t)offset & 0x7F;
    if (mode == BSP_RTC_OFFSET_MODE_4_MIN) {
        val |= 0x80;
    }
    return bsp_i2c_write_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_OFFSET, &val, 1);
}

esp_err_t bsp_rtc_set_alarm(const bsp_rtc_alarm_t *alarm)
{
    if (alarm == NULL) return ESP_ERR_INVALID_ARG;

    // Bit 7 = 0 enables matching; Bit 7 = 1 disables matching (ignored)
    uint8_t alarm_regs[5] = {
        (alarm->second  >= 0) ? (uint8_t)(binary_to_bcd((uint8_t)alarm->second)  & 0x7F) : 0x80,
        (alarm->minute  >= 0) ? (uint8_t)(binary_to_bcd((uint8_t)alarm->minute)  & 0x7F) : 0x80,
        (alarm->hour    >= 0) ? (uint8_t)(binary_to_bcd((uint8_t)alarm->hour)    & 0x3F) : 0x80,
        (alarm->day     >= 0) ? (uint8_t)(binary_to_bcd((uint8_t)alarm->day)     & 0x3F) : 0x80,
        (alarm->weekday >= 0) ? (uint8_t)(binary_to_bcd((uint8_t)alarm->weekday) & 0x07) : 0x80,
    };

    esp_err_t ret = bsp_i2c_write_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_ALARM_SEC, alarm_regs, sizeof(alarm_regs));
    if (ret != ESP_OK) return ret;

    // Read-modify-write Control_2: enable AIE (bit 7), clear AF (bit 6 = 0), keep COF disabled
    uint8_t ctrl2 = 0;
    bsp_i2c_read_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_CONTROL_2, &ctrl2, 1);
    ctrl2 = (ctrl2 & ~CTRL2_AF) | CTRL2_AIE | CTRL2_COF_OFF;
    return bsp_i2c_write_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_CONTROL_2, &ctrl2, 1);
}

esp_err_t bsp_rtc_clear_alarm(void)
{
    uint8_t disabled[5] = {0x80, 0x80, 0x80, 0x80, 0x80};
    bsp_i2c_write_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_ALARM_SEC, disabled, sizeof(disabled));

    uint8_t ctrl2 = 0;
    bsp_i2c_read_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_CONTROL_2, &ctrl2, 1);
    ctrl2 = (ctrl2 & ~(CTRL2_AIE | CTRL2_AF)) | CTRL2_COF_OFF;
    return bsp_i2c_write_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_CONTROL_2, &ctrl2, 1);
}

esp_err_t bsp_rtc_set_countdown_timer(uint8_t seconds)
{
    if (seconds == 0) return ESP_ERR_INVALID_ARG;

    // 1. Disable timer before reloading value
    uint8_t mode = 0x00;
    bsp_i2c_write_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_TIMER_MOD, &mode, 1);

    // 2. Clear any pending TF and AF flags so INT pin is de-asserted (HIGH)
    uint8_t ctrl2 = 0;
    bsp_i2c_read_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_CONTROL_2, &ctrl2, 1);
    ctrl2 = (ctrl2 & ~(CTRL2_TF | CTRL2_AF)) | CTRL2_COF_OFF;
    bsp_i2c_write_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_CONTROL_2, &ctrl2, 1);

    // 3. Set timer countdown value
    bsp_i2c_write_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_TIMER_VAL, &seconds, 1);

    // 4. Configure 1Hz source, enable timer (TE=1), enable INT (TIE=1), level mode (TI_TP=0)
    mode = TIMER_MODE_TCF_1HZ | TIMER_MODE_TE | TIMER_MODE_TIE;
    return bsp_i2c_write_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_TIMER_MOD, &mode, 1);
}

esp_err_t bsp_rtc_clear_countdown_timer(void)
{
    uint8_t mode = 0x00;
    bsp_i2c_write_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_TIMER_MOD, &mode, 1);

    uint8_t ctrl2 = 0;
    bsp_i2c_read_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_CONTROL_2, &ctrl2, 1);
    ctrl2 = (ctrl2 & ~(CTRL2_TF | CTRL2_AF)) | CTRL2_COF_OFF;
    return bsp_i2c_write_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_CONTROL_2, &ctrl2, 1);
}

esp_err_t bsp_rtc_get_and_clear_interrupts(bool *alarm_flag, bool *timer_flag)
{
    uint8_t ctrl2 = 0;
    esp_err_t ret = bsp_i2c_read_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_CONTROL_2, &ctrl2, 1);
    if (ret != ESP_OK) return ret;

    if (alarm_flag) *alarm_flag = (ctrl2 & CTRL2_AF) != 0;
    if (timer_flag) *timer_flag = (ctrl2 & CTRL2_TF) != 0;

    // Clear AF and TF flags by writing 0 while preserving AIE and COF
    uint8_t clear_ctrl2 = (ctrl2 & ~(CTRL2_AF | CTRL2_TF)) | CTRL2_COF_OFF;
    return bsp_i2c_write_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_CONTROL_2, &clear_ctrl2, 1);
}

esp_err_t bsp_rtc_disable_clkout(void)
{
    uint8_t ctrl2 = 0;
    esp_err_t ret = bsp_i2c_read_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_CONTROL_2, &ctrl2, 1);
    if (ret != ESP_OK) return ret;

    // Set bits 2:0 to 111b (0x07 = COF_OFF) to disable CLKOUT
    ctrl2 |= CTRL2_COF_OFF;
    return bsp_i2c_write_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_CONTROL_2, &ctrl2, 1);
}

esp_err_t bsp_rtc_stop_oscillator(void)
{
    uint8_t ctrl1 = 0;
    esp_err_t ret = bsp_i2c_read_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_CONTROL_1, &ctrl1, 1);
    if (ret != ESP_OK) return ret;

    ctrl1 |= CTRL1_STOP; // STOP = 1 halts the 32kHz crystal oscillator and divider chain
    ret = bsp_i2c_write_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_CONTROL_1, &ctrl1, 1);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "PCF85063A 32kHz crystal oscillator halted (STOP=1)");
    }
    return ret;
}

esp_err_t bsp_rtc_start_oscillator(void)
{
    uint8_t ctrl1 = 0;
    esp_err_t ret = bsp_i2c_read_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_CONTROL_1, &ctrl1, 1);
    if (ret != ESP_OK) return ret;

    ctrl1 &= ~CTRL1_STOP; // STOP = 0 resumes quartz crystal oscillator
    ctrl1 |= CTRL1_CAP_SEL_12_5PF;
    ret = bsp_i2c_write_reg(BSP_I2C_ADDR_PCF85063, BSP_RTC_REG_CONTROL_1, &ctrl1, 1);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "PCF85063A 32kHz crystal oscillator running (STOP=0)");
    }
    return ret;
}

esp_err_t bsp_rtc_enable_wakeup(bool deep_sleep)
{
    gpio_pullup_en((gpio_num_t)BSP_PIN_RTC_INT);
    gpio_pulldown_dis((gpio_num_t)BSP_PIN_RTC_INT);

    if (deep_sleep) {
        return esp_sleep_enable_ext0_wakeup((gpio_num_t)BSP_PIN_RTC_INT, 0);
    } else {
        gpio_wakeup_enable((gpio_num_t)BSP_PIN_RTC_INT, GPIO_INTR_LOW_LEVEL);
        return esp_sleep_enable_gpio_wakeup();
    }
}
