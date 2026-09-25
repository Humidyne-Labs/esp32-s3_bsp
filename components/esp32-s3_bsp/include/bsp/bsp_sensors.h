/**
 * @file bsp_sensors.h
 * @brief Sensirion SHTC3 High-Precision I2C Environmental Sensor Driver
 * 
 * Hardware Target:
 *  - Sensor: Sensirion SHTC3 (I2C address: 0x70)
 *  - Operating Voltage: 1.62V - 3.6V
 *  - Temperature Range: -40 °C to +125 °C (233.15 K to 398.15 K)
 *  - Humidity Range: 0 % to 100 % RH
 * 
 * Data Formats:
 *  - Temperature is returned in **native Kelvin (K)** for unit-agnostic IoT processing.
 *  - Relative Humidity is returned in **percentage (% RH)**.
 * 
 * @attribution
 * - Sensor: Sensirion AG (https://www.sensirion.com)
 * - BSP Implementation: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_SENSORS_H
#define BSP_SENSORS_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief SHTC3 Environmental Sensor Telemetry Data
 */
typedef struct {
    float temperature_k;      /*!< Temperature in native Kelvin (K) (e.g. 297.35 K = 24.2 °C = 75.6 °F) */
    float humidity_percent;   /*!< Relative Humidity in percent (% RH) (0.0 to 100.0) */
    bool  valid;              /*!< True if sensor CRC-8 checksum verification succeeded */
} bsp_shtc3_data_t;

/**
 * @brief Initialize Sensirion SHTC3 Environmental Sensor
 * 
 * Wakes the sensor from sleep, verifies the hardware Product ID (0x0847 or 0x0807),
 * and puts it into ultra-low power standby.
 * 
 * @return esp_err_t ESP_OK on success, or ESP_ERR_NOT_FOUND if sensor is absent
 */
esp_err_t bsp_shtc3_init(void);

/**
 * @brief Read Temperature and Relative Humidity
 * 
 * Executes a normal-power measurement sequence with clock stretching disabled,
 * validates the 8-bit CRC polynomial (0x31) on both data words, and converts
 * raw ADC words into physical units.
 * 
 * Conversion Equations:
 *  - Temperature: T_Kelvin = 228.15 + (175.0 * raw_temp / 65535.0)
 *  - Humidity:    RH_%     = 100.0 * (raw_rh / 65535.0)
 * 
 * @param[out] out_data Destination struct to receive telemetry
 * @return esp_err_t ESP_OK on successful read and valid CRC
 */
esp_err_t bsp_shtc3_read(bsp_shtc3_data_t *out_data);

/**
 * @brief Put SHTC3 Sensor into Ultra-Low Power Sleep Mode (< 0.6 µA)
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_shtc3_sleep(void);

/**
 * @brief Wake SHTC3 Sensor from Sleep Mode
 * 
 * Must be followed by a minimum 240 µs wake-up delay before issuing commands.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_shtc3_wakeup(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_SENSORS_H */
