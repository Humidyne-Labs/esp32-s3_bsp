/**
 * @file bsp_assets.c
 * @brief Zero-Copy Flash Assets & LVGL v9 Image Decoder Implementation
 * 
 * Performance Overview:
 *  - Maps asset binary files directly from SPI flash into CPU memory address space via MMU.
 *  - LVGL v9 image decoder streams images directly from MMU flash memory pointer
 *    without copying into RAM (Zero-Copy decoding for 1-bit monochrome bitmaps, icons, etc.).
 * 
 * @attribution
 * - Espressif Systems (esp_mmap_assets, esp_lv_fs)
 * - LVGL Community
 * - BSP Unification: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "esp_log.h"
#include "esp_mmap_assets.h"
#include "esp_lv_fs.h"
#include "lvgl.h"
#include "lvgl_private.h"
#include "bsp/bsp_assets.h"

static const char *TAG = "bsp_assets";

static mmap_assets_handle_t s_asset_handle = NULL;
static esp_lv_fs_handle_t   s_lv_fs_handle = NULL;
static int                  s_asset_count  = 0;
static char                 s_drive_letter = '\0';

/**
 * @brief Calculate the exact packed row stride in bytes for an image format
 */
static inline uint32_t get_packed_stride(uint32_t w, lv_color_format_t cf)
{
    uint32_t bpp = lv_color_format_get_bpp(cf);
    return (w * bpp + 7) / 8;
}

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
        ESP_LOGE(TAG, "[get_ptr] Assets not initialized or empty");
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

    ESP_LOGW(TAG, "[get_ptr] Asset '%s' not found in MMAP table", search_name);
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
    if (src_hdr->magic != LV_IMAGE_HEADER_MAGIC) {
        return LV_RESULT_INVALID;
    }

    *header = *src_hdr;

    // Ensure valid stride for 1-bit and packed formats
    if (header->stride == 0) {
        header->stride = get_packed_stride(header->w, (lv_color_format_t)header->cf);
    }

    return LV_RESULT_OK;
}

static lv_result_t mmap_decoder_open_cb(lv_image_decoder_t *decoder, lv_image_decoder_dsc_t *dsc)
{
    LV_UNUSED(decoder);
    if (dsc == NULL || dsc->src_type != LV_IMAGE_SRC_FILE) return LV_RESULT_INVALID;

    size_t total_size = 0;
    const uint8_t *flash_ptr = get_asset_flash_ptr((const char *)dsc->src, &total_size);
    if (flash_ptr == NULL || total_size <= sizeof(lv_image_header_t)) {
        return LV_RESULT_INVALID;
    }

    lv_draw_buf_t *draw_buf = (lv_draw_buf_t *)lv_malloc_zeroed(sizeof(lv_draw_buf_t));
    if (draw_buf == NULL) return LV_RESULT_INVALID;

    // Start offset after 12-byte header
    uint32_t header_offset = sizeof(lv_image_header_t);

    // If indexed format (I1, I2, I4, I8), skip the palette table to reach raw pixel bits
    if (LV_COLOR_FORMAT_IS_INDEXED(dsc->header.cf)) {
        header_offset += LV_COLOR_INDEXED_PALETTE_SIZE(dsc->header.cf) * sizeof(lv_color32_t);
    }

    const uint8_t *pixel_data = flash_ptr + header_offset;
    uint32_t       pixel_size = total_size - header_offset;

    // Calculate stride for any bpp (1, 2, 4, 8, 16, 24, 32)
    uint32_t bpp    = lv_color_format_get_bpp((lv_color_format_t)dsc->header.cf);
    uint32_t stride = (dsc->header.w * bpp + 7) / 8;

    lv_draw_buf_init(
        draw_buf,
        dsc->header.w,
        dsc->header.h,
        (lv_color_format_t)dsc->header.cf,
        stride,
        (void *)pixel_data,
        pixel_size
    );

    draw_buf->data           = (void *)pixel_data;
    draw_buf->unaligned_data = (void *)pixel_data;
    draw_buf->header.stride  = stride;
    dsc->header.stride       = stride;

    dsc->decoded       = draw_buf;
    dsc->user_data     = draw_buf;
    dsc->args.no_cache = true;

    return LV_RESULT_OK;
}

static void mmap_decoder_close_cb(lv_image_decoder_t *decoder, lv_image_decoder_dsc_t *dsc)
{
    LV_UNUSED(decoder);
    if (dsc && dsc->user_data) {
        lv_free(dsc->user_data);
        dsc->user_data = NULL;
        dsc->decoded   = NULL;
    }
}

static esp_err_t bsp_mmap_lvgl_decoder_init(void)
{
    lv_image_decoder_t *dec = lv_image_decoder_create();
    if (dec == NULL) {
        ESP_LOGE(TAG, "Failed to create LVGL image decoder");
        return ESP_ERR_NO_MEM;
    }

    lv_image_decoder_set_info_cb(dec,  mmap_decoder_info_cb);
    lv_image_decoder_set_open_cb(dec,  mmap_decoder_open_cb);
    lv_image_decoder_set_close_cb(dec, mmap_decoder_close_cb);

    ESP_LOGI(TAG, "Zero-copy MMAP 1-bit/mono image decoder registered for LVGL");
    return ESP_OK;
}

esp_err_t bsp_assets_init(const char *partition_label, const char drive_letter, int max_files, uint32_t checksum)
{
    if (partition_label == NULL) return ESP_ERR_INVALID_ARG;

    s_drive_letter = drive_letter;

    mmap_assets_config_t config = {
        .partition_label   = partition_label,
        .max_files         = max_files,
        .checksum          = checksum,
        .flags             = {
            .mmap_enable   = true,
            .app_bin_check = true,
            .use_fs        = false,
        },
    };

    esp_err_t ret = mmap_assets_new(&config, &s_asset_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount mmap assets partition '%s': %s", partition_label, esp_err_to_name(ret));
        return ret;
    }

    s_asset_count = mmap_assets_get_stored_files(s_asset_handle);
    if (s_asset_count <= 0) {
        s_asset_count = max_files;
    }

    /* Register LVGL File System Bridge */
    const fs_cfg_t fs_cfg = {
        .fs_letter = drive_letter,
        .fs_nums   = s_asset_count,
        .fs_assets = s_asset_handle,
    };
    ret = esp_lv_fs_desc_init(&fs_cfg, &s_lv_fs_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "esp_lv_fs_desc_init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Register Zero-Copy MMAP Decoder */
    ret = bsp_mmap_lvgl_decoder_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize zero-copy MMAP LVGL decoder: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Drive '%c:' mounted with %d assets (Zero-Copy MMAP enabled)", drive_letter, s_asset_count);
    return ESP_OK;
}

esp_err_t init_drive(const char *partition_label, const char drive_letter, int max_files, uint32_t checksum)
{
    return bsp_assets_init(partition_label, drive_letter, max_files, checksum);
}
