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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "bsp/bsp_display.h"
#include "bsp/bsp_touch.h"
#include "bsp/bsp_lvgl.h"
#include "sdkconfig.h"

#ifdef CONFIG_BSP_ENABLE_TOUCH
    // If CONFIG_BSP_ENABLE_TOUCH is defined, apply the unused attribute to suppress warnings
    #define UNUSED_FUNC __attribute__((unused))
#else
    // If CONFIG_BSP_ENABLE_TOUCH is not defined, the macro evaluates to nothing
    #define UNUSED_FUNC
#endif

static const char *TAG = "bsp_lvgl";
static lv_display_t *s_lv_display = NULL;
UNUSED_FUNC static lv_indev_t *s_lv_touch_indev = NULL;
static SemaphoreHandle_t s_lvgl_mutex = NULL;

static uint32_t s_flush_counter = 0;
static bool s_first_boot_flush = true;
#define PARTIAL_REFRESH_LIMIT 20
#define LVGL_I1_PALETTE_SIZE 8

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

static void lvgl_display_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    const uint8_t *buf = px_map + LVGL_I1_PALETTE_SIZE;

    uint16_t width = (area->x2 - area->x1 + 1);
    uint16_t height = (area->y2 - area->y1 + 1);
    uint32_t stride = lv_draw_buf_width_to_stride(width, LV_COLOR_FORMAT_I1);

    // 1. Copy LVGL 1-bit rendered pixels into the display driver's frame buffer
    for (int y = 0; y < height; y++) {
        const uint8_t *line_src = buf + (y * stride);
        for (int x = 0; x < width; x++) {
            uint8_t bit_val = (line_src[x >> 3] >> (7 - (x & 0x07))) & 0x01;

            bsp_display_draw_pixel(
                area->x1 + x, 
                area->y1 + y, 
                (bit_val != 0) ? BSP_DISPLAY_COLOR_WHITE : BSP_DISPLAY_COLOR_BLACK
            );
        }
    }

    // 2. Expand dirty union box across all batched invalidation areas
    if (area->x1 < s_dirty_x1) s_dirty_x1 = area->x1;
    if (area->y1 < s_dirty_y1) s_dirty_y1 = area->y1;
    if (area->x2 > s_dirty_x2) s_dirty_x2 = area->x2;
    if (area->y2 > s_dirty_y2) s_dirty_y2 = area->y2;

    bool is_last = lv_display_flush_is_last(disp);

    // 3. Only trigger the hardware refresh when ALL areas in the frame are drawn
    if (is_last) {
        if (s_first_boot_flush) {
            // First frame must be a full refresh to clear physical particle state
            bsp_display_flush();
            s_first_boot_flush = false;
            s_flush_counter = 0;
        } else {
            s_flush_counter++;
            if (s_flush_counter >= PARTIAL_REFRESH_LIMIT) {
                bsp_display_flush();
                s_flush_counter = 0;
            } else if (s_dirty_x2 >= 0 && s_dirty_y2 >= 0) {
                // Refresh the combined bounding box covering all updated labels
                bsp_display_flush_partial_area(s_dirty_x1, s_dirty_y1, s_dirty_x2, s_dirty_y2);
            }
        }

        // Reset the dirty bounding box for the next frame
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

    s_lv_display = lv_display_create(BSP_DISPLAY_WIDTH, BSP_DISPLAY_HEIGHT);
    if (s_lv_display == NULL) {
        ESP_LOGE(TAG, "Failed to create LVGL display");
        return ESP_FAIL;
    }

    lv_display_set_color_format(s_lv_display, LV_COLOR_FORMAT_I1);

    uint32_t buffer_lines = 40;
    size_t buf_size = ((BSP_DISPLAY_WIDTH + 7) / 8) * buffer_lines + LVGL_I1_PALETTE_SIZE;
    uint8_t *buf1 = (uint8_t *)malloc(buf_size);
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
    ESP_LOGI(TAG, "LVGL task started");
    while (1) {
        bsp_lvgl_lock();
        lv_tick_inc(10);
        lv_timer_handler();
        bsp_lvgl_unlock();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}