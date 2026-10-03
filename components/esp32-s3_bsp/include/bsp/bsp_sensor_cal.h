/**
 * @file bsp_sensor_cal.h
 * @brief Decoupled MCU-Guided Thermal & Humidity Calibration Subsystem for Sensirion SHTC3
 *
 * Architecture:
 *  - Wraps the low-level Sensirion SHTC3 driver (`bsp_sensors.h`) without modifying it.
 *  - Uses the ESP32-S3 internal die temperature sensor (`driver/temperature_sensor.h`).
 *  - Applies an Exponential Moving Average (EMA) low-pass filter to MCU junction temperature spikes.
 *  - Solves a two-node thermal divider model for PCB heat conduction.
 *  - Applies Magnus-Tetens vapor pressure equalization for accurate relative humidity.
 *
 * Board Profile:
 *  - Waveshare ESP32-S3 ePaper 1.54 V2 (53 mm x 40 mm, 4-6 layer PCB)
 *  - Default Thermal Coupling Ratio K = 0.380
 *
 * @version 1.0.0
 * @attribution
 * - Architecture & Development: HUMIDYNE LABS / Humiditron
 * - AI Systems Co-Developer: Gemini (Google DeepMind)
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_SENSOR_CAL_H
#define BSP_SENSOR_CAL_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "bsp/bsp_sensors.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calibrated Environmental & Diagnostic Sensor Telemetry
 */
typedef struct {
    // 1. Telemetry Metrics (Compensated when enabled, Raw SHTC3 when deactivated)
    float temperature_c;        ///< Temperature in Celsius (°C)
    float temperature_f;        ///< Temperature in Fahrenheit (°F)
    float temperature_k;        ///< Temperature in Kelvin (K)
    float humidity_percent;     ///< Relative Humidity (%RH)
    float dew_point_c;          ///< Dew Point in Celsius (°C)
    float dew_point_f;          ///< Dew Point in Fahrenheit (°F)
    float dew_point_k;          ///< Dew Point in Kelvin (K)
    float absolute_humidity_g;  ///< Absolute Humidity (g/m³)

    // 2. Raw Sensor Telemetry & Calibration Diagnostic Metadata
    float raw_temperature_c;    ///< Pure Uncompensated Raw SHTC3 Temperature (°C)
    float raw_humidity_percent; ///< Pure Uncompensated Raw SHTC3 Relative Humidity (%RH)
    float die_temp_c;           ///< Filtered ESP32-S3 MCU Junction Temperature (°C)
    float thermal_offset_c;     ///< Applied Thermal Offset (°C) (0.0°C when deactivated)
    bool  compensated;          ///< True if thermal compensation was active, false if bypassed
    bool  valid;                ///< True if SHTC3 CRC verified
} bsp_sensor_cal_data_t;

/**
 * @brief Initialize Thermal Calibration Subsystem & MCU Internal Temp Sensor
 *
 * Instantiates the ESP32-S3 internal TSENS peripheral, sets default K and alpha filter
 * weights from Kconfig, and initializes raw SHTC3 hardware via bsp_shtc3_init().
 *
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_sensor_cal_init(void);

/**
 * @brief Read Calibrated & Compensated Environmental Telemetry
 *
 * Reads raw SHTC3 telemetry, queries the MCU die temperature, applies EMA filtering,
 * solves the two-node thermal divider model, and equalizes relative humidity.
 * If dynamic compensation is disabled, outputs match 100% raw uncalibrated SHTC3 data.
 *
 * @param[out] out_data Destination struct to receive calibrated metrics
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_sensor_cal_read(bsp_sensor_cal_data_t *out_data);

/**
 * @brief Read Raw ESP32-S3 MCU Junction Temperature (°C)
 *
 * @param[out] out_die_temp Pointer to receive die temperature in Celsius
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_mcu_temp_read(float *out_die_temp);

/**
 * @brief Set Board Thermal Coupling Constant (K)
 *
 * @param[in] k Thermal coupling ratio R_amb / R_pcb (default 0.380f)
 * @details Memory ownership: none. Behavior: Non-blocking. Thread safety: thread-safe.
 */
void bsp_sensor_cal_set_k(float k);

/**
 * @brief Get Active Board Thermal Coupling Constant (K)
 *
 * @return float Active K value
 * @details Memory ownership: none. Behavior: Non-blocking. Thread safety: thread-safe.
 */
float bsp_sensor_cal_get_k(void);

/**
 * @brief Set MCU Die Temp EMA Low-Pass Filter Alpha Weight
 *
 * @param[in] alpha Filter weight from 0.001 to 1.0 (default 0.050f)
 * @details Memory ownership: none. Behavior: Non-blocking. Thread safety: thread-safe.
 */
void bsp_sensor_cal_set_alpha(float alpha);

/**
 * @brief Get Active MCU Die Temp EMA Filter Alpha Weight
 *
 * @return float Active alpha value
 * @details Memory ownership: none. Behavior: Non-blocking. Thread safety: thread-safe.
 */
float bsp_sensor_cal_get_alpha(void);

/**
 * @brief Enable or Disable Dynamic Thermal Compensation
 *
 * @param[in] enable true to apply dynamic MCU thermal compensation, false to return raw uncalibrated data
 * @details Memory ownership: none. Behavior: Non-blocking. Thread safety: thread-safe.
 */
void bsp_sensor_cal_enable(bool enable);

/**
 * @brief Check if Dynamic Thermal Compensation is Currently Enabled
 *
 * @return bool true if enabled
 * @details Memory ownership: none. Behavior: Non-blocking. Thread safety: thread-safe.
 */
bool bsp_sensor_cal_is_enabled(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_SENSOR_CAL_H */
