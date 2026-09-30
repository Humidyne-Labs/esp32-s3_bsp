/**
 * @file main.c
 * @brief Standalone Reference Example for BLE GATT Wi-Fi Provisioning
 * 
 * This example demonstrates how to initialize the BLE provisioning service,
 * register status callbacks, generate a standard QR code payload, and manage
 * the provisioning lifecycle.
 * 
 * Target Board: Waveshare ESP32-S3 ePaper 1.54 V2
 * MCU: ESP32-S3-PICO-1-N8R8
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "bsp/bsp_prov.h"

static const char *TAG = "prov_example";

/**
 * @brief Provisioning Event Callback
 */
static void provisioning_event_callback(bsp_prov_event_t event, void *event_data, void *user_data)
{
    /* Prevent unused parameter warnings */
    (void)event_data;
    (void)user_data;

    switch (event) {
        case BSP_PROV_EVENT_STARTED:
            ESP_LOGI(TAG, "Provisioning Event: BLE Advertising Started");
            break;
        case BSP_PROV_EVENT_CRED_RECEIVED:
            ESP_LOGI(TAG, "Provisioning Event: Wi-Fi Credentials Received");
            break;
        case BSP_PROV_EVENT_CRED_SUCCESS:
            ESP_LOGI(TAG, "Provisioning Event: Station Connected Successfully!");
            break;
        case BSP_PROV_EVENT_CRED_FAILED:
            ESP_LOGE(TAG, "Provisioning Event: Station Connection Failed");
            break;
        case BSP_PROV_EVENT_FINISHED:
            ESP_LOGI(TAG, "Provisioning Event: Service Finished & BLE De-initialized");
            break;
        default:
            break;
    }
}

void app_main(void)
{
    // 1. Initialize Non-Volatile Storage (NVS) - Required for Wi-Fi/BLE configurations
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Initialize Default Event Loop - Required for system and provisioning events
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(TAG, "=================================================");
    ESP_LOGI(TAG, "Starting BLE Wi-Fi Provisioning Service...");
    ESP_LOGI(TAG, "=================================================");

    // Define Proof-of-Possession (PoP) and custom service name
    const char *pop = "ESP32S3_PoP";
    const char *service_name = "PROV_WAVESHARE";

    // 3. Start BLE Provisioning Service
    ESP_ERROR_CHECK(bsp_prov_start(service_name, pop, provisioning_event_callback, NULL));

    // 4. Generate and Print QR Code Payload for Mobile App Pairing
    char qr_payload[128] = {0};
    ESP_ERROR_CHECK(bsp_prov_get_qr_payload(pop, qr_payload, sizeof(qr_payload)));
    
    ESP_LOGI(TAG, "-------------------------------------------------");
    ESP_LOGI(TAG, "QR Code Payload Generated:");
    ESP_LOGI(TAG, "%s", qr_payload);
    ESP_LOGI(TAG, "-------------------------------------------------");

    // 5. Monitor Provisioning Status with a 120-second timeout
    int timeout_sec = 120;
    while (bsp_prov_is_running() && timeout_sec > 0) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        timeout_sec--;
        
        if (timeout_sec % 10 == 0) {
            ESP_LOGI(TAG, "Provisioning active... Timeout in %d seconds", timeout_sec);
        }
    }

    // 6. Stop Provisioning if timeout is reached without completion
    if (bsp_prov_is_running()) {
        ESP_LOGW(TAG, "Provisioning timeout reached. Stopping service...");
        bsp_prov_stop();
    }

    ESP_LOGI(TAG, "Provisioning session ended. Entering idle loop.");
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}