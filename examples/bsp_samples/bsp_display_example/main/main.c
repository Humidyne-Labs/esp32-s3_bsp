/**
 * @file main.c
 * @brief SSD1681 1.54" Monochrome e-Paper Display Example
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "bsp/pinout.h"
#include "bsp/bsp_display.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing SSD1681 e-Paper Display...");
    ESP_ERROR_CHECK(bsp_display_init());

    // Clear buffer to all white
    ESP_LOGI(TAG, "Clearing screen buffer...");
    bsp_display_clear();

    // Draw frame boundary and diagonal pattern
    ESP_LOGI(TAG, "Drawing pattern on framebuffer...");
    for (uint16_t i = 0; i < 200; i++) {
        bsp_display_draw_pixel(i, 0, BSP_DISPLAY_COLOR_BLACK);
        bsp_display_draw_pixel(i, 199, BSP_DISPLAY_COLOR_BLACK);
        bsp_display_draw_pixel(0, i, BSP_DISPLAY_COLOR_BLACK);
        bsp_display_draw_pixel(199, i, BSP_DISPLAY_COLOR_BLACK);

        bsp_display_draw_pixel(i, i, BSP_DISPLAY_COLOR_BLACK);
        bsp_display_draw_pixel(i, 199 - i, BSP_DISPLAY_COLOR_BLACK);
    }

    // Flush full display buffer to panel
    ESP_LOGI(TAG, "Flushing full screen...");
    bsp_display_flush();
    ESP_ERROR_CHECK(bsp_display_wait_busy(5000));

    vTaskDelay(pdMS_TO_TICKS(1000));

    // Draw a rectangle in center for partial update test
    ESP_LOGI(TAG, "Drawing partial update region...");
    for (uint16_t y = 80; y <= 120; y++) {
        for (uint16_t x = 80; x <= 120; x++) {
            bsp_display_draw_pixel(x, y, BSP_DISPLAY_COLOR_BLACK);
        }
    }

    // Perform partial refresh
    ESP_LOGI(TAG, "Flushing partial area...");
    bsp_display_flush_partial_area(80, 80, 120, 120);
    ESP_ERROR_CHECK(bsp_display_wait_busy(2000));

    // Put panel into sleep mode
    ESP_LOGI(TAG, "Putting display to sleep...");
    ESP_ERROR_CHECK(bsp_display_sleep());

    ESP_LOGI(TAG, "Display example completed.");
}