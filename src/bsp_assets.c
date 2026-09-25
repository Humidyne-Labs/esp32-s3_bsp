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
#include <ctype.h>
#include "esp_log.h"
#include "lvgl.h"
#include "esp_lv_fs.h"
#include "bsp/bsp_assets.h"

static const char *TAG = "bsp_assets";

static mmap_assets_handle_t s_asset_handle = NULL;
static esp_lv_fs_handle_t   s_lv_fs_handle = NULL;
static int                  s_asset_count  = 0;
static char                 s_drive_letter = '\0';

static const char *normalize_asset_path(const char *path)
{
    if (path == NULL) return NULL;
    const char *p = path;

    if (s_drive_letter != '\0' &&
        toupper((unsigned char)p[0]) == toupper((unsigned char)s_drive_letter) &&
        p[1] == ':') {
        p += 2;
    }

    while (*p == '/' || *p == '\\') {
        p++;
    }
    return p;
}

static const uint8_t *get_asset_flash_ptr(const char *path, size_t *out_size)
{
    if (s_asset_handle == NULL || s_asset_count <= 0) {
        ESP_LOGE(TAG, "Assets partition not initialized or empty");
        return NULL;
    }

    const char *search_name = normalize_asset_path(path);

    for (int i = 0; i < s_asset_count; i++) {
        const char *name = mmap_assets_get_name(s_asset_handle, i);
        if (!name) continue;

        const char *name_basename = strrchr(name, '/');
        name_basename = (name_basename != NULL) ? name_basename + 1 : name;

        if (strcmp(name, search_name) == 0 || strcmp(name_basename, search_name) == 0) {
            size_t size = (size_t)mmap_assets_get_size(s_asset_handle, i);
            const uint8_t *mem = (const uint8_t *)mmap_assets_get_mem(s_asset_handle, i);
            if (out_size) *out_size = size;
            return mem;
        }
    }

    ESP_LOGW(TAG, "Asset '%s' not found in partition", search_name);
    return NULL;
}

static lv_result_t mmap_decoder_info_cb(lv_image_decoder_t *decoder, lv_image_decoder_dsc_t *dsc, lv_image_header_t *header)
{
    LV_UNUSED(decoder);

    if (dsc == NULL || dsc->src_type != LV_IMAGE_SRC_FILE) {
        return LV_RESULT_INVALID;
    }

    const char *src_str = (const char *)dsc->src;
    if (s_drive_letter != '\0') {
        if (toupper((unsigned char)src_str[0]) != toupper((unsigned char)s_drive_letter) || src_str[1] != ':') {
            return LV_RESULT_INVALID;
        }
    }

    size_t size = 0;
    const uint8_t *flash_ptr = get_asset_flash_ptr(src_str, &size);
    if (flash_ptr == NULL || size < sizeof(lv_image_header_t)) {
        return LV_RESULT_INVALID;
    }

    const lv_image_header_t *src_hdr = (const lv_image_header_t *)flash_ptr;
    header->cf     = src_hdr->cf;
    header->w      = src_hdr->w;
    header->h      = src_hdr->h;
    header->stride = src_hdr->stride;
    header->flags  = src_hdr->flags;
    header->magic  = src_hdr->magic;

    return LV_RESULT_OK;
}

static lv_result_t mmap_decoder_open_cb(lv_image_decoder_t *decoder, lv_image_decoder_dsc_t *dsc)
{
    LV_UNUSED(decoder);

    if (dsc == NULL || dsc->src_type != LV_IMAGE_SRC_FILE) {
        return LV_RESULT_INVALID;
    }

    const char *src_str = (const char *)dsc->src;
    if (s_drive_letter != '\0') {
        if (toupper((unsigned char)src_str[0]) != toupper((unsigned char)s_drive_letter) || src_str[1] != ':') {
            return LV_RESULT_INVALID;
        }
    }

    size_t size = 0;
    const uint8_t *flash_ptr = get_asset_flash_ptr(src_str, &size);
    if (flash_ptr == NULL || size < sizeof(lv_image_header_t)) {
        return LV_RESULT_INVALID;
    }

    const uint8_t *pixel_data = flash_ptr + sizeof(lv_image_header_t);
    dsc->img_data = pixel_data;
    return LV_RESULT_OK;
}

static void mmap_decoder_close_cb(lv_image_decoder_t *decoder, lv_image_decoder_dsc_t *dsc)
{
    LV_UNUSED(decoder);
    LV_UNUSED(dsc);
}

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
    esp_lv_fs_config_t fs_cfg = {
        .fs_type      = ESP_LV_FS_TYPE_MEM,
        .fs_letter    = drive_letter,
        .asset_handle = s_asset_handle,
    };
    esp_lv_fs_init(&fs_cfg, &s_lv_fs_handle);

    /* Register LVGL Zero-Copy Image Decoder */
    lv_image_decoder_t *decoder = lv_image_decoder_create();
    if (decoder) {
        lv_image_decoder_set_info_cb(decoder, mmap_decoder_info_cb);
        lv_image_decoder_set_open_cb(decoder, mmap_decoder_open_cb);
        lv_image_decoder_set_close_cb(decoder, mmap_decoder_close_cb);
        ESP_LOGI(TAG, "Registered Zero-Copy LVGL v9 Image Decoder");
    }

    return ESP_OK;
}

esp_err_t init_drive(const char *partition_label, const char drive_letter, int max_files, uint32_t checksum)
{
    return bsp_assets_init(partition_label, drive_letter, max_files, checksum);
}
