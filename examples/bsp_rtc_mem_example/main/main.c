/**
 * @file main.c
 * @brief Reference example for ESP32-S3 RTC Slow Memory Persistent Storage
 * 
 * Demonstrates initialization, state tracking, scratchpad read/write, 
 * and display frame buffer persistence using the RTC Slow SRAM wrapper.
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "bsp/bsp_rtc_mem.h"

static const char *TAG = "rtc_mem_demo";

void app_main(void)
{
    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "      BSP RTC Slow Memory Verification Tool       ");
    ESP_LOGI(TAG, "==================================================");

    // 1. Initialize the RTC Memory State
    ESP_LOGI(TAG, "Initializing RTC Memory...");
    ESP_ERROR_CHECK(bsp_rtc_mem_init());

    // 2. Retrieve and display the persistent boot counter
    uint32_t boot_count = bsp_rtc_mem_get_boot_count();
    ESP_LOGI(TAG, "Current System Boot Count: %lu", (unsigned long)boot_count);

    // 3. Verify Scratchpad Read/Write Operations
    const char *test_message = "HumidOS_RTC_SRAM_OK";
    uint8_t read_buffer[32] = {0};

    ESP_LOGI(TAG, "Writing to scratchpad: '%s'", test_message);
    ESP_ERROR_CHECK(bsp_rtc_mem_write_scratchpad((const uint8_t *)test_message, strlen(test_message) + 1));

    ESP_LOGI(TAG, "Reading back from scratchpad...");
    ESP_ERROR_CHECK(bsp_rtc_mem_read_scratchpad(read_buffer, sizeof(read_buffer)));
    ESP_LOGI(TAG, "Scratchpad content read: '%s'", (char *)read_buffer);

    // 4. Verify 5000-byte EPD Frame Buffer Persistence
    uint8_t tx_frame[5000];
    uint8_t rx_frame[5000];

    for (int i = 0; i < 5000; i++) {
        tx_frame[i] = (uint8_t)(i & 0xFF);
    }

    ESP_LOGI(TAG, "Saving 5000-byte frame to RTC Slow Memory...");
    ESP_ERROR_CHECK(bsp_rtc_mem_save_display_frame(tx_frame, 5000));

    if (bsp_rtc_mem_has_display_frame()) {
        ESP_LOGI(TAG, "RTC Memory reports valid display frame exists.");
    }

    ESP_LOGI(TAG, "Loading 5000-byte frame back from RTC Slow Memory...");
    ESP_ERROR_CHECK(bsp_rtc_mem_load_display_frame(rx_frame, 5000));

    // 5. Configure and Verify Next Initialization Mode
    bsp_init_mode_t target_mode = BSP_INIT_MODE_FAST;
    ESP_LOGI(TAG, "Setting next init mode to: %d (FAST)", (int)target_mode);
    bsp_rtc_mem_set_next_init_mode(target_mode);

    // 6. Inspect direct state structure pointer
    bsp_rtc_state_t *state = bsp_rtc_mem_get_state();
    if (state != NULL) {
        ESP_LOGI(TAG, "RTC State Struct Diagnostics:");
        ESP_LOGI(TAG, "  Magic Token:       0x%08lX", (unsigned long)state->magic);
        ESP_LOGI(TAG, "  Boot Count:        %lu", (unsigned long)state->boot_count);
        ESP_LOGI(TAG, "  Next Init Mode:    %u", state->next_init_mode);
    }

    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "  RTC Memory Demo Completed Successfully. Idling. ");
    ESP_LOGI(TAG, "==================================================");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}