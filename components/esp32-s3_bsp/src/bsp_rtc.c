/**
 * @file bsp_rtc.c
 * @brief NXP PCF85063A Real-Time Clock Driver Implementation
 * 
 * Register Layout:
 *  - 0x00: Control_1
 *  - 0x01: Control_2
 *  - 0x02: Offset
 *  - 0x03: RAM_byte
 *  - 0x04: Seconds (BCD 00-59)
 *  - 0x05: Minutes (BCD 00-59)
 *  - 0x06: Hours   (BCD 00-23)
 *  - 0x07: Days    (BCD 01-31)
 *  - 0x08: Weekdays(0-6)
 *  - 0x09: Months  (BCD 01-12)
 *  - 0x0A: Years   (BCD 00-99, offset from 2000)
 * 
 * @attribution
 * - NXP Semiconductors
 * - BSP Implementation: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "bsp/bsp_i2c.h"
#include "bsp/bsp_rtc.h"

static const char *TAG = "bsp_rtc";

#define PCF85063_REG_CTRL1    0x00
#define PCF85063_REG_SECONDS  0x04

/* BCD Conversion Helpers */
static inline uint8_t bcd2bin(uint8_t val) { return (val & 0x0F) + ((val >> 4) * 10); }
static inline uint8_t bin2bcd(uint8_t val) { return (val % 10) | ((val / 10) << 4); }

esp_err_t bsp_rtc_init(void)
{
    ESP_LOGI(TAG, "Initializing PCF85063A RTC on I2C 0x%02X", BSP_I2C_ADDR_PCF85063);

    // Read Control 1 register
    uint8_t reg_ctrl1 = 0;
    esp_err_t err = bsp_i2c_read_reg(BSP_I2C_ADDR_PCF85063, PCF85063_REG_CTRL1, &reg_ctrl1, 1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read RTC Ctrl1 register: %s", esp_err_to_name(err));
        return err;
    }

    // Clear STOP bit (Bit 5) to ensure 32.768kHz clock is running in 24-hour mode
    reg_ctrl1 &= ~(1 << 5);
    reg_ctrl1 &= ~(1 << 2); // 24-hour format
    err = bsp_i2c_write_reg(BSP_I2C_ADDR_PCF85063, PCF85063_REG_CTRL1, &reg_ctrl1, 1);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "PCF85063A RTC initialized successfully");
    }
    return err;
}

esp_err_t bsp_rtc_get_datetime(bsp_rtc_datetime_t *out_dt)
{
    if (out_dt == NULL) return ESP_ERR_INVALID_ARG;

    uint8_t buf[7] = {0};
    esp_err_t err = bsp_i2c_read_reg(BSP_I2C_ADDR_PCF85063, PCF85063_REG_SECONDS, buf, sizeof(buf));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read RTC datetime registers: %s", esp_err_to_name(err));
        return err;
    }

    out_dt->second  = bcd2bin(buf[0] & 0x7F);
    out_dt->minute  = bcd2bin(buf[1] & 0x7F);
    out_dt->hour    = bcd2bin(buf[2] & 0x3F);
    out_dt->day     = bcd2bin(buf[3] & 0x3F);
    out_dt->weekday = buf[4] & 0x07;
    out_dt->month   = bcd2bin(buf[5] & 0x1F);
    out_dt->year    = 2000 + bcd2bin(buf[6]);

    return ESP_OK;
}

esp_err_t bsp_rtc_set_datetime(const bsp_rtc_datetime_t *in_dt)
{
    if (in_dt == NULL) return ESP_ERR_INVALID_ARG;

    uint8_t buf[7];
    buf[0] = bin2bcd(in_dt->second);
    buf[1] = bin2bcd(in_dt->minute);
    buf[2] = bin2bcd(in_dt->hour);
    buf[3] = bin2bcd(in_dt->day);
    buf[4] = in_dt->weekday & 0x07;
    buf[5] = bin2bcd(in_dt->month);
    buf[6] = bin2bcd((uint8_t)(in_dt->year >= 2000 ? in_dt->year - 2000 : in_dt->year));

    esp_err_t err = bsp_i2c_write_reg(BSP_I2C_ADDR_PCF85063, PCF85063_REG_SECONDS, buf, sizeof(buf));
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "RTC Set Time -> %04d-%02d-%02d %02d:%02d:%02d",
                 in_dt->year, in_dt->month, in_dt->day, in_dt->hour, in_dt->minute, in_dt->second);
    }
    return err;
}
