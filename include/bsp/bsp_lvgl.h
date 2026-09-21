/**
 * @file bsp_lvgl.h
 * @brief lvgl port
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics (https://www.waveshare.com)
 * - Microcontroller: Espressif Systems ESP32-S3 (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron
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

void bsp_lvgl_lock(void);

void bsp_lvgl_unlock(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_LVGL_H */
