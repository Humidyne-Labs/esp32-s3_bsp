/**
 * @file bsp_button.h
 * @brief Button driver with debouncing, multi-press, hold detection, and shutdown hooks.
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics (https://www.waveshare.com)
 * - Microcontroller: Espressif Systems ESP32-S3 (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_BUTTON_H
#define BSP_BUTTON_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BSP_BUTTON_BOOT = 0,
    BSP_BUTTON_POWER,
    BSP_BUTTON_COUNT,
} bsp_button_t;

typedef enum {
    BSP_BUTTON_EVENT_PRESS_DOWN = 0, /**< Triggered immediately when button is pressed */
    BSP_BUTTON_EVENT_PRESS_UP,       /**< Triggered when button is released */
    BSP_BUTTON_EVENT_SINGLE_CLICK,   /**< Triggered after a quick tap & release */
    BSP_BUTTON_EVENT_DOUBLE_CLICK,   /**< Triggered after two quick successive taps */
    BSP_BUTTON_EVENT_LONG_PRESS,     /**< Triggered when held past the custom hold threshold */
    BSP_BUTTON_EVENT_MAX
} bsp_button_event_t;

/**
 * @brief Button event callback signature.
 */
typedef void (*bsp_button_cb_t)(bsp_button_t button, bsp_button_event_t event, void *user_data);

/**
 * @brief Shutdown callback signature executed before power rail is cut.
 */
typedef void (*bsp_power_off_cb_t)(void *user_data);

/**
 * @brief Configuration structure for button timings and behavior.
 */
typedef struct {
    uint32_t debounce_ms;        /**< Debounce interval (default: 20ms) */
    uint32_t click_timeout_ms;   /**< Max time between taps for double-click (default: 300ms) */
    uint32_t long_press_ms;      /**< Hold duration to trigger LONG_PRESS event (default: 2500ms) */
    bool auto_power_off_on_hold; /**< If true, hold on POWER button calls bsp_power_off() */
} bsp_button_config_t;

/**
 * @brief Initialize button GPIOs, start background debouncing timer, and hold battery power latch.
 *
 * @param config Pointer to custom configuration, or NULL for default settings.
 * @return esp_err_t ESP_OK on success.
 */
esp_err_t bsp_button_init(const bsp_button_config_t *config);

/**
 * @brief Register a callback for a specific button and event.
 */
esp_err_t bsp_button_register_cb(bsp_button_t button, bsp_button_event_t event, bsp_button_cb_t cb, void *user_data);

/**
 * @brief Remove a previously registered button event callback.
 */
esp_err_t bsp_button_unregister_cb(bsp_button_t button, bsp_button_event_t event);

/**
 * @brief Register a pre-shutdown callback executed before cutting power in bsp_power_off().
 * 
 * @param cb Callback to run (save state, turn off display, flush storage).
 * @param user_data Custom pointer passed to callback.
 */
esp_err_t bsp_power_register_shutdown_cb(bsp_power_off_cb_t cb, void *user_data);

/**
 * @brief Read the instantaneous pressed state of a button (active-low).
 */
bool bsp_button_is_pressed(bsp_button_t button);

/**
 * @brief Lock power latch ON (`BSP_GPIO_BAT_CTRL` -> HIGH).
 *        Must be asserted at startup to stay powered on battery.
 */
esp_err_t bsp_power_hold(void);

/**
 * @brief Gracefully executes shutdown callback, releases power latch, and enters deep sleep.
 */
void bsp_power_off(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_BUTTON_H */