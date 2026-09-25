/**
 * @file bsp_button.h
 * @brief Tactile Button Interrupt & Debouncing Subsystem
 * 
 * Hardware Target:
 *  - BOOT Button: GPIO 0 (Active Low, internal pull-up)
 *  - POWER Button: GPIO 3 (Active Low, internal pull-up)
 * 
 * Event Recognition:
 *  - Single Click (< 500 ms press)
 *  - Double Click
 *  - Long Press (Configurable, e.g. 5000 ms for factory reset wipe)
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

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BSP_BUTTON_BOOT = 0,    /*!< GPIO 0 User / Boot Button */
    BSP_BUTTON_POWER = 1,   /*!< GPIO 3 Power / Wakeup Button */
    BSP_BUTTON_MAX
} bsp_button_t;

typedef enum {
    BSP_BUTTON_EVENT_PRESS_DOWN,
    BSP_BUTTON_EVENT_PRESS_UP,
    BSP_BUTTON_EVENT_SINGLE_CLICK,
    BSP_BUTTON_EVENT_DOUBLE_CLICK,
    BSP_BUTTON_EVENT_LONG_PRESS,
} bsp_button_event_t;

typedef void (*bsp_button_cb_t)(bsp_button_t btn, bsp_button_event_t evt, void *user_data);

/**
 * @brief Initialize Hardware Button Interrupts and Debounce Timer
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_button_init(void);

/**
 * @brief Register Callback for Button Event
 * 
 * @param btn Target button (BOOT or POWER)
 * @param evt Target event type
 * @param cb Callback function
 * @param user_data Custom user pointer passed to callback
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_button_register_cb(bsp_button_t btn, bsp_button_event_t evt, bsp_button_cb_t cb, void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* BSP_BUTTON_H */
