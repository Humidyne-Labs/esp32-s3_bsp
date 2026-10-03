/**
 * @file main.c
 * @brief Standalone BSP Initialization and Hardware Identification Example
 *
 * Target Hardware:
 *  - Microcontroller: ESP32-S3-PICO-1-N8R8
 *  - Board: Waveshare ESP32-S3 ePaper 1.54 V2
 *
 * Demonstrates modular board initialization, device hardware identification,
 * and key generation using the master BSP driver.
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "bsp/bsp.h"

static const char *TAG = "bsp_example";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting BSP Initialization Example...");

    // Configure BSP hardware options
    bsp_config_t bsp_cfg = BSP_CONFIG_DEFAULT();
    bsp_cfg.init_sdcard = false; // MicroSD card not mounted for this test
    bsp_cfg.start_lvgl  = false; // Disable LVGL rendering task for pure board init demo

    // Initialize board peripherals with custom configuration
    ESP_ERROR_CHECK(bsp_board_init_with_config(&bsp_cfg));
    ESP_LOGI(TAG, "Board peripherals initialized successfully.");

    // Query unique hardware device ID derived from system MAC
    char device_id[32] = {0};
    ESP_ERROR_CHECK(bsp_get_device_id(device_id, sizeof(device_id)));
    ESP_LOGI(TAG, "Device Hardware ID: %s", device_id);

    // Query human-readable device name
    char device_name[32] = {0};
    ESP_ERROR_CHECK(bsp_get_device_name(device_name, sizeof(device_name)));
    ESP_LOGI(TAG, "Device Name: %s", device_name);

    // Generate an unambiguous random proof-of-possession key
    char claim_key[16] = {0};
    ESP_ERROR_CHECK(bsp_generate_unambiguous_key(claim_key, 8, NULL));
    ESP_LOGI(TAG, "Generated Claim Key (8 chars): %s", claim_key);

    ESP_LOGI(TAG, "BSP Example Execution Finished. Hardware Ready.");
}