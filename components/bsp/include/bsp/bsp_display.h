/**
 * @file bsp_display.h
 * @brief ESP32-S3 ePaper BSP - 1.54 inch Monochrome e-Paper Display Driver
 * 
 * @attribution
 * - e-Paper Controller & Waveform LUTs: Waveshare Electronics (https://www.waveshare.com)
 * - SPI Driver Framework: Espressif Systems (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_DISPLAY_H
#define BSP_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "bsp/pinout.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_DISPLAY_WIDTH       (200)
#define BSP_DISPLAY_HEIGHT      (200)
#define BSP_DISPLAY_BUFFER_SIZE (BSP_DISPLAY_WIDTH * BSP_DISPLAY_HEIGHT / 8)

typedef enum {
    BSP_DISPLAY_COLOR_WHITE = 0xFF,
    BSP_DISPLAY_COLOR_BLACK = 0x00,
} bsp_display_color_t;

/**
 * @brief Initialize the e-Paper SPI display bus and GPIO pins
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_display_init(void);

/**
 * @brief Initialize display for partial refresh mode
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_display_init_partial(void);

/**
 * @brief Clear internal frame buffer to white
 */
void bsp_display_clear(void);

/**
 * @brief Send frame buffer to e-Paper display (full refresh)
 */
void bsp_display_flush(void);

/**
 * @brief Send frame buffer to e-Paper display (partial refresh)
 */
void bsp_display_flush_partial(void);

/**
 * @brief Set pixel color in frame buffer
 * 
 * @param x X coordinate (0-199)
 * @param y Y coordinate (0-199)
 * @param color BSP_DISPLAY_COLOR_WHITE or BSP_DISPLAY_COLOR_BLACK
 */
void bsp_display_draw_pixel(uint16_t x, uint16_t y, bsp_display_color_t color);

/**
 * @brief Direct access to frame buffer
 * 
 * @return uint8_t* Pointer to 5000-byte frame buffer
 */
uint8_t *bsp_display_get_buffer(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_DISPLAY_H */
