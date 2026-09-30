/**
 * @file main.c
 * @brief ThingsBoard Telemetry and RPC Reference Example
 * 
 * Demonstrates initialization of the ThingsBoard client, 
 * periodic telemetry reporting, and RPC command handling.
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "bsp/bsp_tb.h"

static const char *TAG = "tb_example";

/**
 * @brief RPC Callback: Handles incoming commands from ThingsBoard
 */
static void my_rpc_handler(const char *request_id, const char *method, const char *params_json, void *user_data)
{
    ESP_LOGI(TAG, "RPC Received: %s, Params: %s", method, params_json);
    
    if (strcmp(method, "reboot") == 0) {
        esp_err_t err = bsp_tb_send_rpc_response(request_id, "{\"status\":\"rebooting\"}");
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to send RPC response: %s", esp_err_to_name(err));
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Allow time for response to transmit
        esp_restart();
    } else {
        esp_err_t err = bsp_tb_send_rpc_response(request_id, "{\"status\":\"unknown_method\"}");
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to send RPC response: %s", esp_err_to_name(err));
        }
    }
}

/**
 * @brief Shared Attribute Callback: Handles server-side configuration updates
 */
static void my_attr_handler(const char *json_payload, void *user_data)
{
    ESP_LOGI(TAG, "Shared Attributes Updated: %s", json_payload);
}

/**
 * @brief Standard ESP-IDF Wi-Fi Station Initialization
 */
static void wifi_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "YOUR_WIFI_SSID",
            .password = "YOUR_WIFI_PASSWORD",
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());
}

void app_main(void)
{
    // 1. Initialize Wi-Fi
    wifi_init();
    ESP_LOGI(TAG, "Wi-Fi initialized, connecting...");

    // 2. Configure ThingsBoard Client
    bsp_tb_config_t tb_cfg = {
        .broker_uri   = "mqtts://thingsboard.cloud:8883",
        .access_token = "YOUR_DEVICE_ACCESS_TOKEN",
        .ca_cert_pem  = NULL, // Uses system TLS certificate bundle
        .rpc_cb       = my_rpc_handler,
        .attr_cb      = my_attr_handler,
        .alarm_cb     = NULL,
        .ota_cb       = NULL,
        .user_data    = NULL
    };

    ESP_ERROR_CHECK(bsp_tb_init(&tb_cfg));

    // 3. Wait for connection
    ESP_LOGI(TAG, "Waiting for ThingsBoard connection...");
    if (bsp_tb_wait_connected(10000) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to connect to ThingsBoard within timeout");
        return;
    }
    ESP_LOGI(TAG, "Connected to ThingsBoard!");

    // 4. Report initial attributes
    ESP_ERROR_CHECK(bsp_tb_report_client_attributes());

    // 5. Main Loop: Send periodic telemetry
    while (1) {
        // Send sample telemetry: 295.15K (22C), 50% RH, 90% Battery, -60dBm RSSI
        esp_err_t ret = bsp_tb_send_telemetry(295.15f, 50.0f, 90, -60);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Telemetry published successfully");
        } else {
            ESP_LOGE(TAG, "Telemetry publish failed: %s", esp_err_to_name(ret));
        }

        vTaskDelay(pdMS_TO_TICKS(30000)); // Report every 30 seconds
    }
}