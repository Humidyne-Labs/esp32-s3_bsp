/**
 * @file bsp_button.h
 * @brief Tactile Button Interrupt & Debouncing Subsystem
 *
 * Hardware Target:
 *  - BOOT Button: GPIO 0 (Active Low, internal pull-up)
 *  - POWER Button: GPIO 18 (Active Low, internal pull-up)
 *
 * Event Recognition:
 *  - Press Down / Press Up
 *  - Single Click (< 500 ms press)
 *  - Double Click
 *  - Long Press (Configurable, e.g. 2500 ms)
 *
 * @version 1.0.0
 * @attribution
 * - Architecture & Development: HUMIDYNE LABS / Humiditron
 * - AI Systems Co-Developer: Gemini (Google DeepMind)
 * - Hardware Target: Waveshare Electronics ESP32-S3 ePaper 1.54 V2
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_BUTTON_H
#define BSP_BUTTON_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "bsp/pinout.h"
#include "bsp/bsp_power.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Hardware Button Identifiers
 */
typedef enum {
    BSP_BUTTON_BOOT  = 0,    ///< GPIO  0 User  / Boot Button
    BSP_BUTTON_POWER = 1,    ///< GPIO 18 Power / Battery Key
    BSP_BUTTON_COUNT, ///< BSP_BUTTON_COUNT value
    BSP_BUTTON_MAX   = BSP_BUTTON_COUNT, ///< BSP_BUTTON_MAX value
} bsp_button_t;

/**
 * @brief Button Event Types
 */
typedef enum {
    BSP_BUTTON_EVENT_PRESS_DOWN = 0, ///< Button transitioned to pressed state
    BSP_BUTTON_EVENT_PRESS_UP,       ///< Button transitioned to released state
    BSP_BUTTON_EVENT_SINGLE_CLICK,   ///< Single click completed
    BSP_BUTTON_EVENT_DOUBLE_CLICK,   ///< Double click detected within click timeout window
    BSP_BUTTON_EVENT_LONG_PRESS,     ///< Button held down past long_press_ms threshold
    BSP_BUTTON_EVENT_MAX
} bsp_button_event_t;

/**
 * @brief Button Timing & Feature Configuration
 */
typedef struct {
    uint32_t debounce_ms;            ///< Debounce settling time in milliseconds (Default: 20 ms)
    uint32_t click_timeout_ms;       ///< Max delay between double clicks in ms (Default: 280 ms)
    uint32_t long_press_ms;          ///< Duration to trigger long press in ms (Default: 2500 ms)
    bool     auto_power_off_on_hold; ///< Auto power off system on POWER long press (Default: true)
} bsp_button_config_t;

/**
 * @brief Button Event Callback Signature
 *
 * @param btn Originating button (BOOT or POWER)
 * @param evt Triggered event type
 * @param user_data Custom user pointer passed during registration
 */
typedef void (*bsp_button_cb_t)(bsp_button_t btn, bsp_button_event_t evt, void *user_data);

/**
 * @brief Initialize Hardware Button Interrupts and Debounce Timer
 *
 * @param[in] config Pointer to timing config, or NULL for defaults
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_button_init(const bsp_button_config_t *config);

/**
 * @brief Stop Button Polling and Debounce Timer
 *
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_button_stop(void);

/**
 * @brief Register Callback for Button Event
 *
 * @param[in] button Target button (BOOT or POWER)
 * @param[in] event Target event type
 * @param[in] cb Callback function
 * @param[in] user_data Custom user pointer passed to callback
 * @return esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG on invalid button/event
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_button_register_cb(bsp_button_t button, bsp_button_event_t event, bsp_button_cb_t cb, void *user_data);

/**
 * @brief Unregister Callback for Button Event
 *
 * @param[in] button Target button
 * @param[in] event Target event type
 * @return esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG on invalid button/event
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_button_unregister_cb(bsp_button_t button, bsp_button_event_t event);

/**
 * @brief Check if button is currently pressed down
 *
 * @param[in] button Target button
 * @return true if pressed (GPIO level LOW), false otherwise
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
bool bsp_button_is_pressed(bsp_button_t button);

/**
 * @brief Synchronously block and wait for a button click (press + release)
 *
 * @param[in] button Target button (BOOT or POWER)
 * @param[in] timeout_ms Maximum time to wait in milliseconds (0 for indefinite blocking)
 * @return esp_err_t ESP_OK on click, ESP_ERR_TIMEOUT on timeout, ESP_ERR_INVALID_ARG on bad button
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_button_wait_for_click(bsp_button_t button, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* BSP_BUTTON_H */
