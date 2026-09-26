/**
 * @file bsp_wifi.c
 * @brief Wi-Fi Station Manager with Fast RTC Reconnect Session Cache Implementation
 * 
 * Battery Optimization Mechanics:
 *  - Fast Reconnect Cache: Stores BSSID (MAC), Channel (1-13), and SSID in RTC slow memory.
 *  - On boot from deep sleep, `wifi_config_t` has `bssid_set = 1` and `channel = cached_ch`,
 *    allowing the ESP32-S3 to bypass passive/active scanning of all 13 channels and lock onto
 *    the AP within ~350 - 400ms!
 * 
 * @attribution
 * - Espressif Systems
 * - BSP Implementation: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "bsp/bsp_nvs.h"
#include "bsp/bsp_wifi.h"

static const char *TAG = "bsp_wifi";

// Event Group Bits
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static EventGroupHandle_t s_wifi_event_group  = NULL;
static esp_netif_t        *s_sta_netif        = NULL;
static bool               s_is_initialized    = false;
static bool               s_is_connected      = false;

/**
 * @brief RTC Fast Reconnect Session Cache Structure
 * Preserved across Deep Sleep cycles in RTC Slow/Fast memory.
 */
typedef struct {
    uint32_t magic;
    uint8_t  bssid[6];
    uint8_t  channel;
    char     ssid[33];
} rtc_wifi_cache_t;

#define RTC_WIFI_CACHE_MAGIC 0x57494649 // "WIFI"
static RTC_DATA_ATTR rtc_wifi_cache_t s_rtc_cache = {0};

/* =========================================================================
 * Wi-Fi Event Handler Callback
 * ========================================================================= */
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGD(TAG, "Wi-Fi Station started; issuing connection request");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        wifi_event_sta_connected_t *event = (wifi_event_sta_connected_t *)event_data;
        ESP_LOGI(TAG, "Associated with AP '%s' (Channel: %d, BSSID: %02X:%02X:%02X:%02X:%02X:%02X)",
                 event->ssid, event->channel,
                 event->bssid[0], event->bssid[1], event->bssid[2],
                 event->bssid[3], event->bssid[4], event->bssid[5]);

        // Save channel and BSSID into RTC Fast Memory for instantaneous reconnect on next wake
        s_rtc_cache.magic   = RTC_WIFI_CACHE_MAGIC;
        s_rtc_cache.channel = event->channel;
        memcpy(s_rtc_cache.bssid, event->bssid, 6);
        strncpy(s_rtc_cache.ssid, (const char *)event->ssid, sizeof(s_rtc_cache.ssid) - 1);

    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Wi-Fi disconnected from AP");
        s_is_connected = false;
        if (s_wifi_event_group) {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Assigned IPv4 Address: " IPSTR, IP2STR(&event->ip_info.ip));
        s_is_connected = true;
        if (s_wifi_event_group) {
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        }
    }
}

/* =========================================================================
 * Public Wi-Fi APIs
 * ========================================================================= */
esp_err_t bsp_wifi_init(void)
{
    if (s_is_initialized) return ESP_OK;

    ESP_LOGI(TAG, "Initializing Wi-Fi Subsystem (Station Mode)");

    ESP_ERROR_CHECK(esp_netif_init());
    esp_err_t err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Failed to create default event loop: %s", esp_err_to_name(err));
        return err;
    }

    s_sta_netif = esp_netif_create_default_wifi_sta();
    if (!s_sta_netif) {
        ESP_LOGE(TAG, "Failed to create default Wi-Fi STA netif");
        return ESP_FAIL;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    s_is_initialized = true;
    return ESP_OK;
}

esp_err_t bsp_wifi_connect(const char *ssid, const char *password, uint32_t timeout_ms)
{
    if (ssid == NULL || strlen(ssid) == 0) return ESP_ERR_INVALID_ARG;

    if (!s_is_initialized) {
        esp_err_t err = bsp_wifi_init();
        if (err != ESP_OK) return err;
    }

    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);

    wifi_config_t wifi_cfg;
    memset(&wifi_cfg, 0, sizeof(wifi_config_t));
    strncpy((char *)wifi_cfg.sta.ssid, ssid, sizeof(wifi_cfg.sta.ssid) - 1);
    if (password && strlen(password) > 0) {
        strncpy((char *)wifi_cfg.sta.password, password, sizeof(wifi_cfg.sta.password) - 1);
    }

    // Check if we have valid RTC fast reconnect cache for this exact SSID
    if (s_rtc_cache.magic == RTC_WIFI_CACHE_MAGIC && strcmp(s_rtc_cache.ssid, ssid) == 0 && s_rtc_cache.channel > 0) {
        ESP_LOGI(TAG, "Applying Fast RTC Reconnect Cache -> Channel: %d, BSSID: %02X:%02X:%02X:%02X:%02X:%02X",
                 s_rtc_cache.channel,
                 s_rtc_cache.bssid[0], s_rtc_cache.bssid[1], s_rtc_cache.bssid[2],
                 s_rtc_cache.bssid[3], s_rtc_cache.bssid[4], s_rtc_cache.bssid[5]);

        wifi_cfg.sta.bssid_set = 1;
        memcpy(wifi_cfg.sta.bssid, s_rtc_cache.bssid, 6);
        wifi_cfg.sta.channel     = s_rtc_cache.channel;
        wifi_cfg.sta.scan_method = WIFI_FAST_SCAN;
    } else {
        wifi_cfg.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Wait for IP event or failure bit
    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,
        pdFALSE,
        pdMS_TO_TICKS(timeout_ms)
    );

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Wi-Fi connection established successfully!");
        return ESP_OK;
    } else {
        ESP_LOGW(TAG, "Wi-Fi connection timed out or failed");
        // Invalidate cache on failure so next attempt does full scan
        bsp_wifi_invalidate_fast_cache();
        return ESP_ERR_TIMEOUT;
    }
}

esp_err_t bsp_wifi_connect_from_nvs(uint32_t timeout_ms)
{
    char ssid[33] = {0};
    char pass[65] = {0};

    if (bsp_nvs_get_str("wifi_ssid", ssid, sizeof(ssid)) != ESP_OK || strlen(ssid) == 0) {
        ESP_LOGW(TAG, "No Wi-Fi credentials found in NVS");
        return ESP_ERR_NVS_NOT_FOUND;
    }

    bsp_nvs_get_str("wifi_pass", pass, sizeof(pass));
    return bsp_wifi_connect(ssid, pass, timeout_ms);
}

esp_err_t bsp_wifi_disconnect(void)
{
    if (!s_is_initialized) return ESP_OK;

    s_is_connected = false;
    esp_wifi_disconnect();
    esp_wifi_stop();
    return ESP_OK;
}

bool bsp_wifi_is_connected(void)
{
    return s_is_connected;
}

esp_err_t bsp_wifi_get_rssi(int *out_rssi)
{
    if (!s_is_connected || out_rssi == NULL) return ESP_ERR_INVALID_STATE;

    wifi_ap_record_t ap_info;
    esp_err_t err = esp_wifi_sta_get_ap_info(&ap_info);
    if (err == ESP_OK) {
        *out_rssi = ap_info.rssi;
        return ESP_OK;
    }
    return err;
}

esp_err_t bsp_wifi_get_ip_str(char *out_ip, size_t max_len)
{
    if (!s_is_connected || out_ip == NULL || !s_sta_netif) return ESP_ERR_INVALID_STATE;

    esp_netif_ip_info_t ip_info;
    if (esp_netif_get_ip_info(s_sta_netif, &ip_info) == ESP_OK) {
        snprintf(out_ip, max_len, IPSTR, IP2STR(&ip_info.ip));
        return ESP_OK;
    }
    return ESP_FAIL;
}

esp_err_t bsp_wifi_save_credentials(const char *ssid, const char *password)
{
    if (ssid == NULL || strlen(ssid) == 0) return ESP_ERR_INVALID_ARG;

    bsp_nvs_set_str("wifi_ssid", ssid);
    bsp_nvs_set_str("wifi_pass", password ? password : "");
    bsp_wifi_invalidate_fast_cache();
    return ESP_OK;
}

void bsp_wifi_invalidate_fast_cache(void)
{
    memset(&s_rtc_cache, 0, sizeof(s_rtc_cache));
    ESP_LOGD(TAG, "Invalidated Fast RTC Reconnect Cache");
}
