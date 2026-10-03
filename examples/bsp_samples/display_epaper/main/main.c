/**
 * @file main.c
 * @brief SSD1681 1.54" 200x200 E-Paper Display Text & Graphics Sample
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

static const char *TAG = "sample_display";

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing Humid1 OS BSP Display Sample...");
    ESP_ERROR_CHECK(bsp_init());

    ESP_LOGI(TAG, "Clearing display framebuffer...");
    bsp_display_clear();

    ESP_LOGI(TAG, "Drawing border and diagonal cross...");
    for (uint16_t i = 0; i < 200; i++) {
        bsp_display_draw_pixel(i, 0, BSP_DISPLAY_COLOR_BLACK);
        bsp_display_draw_pixel(i, 199, BSP_DISPLAY_COLOR_BLACK);
        bsp_display_draw_pixel(0, i, BSP_DISPLAY_COLOR_BLACK);
        bsp_display_draw_pixel(199, i, BSP_DISPLAY_COLOR_BLACK);

        bsp_display_draw_pixel(i, i, BSP_DISPLAY_COLOR_BLACK);
        bsp_display_draw_pixel(i, 199 - i, BSP_DISPLAY_COLOR_BLACK);
    }

    ESP_LOGI(TAG, "Flushing full screen update...");
    bsp_display_flush();
    bsp_display_wait_busy(5000);

    ESP_LOGI(TAG, "Display sample completed.");
}
