/**
 * @file bsp_sensors.c
 * @brief Sensirion SHTC3 I2C Environmental Sensor Driver Implementation
 * 
 * Communication Protocol:
 *  - I2C Address: 0x70
 *  - Commands: 16-bit command words (Big Endian)
 *  - Data format: 2 bytes data + 1 byte CRC for Temperature, followed by
 *                 2 bytes data + 1 byte CRC for Relative Humidity.
 *  - CRC Polynomial: P(x) = x^8 + x^5 + x^4 + 1 = 0x31 (Init = 0xFF)
 * 
 * @attribution
 * - Sensirion AG (Datasheet SHTC3 Revision 1 - May 2019)
 * - BSP Unification: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp/bsp_i2c.h"
#include "bsp/bsp_sensors.h"

static const char *TAG = "bsp_sensors";

// SHTC3 16-bit Command Words
#define SHTC3_CMD_WAKEUP              0x3517  /*!< Wakeup command */
#define SHTC3_CMD_SLEEP               0xB098  /*!< Sleep command */
#define SHTC3_CMD_READ_ID             0xEFC8  /*!< Read ID register */
#define SHTC3_CMD_MEAS_NORMAL_T_FIRST 0x7CA2  /*!< Measure Normal Power: Temp First, Clock Stretching Disabled */

/* =========================================================================
 * Sensirion CRC-8 Polynomial Calculation
 * ========================================================================= */
static uint8_t shtc3_crc8(const uint8_t *data, size_t len)
{
    uint8_t crc = 0xFF; // Initialization value
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 8; bit > 0; --bit) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31; // Polynomial 0x31
            } else {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}

/* =========================================================================
 * Public SHTC3 Driver APIs
 * ========================================================================= */
esp_err_t bsp_shtc3_wakeup(void)
{
    uint8_t cmd[2] = { (uint8_t)(SHTC3_CMD_WAKEUP >> 8), (uint8_t)(SHTC3_CMD_WAKEUP & 0xFF) };
    esp_err_t err = bsp_i2c_write(BSP_I2C_ADDR_SHTC3, cmd, sizeof(cmd));
    // SHTC3 requires up to 240 µs wake-up duration before accepting subsequent commands
    vTaskDelay(pdMS_TO_TICKS(1));
    return err;
}

esp_err_t bsp_shtc3_sleep(void)
{
    uint8_t cmd[2] = { (uint8_t)(SHTC3_CMD_SLEEP >> 8), (uint8_t)(SHTC3_CMD_SLEEP & 0xFF) };
    return bsp_i2c_write(BSP_I2C_ADDR_SHTC3, cmd, sizeof(cmd));
}

esp_err_t bsp_shtc3_init(void)
{
    ESP_LOGI(TAG, "Initializing SHTC3 Environmental Sensor on I2C 0x%02X", BSP_I2C_ADDR_SHTC3);

    // 1. Wake up sensor
    bsp_shtc3_wakeup();

    // 2. Query Device ID register (Expected ID mask: 0x0807 or 0x0847)
    uint8_t id_cmd[2] = { (uint8_t)(SHTC3_CMD_READ_ID >> 8), (uint8_t)(SHTC3_CMD_READ_ID & 0xFF) };
    esp_err_t err = bsp_i2c_write(BSP_I2C_ADDR_SHTC3, id_cmd, sizeof(id_cmd));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write Read ID command to SHTC3: %s", esp_err_to_name(err));
        return err;
    }

    uint8_t id_buf[3] = {0};
    err = bsp_i2c_read(BSP_I2C_ADDR_SHTC3, id_buf, sizeof(id_buf));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read ID from SHTC3: %s", esp_err_to_name(err));
        return err;
    }

    // Verify CRC of ID register
    if (shtc3_crc8(id_buf, 2) != id_buf[2]) {
        ESP_LOGE(TAG, "SHTC3 ID CRC Check failed!");
        return ESP_ERR_INVALID_CRC;
    }

    uint16_t id_val = ((uint16_t)id_buf[0] << 8) | id_buf[1];
    ESP_LOGI(TAG, "SHTC3 Sensor detected successfully (ID: 0x%04X)", id_val);

    // 3. Put sensor into standby sleep mode
    bsp_shtc3_sleep();
    return ESP_OK;
}

esp_err_t bsp_shtc3_read(bsp_shtc3_data_t *out_data)
{
    if (out_data == NULL) return ESP_ERR_INVALID_ARG;
    memset(out_data, 0, sizeof(bsp_shtc3_data_t));

    // 1. Wake up sensor from standby
    esp_err_t err = bsp_shtc3_wakeup();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Wakeup failed before measurement");
        return err;
    }

    // 2. Trigger Normal Measurement (Temp First, Clock Stretching Disabled)
    uint8_t meas_cmd[2] = { (uint8_t)(SHTC3_CMD_MEAS_NORMAL_T_FIRST >> 8), (uint8_t)(SHTC3_CMD_MEAS_NORMAL_T_FIRST & 0xFF) };
    err = bsp_i2c_write(BSP_I2C_ADDR_SHTC3, meas_cmd, sizeof(meas_cmd));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to issue measurement command");
        bsp_shtc3_sleep();
        return err;
    }

    // 3. Wait for measurement conversion (Max duration: 12.1 ms for normal mode)
    vTaskDelay(pdMS_TO_TICKS(15));

    // 4. Read 6-byte result: [Temp_MSB, Temp_LSB, Temp_CRC, RH_MSB, RH_LSB, RH_CRC]
    uint8_t rx_buf[6] = {0};
    err = bsp_i2c_read(BSP_I2C_ADDR_SHTC3, rx_buf, sizeof(rx_buf));

    // Put sensor back to sleep immediately after bus read to conserve energy
    bsp_shtc3_sleep();

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read measurement bytes from SHTC3: %s", esp_err_to_name(err));
        return err;
    }

    // 5. Validate CRC on Temperature and Relative Humidity
    if (shtc3_crc8(&rx_buf[0], 2) != rx_buf[2]) {
        ESP_LOGE(TAG, "Temperature CRC mismatch (calc: 0x%02X, got: 0x%02X)",
                 shtc3_crc8(&rx_buf[0], 2), rx_buf[2]);
        return ESP_ERR_INVALID_CRC;
    }
    if (shtc3_crc8(&rx_buf[3], 2) != rx_buf[5]) {
        ESP_LOGE(TAG, "Humidity CRC mismatch (calc: 0x%02X, got: 0x%02X)",
                 shtc3_crc8(&rx_buf[3], 2), rx_buf[5]);
        return ESP_ERR_INVALID_CRC;
    }

    // 6. Convert Raw Words to Physical Units
    uint16_t raw_temp = ((uint16_t)rx_buf[0] << 8) | rx_buf[1];
    uint16_t raw_rh   = ((uint16_t)rx_buf[3] << 8) | rx_buf[4];

    // Equations from Datasheet Section 5.1:
    // T_Celsius = -45 + 175 * (raw_temp / 65535.0)
    // T_Kelvin  = T_Celsius + 273.15 = 228.15 + (175.0 * raw_temp / 65535.0)
    out_data->temperature_k    = 228.15f + (175.0f * (float)raw_temp / 65535.0f);
    out_data->humidity_percent = 100.0f * ((float)raw_rh / 65535.0f);
    out_data->valid            = true;

    return ESP_OK;
}
