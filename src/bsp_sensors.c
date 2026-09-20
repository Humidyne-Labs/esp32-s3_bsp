/**
 * @file bsp_sensors.c
 * @brief ESP32-S3 ePaper BSP - SHTC3 Sensor Driver Implementation (Native Kelvin)
 * 
 * @note Based on Sensirion SHTC3 driver & Waveshare ePaper sample library.
 * @copyright Copyright (c) 2026 Humidyne Labs / humid1-os-stage
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "bsp/bsp_i2c.h"
#include "bsp/bsp_sensors.h"

static const char *TAG = "bsp_sensors";

static i2c_master_dev_handle_t s_shtc3_dev_handle = NULL;
static bsp_shtc3_power_mode_t s_shtc3_power_mode =
#if CONFIG_BSP_SHTC3_LOW_POWER
    BSP_SHTC3_POWER_MODE_LOW;
#else
    BSP_SHTC3_POWER_MODE_NORMAL;
#endif

static uint16_t shtc3_measurement_command(void)
{
    return s_shtc3_power_mode == BSP_SHTC3_POWER_MODE_LOW ? 0x609C : 0x7866;
}

static uint8_t shtc3_calc_crc(const uint8_t *data, size_t len)
{
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 8; bit > 0; --bit) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}

esp_err_t bsp_shtc3_init(void)
{
    if (s_shtc3_dev_handle != NULL) return ESP_OK;

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BSP_SHTC3_I2C_ADDR,
        .scl_speed_hz = 100000,
    };

    esp_err_t ret = bsp_i2c_add_device(&dev_cfg, &s_shtc3_dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add SHTC3 to I2C bus: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Wakeup command: 0x3517 */
    uint8_t cmd_wakeup[2] = {0x35, 0x17};
    bsp_i2c_write_reg(s_shtc3_dev_handle, -1, cmd_wakeup, 2);
    vTaskDelay(pdMS_TO_TICKS(10));

    ESP_LOGI(TAG, "SHTC3 sensor driver initialized (Native Kelvin mode)");
    return ESP_OK;
}

esp_err_t bsp_shtc3_set_power_mode(bsp_shtc3_power_mode_t mode)
{
    if (mode != BSP_SHTC3_POWER_MODE_NORMAL && mode != BSP_SHTC3_POWER_MODE_LOW) {
        return ESP_ERR_INVALID_ARG;
    }

    s_shtc3_power_mode = mode;
    return ESP_OK;
}

esp_err_t bsp_shtc3_read(bsp_shtc3_data_t *data)
{
    if (data == NULL) return ESP_ERR_INVALID_ARG;
    if (s_shtc3_dev_handle == NULL) {
        esp_err_t ret = bsp_shtc3_init();
        if (ret != ESP_OK) return ret;
    }

    /* Wakeup command: 0x3517 */
    uint8_t cmd_wakeup[2] = {0x35, 0x17};
    bsp_i2c_write_reg(s_shtc3_dev_handle, -1, cmd_wakeup, 2);
    vTaskDelay(pdMS_TO_TICKS(10));

    uint16_t measurement_command = shtc3_measurement_command();
    uint8_t cmd_meas[2] = {
        (uint8_t)(measurement_command >> 8),
        (uint8_t)(measurement_command & 0xFF),
    };
    esp_err_t ret = bsp_i2c_write_reg(s_shtc3_dev_handle, -1, cmd_meas, 2);
    if (ret != ESP_OK) return ret;

    vTaskDelay(pdMS_TO_TICKS(20));

    uint8_t rx[6] = {0};
    ret = bsp_i2c_read_reg(s_shtc3_dev_handle, -1, rx, 6);
    if (ret != ESP_OK) return ret;

    /* Sleep command: 0xB098 */
    uint8_t cmd_sleep[2] = {0xB0, 0x98};
    bsp_i2c_write_reg(s_shtc3_dev_handle, -1, cmd_sleep, 2);

    /* Verify CRC */
    if (shtc3_calc_crc(&rx[0], 2) != rx[2] || shtc3_calc_crc(&rx[3], 2) != rx[5]) {
        ESP_LOGE(TAG, "SHTC3 CRC verification failed");
        return ESP_ERR_INVALID_CRC;
    }

    uint16_t raw_temp = ((uint16_t)rx[0] << 8) | rx[1];
    uint16_t raw_humi = ((uint16_t)rx[3] << 8) | rx[4];

    /* Convert directly to Kelvin: T_K = (175.0 * raw / 65536) - 45.0 + 273.15 = (175.0 * raw / 65536) + 228.15 */
    data->temperature_k = (175.0f * (float)raw_temp / 65536.0f) + 228.15f;
    data->humidity_percent = (100.0f * (float)raw_humi / 65536.0f);

    return ESP_OK;
}
