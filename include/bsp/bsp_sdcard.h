/**
 * @file bsp_sdcard.h
 * @brief MicroSD Card SPI-Mode FATFS Storage Driver
 * 
 * Hardware Target:
 *  - Interface: SPI Mode (MOSI: GPIO 7, MISO: GPIO 8, SCK: GPIO 6, CS: GPIO 21)
 *  - File System: FATFS mounted at "/sdcard"
 * 
 * @attribution
 * - BSP Implementation: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_SDCARD_H
#define BSP_SDCARD_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Mount MicroSD Card over SPI FATFS Subsystem
 * 
 * @param mount_point Base path for file system (e.g. "/sdcard")
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_sdcard_mount(const char *mount_point);

/**
 * @brief Unmount MicroSD Card and Release SPI Resources
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_sdcard_unmount(void);

/**
 * @brief Check if MicroSD Card is Currently Mounted
 * 
 * @return true if mounted, false otherwise
 */
bool bsp_sdcard_is_mounted(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_SDCARD_H */
