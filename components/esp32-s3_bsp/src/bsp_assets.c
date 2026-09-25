/**
 * @file bsp_assets.c
 * @brief Zero-Copy Flash Assets & LVGL Image Decoder implementation
 * 
 * @attribution
 * - Microcontroller: Espressif Systems ESP32-S3 (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_mmap_assets.h"
#include "esp_lv_fs.h"
#include "bsp/bsp_assets.h"

static const char *TAG = "bsp_assets";

static mmap_assets_handle_t s_asset_handle = NULL;
static esp_lv_fs_handle_t   s_lv_fs_handle = NULL;
static int                  s_asset_count  = 0;
static char                 s_drive_letter = '\0';

esp_err_t bsp_assets_init(const char *partition_label, const char drive_letter, int max_files, uint32_t checksum)
{
    if (partition_label == NULL) return ESP_ERR_INVALID_ARG;

    mmap_assets_config_t config = {
        .partition_label = partition_label,
        .max_files       = max_files,
        .checksum        = checksum,
        .flags           = {
            .mmap_enable = 1,
            .app_bin_check = 0,
        },
    };

    esp_err_t ret = mmap_assets_new(&config, &s_asset_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount mmap assets partition '%s': %s", partition_label, esp_err_to_name(ret));
        return ret;
    }

    s_asset_count = mmap_assets_get_stored_files(s_asset_handle);
    s_drive_letter = drive_letter;
    ESP_LOGI(TAG, "Mounted partition '%s' as Drive '%c:' (%d assets loaded)", 
             partition_label, drive_letter, s_asset_count);

    /* Register LVGL File System Bridge */
    const fs_cfg_t fs_cfg = {
        .fs_letter = drive_letter,
        .fs_nums   = max_files,
        .fs_assets = s_asset_handle,
    };
    ret = esp_lv_fs_desc_init(&fs_cfg, &s_lv_fs_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "esp_lv_fs_desc_init failed: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Registered LVGL File System for Drive '%c:'", drive_letter);
    }

    return ESP_OK;
}

esp_err_t init_drive(const char *partition_label, const char drive_letter, int max_files, uint32_t checksum)
{
    return bsp_assets_init(partition_label, drive_letter, max_files, checksum);
}
