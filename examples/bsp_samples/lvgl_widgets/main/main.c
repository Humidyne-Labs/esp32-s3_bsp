/**
 * @file main.c
 * @brief LVGL 9 UI Rendering Sample for 1-Bit Monochrome E-Paper Display
 *
 * @version 1.0.0
 * @attribution
 * - Architecture & Development: HUMIDYNE LABS / Humiditron
 * - AI Systems Co-Developer: Gemini (Google DeepMind)
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "bsp/bsp.h"

static const char *TAG = "sample_lvgl";

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing Humid1 OS LVGL 9 Sample...");
    ESP_ERROR_CHECK(bsp_init());

    ESP_LOGI(TAG, "Locking LVGL mutex and rendering widgets...");
    if (bsp_lvgl_lock(1000)) {
        lv_obj_t *scr = lv_screen_active();
        lv_obj_set_style_bg_color(scr, lv_color_white(), 0);

        lv_obj_t *title = lv_label_create(scr);
        lv_label_set_text(title, "HUMID1 OS");
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

        lv_obj_t *subtitle = lv_label_create(scr);
        lv_label_set_text(subtitle, "LVGL 9 UI Sample");
        lv_obj_align(subtitle, LV_ALIGN_CENTER, 0, 0);

        bsp_lvgl_unlock();
    }

    ESP_LOGI(TAG, "LVGL 9 Widget sample running.");
}
