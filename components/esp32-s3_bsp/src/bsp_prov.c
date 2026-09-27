/**
 * @file bsp_prov.c
 * @brief Turnkey BLE GATT Wi-Fi Provisioning & QR Code Generator Implementation
 * 
 * @attribution
 * - Espressif Systems ESP-IDF network_provisioning
 * - BSP Architecture: Humidyne Labs / Humiditron (2026)
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
#include "lvgl.h"
#include "bsp/bsp.h"
#include "bsp/bsp_prov.h"
#include "bsp/bsp_wifi.h"

static const char *TAG = "bsp_prov";

static bool                 s_prov_running         = false;
static bsp_prov_event_cb_t  s_prov_cb              = NULL;
static void                 *s_prov_user_data      = NULL;
static char                 s_active_serv_name[48] = {0};

static void prov_event_handler(void *user_data, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base != NETWORK_PROV_EVENT) return;

    switch (event_id) {
    case NETWORK_PROV_START:
        ESP_LOGI(TAG, "BLE Provisioning service started. Service name: %s", s_active_serv_name);
        if (s_prov_cb) s_prov_cb(BSP_PROV_EVENT_STARTED, NULL, s_prov_user_data);
        break;

    case NETWORK_PROV_WIFI_CRED_RECV: {
        wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
        ESP_LOGI(TAG, "Received Wi-Fi credentials via BLE for SSID: %s", (const char *)wifi_sta_cfg->ssid);
        bsp_wifi_save_credentials((const char *)wifi_sta_cfg->ssid, (const char *)wifi_sta_cfg->password);
        if (s_prov_cb) s_prov_cb(BSP_PROV_EVENT_CRED_RECEIVED, wifi_sta_cfg, s_prov_user_data);
        break;
    }

    case NETWORK_PROV_WIFI_CRED_FAIL: {
        network_prov_wifi_sta_fail_reason_t *reason = (network_prov_wifi_sta_fail_reason_t *)event_data;
        ESP_LOGE(TAG, "Provisioning failed! Reason code: %d", (int)*reason);
        if (s_prov_cb) s_prov_cb(BSP_PROV_EVENT_CRED_FAILED, reason, s_prov_user_data);
        break;
    }

    case NETWORK_PROV_WIFI_CRED_SUCCESS:
        ESP_LOGI(TAG, "Provisioning credentials verified and connected successfully");
        if (s_prov_cb) s_prov_cb(BSP_PROV_EVENT_CRED_SUCCESS, NULL, s_prov_user_data);
        break;

    case NETWORK_PROV_END:
        ESP_LOGI(TAG, "Provisioning complete. Releasing Bluetooth resources...");
        network_prov_mgr_deinit();
        s_prov_running = false;
        if (s_prov_cb) s_prov_cb(BSP_PROV_EVENT_FINISHED, NULL, s_prov_user_data);
        break;

    default:
        break;
    }
}

esp_err_t bsp_prov_start(const char *custom_service_name, const char *pop, bsp_prov_event_cb_t cb, void *user_data)
{
    if (s_prov_running) {
        ESP_LOGW(TAG, "BLE provisioning is already active");
        return ESP_OK;
    }

    s_prov_cb        = cb;
    s_prov_user_data = user_data;

    esp_err_t ret = bsp_wifi_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Wi-Fi base: %s", esp_err_to_name(ret));
        return ret;
    }

    network_prov_mgr_config_t config = {
        .scheme               = network_prov_scheme_ble,
        .scheme_event_handler = NETWORK_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM,
        .app_event_handler    = NETWORK_PROV_EVENT_HANDLER_NONE,
    };

    ret = network_prov_mgr_init(config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init network_prov_mgr: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_event_handler_register(NETWORK_PROV_EVENT, ESP_EVENT_ANY_ID, &prov_event_handler, NULL);
    if (ret != ESP_OK) return ret;

    if (custom_service_name != NULL && strlen(custom_service_name) > 0) {
        strncpy(s_active_serv_name, custom_service_name, sizeof(s_active_serv_name) - 1);
    } else {
        char dev_id[32] = {0};
        bsp_get_device_id(dev_id, sizeof(dev_id));
        snprintf(s_active_serv_name, sizeof(s_active_serv_name), "PROV_%s", dev_id);
    }

    network_prov_security_t security = (pop != NULL && strlen(pop) > 0) ? NETWORK_PROV_SECURITY_1 : NETWORK_PROV_SECURITY_0;

    ret = network_prov_mgr_start_provisioning(security, (const void *)pop, s_active_serv_name, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start BLE provisioning: %s", esp_err_to_name(ret));
        network_prov_mgr_deinit();
        return ret;
    }

    s_prov_running = true;
    ESP_LOGI(TAG, "Provisioning active: Name='%s', Security=%d", s_active_serv_name, (int)security);
    return ESP_OK;
}

bool bsp_prov_is_running(void)
{
    return s_prov_running;
}

void bsp_prov_stop(void)
{
    if (s_prov_running) {
        network_prov_mgr_stop_provisioning();
        network_prov_mgr_deinit();
        s_prov_running = false;
        ESP_LOGI(TAG, "Provisioning stopped");
    }
}

esp_err_t bsp_prov_get_qr_payload(const char *pop, char *dest, size_t max_len)
{
    if (dest == NULL || max_len < 64) {
        return ESP_ERR_INVALID_ARG;
    }

    char name[48] = {0};
    if (strlen(s_active_serv_name) > 0) {
        strncpy(name, s_active_serv_name, sizeof(name) - 1);
    } else {
        char dev_id[32] = {0};
        bsp_get_device_id(dev_id, sizeof(dev_id));
        snprintf(name, sizeof(name), "PROV_%s", dev_id);
    }

    if (pop != NULL && strlen(pop) > 0) {
        snprintf(dest, max_len, "{\"ver\":\"v1\",\"name\":\"%s\",\"pop\":\"%s\",\"transport\":\"ble\"}", name, pop);
    } else {
        snprintf(dest, max_len, "{\"ver\":\"v1\",\"name\":\"%s\",\"transport\":\"ble\"}", name);
    }

    return ESP_OK;
}

lv_obj_t *bsp_prov_render_qr_code(lv_obj_t *parent, int32_t size, const char *pop)
{
    if (parent == NULL || size <= 0) {
        return NULL;
    }

    char payload[128] = {0};
    if (bsp_prov_get_qr_payload(pop, payload, sizeof(payload)) != ESP_OK) {
        return NULL;
    }

    lv_obj_t *qr = lv_qrcode_create(parent);
    if (qr == NULL) {
        ESP_LOGE(TAG, "Failed to allocate LVGL QR Code widget");
        return NULL;
    }

    lv_qrcode_set_size(qr, size);
    lv_qrcode_set_dark_color(qr, lv_color_black());
    lv_qrcode_set_light_color(qr, lv_color_white());
    lv_qrcode_update(qr, payload, strlen(payload));
    lv_obj_center(qr);

    ESP_LOGI(TAG, "Rendered QR code (%ldx%ld px) with payload: %s", (long)size, (long)size, payload);
    return qr;
}
