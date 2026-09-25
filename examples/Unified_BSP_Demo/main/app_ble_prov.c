/**
 * @file app_ble_prov.c
 * @brief BLE GATT Wi-Fi Provisioning Manager implementation
 * 
 * @attribution
 * - Espressif Systems ESP-IDF wifi_provisioning
 * - Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_ble.h"
#include "bsp/bsp.h"
#include "app_ble_prov.h"

static const char *TAG = "app_ble_prov";

static bool s_prov_running = false;
static app_ble_prov_done_cb_t s_prov_done_cb = NULL;

static void prov_event_handler(void *user_data, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_PROV_EVENT) {
        switch (event_id) {
        case WIFI_PROV_START:
            ESP_LOGI(TAG, "BLE Provisioning service started. Ready for Chrome Web Bluetooth pairing.");
            break;
        case WIFI_PROV_CRED_RECV: {
            wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
            ESP_LOGI(TAG, "Received Wi-Fi credentials via BLE for SSID: %s", (const char *)wifi_sta_cfg->ssid);
            /* Store into NVS */
            bsp_nvs_set_wifi_credentials((const char *)wifi_sta_cfg->ssid, (const char *)wifi_sta_cfg->password);
            break;
        }
        case WIFI_PROV_CRED_FAIL: {
            wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
            ESP_LOGE(TAG, "Provisioning failed! Reason: %s",
                     (*reason == WIFI_PROV_STA_AUTH_ERROR) ? "Authentication failed" : "AP not found");
            break;
        }
        case WIFI_PROV_CRED_SUCCESS:
            ESP_LOGI(TAG, "Provisioning credentials applied successfully");
            break;
        case WIFI_PROV_END:
            ESP_LOGI(TAG, "Provisioning service finished. De-initializing BLE stack...");
            wifi_prov_mgr_deinit();
            s_prov_running = false;
            if (s_prov_done_cb) {
                s_prov_done_cb();
            }
            break;
        default:
            break;
        }
    }
}

esp_err_t app_ble_prov_start(const char *service_name, const char *pop, app_ble_prov_done_cb_t done_cb)
{
    if (s_prov_running) {
        ESP_LOGW(TAG, "BLE provisioning is already active");
        return ESP_OK;
    }

    s_prov_done_cb = done_cb;

    /* Initialize provisioning manager with BLE scheme */
    wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_ble,
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM,
        .app_info = NULL,
    };

    esp_err_t ret = wifi_prov_mgr_init(config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize wifi_prov_mgr: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &prov_event_handler, NULL);
    if (ret != ESP_OK) return ret;

    wifi_prov_security_t security = WIFI_PROV_SECURITY_0;
    const char *service_key = NULL;

    if (pop != NULL && strlen(pop) > 0) {
        security = WIFI_PROV_SECURITY_1;
    }

    ESP_LOGI(TAG, "Starting BLE Provisioning Service with name: %s", service_name);
    ret = wifi_prov_mgr_start_provisioning(security, pop, service_name, service_key);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start BLE provisioning: %s", esp_err_to_name(ret));
        wifi_prov_mgr_deinit();
        return ret;
    }

    s_prov_running = true;
    return ESP_OK;
}

bool app_ble_prov_is_running(void)
{
    return s_prov_running;
}

void app_ble_prov_stop(void)
{
    if (s_prov_running) {
        wifi_prov_mgr_stop_provisioning();
        wifi_prov_mgr_deinit();
        s_prov_running = false;
        ESP_LOGI(TAG, "BLE Provisioning stopped manually");
    }
}
