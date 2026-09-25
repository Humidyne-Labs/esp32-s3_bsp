/**
 * @file bsp_touch.h
 * @brief FocalTech FT6336 Capacitive Touch Controller Driver & LVGL Input Bridge
 * 
 * Hardware Target:
 *  - Controller: FocalTech FT6336 (I2C Address: 0x38)
 *  - Resolution: 200 x 200 Pixels
 *  - Pins: Reset GPIO 7, Interrupt GPIO 21
 * 
 * @attribution
 * - FocalTech Systems Co., Ltd.
 * - BSP Implementation: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_TOUCH_H
#define BSP_TOUCH_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize FT6336 Capacitive Touch Hardware and Perform Reset
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_touch_init(void);

/**
 * @brief Hardware Reset the FT6336 Controller
 */
void bsp_touch_reset(void);

/**
 * @brief Read Current Touch Coordinate and State
 * 
 * @param[out] x Pointer to receive X coordinate (0 - 199)
 * @param[out] y Pointer to receive Y coordinate (0 - 199)
 * @return true if panel is touched, false otherwise
 */
bool bsp_touch_read(uint16_t *x, uint16_t *y);

/**
 * @brief Put FT6336 Touch Controller into Low Power Sleep Mode
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_touch_sleep(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_TOUCH_H */
