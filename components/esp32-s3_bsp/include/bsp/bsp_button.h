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
 * @attribution
 * - BSP Implementation: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_BUTTON_H
#define BSP_BUTTON_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "bsp/pinout.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BSP_BUTTON_BOOT = 0,    /*!< GPIO 0 User / Boot Button */
    BSP_BUTTON_POWER = 1,   /*!< GPIO 18 Power / Battery Key */
    BSP_BUTTON_COUNT,
    BSP_BUTTON_MAX = BSP_BUTTON_COUNT,
} bsp_button_t;

typedef enum {
    BSP_BUTTON_EVENT_PRESS_DOWN = 0,
    BSP_BUTTON_EVENT_PRESS_UP,
    BSP_BUTTON_EVENT_SINGLE_CLICK,
    BSP_BUTTON_EVENT_DOUBLE_CLICK,
    BSP_BUTTON_EVENT_LONG_PRESS,
    BSP_BUTTON_EVENT_MAX
} bsp_button_event_t;

typedef struct {
    uint32_t debounce_ms;
    uint32_t click_timeout_ms;
    uint32_t long_press_ms;
    bool     auto_power_off_on_hold;
} bsp_button_config_t;

typedef void (*bsp_button_cb_t)(bsp_button_t btn, bsp_button_event_t evt, void *user_data);
typedef void (*bsp_power_off_cb_t)(void *user_data);

/**
 * @brief Initialize Hardware Button Interrupts and Debounce Timer
 * 
 * @param config Pointer to timing config, or NULL for defaults
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_button_init(const bsp_button_config_t *config);

/**
 * @brief Register Callback for Button Event
 * 
 * @param button Target button (BOOT or POWER)
 * @param event Target event type
 * @param cb Callback function
 * @param user_data Custom user pointer passed to callback
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_button_register_cb(bsp_button_t button, bsp_button_event_t event, bsp_button_cb_t cb, void *user_data);

/**
 * @brief Unregister Callback for Button Event
 * 
 * @param button Target button
 * @param event Target event type
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_button_unregister_cb(bsp_button_t button, bsp_button_event_t event);

/**
 * @brief Check if button is currently pressed down
 * 
 * @param button Target button
 * @return true if pressed, false otherwise
 */
bool bsp_button_is_pressed(bsp_button_t button);

/**
 * @brief Register system shutdown callback hook
 * 
 * @param cb Callback invoked before power latch drops
 * @param user_data Custom user data pointer
 * @return esp_err_t ESP_OK
 */
esp_err_t bsp_power_register_shutdown_cb(bsp_power_off_cb_t cb, void *user_data);

/**
 * @brief Execute Clean Hardware Shutdown (Drop BAT_CTRL latch)
 */
void bsp_power_off(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_BUTTON_H */
