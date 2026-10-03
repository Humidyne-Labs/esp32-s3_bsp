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
 * @version 1.0.0
 * @attribution
 * - Architecture & Development: HUMIDYNE LABS / Humiditron
 * - AI Systems Co-Developer: Gemini (Google DeepMind)
 * - Peripheral Driver Basis: Waveshare Electronics SHTC3 Sensor Code & Datasheet
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BSP_SENSORS_H
#define BSP_SENSORS_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
typedef struct {
    float temperature_k;      // !< Temperature in native Kelvin (K)    (e.g. 297.35 K = 24.2 °C = 75.6 °F) ///< K value
    float humidity_percent;   // !< Relative Humidity in percent (% RH) (0.0 to 100.0) ///< humidity_percent value
    bool  valid;              // !< True if sensor CRC-8 checksum verification succeeded ///< valid value
} bsp_shtc3_data_t;
*/

/**
 * @brief SHTC3 Environmental Sensor Telemetry Data
 */
typedef struct {
    float temperature_c;        ///< Temperature in Celsius    (°C)
	float temperature_f;        ///< Temperature in Fahrenheit (°F)
    float temperature_k;        ///< Temperature in Kelvin     (K)
    float humidity_percent;     ///< Relative Humidity         (%RH)
    float dew_point_c;          ///< Dew point in Celsius      (°C)
	float dew_point_f;          ///< Dew point in Fahrenheit   (°F)
	float dew_point_k;          ///< Dew point in Kelvin       (K)
    float absolute_humidity_g;  ///< Absolute Humidity in      (g/m³)
    bool  valid; ///< valid value
} bsp_shtc3_data_t;

/**
 * @brief Initialize Sensirion SHTC3 Environmental Sensor
 *
 * Wakes the sensor from sleep, verifies the hardware Product ID,
 * and puts it into ultra-low power standby.
 *
 * @return esp_err_t ESP_OK on success, or ESP_ERR_NOT_FOUND if sensor is absent
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_shtc3_init(void);

/**
 * @brief Generic sensor subsystem initialization alias
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 * @return esp_err_t ESP_OK on success, or appropriate ESP error code.
 */
static inline esp_err_t bsp_sensors_init(void) { return bsp_shtc3_init(); }

/**
 * @brief Read Temperature and Relative Humidity
 *
 * Executes a normal-power measurement sequence with clock stretching disabled,
 * validates the 8-bit CRC polynomial (0x31) on both data words, and converts
 * raw ADC words into physical units.
 *
 * Conversion Equations:
 *  - Temperature: T_Kelvin = 228.15 + (175.0 * raw_temp / 65536.0)
 *  - Humidity:    RH_%     = 100.0 * (raw_rh / 65536.0)
 *
 * @param[out] out_data Destination struct to receive telemetry
 * @return esp_err_t ESP_OK on successful read and valid CRC
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_shtc3_read(bsp_shtc3_data_t *out_data);

/**
 * @brief Read Temperature and Relative Humidity
 *
 * Executes a low-power measurement sequence with clock stretching disabled,
 * validates the 8-bit CRC polynomial (0x31) on both data words, and converts
 * raw ADC words into physical units.
 *
 * @param[out] out_data Destination struct to receive telemetry
 * @return esp_err_t ESP_OK on successful read and valid CRC
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_shtc3_read_lp(bsp_shtc3_data_t *out_data);

/**
 * @brief Put SHTC3 Sensor into Ultra-Low Power Sleep Mode (< 0.6 µA)
 *
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_shtc3_sleep(void);

/**
 * @brief Wake SHTC3 Sensor from Sleep Mode
 *
 * Must be followed by a minimum 240 µs wake-up delay before issuing commands.
 *
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_shtc3_wakeup(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_SENSORS_H */
