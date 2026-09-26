/**
 * @file bsp_power.h
 * @brief Power Management, LDO Power Latch, and Battery Voltage Monitor
 * 
 * Hardware Description:
 *  - Power Hold Latch: GPIO 17 (BAT_CTRL) maintains LDO regulator power from battery.
 *  - Battery Voltage: ADC1 CH3 (GPIO 4 / BAT_ADC) with 1:2 resistive divider (R1=100k, R2=100k).
 *  - Status LED: GPIO 3 (Active High).
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
#include "bsp/pinout.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize Power Subsystem & Battery ADC Monitor
 * 
 * Configures GPIO 17 HIGH to keep the board powered, configures status LED (GPIO 3),
 * and calibrates ADC1 Channel 3 for battery voltage sensing.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_power_init(void);

/**
 * @brief Assert Power Latch (GPIO 17 HIGH) to keep LDO active
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_power_hold(void);

/**
 * @brief Release Power Latch (GPIO 17 LOW) to shut off battery power
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_power_release(void);

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
 * Samples ADC1 CH3 (GPIO 4), applies calibration curve, and compensates for the 1:2 divider.
 * 
 * @param[out] out_mv Calculated battery voltage in millivolts (e.g. 4150 mV = 4.15V)
 * @param[out] out_raw Optional pointer to receive raw ADC reading (can be NULL)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_battery_get_voltage(uint32_t *out_mv, uint32_t *out_raw);

/**
 * @brief Calculate Approximate Battery Remaining Percentage (0 - 100%)
 * 
 * Uses non-linear Li-Po state-of-charge curve mapped between 3.30V (0%) and 4.20V (100%).
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
 * Configures timer and button wakeups, turns off status LED, and starts deep sleep.
 * 
 * @param duration_sec Sleep duration in seconds (0 for indefinite wakeup by button)
 * @return esp_err_t ESP_OK
 */
esp_err_t bsp_power_enter_deep_sleep(uint32_t duration_sec);

/**
 * @brief Enter Light Sleep Mode
 * 
 * Pauses CPU, powers down radios, and retains all RAM/tasks.
 * Resumes execution at the next line of code without board re-initialization.
 * 
 * @param duration_sec Sleep duration in seconds (0 for indefinite wakeup by button)
 * @return esp_err_t ESP_OK upon wakeup
 */
esp_err_t bsp_power_enter_light_sleep(uint32_t duration_sec);

#ifdef __cplusplus
}
#endif

#endif /* BSP_POWER_H */
