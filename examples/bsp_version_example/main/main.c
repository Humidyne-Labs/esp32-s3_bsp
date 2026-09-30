/**
 * @file main.c
 * @brief Reference example for bsp_version.h
 * 
 * Demonstrates how to retrieve and log the BSP semantic versioning
 * information using the provided BSP versioning API.
 */

#include <stdio.h>
#include <inttypes.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp/bsp_version.h"

static const char *TAG = "bsp_version_example";

/**
 * @brief Main application entry point
 */
void app_main(void)
{
    ESP_LOGI(TAG, "Initializing BSP Version Verification...");

    /* 
     * 1. Retrieve version as a string (e.g., "1.4.0")
     */
    const char *version_str = bsp_get_version();
    ESP_LOGI(TAG, "BSP Version String: %s", version_str);

    /* 
     * 2. Retrieve version as an encoded integer (MAJOR << 16 | MINOR << 8 | PATCH)
     */
    uint32_t version_val = bsp_get_version_val();
    
    uint8_t major = (uint8_t)((version_val >> 16) & 0xFF);
    uint8_t minor = (uint8_t)((version_val >> 8) & 0xFF);
    uint8_t patch = (uint8_t)(version_val & 0xFF);

    ESP_LOGI(TAG, "BSP Version Integer: 0x%08" PRIX32, version_val);
    ESP_LOGI(TAG, "Parsed Version: %u.%u.%u", major, minor, patch);

    /* 
     * 3. Verify against compile-time macros
     */
    if (version_val == BSP_CURRENT_VERSION) {
        ESP_LOGI(TAG, "Version check passed: Runtime matches compile-time constant.");
    } else {
        ESP_LOGE(TAG, "Version mismatch detected!");
    }

    ESP_LOGI(TAG, "Example complete. Entering idle state.");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}