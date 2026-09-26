/**
 * @file app_ble_prov.c
 * @brief BLE GATT Wi-Fi Provisioning Manager implementation (ESP-IDF v6.1)
 * 
 * @attribution
 * - Espressif Systems ESP-IDF network_provisioning
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
#include "network_provisioning/manager.h"
#include "network_provisioning/scheme_ble.h"
#include "bsp/bsp.h"
#include "app_ble_prov.h"

static const char *TAG = "app_ble_prov";

static bool s_prov_running                   = false;
static app_ble_prov_done_cb_t s_prov_done_cb = NULL;

static void prov_event_handler(void *user_data, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == NETWORK_PROV_EVENT) {
        switch (event_id) {
        case NETWORK_PROV_START:
            ESP_LOGI(TAG, "BLE Provisioning service started. Ready for Chrome Web Bluetooth pairing.");
            break;
        case NETWORK_PROV_WIFI_CRED_RECV: {
            wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
            ESP_LOGI(TAG, "Received Wi-Fi credentials via BLE for SSID: %s", (const char *)wifi_sta_cfg->ssid);
            /* Store into NVS and invalidate fast cache */
            bsp_wifi_save_credentials((const char *)wifi_sta_cfg->ssid, (const char *)wifi_sta_cfg->password);
            break;
        }
        case NETWORK_PROV_WIFI_CRED_FAIL: {
            network_prov_wifi_sta_fail_reason_t *reason = (network_prov_wifi_sta_fail_reason_t *)event_data;
            ESP_LOGE(TAG, "Provisioning failed! Reason: %s",
                     (*reason == NETWORK_PROV_WIFI_STA_AUTH_ERROR) ? "Authentication failed" : "AP not found");
            break;
        }
        case NETWORK_PROV_WIFI_CRED_SUCCESS:
            ESP_LOGI(TAG, "Provisioning credentials applied successfully");
            break;
        case NETWORK_PROV_END:
            ESP_LOGI(TAG, "Provisioning service finished. De-initializing BLE stack...");
            network_prov_mgr_deinit();
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

    /* 1. Ensure TCP/IP stack, default event loop, and Wi-Fi subsystem are initialized */
    esp_err_t ret = bsp_wifi_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Wi-Fi base for provisioning: %s", esp_err_to_name(ret));
        return ret;
    }

    /* 2. Initialize provisioning manager with BLE scheme */
    network_prov_mgr_config_t config = {
        .scheme = network_prov_scheme_ble,
        .scheme_event_handler = NETWORK_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM,
        .app_event_handler = NETWORK_PROV_EVENT_HANDLER_NONE,
    };

    ret = network_prov_mgr_init(config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize network_prov_mgr: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_event_handler_register(NETWORK_PROV_EVENT, ESP_EVENT_ANY_ID, &prov_event_handler, NULL);
    if (ret != ESP_OK) return ret;

#if defined(CONFIG_ESP_PROTOCOMM_SUPPORT_SECURITY_VERSION_1)
    network_prov_security_t security = (pop != NULL && strlen(pop) > 0) ? NETWORK_PROV_SECURITY_1 : NETWORK_PROV_SECURITY_0;
    if (security == NETWORK_PROV_SECURITY_1) {
        ESP_LOGI(TAG, "BLE Provisioning Security: SECURITY_1 (Curve25519 + AES-CTR-128 Enabled, PoP PIN: '%s')", pop);
    } else {
        ESP_LOGW(TAG, "BLE Provisioning Security: SECURITY_0 (Open / Unencrypted)");
    }
#elif defined(CONFIG_ESP_PROTOCOMM_SUPPORT_SECURITY_VERSION_2)
    network_prov_security_t security = NETWORK_PROV_SECURITY_2;
    ESP_LOGI(TAG, "BLE Provisioning Security: SECURITY_2 (SRP6a + AES-GCM Enabled)");
#else
    network_prov_security_t security = NETWORK_PROV_SECURITY_0;
    ESP_LOGW(TAG, "BLE Provisioning Security: SECURITY_0 (Open / Unencrypted)");
#endif
    const char *service_key = NULL;

    ESP_LOGI(TAG, "Starting BLE Provisioning Service with name: %s", service_name);
    ret = network_prov_mgr_start_provisioning(security, pop, service_name, service_key);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start BLE provisioning: %s", esp_err_to_name(ret));
        network_prov_mgr_deinit();
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
        network_prov_mgr_stop_provisioning();
        network_prov_mgr_deinit();
        s_prov_running = false;
        ESP_LOGI(TAG, "BLE Provisioning stopped manually");
    }
}
