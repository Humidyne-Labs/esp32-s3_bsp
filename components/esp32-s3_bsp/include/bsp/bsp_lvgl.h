/**
 * @file bsp_lvgl.h
 * @brief LVGL v9 FreeRTOS Integration Port & Thread-Safe Mutex Lock API
 * 
 * FreeRTOS Multithreading Model:
 *  - Spawns a dedicated FreeRTOS render task (`bsp_lvgl_port_task`) pinned to Core 1 at Priority 5.
 *  - All external task accesses to LVGL API objects MUST be encapsulated between
 *    `bsp_lvgl_lock()` and `bsp_lvgl_unlock()` to avoid rendering collisions.
 * 
 * @attribution
 * - LVGL Community (https://lvgl.io)
 * - BSP Implementation: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_LVGL_H
#define BSP_LVGL_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize LVGL v9 Graphics Subsystem and Register Display Port
 * 
 * Allocates draw buffers and registers the SSD1681 1-bit monochrome flush callback.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_lvgl_init(void);

/**
 * @brief Start LVGL Background FreeRTOS Execution Task
 * 
 * @param priority Task priority (Default: 5)
 * @param core_id CPU Core affinity (Default: 1 - Core 1)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_lvgl_start(int priority, int core_id);

/**
 * @brief Acquire LVGL Reentrant Mutex Lock
 * 
 * Must be called prior to modifying any UI widgets or invoking lvgl functions from external tasks.
 * 
 * @return true if mutex was successfully acquired
 */
bool bsp_lvgl_lock(void);

/**
 * @brief Release LVGL Reentrant Mutex Lock
 */
void bsp_lvgl_unlock(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_LVGL_H */
