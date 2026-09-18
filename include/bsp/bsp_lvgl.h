/**
 * @file bsp_lvgl.h
 * @brief ESP32-S3 ePaper BSP - LVGL v9 Graphics Library Port
 * 
 * @attribution
 * - Graphics Engine: LVGL - Light and Versatile Graphics Library (https://lvgl.io)
 *   Licensed under MIT License by the LVGL Team.
 * - Hardware Interface & Flush Porting: Waveshare Electronics & Humidyne Labs
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_LVGL_H
#define BSP_LVGL_H

#include "esp_err.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize LVGL v9 for e-Paper display and touch input port
 * 
 * Configures LVGL display driver with e-paper flush callback and optional touch input device.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_lvgl_init(void);

/**
 * @brief LVGL timer task tick handler (call periodically or from FreeRTOS task)
 */
void bsp_lvgl_port_task(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* BSP_LVGL_H */
