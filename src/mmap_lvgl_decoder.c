/**
 * @file mmap_lvgl_decoder.c
 * @brief Zero-Copy Decoder for LVGL v9 using esp_mmap_assets.
 * 
 * SPDX-License-Identifier: MIT
 */

#include "esp_log.h"
#include "lvgl_private.h"
#include "esp_lv_fs.h"
#include "bsp/mmap_lvgl_decoder.h"
#include <string.h>
#include <ctype.h>

static const char *TAG = "mmap_decoder";

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
        ESP_LOGE(TAG, "[get_ptr] Assets not initialized or empty!");
        return NULL;
    }

    const char *search_name = normalize_asset_path(path);
    //ESP_LOGI(TAG, "[get_ptr] Resolving '%s' -> '%s'", path, search_name);

    for (int i = 0; i < s_asset_count; i++) {
        const char *name = mmap_assets_get_name(s_asset_handle, i);
        if (!name) continue;

        const char *name_basename = strrchr(name, '/');
        name_basename = (name_basename != NULL) ? name_basename + 1 : name;

        if (strcmp(name, search_name) == 0 || strcmp(name_basename, search_name) == 0) {
            size_t size = (size_t)mmap_assets_get_size(s_asset_handle, i);
            const uint8_t *mem = (const uint8_t *)mmap_assets_get_mem(s_asset_handle, i);

            //ESP_LOGI(TAG, "[get_ptr] MATCH [%d]: '%s' @ %p (%u bytes)", i, name, mem, (unsigned int)size);
            if (out_size) *out_size = size;
            return mem;
        }
    }

    ESP_LOGW(TAG, "[get_ptr] Asset '%s' NOT FOUND", search_name);
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
        //ESP_LOGE(TAG, "[info_cb] Invalid magic 0x%02X in %s", src_hdr->magic, src_str);
        return LV_RESULT_INVALID;
    }

    *header = *src_hdr;

    // Ensure valid stride
    if (header->stride == 0) {
        header->stride = get_packed_stride(header->w, (lv_color_format_t)header->cf);
    }

    /*
    ESP_LOGI(TAG, "[info_cb] %s: %dx%d, cf=%d, stride=%d (packed=%u), total_size=%u",
             src_str, header->w, header->h, header->cf, header->stride,
             (unsigned int)get_packed_stride(header->w, (lv_color_format_t)header->cf),
             (unsigned int)size);
    */
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

    lv_draw_buf_t *draw_buf = lv_malloc_zeroed(sizeof(lv_draw_buf_t));
    if (draw_buf == NULL) return LV_RESULT_INVALID;

    // Start offset after 12-byte header
    uint32_t header_offset = sizeof(lv_image_header_t);

    // If indexed (I1, I2, I4, I8), skip the palette table to reach the raw pixel bits
    if (LV_COLOR_FORMAT_IS_INDEXED(dsc->header.cf)) {
        header_offset += LV_COLOR_INDEXED_PALETTE_SIZE(dsc->header.cf) * sizeof(lv_color32_t);
    }

    const uint8_t *pixel_data = flash_ptr + header_offset;
    uint32_t pixel_size = total_size - header_offset;

    // Correctly calculates stride for any bpp (1, 2, 4, 8, 16, 24, 32)
    uint32_t bpp = lv_color_format_get_bpp((lv_color_format_t)dsc->header.cf);
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

static esp_err_t mmap_lvgl_decoder_init(void)
{
    lv_image_decoder_t *dec = lv_image_decoder_create();
    if (dec == NULL) {
        ESP_LOGE(TAG, "Failed to create LVGL image decoder");
        return ESP_ERR_NO_MEM;
    }

    lv_image_decoder_set_info_cb(dec,  mmap_decoder_info_cb);
    lv_image_decoder_set_open_cb(dec,  mmap_decoder_open_cb);
    lv_image_decoder_set_close_cb(dec, mmap_decoder_close_cb);

    ESP_LOGI(TAG, "Zero-copy MMAP image decoder registered successfully");
    return ESP_OK;
}

esp_err_t init_drive(const char* partition_label, const char drive_letter, int max_files, uint32_t checksum) 
{
    s_drive_letter = drive_letter;

    mmap_assets_config_t asset_cfg = {};
    asset_cfg.partition_label      = partition_label;
    asset_cfg.max_files            = max_files;
    asset_cfg.checksum             = checksum;
    asset_cfg.flags.mmap_enable    = true;
    asset_cfg.flags.use_fs         = false;
    asset_cfg.flags.app_bin_check  = true;

    esp_err_t ret = mmap_assets_new(&asset_cfg, &s_asset_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to map assets partition (0x%x)", ret);
        return ret;
    }

    s_asset_count = mmap_assets_get_stored_files(s_asset_handle);
    if (s_asset_count <= 0) {
        s_asset_count = max_files;
    }

    fs_cfg_t fs_cfg  = {};
    fs_cfg.fs_letter = drive_letter;
    fs_cfg.fs_nums   = s_asset_count;
    fs_cfg.fs_assets = s_asset_handle;

    ret = esp_lv_fs_desc_init(&fs_cfg, &s_lv_fs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register LVGL FS (0x%x)", ret);
        return ret;
    }

    ret = mmap_lvgl_decoder_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init decoder (0x%x)", ret);
        return ret;
    }

    ESP_LOGI(TAG, "Drive '%c:' initialized with %d assets", drive_letter, s_asset_count);
    return ESP_OK;
}