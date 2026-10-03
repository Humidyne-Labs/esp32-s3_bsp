/**
 * @file bsp_display.h
 * @brief SSD1681 1.54" 200x200 Monochrome e-Paper Display Driver
 *
 * Hardware Target:
 *  - Controller: Solomon Systech SSD1681
 *  - Resolution: 200 x 200 Pixels (1-bit monochrome: 1=White, 0=Black)
 *  - Interface: 4-wire SPI (MOSI: GPIO 13, SCK: GPIO 12, CS: GPIO 11, DC: GPIO 10, RST: GPIO 9, BUSY: GPIO 8, 3V3_EN: GPIO 6)
 *  - Power Characteristic: Bi-stable (retains image with 0 µA power draw)
 *
 * @version 1.0.0
 * @attribution
 * - Architecture & Development: HUMIDYNE LABS / Humiditron
 * - AI Systems Co-Developer: Gemini (Google DeepMind)
 * - Peripheral Driver Basis: Waveshare Electronics 1.54" V2 SPI e-Paper Display Codebase
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BSP_DISPLAY_H
#define BSP_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"
#include "bsp/pinout.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Monochrome Pixel Colors
 */
typedef enum {
    BSP_DISPLAY_COLOR_BLACK = 0, ///< BSP_DISPLAY_COLOR_BLACK value
    BSP_DISPLAY_COLOR_WHITE = 1, ///< BSP_DISPLAY_COLOR_WHITE value
} bsp_display_color_t;

/**
 * @brief 1-Bit Packed Framebuffer Size (200 * 200 / 8 = 5000 bytes)
 */
#define BSP_DISPLAY_BUFFER_SIZE ((BSP_DISPLAY_WIDTH * BSP_DISPLAY_HEIGHT) / 8)

/**
 * @brief Initialize SSD1681 SPI Hardware Interface and Panel Controller
 *
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_display_init(void);

/**
 * @brief Clear Entire In-Memory Framebuffer to Pure White
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
void bsp_display_clear(void);

/**
 * @brief Draw Single Pixel to Framebuffer
 *
 * @param[in] x Coordinate (0 to 199)
 * @param[in] y Coordinate (0 to 199)
 * @param[in] color Pixel color (BLACK or WHITE)
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
void bsp_display_draw_pixel(uint16_t x, uint16_t y, bsp_display_color_t color);

/**
 * @brief Retrieve Pointer to In-Memory Framebuffer (5000 bytes)
 *
 * @return uint8_t* Buffer pointer
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
uint8_t *bsp_display_get_buffer(void);

/**
 * @brief Full OTP Waveform Hardware Refresh of In-Memory Framebuffer to Screen
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
void bsp_display_flush(void);

/**
 * @brief Fast Partial Waveform Refresh of Full Screen Area
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
void bsp_display_flush_partial(void);

/**
 * @brief Fast Partial Waveform Refresh of Specific Bounding Box Area
 *
 * @param[in] x_start Left coordinate
 * @param[in] y_start Top coordinate
 * @param[in] x_end   Right coordinate
 * @param[in] y_end   Bottom coordinate
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
void bsp_display_flush_partial_area(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end);

/**
 * @brief Transmit Arbitrary 1-bit Monochrome Framebuffer to SSD1681 Controller
 *
 * @param[in] buffer Pointer to 5000-byte 1-bit packed bitmap buffer
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_display_write_frame(const uint8_t *buffer);

/**
 * @brief Trigger Physical e-Paper Waveform Refresh Cycle
 *
 * @param[in] partial_mode true for fast partial refresh (~0.3s), false for full LUT refresh (~1.5s)
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_display_refresh(bool partial_mode);

/**
 * @brief Put SSD1681 Controller into Deep Sleep Mode (< 1 µA)
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
void bsp_display_deep_sleep(void);

/**
 * @brief Alias for bsp_display_deep_sleep
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 * @return esp_err_t ESP_OK on success, or appropriate ESP error code.
 */
esp_err_t bsp_display_sleep(void);

/**
 * @brief Wait until E-Paper BUSY hardware signal goes inactive (Low)
 *
 * @param[in] timeout_ms Maximum safety timeout in milliseconds before returning ESP_ERR_TIMEOUT
 * @return esp_err_t ESP_OK when panel is ready, or ESP_ERR_TIMEOUT
 * @note This function returns IMMEDIATELY as soon as the physical panel update completes
 *       (typically ~0.3s for partial refresh, ~1.5s for full refresh). The timeout_ms argument
 *       is a maximum safety limit, NOT a fixed sleep/delay duration.
 * @note To hold a screen visible for a fixed time (e.g. 10 seconds), call vTaskDelay() separately
 *       after this function returns.
 * @details Memory ownership: none. Behavior: Polling wait with TaskDelay yield. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_display_wait_busy(uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* BSP_DISPLAY_H */
