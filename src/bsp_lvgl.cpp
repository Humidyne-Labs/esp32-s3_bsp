/**
 * @file bsp_lvgl.cpp
 * @brief lvgl port
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics (https://www.waveshare.com)
 * - Microcontroller: Espressif Systems ESP32-S3 (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include <algorithm>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "bsp/bsp_display.h"
#include "bsp/bsp_touch.h"
#include "bsp/bsp_lvgl.h"
#include "sdkconfig.h"

#ifndef CONFIG_BSP_ENABLE_TOUCH
    #define UNUSED_FUNC __attribute__((unused))
#else
    #define UNUSED_FUNC
#endif

static const char *TAG                          = "bsp_lvgl";
static lv_display_t *s_lv_display               = NULL;
UNUSED_FUNC static lv_indev_t *s_lv_touch_indev = NULL;
static SemaphoreHandle_t s_lvgl_mutex           = NULL;

static uint32_t s_flush_counter = 0;
static bool s_first_boot_flush  = true;
#define PARTIAL_REFRESH_LIMIT 20
#define LVGL_I1_PALETTE_SIZE  8

// Accumulator for batched partial redraw bounding boxes
static int16_t s_dirty_x1 = 32767, s_dirty_y1 = 32767;
static int16_t s_dirty_x2 = -1,    s_dirty_y2 = -1;

void bsp_lvgl_lock(void) {
    if (s_lvgl_mutex) {
        xSemaphoreTake(s_lvgl_mutex, portMAX_DELAY);
    }
}

void bsp_lvgl_unlock(void) {
    if (s_lvgl_mutex) {
        xSemaphoreGive(s_lvgl_mutex);
    }
}

static uint32_t lvgl_tick_get_cb(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000ULL);
}

static void lvgl_display_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    const uint8_t *src_buf = px_map + LVGL_I1_PALETTE_SIZE;
    uint8_t *dest_buf = bsp_display_get_buffer();

    uint16_t area_w = (area->x2 - area->x1 + 1);
    uint16_t area_h = (area->y2 - area->y1 + 1);
    uint32_t src_stride = lv_draw_buf_width_to_stride(area_w, LV_COLOR_FORMAT_I1);

    // --- PX_MAP INSPECTION BLOCK ---
    /*
    if (px_map != NULL) {
        uint32_t total_src_bytes = area_h * src_stride;
        uint32_t set_bits_count = 0;
        
        // Count active bits to see if it's completely empty or full
        for (uint32_t i = 0; i < total_src_bytes; i++) {
            // Built-in compiler function to quickly count set bits (population count)
            set_bits_count += __builtin_popcount(src_buf[i]); 
        }

        uint32_t total_possible_bits = total_src_bytes * 8;
        ESP_LOGI(TAG, "[INSPECT] Total buffer size: %lu bytes (%lu bits)", total_src_bytes, total_possible_bits);
        ESP_LOGI(TAG, "[INSPECT] Set bits: %lu / %lu (%.1f%% filled)", 
                 set_bits_count, total_possible_bits, ((float)set_bits_count / total_possible_bits) * 100.0);
        
        // Safe hex dump of just the first line to check structure without flooding logs
        if (src_stride > 0) {
            char hex_dump[64] = {0};
            uint32_t dump_len = (src_stride > 16) ? 16 : src_stride;
            for(uint32_t i = 0; i < dump_len; i++) {
                sprintf(&hex_dump[i * 3], "%02X ", src_buf[i]);
            }
            ESP_LOGI(TAG, "[INSPECT] First line start bytes: %s", hex_dump);
        }
    } else {
        ESP_LOGE(TAG, "[INSPECT] px_map is NULL!");
    }
    */
    // --- END INSPECTION BLOCK ---

    // Optimized Direct Monochrome Bit Blit
    // If area is byte-aligned (multiple of 8), perform fast-path copy
    if ((area->x1 % 8 == 0) && (area_w % 8 == 0) && (dest_buf != NULL)) {
        uint16_t bytes_per_line = area_w / 8;
        uint16_t start_byte_x = area->x1 / 8;

        for (uint16_t y = 0; y < area_h; y++) {
            uint16_t dst_y = area->y1 + y;
            uint32_t dst_offset = (dst_y * (BSP_DISPLAY_WIDTH / 8)) + start_byte_x;
            const uint8_t *src_line = src_buf + (y * src_stride);
            memcpy(&dest_buf[dst_offset], src_line, bytes_per_line);
        }
    } else {
        // Pixel fallback for unaligned fractional bounding boxes
        for (uint16_t y = 0; y < area_h; y++) {
            const uint8_t *src_line = src_buf + (y * src_stride);
            uint16_t dst_y = area->y1 + y;

            for (uint16_t x = 0; x < area_w; x++) {
                uint8_t bit_val = (src_line[x >> 3] >> (7 - (x & 0x07))) & 0x01;
                bsp_display_draw_pixel(
                    area->x1 + x,
                    dst_y,
                    (bit_val != 0) ? BSP_DISPLAY_COLOR_WHITE : BSP_DISPLAY_COLOR_BLACK
                );
            }
        }
    }

    // Accumulate bounding box
    s_dirty_x1 = std::min<int16_t>(s_dirty_x1, area->x1);
    s_dirty_y1 = std::min<int16_t>(s_dirty_y1, area->y1);
    s_dirty_x2 = std::max<int16_t>(s_dirty_x2, area->x2);
    s_dirty_y2 = std::max<int16_t>(s_dirty_y2, area->y2);

    // When all batched redraw areas are rendered, trigger hardware update
    if (lv_display_flush_is_last(disp)) {
        if (s_first_boot_flush) {
            bsp_display_flush(); // Full OTP update on boot
            s_first_boot_flush = false;
            s_flush_counter = 0;
        } else {
            s_flush_counter++;
            if (s_flush_counter >= PARTIAL_REFRESH_LIMIT) {
                bsp_display_flush(); // Full refresh to neutralize charge build-up
                s_flush_counter = 0;
            } else if (s_dirty_x2 >= 0 && s_dirty_y2 >= 0) {
                bsp_display_flush_partial_area(s_dirty_x1, s_dirty_y1, s_dirty_x2, s_dirty_y2);
            }
        }

        // Reset bounding box
        s_dirty_x1 = 32767; s_dirty_y1 = 32767;
        s_dirty_x2 = -1;    s_dirty_y2 = -1;
    }

    lv_display_flush_ready(disp);
}

UNUSED_FUNC static void lvgl_touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    uint16_t touch_x = 0;
    uint16_t touch_y = 0;

    bool touched = bsp_touch_read(&touch_x, &touch_y);
    if (touched) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = touch_x;
        data->point.y = touch_y;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

esp_err_t bsp_lvgl_init(void)
{
    if (s_lvgl_mutex == NULL) {
        s_lvgl_mutex = xSemaphoreCreateMutex();
        assert(s_lvgl_mutex != NULL);
    }

    esp_err_t ret = bsp_display_init();
    if (ret != ESP_OK) return ret;

#if CONFIG_BSP_ENABLE_TOUCH
    ret = bsp_touch_init();
    if (ret != ESP_OK) return ret;
#endif

    lv_init();
    lv_tick_set_cb(lvgl_tick_get_cb);

    s_lv_display = lv_display_create(BSP_DISPLAY_WIDTH, BSP_DISPLAY_HEIGHT);
    if (s_lv_display == NULL) {
        ESP_LOGE(TAG, "Failed to create LVGL display");
        return ESP_FAIL;
    }

    lv_display_set_color_format(s_lv_display, LV_COLOR_FORMAT_I1);

    // Allocate partial render buffer (e.g. 40 lines)
    uint32_t buffer_lines = 40;
    size_t buf_size = ((BSP_DISPLAY_WIDTH + 7) / 8) * buffer_lines + LVGL_I1_PALETTE_SIZE;
    uint8_t *buf1 = (uint8_t *)heap_caps_malloc(buf_size, MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
    if (buf1 == NULL) {
        buf1 = (uint8_t *)malloc(buf_size);
    }
    assert(buf1 != NULL);

    lv_display_set_buffers(s_lv_display, buf1, NULL, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(s_lv_display, lvgl_display_flush_cb);

#if CONFIG_BSP_ENABLE_TOUCH
    s_lv_touch_indev = lv_indev_create();
    if (s_lv_touch_indev != NULL) {
        lv_indev_set_type(s_lv_touch_indev, LV_INDEV_TYPE_POINTER);
        lv_indev_set_read_cb(s_lv_touch_indev, lvgl_touch_read_cb);
    }
#endif

    ESP_LOGI(TAG, "LVGL v9 port initialized in 1-bit monochrome mode");
    return ESP_OK;
}

void bsp_lvgl_port_task(void *pvParameters)
{
    ESP_LOGI(TAG, "LVGL task running");
    while (1) {
        bsp_lvgl_lock();
        uint32_t delay_ms = lv_timer_handler();
        bsp_lvgl_unlock();

        if (delay_ms < 5)  delay_ms = 5;
        if (delay_ms > 50) delay_ms = 50;
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}