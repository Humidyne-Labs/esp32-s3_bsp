/**
 * @file bsp_display.h
 * @brief SSD1681 1.54" 200x200 Monochrome e-Paper Display Driver
 * 
 * Hardware Target:
 *  - Controller: Solomon Systech SSD1681
 *  - Resolution: 200 x 200 Pixels (1-bit monochrome: 1=White, 0=Black)
 *  - Interface: 4-wire SPI (MOSI, SCK, CS, DC, RST, BUSY)
 *  - Power Characteristic: Bi-stable (retains image with 0 µA power draw)
 * 
 * @attribution
 * - Solomon Systech Limited (SSD1681 Datasheet)
 * - Waveshare Electronics
 * - BSP Unification: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_DISPLAY_H
#define BSP_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize SSD1681 SPI Hardware Interface and Panel Controller
 * 
 * Configures SPI Master bus (GPIO 6, 7, 18), sets DC/RST/BUSY pins, and performs panel init.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_display_init(void);

/**
 * @brief Clear Entire e-Paper Panel to Pure White
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_display_clear(void);

/**
 * @brief Transmit 1-bit Monochrome Framebuffer to SSD1681 Controller
 * 
 * @param buffer Pointer to 5000-byte 1-bit packed bitmap buffer (200x200 / 8 bytes)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_display_write_frame(const uint8_t *buffer);

/**
 * @brief Trigger Physical e-Paper Waveform Refresh Cycle
 * 
 * @param partial_mode true for fast partial refresh (~0.3s), false for full LUT refresh (~1.5s)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_display_refresh(bool partial_mode);

/**
 * @brief Put SSD1681 Controller into Deep Sleep Mode (< 1 µA)
 * 
 * Preserves the displayed image on the physical panel indefinitely without power.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_display_sleep(void);

/**
 * @brief Wait until E-Paper BUSY line goes inactive (Low)
 * 
 * @param timeout_ms Maximum time to wait in milliseconds
 * @return esp_err_t ESP_OK when panel is ready, or ESP_ERR_TIMEOUT
 */
esp_err_t bsp_display_wait_idle(uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* BSP_DISPLAY_H */
