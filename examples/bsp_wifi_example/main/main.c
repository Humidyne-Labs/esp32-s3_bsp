/**
 * @file main.c
 * @brief Wi-Fi Station Manager Reference Example
 * 
 * Demonstrates initialization, credential management, and connection
 * using the bsp_wifi.h interface.
 */

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "bsp/bsp_wifi.h"

static const char *TAG = "wifi_example";

void app_main(void)
{
    // 1. Initialize NVS (Required for credential storage)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Initialize Wi-Fi Subsystem
    ESP_ERROR_CHECK(bsp_wifi_init());

    // 3. Save credentials
    ESP_LOGI(TAG, "Saving credentials to NVS...");
    ESP_ERROR_CHECK(bsp_wifi_save_credentials("MySSID", "MyPassword"));

    // 4. Attempt connection from NVS
    ESP_LOGI(TAG, "Attempting connection from NVS...");
    ret = bsp_wifi_connect_from_nvs(10000);

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Wi-Fi Connected!");

        // 5. Retrieve and display network info
        int rssi = 0;
        char ip_str[16] = {0};

        if (bsp_wifi_get_rssi(&rssi) == ESP_OK) {
            ESP_LOGI(TAG, "Signal Strength: %d dBm", rssi);
        }

        if (bsp_wifi_get_ip_str(ip_str, sizeof(ip_str)) == ESP_OK) {
            ESP_LOGI(TAG, "Local IP: %s", ip_str);
        }
        
        if (bsp_wifi_is_connected()) {
            ESP_LOGI(TAG, "Verification: Wi-Fi is confirmed connected.");
        }
    } else {
        ESP_LOGE(TAG, "Failed to connect: %s", esp_err_to_name(ret));
    }

    // 6. Cleanup
    ESP_LOGI(TAG, "Disconnecting...");
    ESP_ERROR_CHECK(bsp_wifi_disconnect());

    ESP_LOGI(TAG, "Example complete. Entering idle state.");
    vTaskDelete(NULL);
}