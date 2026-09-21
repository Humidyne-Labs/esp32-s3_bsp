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

#include <stdbool.h>
#include "esp_log.h"
#include "bsp/bsp_i2c.h"
#include "bsp/bsp_rtc.h"

#define BSP_RTC_I2C_ADDRESS 0x51
#define BSP_RTC_REG_CONTROL_1 0x00
#define BSP_RTC_REG_CONTROL_2 0x01
#define BSP_RTC_REG_TIME 0x04

static const char *TAG = "bsp_rtc";
static i2c_master_dev_handle_t s_rtc_handle = NULL;

static uint8_t bcd_to_binary(uint8_t value)
{
    return (uint8_t)((value & 0x0F) + ((value >> 4) * 10));
}

static uint8_t binary_to_bcd(uint8_t value)
{
    return (uint8_t)(((value / 10) << 4) | (value % 10));
}

static bool datetime_is_valid(const bsp_rtc_datetime_t *datetime)
{
    return datetime != NULL &&
           datetime->year >= 2000 && datetime->year <= 2099 &&
           datetime->month >= 1 && datetime->month <= 12 &&
           datetime->day >= 1 && datetime->day <= 31 &&
           datetime->weekday <= 6 &&
           datetime->hour <= 23 && datetime->minute <= 59 &&
           datetime->second <= 59;
}

esp_err_t bsp_rtc_init(void)
{
    if (s_rtc_handle != NULL) {
        return ESP_OK;
    }

    const i2c_device_config_t device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BSP_RTC_I2C_ADDRESS,
        .scl_speed_hz = 400000,
    };
    esp_err_t ret = bsp_i2c_add_device(&device_config, &s_rtc_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add PCF85063A: %s", esp_err_to_name(ret));
        return ret;
    }

    const uint8_t control_value = 0x00;
    ret = bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_CONTROL_1, &control_value, 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize PCF85063A: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_CONTROL_2, &control_value, 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to clear PCF85063A control flags: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "PCF85063A RTC initialized");
    return ESP_OK;
}

esp_err_t bsp_rtc_get_datetime(bsp_rtc_datetime_t *datetime)
{
    if (datetime == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (s_rtc_handle == NULL) {
        esp_err_t ret = bsp_rtc_init();
        if (ret != ESP_OK) {
            return ret;
        }
    }

    uint8_t registers[7] = {0};
    esp_err_t ret = bsp_i2c_read_reg(s_rtc_handle, BSP_RTC_REG_TIME, registers, sizeof(registers));
    if (ret != ESP_OK) {
        return ret;
    }

    datetime->second = bcd_to_binary(registers[0] & 0x7F);
    datetime->minute = bcd_to_binary(registers[1] & 0x7F);
    datetime->hour = bcd_to_binary(registers[2] & 0x3F);
    datetime->day = bcd_to_binary(registers[3] & 0x3F);
    datetime->weekday = bcd_to_binary(registers[4] & 0x07);
    datetime->month = bcd_to_binary(registers[5] & 0x1F);
    datetime->year = (uint16_t)(2000 + bcd_to_binary(registers[6]));
    return ESP_OK;
}

esp_err_t bsp_rtc_set_datetime(const bsp_rtc_datetime_t *datetime)
{
    if (!datetime_is_valid(datetime)) {
        return ESP_ERR_INVALID_ARG;
    }
    if (s_rtc_handle == NULL) {
        esp_err_t ret = bsp_rtc_init();
        if (ret != ESP_OK) {
            return ret;
        }
    }

    const uint8_t registers[7] = {
        binary_to_bcd(datetime->second),
        binary_to_bcd(datetime->minute),
        binary_to_bcd(datetime->hour),
        binary_to_bcd(datetime->day),
        binary_to_bcd(datetime->weekday),
        binary_to_bcd(datetime->month),
        binary_to_bcd((uint8_t)(datetime->year - 2000)),
    };
    return bsp_i2c_write_reg(s_rtc_handle, BSP_RTC_REG_TIME, registers, sizeof(registers));
}
