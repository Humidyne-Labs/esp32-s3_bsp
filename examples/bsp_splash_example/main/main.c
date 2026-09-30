/**
 * @file main.c
 * @brief Standalone Reference Example for System Splash & Audio Chime Callbacks
 *
 * Demonstrates how to register, inspect, trigger, and unregister UI splash
 * screens and audio chime notification events using the BSP Splash Framework.
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "bsp/bsp_splash.h"

static const char *TAG = "splash_example";

static void on_splash_event(bsp_splash_type_t type, void *user_data)
{
    const char *context = (const char *)user_data;
    ESP_LOGI(TAG, "[UI] Splash Callback Triggered -> Type: %d, Context: %s",
             (int)type, context ? context : "N/A");
}

static void on_chime_event(bsp_chime_type_t type, void *user_data)
{
    const char *context = (const char *)user_data;
    ESP_LOGI(TAG, "[AUDIO] Chime Callback Triggered -> Type: %d, Context: %s",
             (int)type, context ? context : "N/A");
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting BSP Splash & Chime Framework Example...");

    // 1. Register Splash Screen Callbacks
    ESP_ERROR_CHECK(bsp_register_splash_cb(BSP_SPLASH_BOOT, on_splash_event, (void *)"Cold Boot UI"));
    ESP_ERROR_CHECK(bsp_register_splash_cb(BSP_SPLASH_SLEEP, on_splash_event, (void *)"Pre-Sleep UI"));

    // 2. Register Audio Chime Callbacks
    ESP_ERROR_CHECK(bsp_register_chime_cb(BSP_CHIME_BOOT, on_chime_event, (void *)"Boot Melody"));
    ESP_ERROR_CHECK(bsp_register_chime_cb(BSP_CHIME_NOTIFY, on_chime_event, (void *)"Notification Chirp"));

    // 3. Verify Registration Status
    if (bsp_has_splash_cb(BSP_SPLASH_BOOT)) {
        ESP_LOGI(TAG, "Verified: BSP_SPLASH_BOOT callback is registered.");
    }
    if (bsp_has_chime_cb(BSP_CHIME_NOTIFY)) {
        ESP_LOGI(TAG, "Verified: BSP_CHIME_NOTIFY callback is registered.");
    }

    // 4. Trigger Events
    ESP_LOGI(TAG, "Triggering Cold Boot UI Splash...");
    ESP_ERROR_CHECK(bsp_trigger_splash(BSP_SPLASH_BOOT));

    ESP_LOGI(TAG, "Triggering Boot Chime...");
    ESP_ERROR_CHECK(bsp_trigger_chime(BSP_CHIME_BOOT));

    ESP_LOGI(TAG, "Triggering Notification Chime...");
    ESP_ERROR_CHECK(bsp_trigger_chime(BSP_CHIME_NOTIFY));

    // 5. Test Handling for Unregistered Events
    esp_err_t err = bsp_trigger_splash(BSP_SPLASH_WAKE);
    if (err == ESP_ERR_NOT_FOUND) {
        ESP_LOGI(TAG, "BSP_SPLASH_WAKE correctly reported as not registered.");
    }

    // 6. Trigger Pre-Sleep Splash
    ESP_LOGI(TAG, "Triggering Sleep UI Splash...");
    ESP_ERROR_CHECK(bsp_trigger_splash(BSP_SPLASH_SLEEP));

    // 7. Cleanup & Unregister Callbacks
    ESP_ERROR_CHECK(bsp_unregister_splash_cb(BSP_SPLASH_BOOT));
    ESP_ERROR_CHECK(bsp_unregister_splash_cb(BSP_SPLASH_SLEEP));
    ESP_ERROR_CHECK(bsp_unregister_chime_cb(BSP_CHIME_BOOT));
    ESP_ERROR_CHECK(bsp_unregister_chime_cb(BSP_CHIME_NOTIFY));

    // 8. Confirm Unregistration
    if (!bsp_has_splash_cb(BSP_SPLASH_BOOT)) {
        ESP_LOGI(TAG, "BSP_SPLASH_BOOT successfully unregistered.");
    }

    ESP_LOGI(TAG, "BSP Splash & Chime Framework Example Finished Successfully.");
}