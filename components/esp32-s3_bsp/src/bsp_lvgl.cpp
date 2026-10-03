/**
 * @file bsp_lvgl.cpp
 * @brief LVGL v9 FreeRTOS Integration Port & Thread-Safe Mutex Lock Implementation
 *
 * @version 1.0.0
 * @attribution
 * - Architecture & Development: HUMIDYNE LABS / Humiditron
 * - AI Systems Co-Developer: Gemini (Google DeepMind)
 * - Graphics Library Basis: LVGL Community (https://lvgl.io)
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
#include "esp_heap_caps.h"
#include "bsp/bsp_display.h"
#include "bsp/bsp_lvgl.h"
#include "bsp/bsp.h"
#include "sdkconfig.h"

static const char *TAG = "bsp_lvgl";

static lv_display_t      *s_lv_display       = NULL;
static TaskHandle_t      s_lvgl_task_handle  = NULL;
static bool              s_lvgl_task_running = false;

static uint32_t s_flush_counter     = 0;
static bool     s_first_boot_flush  = true;

#define PARTIAL_REFRESH_LIMIT 20
#define LVGL_I1_PALETTE_SIZE  8

// Accumulator for batched partial redraw bounding boxes
static int16_t s_dirty_x1 = 32767, s_dirty_y1 = 32767;
static int16_t s_dirty_x2 = -1,    s_dirty_y2 = -1;

#if CONFIG_LV_USE_LOG
static void bsp_lvgl_log_cb(lv_log_level_t level, const char *buf)
{
    ESP_LOGI("LVGL_SYS", "%s", buf);
}
#endif

// Forward declaration of port task
static void bsp_lvgl_port_task(void *pvParameters);

bool bsp_lvgl_lock(void) {
    lv_lock();
    return true;
}

void bsp_lvgl_unlock(void) {
    lv_unlock();
}

bool bsp_lvgl_lock_isr(void) {
    return (lv_lock_isr() == LV_RESULT_OK);
}

static uint32_t lvgl_tick_get_cb(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000ULL);
}

static void lvgl_display_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    //ESP_LOGI(TAG, "LVGL flush callback: area=(%d,%d)-(%d,%d), px_map=%p",
    //         area->x1, area->y1, area->x2, area->y2, (void *)px_map);

    // Color Format Defense Guard: Ensure display format is LV_COLOR_FORMAT_I1
    lv_color_format_t cf = lv_display_get_color_format(disp);
    if (cf != LV_COLOR_FORMAT_I1) {
        ESP_LOG_LEVEL(ESP_LOG_WARN, TAG, "Unsupported color format 0x%02X; expected LV_COLOR_FORMAT_I1 (0x%02X)",
                       (unsigned)cf, (unsigned)LV_COLOR_FORMAT_I1);
        lv_display_flush_ready(disp);
        return;
    }

    // Warm-Boot Anti-Flicker Guard: If active screen has 0 child widgets, application UI is not yet built.
    // Drop EPD bit-blit to protect s_frame_buffer & SSD1681 RAM from being wiped by an empty white canvas.
    lv_obj_t *act_scr = lv_display_get_screen_active(disp);
    if (act_scr != NULL && lv_obj_get_child_count(act_scr) == 0) {
        lv_display_flush_ready(disp);
        return;
    }

    const uint8_t *src_buf  = px_map + LVGL_I1_PALETTE_SIZE;
    uint8_t       *dest_buf = bsp_display_get_buffer();

    uint16_t area_w     = (area->x2 - area->x1 + 1);
    uint16_t area_h     = (area->y2 - area->y1 + 1);
    uint32_t src_stride = lv_draw_buf_width_to_stride(area_w, LV_COLOR_FORMAT_I1);

    // Optimized Direct Monochrome Bit Blit (Direct 1-to-1 Copy to SSD1681 Framebuffer)
    if ((area->x1 % 8 == 0) && (area_w % 8 == 0) && (dest_buf != NULL)) {
        uint16_t bytes_per_line = area_w   / 8;
        uint16_t start_byte_x   = area->x1 / 8;

        for (uint16_t y = 0; y < area_h; y++) {
            uint16_t      dst_y      = area->y1 + y;
            uint32_t      dst_offset = (dst_y * (BSP_DISPLAY_WIDTH / 8)) + start_byte_x;
            const uint8_t *src       = src_buf + (y * src_stride);
            uint8_t       *dst       = dest_buf + dst_offset;

            // 32-Bit Word Vector Fast Path (Direct Copy)
            if (((uintptr_t)src % 4 == 0) && ((uintptr_t)dst % 4 == 0) && (bytes_per_line % 4 == 0)) {
                const uint32_t *src32 = (const uint32_t *)src;
                uint32_t       *dst32 = (uint32_t *)dst;
                uint16_t       words  = bytes_per_line / 4;

                for (uint16_t i = 0; i < words; i++) {
                    dst32[i] = src32[i]; // Direct 32-bit word copy (32 pixels in 1 CPU cycle)
                }
            } else {
                memcpy(dst, src, bytes_per_line);
            }
        }
    } else {
        // Pixel fallback for unaligned fractional bounding boxes
        for (uint16_t y = 0; y < area_h; y++) {
            const uint8_t *src_line = src_buf + (y * src_stride);
            uint16_t      dst_y     = area->y1 + y;

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
            s_flush_counter    = 0;
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
    //ESP_LOGI("TAG", "LVGL flush callback completed: Area (%d,%d)-(%d,%d), Flush Count %u",
    //         area->x1, area->y1, area->x2, area->y2, (unsigned)s_flush_counter);
}

esp_err_t bsp_lvgl_init(void)
{
    if (s_lv_display != NULL) {
        return ESP_OK;
    }

    esp_err_t ret = bsp_display_init();
    if (ret != ESP_OK) return ret;

    lv_init();
#if CONFIG_LV_USE_LOG
    lv_log_register_print_cb(bsp_lvgl_log_cb);
#endif
    lv_tick_set_cb(lvgl_tick_get_cb);

    s_lv_display = lv_display_create(BSP_DISPLAY_WIDTH, BSP_DISPLAY_HEIGHT);
    if (s_lv_display == NULL) {
        ESP_LOGE(TAG, "Failed to create LVGL display");
        return ESP_FAIL;
    }

    lv_display_set_color_format(s_lv_display, LV_COLOR_FORMAT_I1);

    // Allocate 64-byte aligned partial render buffer (40 lines) using stride math
    uint32_t buffer_lines = 40;
    uint32_t stride       = lv_draw_buf_width_to_stride(BSP_DISPLAY_WIDTH, LV_COLOR_FORMAT_I1);
    size_t   buf_size     = (stride * buffer_lines) + LVGL_I1_PALETTE_SIZE;
    uint8_t *buf1         = (uint8_t *)heap_caps_aligned_calloc(64, 1, buf_size, MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
    if (buf1 == NULL) {
        buf1 = (uint8_t *)heap_caps_aligned_calloc(64, 1, buf_size, MALLOC_CAP_8BIT);
    }
    assert(buf1 != NULL);

    // Explicitly initialize 1-bit monochrome palette (Index 0 = Black, Index 1 = White)
    lv_color32_t palette[2];
    palette[0] = lv_color32_make(0x00, 0x00, 0x00, 0xFF);
    palette[1] = lv_color32_make(0xFF, 0xFF, 0xFF, 0xFF);
    memcpy(buf1, palette, LVGL_I1_PALETTE_SIZE);

    // Initialize the pixel area of buf1 to clean white (1-bits)
    memset(buf1 + LVGL_I1_PALETTE_SIZE, 0xFF, buf_size - LVGL_I1_PALETTE_SIZE);

    lv_display_set_buffers(s_lv_display, buf1, NULL, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(s_lv_display, lvgl_display_flush_cb);

    // If waking from deep/light sleep, default first flush to fast partial update to avoid full screen flash
    bsp_init_mode_t rec_mode = bsp_get_recommended_init_mode();
    if (rec_mode == BSP_INIT_MODE_FAST) {
        s_first_boot_flush = false;
        ESP_LOGI(TAG, "Wake cycle detected (mode %d): LVGL first flush configured for fast partial update", (int)rec_mode);
    } else {
        s_first_boot_flush = true;
    }

    ESP_LOGI(TAG, "LVGL v9 port initialized in 1-bit monochrome mode");
    return ESP_OK;
}

void bsp_lvgl_set_first_flush_mode(bool full_refresh)
{
    s_first_boot_flush = full_refresh;
}

esp_err_t bsp_lvgl_start(int task_priority, int core_id)
{
    if (s_lvgl_task_handle != NULL) {
        ESP_LOGW(TAG, "LVGL task is already running");
        return ESP_OK;
    }

    esp_err_t ret = bsp_lvgl_init();
    if (ret != ESP_OK) return ret;

    s_lvgl_task_running = true;

    BaseType_t res;
    if (core_id >= 0 && core_id <= 1) {
        res = xTaskCreatePinnedToCore(
            bsp_lvgl_port_task,
            "bsp_lvgl_task",
            8192,
            NULL,
            task_priority > 0 ? task_priority : 5,
            &s_lvgl_task_handle,
            core_id
        );
    } else {
        res = xTaskCreate(
            bsp_lvgl_port_task,
            "bsp_lvgl_task",
            8192,
            NULL,
            task_priority > 0 ? task_priority : 5,
            &s_lvgl_task_handle
        );
    }

    if (res != pdPASS) {
        ESP_LOGE(TAG, "Failed to create LVGL FreeRTOS task");
        s_lvgl_task_running = false;
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "LVGL background task started (Priority %d, Core %d)",
             task_priority > 0 ? task_priority : 5, core_id);
    return ESP_OK;
}

esp_err_t bsp_lvgl_stop(void)
{
    if (s_lvgl_task_handle == NULL) {
        return ESP_OK;
    }

    s_lvgl_task_running = false;
    vTaskDelete(s_lvgl_task_handle);
    s_lvgl_task_handle = NULL;
    ESP_LOGI(TAG, "LVGL task stopped");
    return ESP_OK;
}

static void bsp_lvgl_port_task(void *pvParameters)
{
    ESP_LOGI(TAG, "LVGL port task active on Core %d", xPortGetCoreID());
    uint32_t loop_cnt = 0;
    while (s_lvgl_task_running) {
        uint32_t delay_ms = lv_timer_handler();

        if (loop_cnt % 100 == 0) {
            ESP_LOGI(TAG, "Port Task Heartbeat #%lu: lv_timer_handler delay = %lu ms", (unsigned long)loop_cnt, (unsigned long)delay_ms);
        }
        loop_cnt++;

        if (delay_ms < 5)  delay_ms = 5;
        if (delay_ms > 50) delay_ms = 50;
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
    vTaskDelete(NULL);
}
