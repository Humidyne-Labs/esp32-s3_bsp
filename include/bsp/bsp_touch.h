/**
 * @file bsp_touch.h
 * @brief Hynitron CST816S Capacitive Single-Point Touch Controller Driver & LVGL Input Bridge
 * 
 * Hardware Target:
 *  - Controller: Hynitron CST816S (I2C Address: 0x15)
 *  - Resolution: 200 x 200 Pixels
 *  - Interrupt: GPIO 19, Reset: GPIO 14
 * 
 * @attribution
 * - Hynitron Microelectronics
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
 * @brief Touch Coordinate and Gesture Data
 */
typedef struct {
    uint16_t x;       /*!< X coordinate (0 - 199) */
    uint16_t y;       /*!< Y coordinate (0 - 199) */
    bool     pressed; /*!< True if finger is in active contact with panel */
    uint8_t  gesture; /*!< Gesture ID (0x01=Slide Up, 0x02=Slide Down, 0x03=Slide Left, 0x04=Slide Right, 0x05=Click) */
} bsp_touch_data_t;

/**
 * @brief Initialize CST816S Capacitive Touch Hardware and Reset Controller
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_touch_init(void);

/**
 * @brief Read Current Touch Coordinate and State
 * 
 * @param[out] out_data Destination struct to receive touch data
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_touch_read(bsp_touch_data_t *out_data);

/**
 * @brief Put CST816S Touch Controller into Ultra-Low Power Sleep Mode (< 5 µA)
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_touch_sleep(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_TOUCH_H */
