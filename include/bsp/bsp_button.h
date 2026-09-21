/**
 * @file bsp_button.h
 * @brief button lib
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
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BSP_BUTTON_BOOT = 0,
    BSP_BUTTON_POWER,
    BSP_BUTTON_COUNT,
} bsp_button_t;

/**
 * @brief Initialize the BOOT and power buttons as active-low inputs.
 *
 * BOOT is GPIO0 and the power/battery key is GPIO18.
 */
esp_err_t bsp_button_init(void);

/**
 * @brief Read whether a button is currently pressed.
 *
 * @param button Button to read.
 * @return true when the selected active-low input is pressed.
 */
bool bsp_button_is_pressed(bsp_button_t button);

#ifdef __cplusplus
}
#endif

#endif /* BSP_BUTTON_H */
