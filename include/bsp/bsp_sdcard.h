/**
 * @file bsp_sdcard.h
 * @brief sdcard lib
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics (https://www.waveshare.com)
 * - Microcontroller: Espressif Systems ESP32-S3 (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_SDCARD_H
#define BSP_SDCARD_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "sdmmc_cmd.h"
#include "bsp/pinout.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_SDCARD_MOUNT_POINT  "/sdcard"

/**
 * @brief Initialize and mount MicroSD card filesystem over SDMMC 1-line interface
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_sdcard_mount(void);

/**
 * @brief Unmount MicroSD card
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_sdcard_unmount(void);

/**
 * @brief Get MicroSD card capacity in Gigabytes
 * 
 * @return float Capacity in GB, or 0.0 if not mounted
 */
float bsp_sdcard_get_capacity_gb(void);

/**
 * @brief Check if MicroSD card is mounted and healthy
 * 
 * @return true if mounted, false otherwise
 */
bool bsp_sdcard_is_mounted(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_SDCARD_H */
