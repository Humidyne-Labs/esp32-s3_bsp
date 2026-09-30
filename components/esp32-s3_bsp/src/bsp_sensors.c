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
#include <math.h>
#include "esp_log.h"

// May Change per ESP-IDF Revision
#include "esp_rom_sys.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp/bsp_i2c.h"
#include "bsp/bsp_sensors.h"

static const char *TAG = "bsp_sensors";

#define C_TO_F(c) (((c) * 1.8f) + 32.0f)          ///< Conversion MACRO Celsius to Fahrenheit
#define C_TO_K(c) ((c) + 273.15f)                 ///< Conversion MACRO Celsius to Kelvin

// SHTC3 16-bit Command Words
#define SHTC3_CMD_WAKEUP                  0x3517  ///< Wakeup command
#define SHTC3_CMD_SLEEP                   0xB098  ///< Sleep command
#define SHTC3_CMD_SWRST                   0x805D  ///< Software Reset Command
#define SHTC3_CMD_READ_ID                 0xEFC8  ///< Read ID register
#define SHTC3_CMD_MEAS_NORMAL_T_FIRST     0x7866  ///< Measure Normal Power:     Temp First, Clock Stretching Disabled
#define SHTC3_CMD_MEAS_NORMAL_R_FIRST     0x58E0  ///< Measure Normal Power: Humidity First, Clock Stretching Disabled

// SHTC3 More 16-bit Command Words (Low-Power-Mode)
#define SHTC3_CMD_MEAS_LOWPWR_T_FIRST     0x609C ///< Measure    Low Power:     Temp First, Clock Stretching Disabled
#define SHTC3_CMD_MEAS_LOWPWR_H_FIRST     0x401A ///< Measure    Low Power: Humidity First, Clock Stretching Disabled

// SHTC3 Even More 16-bit Command Words (Clock-Strech-EN)
#define SHTC3_CMD_MEAS_NORMAL_CS_T_FIRST  0x7CA2 ///< Measure Normal Power:     Temp First, Clock Stretching Enabled
#define SHTC3_CMD_MEAS_NORMAL_CS_R_FIRST  0x5C24 ///< Measure Normal Power: Humidity First, Clock Stretching Enabled
#define SHTC3_CMD_MEAS_LOWPWR_CS_T_FIRST  0x6458 ///< Measure    Low Power:     Temp First, Clock Stretching Enabled
#define SHTC3_CMD_MEAS_LOWPWR_CS_R_FIRST  0x44DE ///< Measure    Low Power: Humidity First, Clock Stretching Enabled


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
 * Sensirion Derived Environmental Metrics (Magnus Formula)
 * ========================================================================= */
static float calc_dew_point(float temp_c, float rh)
{
    // Sensirion Magnus constants for -45°C to 60°C range
    const float m  = 17.62f;
    const float tn = 243.12f;

    if (rh < 0.01f)  rh = 0.01f;
    if (rh > 100.0f) rh = 100.0f;

    float gamma = logf(rh / 100.0f) + ((m * temp_c) / (tn + temp_c));
    return (tn * gamma) / (m - gamma);
}

static float calc_absolute_humidity(float temp_c, float rh)
{
    // Absolute Humidity (g/m³) = 216.7 * [ (RH/100) * A * exp(m*T / (tn+T)) / (273.15 + T) ]
    const float m  = 17.62f;
    const float tn = 243.12f;
    const float a  = 6.112f; // hPa

    if (rh < 0.0f)   rh = 0.0f;
    if (rh > 100.0f) rh = 100.0f;

    float vapor_pressure = (rh / 100.0f) * a * expf((m * temp_c) / (tn + temp_c));
    return 216.7f * (vapor_pressure / (273.15f + temp_c));
}

/* =========================================================================
 * Internal Helper: Common Measurement Execution Pipeline
 * ========================================================================= */
static esp_err_t shtc3_execute_measurement(uint16_t cmd, bool low_power, bsp_shtc3_data_t *out_data)
{
    if (out_data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    memset(out_data, 0, sizeof(bsp_shtc3_data_t));

    // 1. Wake up sensor from standby (needs <= 240 µs)
    esp_err_t err = bsp_shtc3_wakeup();
    if (err != ESP_OK) {
        return err;
    }

    // 2. Issue Measurement Command
    uint8_t meas_cmd[2] = { (uint8_t)(cmd >> 8), (uint8_t)(cmd & 0xFF) };
    err = bsp_i2c_write(BSP_I2C_ADDR_SHTC3, meas_cmd, sizeof(meas_cmd));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send measurement command 0x%04X: %s", cmd, esp_err_to_name(err));
        bsp_shtc3_sleep();
        return err;
    }

    // 3. Wait for conversion
    if (low_power) {
        // Low Power conversion is max 0.8 ms (800 µs).
        // A busy-wait delay avoids FreeRTOS tick quantizations (e.g., 10-20 ms on 100 Hz ticks).
        esp_rom_delay_us(1000); // Internal and Unstable APIs
    } else {
        // Normal conversion is max 12.1 ms. Yield RTOS scheduler.
        vTaskDelay(pdMS_TO_TICKS(15) + 1);
    }

    // 4. Read 6-byte result: [T_MSB, T_LSB, T_CRC, RH_MSB, RH_LSB, RH_CRC]
    uint8_t rx_buf[6] = {0};
    err = bsp_i2c_read(BSP_I2C_ADDR_SHTC3, rx_buf, sizeof(rx_buf));

    // Always put sensor to sleep immediately to minimize self-heating
    bsp_shtc3_sleep();

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read data bytes: %s", esp_err_to_name(err));
        return err;
    }

    // 5. Validate CRC8 on Temperature and Relative Humidity
    if (shtc3_crc8(&rx_buf[0], 2) != rx_buf[2]) {
        ESP_LOGE(TAG, "Temperature CRC mismatch");
        return ESP_ERR_INVALID_CRC;
    }
    if (shtc3_crc8(&rx_buf[3], 2) != rx_buf[5]) {
        ESP_LOGE(TAG, "Humidity CRC mismatch");
        return ESP_ERR_INVALID_CRC;
    }

    // 6. Convert Raw Words to Physical Units
    uint16_t raw_temp = ((uint16_t)rx_buf[0] << 8) | rx_buf[1];
    uint16_t raw_rh   = ((uint16_t)rx_buf[3] << 8) | rx_buf[4];

    out_data->temperature_c       = -45.0f + (175.0f * (float)raw_temp / 65536.0f);
	out_data->temperature_f       = C_TO_F(out_data->temperature_c);
    out_data->temperature_k       = C_TO_K(out_data->temperature_c);
    out_data->humidity_percent    = 100.0f * ((float)raw_rh / 65536.0f);
    out_data->dew_point_c         = calc_dew_point(out_data->temperature_c, out_data->humidity_percent);
	out_data->dew_point_f         = C_TO_F(out_data->dew_point_c);
	out_data->dew_point_k         = C_TO_K(out_data->dew_point_c);
    out_data->absolute_humidity_g = calc_absolute_humidity(out_data->temperature_c, out_data->humidity_percent);
    out_data->valid               = true;

    return ESP_OK;
}

/* =========================================================================
 * Public SHTC3 Driver APIs
 * ========================================================================= */
esp_err_t bsp_shtc3_wakeup(void)
{
    uint8_t cmd[2] = { (uint8_t)(SHTC3_CMD_WAKEUP >> 8), (uint8_t)(SHTC3_CMD_WAKEUP & 0xFF) };
    esp_err_t err = bsp_i2c_write(BSP_I2C_ADDR_SHTC3, cmd, sizeof(cmd));
    // SHTC3 requires up to 240 µs wake-up duration before accepting subsequent commands
    esp_rom_delay_us(250); // Internal and Unstable APIs
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
    esp_err_t err = bsp_shtc3_wakeup();
    if (err != ESP_OK) return err;

    // 2. Query Device ID register (Expected ID mask: 0x0807 or 0x0847)
    uint8_t id_cmd[2] = { (uint8_t)(SHTC3_CMD_READ_ID >> 8), (uint8_t)(SHTC3_CMD_READ_ID & 0xFF) };
    err = bsp_i2c_write(BSP_I2C_ADDR_SHTC3, id_cmd, sizeof(id_cmd));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write Read ID command to SHTC3: %s", esp_err_to_name(err));
		bsp_shtc3_sleep();
        return err;
    }

    uint8_t id_buf[3] = {0};
    err = bsp_i2c_read(BSP_I2C_ADDR_SHTC3, id_buf, sizeof(id_buf));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read ID from SHTC3: %s", esp_err_to_name(err));
		bsp_shtc3_sleep();
        return err;
    }

    // Verify CRC of ID register
    if (shtc3_crc8(id_buf, 2) != id_buf[2]) {
        ESP_LOGE(TAG, "SHTC3 ID CRC Check failed!");
		bsp_shtc3_sleep();
        return ESP_ERR_INVALID_CRC;
    }

	// Bits 11 and 5:0 form the product code mask (0x083F must equal 0x0807)
	uint16_t id_val = ((uint16_t)id_buf[0] << 8) | id_buf[1];
    if ((id_val & 0x083F) != 0x0807) ESP_LOGW(TAG, "Unrecognized Sensor ID: 0x%04X", id_val);

    // 3. Put sensor into standby sleep mode
    bsp_shtc3_sleep();
    return ESP_OK;
}

esp_err_t bsp_shtc3_read(bsp_shtc3_data_t *out_data) {
    // Normal Mode: Temp First, Clock Stretching Disabled (0x7866)
    return shtc3_execute_measurement(SHTC3_CMD_MEAS_NORMAL_T_FIRST, false, out_data);
}

esp_err_t bsp_shtc3_read_lp(bsp_shtc3_data_t *out_data) {
    // Low Power Mode: Temp First, Clock Stretching Disabled (0x609C)
    return shtc3_execute_measurement(SHTC3_CMD_MEAS_LOWPWR_T_FIRST, true, out_data);
}
