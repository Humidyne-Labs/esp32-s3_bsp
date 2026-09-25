/**
 * @file bsp_sdcard.h
 * @brief MicroSD Card SDMMC 1-Bit Mode FATFS Storage Driver
 * 
 * Hardware Target:
 *  - Interface: 1-bit SDMMC (CLK: GPIO 39, MISO/D0: GPIO 40, MOSI/CMD: GPIO 41)
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
#include "bsp/pinout.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_SDCARD_MOUNT_POINT "/sdcard"

/**
 * @brief Mount MicroSD Card over 1-bit SDMMC FATFS Subsystem
 * 
 * @return esp_err_t ESP_OK on success, ESP_ERR_NOT_FOUND if card missing
 */
esp_err_t bsp_sdcard_mount(void);

/**
 * @brief Unmount MicroSD Card and Release SDMMC Resources
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

/**
 * @brief Retrieve total capacity of mounted MicroSD Card in Gigabytes
 * 
 * @return float Capacity in GB
 */
float bsp_sdcard_get_capacity_gb(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_SDCARD_H */
