/**
 * @file bsp_assets.h
 * @brief Zero-Copy Flash Assets & LVGL v9 Image Decoder using esp_mmap_assets
 * 
 * Performance Overview:
 *  - Maps asset binary files directly from SPI flash into the CPU's memory address space via MMU.
 *  - Allows LVGL v9 to stream and render images (such as space_cat.bin) directly from flash
 *    without allocating RAM framebuffers (zero-copy decoding).
 * 
 * @attribution
 * - Espressif Systems (esp_mmap_assets component)
 * - BSP Unification: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_ASSETS_H
#define BSP_ASSETS_H

#include <stdint.h>
#include "esp_err.h"
#include "esp_mmap_assets.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize and mount MMAP storage partition for LVGL zero-copy asset decoding
 * 
 * @param partition_label Partition table name (e.g. "storage")
 * @param drive_letter Virtual drive letter for LVGL file system (e.g. 'S' for "S:image.bin")
 * @param max_files Maximum asset file count
 * @param checksum Expected MMAP checksum
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_assets_init(const char *partition_label, const char drive_letter, int max_files, uint32_t checksum);

/**
 * @brief Backward-compatible alias for bsp_assets_init
 */
esp_err_t init_drive(const char *partition_label, const char drive_letter, int max_files, uint32_t checksum);

#ifdef __cplusplus
}
#endif

#endif /* BSP_ASSETS_H */
