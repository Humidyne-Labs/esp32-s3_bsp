/**
 * @file bsp_power.h
 * @brief Power Management, LDO Power Latch, Battery Monitor, Shutdown & Dual Sleep Modes
 *
 * Hardware Description:
 *  - Power Hold Latch: GPIO 17 (BAT_CTRL) maintains LDO regulator power from battery.
 *  - Battery Voltage: ADC1 CH3 (GPIO 4 / BAT_ADC) with 1:2 resistive divider (R1=100k, R2=100k).
 *  - Status LED: GPIO 3 (Active Low).
 *
 * Sleep Architecture:
 *  - Light Sleep: Preserves CPU/SRAM state, fast resume.
 *  - Deep Sleep: Powers down CPU/peripherals, preserves RTC Slow Memory and state flags.
 *  - Wake Sources: ESP32-S3 Internal Timer, PCF85063A External RTC INT (GPIO 5), BOOT/POWER Buttons.
 *
 * Shutdown Architecture:
 *  - Clean shutdown sequence triggers splash screen & chimes, executes lifecycle on_shutdown hooks,
 *    stops background FreeRTOS tasks, powers down display/audio, and releases GPIO 17 latch.
 *
 * @version 1.0.0
 * @attribution
 * - Architecture & Development: HUMIDYNE LABS / Humiditron
 * - AI Systems Co-Developer: Gemini (Google DeepMind)
 * - Hardware Target: Waveshare Electronics ESP32-S3 ePaper 1.54 V2
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_POWER_H
#define BSP_POWER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "esp_system.h"
#include "esp_sleep.h"
#include "bsp/pinout.h"
#include "bsp/bsp_rtc_mem.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief System shutdown / power-off callback function pointer
 */
typedef void (*bsp_power_off_cb_t)(void *user_data);

/**
 * @brief Wakeup Source Selection Flags
 */
typedef enum {
    BSP_WAKE_SRC_TIMER        = (1 << 0), ///< ESP32-S3 Internal RTC Sleep Timer
    BSP_WAKE_SRC_EXTERNAL_RTC = (1 << 1), ///< External PCF85063A RTC INT on GPIO 5
    BSP_WAKE_SRC_BUTTONS      = (1 << 2), ///< Hardware BOOT0 (GPIO 0) and POWER (GPIO 18) keys
    BSP_WAKE_SRC_ALL          = (BSP_WAKE_SRC_TIMER | BSP_WAKE_SRC_EXTERNAL_RTC | BSP_WAKE_SRC_BUTTONS), ///< BSP_WAKE_SRC_ALL value
} bsp_wake_source_mask_t;

/**
 * @brief Unified Sleep Configuration
 */
typedef struct {
    bsp_sleep_mode_t       mode;           ///< Target sleep mode (Light or Deep)
    uint32_t               duration_sec;   ///< Sleep duration in seconds (0 for indefinite / button only)
    bsp_wake_source_mask_t wake_sources;   ///< Bitmask of enabled wake triggers
    bsp_init_mode_t        next_init_mode; ///< Hardware initialization mode to perform on wake
} bsp_sleep_config_t;

/**
 * @brief Default Deep Sleep Configuration Macro
 */
#define BSP_SLEEP_CONFIG_DEFAULT() { \
    .mode           = BSP_SLEEP_MODE_DEEP, \
    .duration_sec   = 0, \
    .wake_sources   = BSP_WAKE_SRC_ALL, \
    .next_init_mode = BSP_INIT_MODE_FAST \
}

/**
 * @brief Initialize Power Subsystem & Battery ADC Monitor
 *
 * Configures GPIO 17 HIGH to keep the board powered, configures status LED (GPIO 3),
 * and calibrates ADC1 Channel 3 for battery voltage sensing.
 *
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_power_init(void);

/**
 * @brief Assert Power Latch (GPIO 17 HIGH) to keep LDO active
 *
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_power_hold(void);

/**
 * @brief Release Power Latch (GPIO 17 LOW) to shut off battery power
 *
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_power_release(void);

/**
 * @brief Register custom shutdown callback hook
 *
 * Invoked during bsp_power_off() before power latch drops.
 *
 * @param[in] cb Callback function
 * @param[in] user_data Custom user data pointer
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_power_register_shutdown_cb(bsp_power_off_cb_t cb, void *user_data);

/**
 * @brief Unregister shutdown callback hook
 *
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_power_unregister_shutdown_cb(void);

/**
 * @brief Turn board completely off
 *
 * Executes shutdown splash & chimes, lifecycle on_shutdown hooks, stops background tasks,
 * isolates power rails, and drops the BAT_CTRL power hold latch. If external power (USB)
 * is present, reboots cleanly.
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
void bsp_power_off(void);

/**
 * @brief Set Status LED Output State
 *
 * @param[in] state true to turn LED on, false to turn LED off
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
void bsp_led_set(bool state);

/**
 * @brief Toggle Status LED Output State
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
void bsp_led_toggle(void);

/**
 * @brief Read Raw and Calibrated Battery Terminal Voltage
 *
 * Samples ADC1 CH3 (GPIO 4), applies calibration curve, and compensates for the 1:2 divider.
 *
 * @param[out] out_mv Calculated battery voltage in millivolts (e.g. 4150 mV = 4.15V)
 * @param[out] out_raw Optional pointer to receive raw ADC reading (can be NULL)
 * @return esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG if out_mv is NULL
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_battery_get_voltage(uint32_t *out_mv, uint32_t *out_raw);

/**
 * @brief Calculate Approximate Battery Remaining Percentage (0 - 100%)
 *
 * Uses non-linear Li-Po state-of-charge curve mapped between 3.30V (0%) and 4.20V (100%).
 *
 * @return uint8_t State of charge percentage (0 to 100)
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
uint8_t bsp_battery_get_percentage(void);

/**
 * @brief Check if Battery is in Low Warning / Critical Condition
 *
 * @param[in] threshold_pct Low battery warning threshold percentage (e.g. 20%)
 * @return true if battery percentage is less than or equal to threshold
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
bool bsp_battery_is_low(uint8_t threshold_pct);

/**
 * @brief Low-Level Sleep Execution Driver (Light or Deep Sleep)
 *
 * Configures hardware wake sources, enables RTC GPIO hold, and executes esp_light_sleep_start()
 * or esp_deep_sleep_start(). Applications should call bsp_lifecycle_enter_sleep() instead.
 *
 * @param[in] config Sleep configuration parameters
 * @return esp_err_t ESP_OK (returns upon wake if Light Sleep)
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_enter_sleep(const bsp_sleep_config_t *config);

/**
 * @brief Get the system reset reason reported by ESP-IDF
 *
 * @return esp_reset_reason_t Reset reason
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_reset_reason_t bsp_get_reset_reason(void);

/**
 * @brief Get the sleep wakeup cause reported by ESP-IDF
 *
 * @return esp_sleep_wakeup_cause_t Wakeup cause
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_sleep_wakeup_cause_t bsp_get_wakeup_cause(void);

/**
 * @brief Determine the recommended hardware initialization mode based on reset & wake history
 *
 * @return bsp_init_mode_t Recommended init mode (FULL, FAST, or MIN)
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
bsp_init_mode_t bsp_get_recommended_init_mode(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_POWER_H */
