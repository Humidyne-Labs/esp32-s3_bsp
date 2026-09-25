/**
 * @file bsp_power.h
 * @brief Power Management, LDO Power Latch, and Battery Voltage Monitor
 * 
 * Hardware Description:
 *  - The board uses a power hold latch (GPIO 2) to maintain power from the onboard LDO regulator.
 *  - Battery voltage is measured through a 1:2 resistive divider (R1=100k, R2=100k) into ADC1 CH4 (GPIO 5).
 *  - Implements battery percentage estimation curves tailored for LiPo / Li-Ion chemistry.
 * 
 * @attribution
 * - Circuit Design: Waveshare Electronics
 * - BSP Unification: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_POWER_H
#define BSP_POWER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize Power Subsystem & Battery ADC Monitor
 * 
 * Latches GPIO 2 HIGH to keep the board powered, configures the status LED (GPIO 1),
 * and calibrates the ADC1 Channel 4 curve for precision battery voltage sampling.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_power_init(void);

/**
 * @brief Set Status LED Output State
 * 
 * @param state true to turn LED on, false to turn LED off
 */
void bsp_led_set(bool state);

/**
 * @brief Toggle Status LED Output State
 */
void bsp_led_toggle(void);

/**
 * @brief Read Raw and Calibrated Battery Terminal Voltage
 * 
 * Samples ADC1 CH4, applies calibration curve, and compensates for the 1:2 hardware divider.
 * 
 * @param[out] out_mv Calculated battery voltage in millivolts (e.g. 4150 mV = 4.15V)
 * @param[out] out_raw Optional pointer to receive raw ADC reading (can be NULL)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_battery_get_voltage(uint32_t *out_mv, uint32_t *out_raw);

/**
 * @brief Calculate Approximate Battery Remaining Percentage (0 - 100%)
 * 
 * Uses a non-linear state-of-charge curve mapped between 3.30V (0%) and 4.20V (100%).
 * 
 * @return uint8_t State of charge percentage (0 to 100)
 */
uint8_t bsp_battery_get_percentage(void);

/**
 * @brief Check if Battery is in Low Warning / Critical Condition
 * 
 * @param threshold_pct Low battery warning threshold percentage (e.g. 20%)
 * @return true if battery percentage is less than or equal to threshold
 */
bool bsp_battery_is_low(uint8_t threshold_pct);

/**
 * @brief Enter Ultra-Low Power Deep Sleep Mode
 * 
 * Configures timer and button wakeups, turns off peripherals, and starts deep sleep.
 * 
 * @param duration_sec Sleep duration in seconds (0 for indefinite wakeup by button)
 * @return esp_err_t ESP_OK
 */
esp_err_t bsp_power_enter_deep_sleep(uint32_t duration_sec);

#ifdef __cplusplus
}
#endif

#endif /* BSP_POWER_H */
