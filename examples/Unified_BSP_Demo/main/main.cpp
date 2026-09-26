/**
 * @file main.cpp
 * @brief ESP32-S3 Touch ePaper BSP - Production Environmental Telemetry Node
 * 
 * Hardware Target: Waveshare ESP32-S3-Touch-ePaper-1.54 V2 (200x200 1-bit Mono EPD)
 * 
 * Operational Lifecycle & Flow:
 *  1. BOOT / PAIRING:
 *     - If no Wi-Fi credentials or connection fails: Show dedicated BLE Provisioning Screen.
 *       (Device stays awake on connection failure so the user can pair).
 *  2. CLAIMING SCREEN:
 *     - Once connected, if unclaimed: Show dedicated Claiming Screen with 6-8 char token.
 *       (Screen shows only the claiming key until claimed or expired).
 *  3. ACTIVE TELEMETRY DASHBOARD (Once claimed & running):
 *     - Shows live SHTC3 metrics, passive card inversion on threshold fault, and low battery flag (<20%).
 *  4. ULTRA-LOW POWER DEEP SLEEP:
 *     - Device spends most of its time in deep sleep.
 *     - Screen contents are preserved with zero power draw on the bi-stable e-Paper panel.
 *     - On wake: reads sensor, connects (<400ms), publishes telemetry (synchronously confirmed),
 *       refreshes e-Paper, resumes sleep.
 *     - On network connection failure: Displays error indicator and STAYS AWAKE (no sleep).
 *  5. POWER OFF:
 *     - Displays space_cat.bin only upon explicit user shutdown via POWER button.
 * 
 * @copyright Copyright (c) 2026 Humidyne Labs / Humiditron
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_app_desc.h"
#include "lvgl.h"
#include "bsp/bsp.h"
#include "app_secrets.h"
#include "app_claiming.h"
#include "app_ble_prov.h"
#include "app_time_sync.h"
#include "app_mqtt.h"
#include "mmap_generate_storage.h"

static const char *TAG = "main_telemetry";

// RTC Fast Memory Variables (Persist across deep sleep)
static RTC_DATA_ATTR uint32_t s_rtc_boot_count  = 0;
static RTC_DATA_ATTR bool     s_rtc_is_claimed  = false;

// Device State
static char s_device_name[32]      = {0};
static char s_claim_key[16]        = {0};
static bool s_is_provisioning_mode = false;
static bool s_network_failed       = false;
static bool s_card_env_inverted    = false;
static bool s_debug_mode           = false;

// UI Widget Handles for Active Screen
static lv_obj_t *s_lbl_clock        = NULL;
static lv_obj_t *s_lbl_header_right = NULL;
static lv_obj_t *s_card_env         = NULL;
static lv_obj_t *s_lbl_temp         = NULL;
static lv_obj_t *s_lbl_humidity     = NULL;
static lv_obj_t *s_lbl_net_status   = NULL;
static lv_obj_t *s_lbl_action_hint  = NULL;

/* =========================================================================
 * Audio Chime Synthesizer
 * ========================================================================= */
static void play_audio_chime(void) 
{
    const uint32_t sample_rate   = 16000;
    const size_t   tone_samples  = sample_rate / 4;
    const size_t   total_samples = tone_samples * 2;
    const size_t   buf_size      = total_samples * sizeof(int16_t);

    int16_t *buf = (int16_t *)malloc(buf_size);
    if (!buf) return;

    float freqs[2] = {880.0f, 1760.0f};
    int16_t *p = buf;

    for (int t = 0; t < 2; t++) {
        for (size_t i = 0; i < tone_samples; i++) {
            float angle = 2.0f * (float)M_PI * freqs[t] * ((float)i / (float)sample_rate);
            float env = sinf((float)M_PI * ((float)i / (float)tone_samples));
            *p++ = (int16_t)(sinf(angle) * env * 12000.0f);
        }
    }

    bsp_audio_set_volume(85.0f);
    bsp_audio_play(buf, buf_size, NULL);
    free(buf);
}

/* =========================================================================
 * UI Screen 1: BLE Provisioning Mode
 * ========================================================================= */
static void show_provisioning_screen(void)
{
    bsp_lvgl_lock();
    lv_obj_t *scr = lv_screen_active();
    lv_obj_clean(scr);

    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    lv_obj_t *card = lv_obj_create(scr);
    lv_obj_set_size(card, 192, 192);
    lv_obj_align(card, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(card, 6, 0);
    lv_obj_set_style_border_width(card, 2, 0);
    lv_obj_set_style_border_color(card, lv_color_black(), 0);

    lv_obj_t *t1 = lv_label_create(card);
    lv_label_set_text(t1, "BLE PAIRING");
    lv_obj_align(t1, LV_ALIGN_TOP_MID, 0, 4);

    lv_obj_t *box = lv_obj_create(card);
    lv_obj_set_size(box, 172, 48);
    lv_obj_align(box, LV_ALIGN_TOP_MID, 0, 32);
    lv_obj_set_style_border_width(box, 1, 0);
    lv_obj_set_style_border_color(box, lv_color_black(), 0);

    lv_obj_t *t2 = lv_label_create(box);
    char buf[64];
    snprintf(buf, sizeof(buf), "%s%s", CONFIG_BLE_PROV_PREFIX, s_device_name);
    lv_label_set_text(t2, buf);
    lv_obj_center(t2);

    lv_obj_t *t3 = lv_label_create(card);
    lv_label_set_text(t3, "Open Chrome Dashboard\nto configure Wi-Fi");
    lv_obj_align(t3, LV_ALIGN_TOP_MID, 0, 92);

    lv_obj_t *t4 = lv_label_create(card);
    lv_label_set_text(t4, "[Device Stays Awake]");
    lv_obj_align(t4, LV_ALIGN_BOTTOM_MID, 0, -4);

    lv_refr_now(NULL);
    bsp_lvgl_unlock();
}

/* =========================================================================
 * UI Screen 2: Dedicated Device Claiming Mode (Shows ONLY Claim Key)
 * ========================================================================= */
static void show_claiming_screen(void)
{
    bsp_lvgl_lock();
    lv_obj_t *scr = lv_screen_active();
    lv_obj_clean(scr);

    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    lv_obj_t *card = lv_obj_create(scr);
    lv_obj_set_size(card, 192, 192);
    lv_obj_align(card, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(card, 6, 0);
    lv_obj_set_style_border_width(card, 2, 0);
    lv_obj_set_style_border_color(card, lv_color_black(), 0);

    lv_obj_t *t1 = lv_label_create(card);
    lv_label_set_text(t1, "CLAIM DEVICE");
    lv_obj_align(t1, LV_ALIGN_TOP_MID, 0, 4);

    lv_obj_t *box = lv_obj_create(card);
    lv_obj_set_size(box, 172, 54);
    lv_obj_align(box, LV_ALIGN_TOP_MID, 0, 32);
    lv_obj_set_style_border_width(box, 2, 0);
    lv_obj_set_style_border_color(box, lv_color_black(), 0);

    lv_obj_t *t2 = lv_label_create(box);
    char buf[32];
    snprintf(buf, sizeof(buf), "[ %s ]", s_claim_key);
    lv_label_set_text(t2, buf);
    lv_obj_center(t2);

    lv_obj_t *t3 = lv_label_create(card);
    lv_label_set_text(t3, "Enter in ThingsBoard\nExpires in 3 mins");
    lv_obj_align(t3, LV_ALIGN_TOP_MID, 0, 96);

    lv_obj_t *t4 = lv_label_create(card);
    lv_label_set_text(t4, "BOOT: Skip / Continue");
    lv_obj_align(t4, LV_ALIGN_BOTTOM_MID, 0, -4);

    lv_refr_now(NULL);
    bsp_lvgl_unlock();
}

/* =========================================================================
 * UI Screen 3: Active Telemetry Dashboard (Once Claimed & Running)
 * ========================================================================= */
static void create_active_telemetry_ui(void) 
{
    char buf[64];
    bsp_lvgl_lock();
    
    lv_obj_t *scr = lv_screen_active();
    lv_obj_clean(scr);
    
    lv_obj_set_style_bg_color(scr, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);

    /* 1. TOP STATUS BAR (Y: 0 .. 22) */
    s_lbl_clock = lv_label_create(scr);
    lv_label_set_text(s_lbl_clock, "--:--");
    lv_obj_align(s_lbl_clock, LV_ALIGN_TOP_LEFT, 4, 3);

    s_lbl_header_right = lv_label_create(scr);
    snprintf(buf, sizeof(buf), "#%lu | --%%", (unsigned long)s_rtc_boot_count);
    lv_label_set_text(s_lbl_header_right, buf);
    lv_obj_align(s_lbl_header_right, LV_ALIGN_TOP_RIGHT, -4, 3);

    static lv_point_precise_t line_top_pts[] = {{0, 22}, {200, 22}};
    lv_obj_t *line_top = lv_line_create(scr);
    lv_line_set_points(line_top, line_top_pts, 2);
    lv_obj_set_style_line_width(line_top, 1, 0);
    lv_obj_set_style_line_color(line_top, lv_color_black(), 0);

    /* 2. ENVIRONMENTAL TELEMETRY CARD (Y: 28 .. 142) */
    s_card_env = lv_obj_create(scr);
    lv_obj_set_size(s_card_env, 192, 114);
    lv_obj_align(s_card_env, LV_ALIGN_TOP_MID, 0, 28);
    lv_obj_set_style_pad_all(s_card_env, 6, 0);
    lv_obj_set_style_radius(s_card_env, 4, 0);
    lv_obj_set_style_border_width(s_card_env, 1, 0);
    lv_obj_set_style_border_color(s_card_env, lv_color_black(), 0);
    lv_obj_set_style_bg_color(s_card_env, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(s_card_env, LV_OPA_COVER, 0);

    s_lbl_temp = lv_label_create(s_card_env);
    lv_label_set_text(s_lbl_temp, "TEMP: --.- °F");
    lv_obj_align(s_lbl_temp, LV_ALIGN_TOP_LEFT, 4, 8);
    lv_obj_set_style_text_color(s_lbl_temp, lv_color_black(), 0);

    s_lbl_humidity = lv_label_create(s_card_env);
    lv_label_set_text(s_lbl_humidity, "HUM:  --.- % RH");
    lv_obj_align(s_lbl_humidity, LV_ALIGN_TOP_LEFT, 4, 38);
    lv_obj_set_style_text_color(s_lbl_humidity, lv_color_black(), 0);

    lv_obj_t *lbl_sub = lv_label_create(s_card_env);
    snprintf(buf, sizeof(buf), "Node: %s", s_device_name);
    lv_label_set_text(lbl_sub, buf);
    lv_obj_align(lbl_sub, LV_ALIGN_BOTTOM_LEFT, 4, -4);
    lv_obj_set_style_text_color(lbl_sub, lv_color_black(), 0);

    /* 3. NETWORK STATUS & SLEEP FOOTER (Y: 148 .. 198) */
    static lv_point_precise_t line_bot_pts[] = {{0, 146}, {200, 146}};
    lv_obj_t *line_bot = lv_line_create(scr);
    lv_line_set_points(line_bot, line_bot_pts, 2);
    lv_obj_set_style_line_width(line_bot, 1, 0);
    lv_obj_set_style_line_color(line_bot, lv_color_black(), 0);

    s_lbl_net_status = lv_label_create(scr);
    lv_label_set_text(s_lbl_net_status, "Wi-Fi: Connecting...");
    lv_obj_align(s_lbl_net_status, LV_ALIGN_TOP_LEFT, 4, 150);

    s_lbl_action_hint = lv_label_create(scr);
    lv_label_set_text(s_lbl_action_hint, "Sleeping between reads");
    lv_obj_align(s_lbl_action_hint, LV_ALIGN_BOTTOM_MID, 0, -3);

    bsp_lvgl_unlock();
}

/* =========================================================================
 * UI Telemetry Updater with Fault Inversion & Low Battery Indicator
 * ========================================================================= */
static void update_active_telemetry_ui(float temp_k, float rh_pct,
                                      uint32_t vbat_mv, uint8_t battery_pct, const char *time_str,
                                      const char *net_status_str)
{
    const app_shared_config_t *cfg = app_mqtt_get_shared_config();
    char buf[48];
    bsp_lvgl_lock();

    // 1. Clock
    if (s_lbl_clock && time_str) {
        lv_label_set_text(s_lbl_clock, time_str);
    }

    // 2. Battery & Low Battery Visual Indicator (< 20%)
    if (s_lbl_header_right) {
        if (battery_pct <= 20) {
            snprintf(buf, sizeof(buf), "! LOW BATT %u%%", battery_pct);
        } else {
            snprintf(buf, sizeof(buf), "#%lu | %u%%", (unsigned long)s_rtc_boot_count, battery_pct);
        }
        lv_label_set_text(s_lbl_header_right, buf);
    }

    // 3. Evaluate Alarm Conditions for Passive Card Inversion
    bool temp_fault = (temp_k <= cfg->alarm_thresholds.temp_low_critical ||
                       temp_k >= cfg->alarm_thresholds.temp_high_critical);
    bool rh_fault   = (rh_pct <= cfg->alarm_thresholds.rh_low_critical ||
                       rh_pct >= cfg->alarm_thresholds.rh_high_critical);
    bool is_alarm   = (temp_fault || rh_fault);

    if (s_card_env) {
        if (is_alarm && !s_card_env_inverted) {
            lv_obj_set_style_bg_color(s_card_env, lv_color_black(), 0);
            if (s_lbl_temp)     lv_obj_set_style_text_color(s_lbl_temp, lv_color_white(), 0);
            if (s_lbl_humidity) lv_obj_set_style_text_color(s_lbl_humidity, lv_color_white(), 0);
            s_card_env_inverted = true;
        } else if (!is_alarm && s_card_env_inverted) {
            lv_obj_set_style_bg_color(s_card_env, lv_color_white(), 0);
            if (s_lbl_temp)     lv_obj_set_style_text_color(s_lbl_temp, lv_color_white(), 0);
            if (s_lbl_humidity) lv_obj_set_style_text_color(s_lbl_humidity, lv_color_white(), 0);
            s_card_env_inverted = false;
        }
    }

    // 4. Temperature Display based on Shared Attribute "temp_unit"
    if (s_lbl_temp) {
        float temp_c = temp_k - 273.15f;
        float temp_f = (temp_c * 1.8f) + 32.0f;
        if (strcmp(cfg->temp_unit, "C") == 0) {
            snprintf(buf, sizeof(buf), "%sTEMP: %.1f °C", temp_fault ? "! " : "", temp_c);
        } else if (strcmp(cfg->temp_unit, "K") == 0) {
            snprintf(buf, sizeof(buf), "%sTEMP: %.1f K", temp_fault ? "! " : "", temp_k);
        } else {
            snprintf(buf, sizeof(buf), "%sTEMP: %.1f °F", temp_fault ? "! " : "", temp_f);
        }
        lv_label_set_text(s_lbl_temp, buf);
    }

    // 5. Humidity Display
    if (s_lbl_humidity) {
        snprintf(buf, sizeof(buf), "%sHUM:  %.1f %% RH", rh_fault ? "! " : "", rh_pct);
        lv_label_set_text(s_lbl_humidity, buf);
    }

    // 6. Network Status
    if (s_lbl_net_status && net_status_str) {
        lv_label_set_text(s_lbl_net_status, net_status_str);
    }

    // Refresh e-Paper display buffer
    lv_refr_now(NULL);
    bsp_lvgl_unlock();
}

/* =========================================================================
 * Button Interaction Callbacks
 * ========================================================================= */
static void on_boot_button(bsp_button_t btn, bsp_button_event_t evt, void *arg) 
{
    if (evt == BSP_BUTTON_EVENT_SINGLE_CLICK) {
        ESP_LOGI(TAG, "BOOT Click: Claiming confirmed / manual refresh");
        play_audio_chime();
        s_rtc_is_claimed = true;
        bsp_nvs_set_u32("tb_claimed", 1);
    } else if (evt == BSP_BUTTON_EVENT_LONG_PRESS) {
        ESP_LOGW(TAG, "BOOT Long Press (5s): Performing Complete Factory Reset Wipe...");
        
        bsp_lvgl_lock();
        lv_obj_t *scr = lv_screen_active();
        lv_obj_clean(scr);
        lv_obj_t *lbl = lv_label_create(scr);
        lv_label_set_text(lbl, "WIPING NVS -> RESET");
        lv_obj_center(lbl);
        lv_refr_now(NULL);
        bsp_lvgl_unlock();

        bsp_nvs_clear_wifi_credentials();
        bsp_wifi_invalidate_fast_cache();
        bsp_nvs_wipe_all();
        s_rtc_is_claimed = false;

        vTaskDelay(pdMS_TO_TICKS(1500));
        esp_restart();
    }
}

static void on_power_button(bsp_button_t btn, bsp_button_event_t evt, void *arg) 
{
    if (evt == BSP_BUTTON_EVENT_SINGLE_CLICK) {
        ESP_LOGI(TAG, "POWER Button: Performing safe shutdown (Showing space_cat.bin)...");
        
        bsp_lvgl_lock();
        lv_obj_t *scr = lv_screen_active();
        lv_obj_clean(scr);

        lv_obj_set_style_bg_color(scr, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);

        lv_obj_t *img = lv_image_create(scr);
        lv_image_set_src(img, "S:space_cat.bin");
        lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
        lv_refr_now(NULL);
        bsp_lvgl_unlock();

        vTaskDelay(pdMS_TO_TICKS(2500));
        bsp_system_shutdown();
    }
}

/* =========================================================================
 * Core 0 Dedicated Task: Networking, Telemetry Publishing, and Sleep Cycle
 * ========================================================================= */
static void network_telemetry_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Core 0 Network & Telemetry task started (Priority 3)");

    // 1. Wi-Fi Sequencing: Fast Reconnect -> NVS -> Fallback -> BLE Provisioning
    esp_err_t wifi_err = bsp_wifi_connect_from_nvs(4000);

    if (wifi_err != ESP_OK && strlen(CONFIG_FALLBACK_WIFI_SSID) > 0) {
        ESP_LOGI(TAG, "Connecting to fallback Wi-Fi from app_secrets.h: %s", CONFIG_FALLBACK_WIFI_SSID);
        wifi_err = bsp_wifi_connect(CONFIG_FALLBACK_WIFI_SSID, CONFIG_FALLBACK_WIFI_PASS, 4000);
    }

    if (wifi_err != ESP_OK) {
        if (s_debug_mode) {
            ESP_LOGW(TAG, "Debug Mode: Wi-Fi offline, skipping BLE provisioning screen and dropping into dashboard");
            s_network_failed = true;
        } else {
            ESP_LOGW(TAG, "Wi-Fi connection failed. Launching BLE Provisioning Screen...");
            s_network_failed = true;
            s_is_provisioning_mode = true;
            
            // Show dedicated BLE Provisioning Screen
            show_provisioning_screen();
            
            char prov_name[64];
            snprintf(prov_name, sizeof(prov_name), "%s%s", CONFIG_BLE_PROV_PREFIX, s_device_name);
            
            app_ble_prov_start(prov_name, NULL, []() {
                ESP_LOGI(TAG, "BLE Provisioning complete. Connecting to Wi-Fi...");
                s_is_provisioning_mode = false;
            });

            // Stay awake on network failure / provisioning mode
            while (s_is_provisioning_mode) {
                vTaskDelay(pdMS_TO_TICKS(500));
            }

            bsp_wifi_connect_from_nvs(5000);
        }
    }

    s_network_failed = !bsp_wifi_is_connected();

    // 2. Network Time Sync
    if (bsp_wifi_is_connected()) {
        app_time_sync_init(NULL);
    }

    // 3. Initialize ThingsBoard Secure MQTTS Client
    if (bsp_wifi_is_connected()) {
        app_mqtt_init(CONFIG_THINGSBOARD_URI,
                      CONFIG_THINGSBOARD_ACCESS_TOKEN,
                      CONFIG_THINGSBOARD_CA_CERT,
                      NULL,
                      [](const app_shared_config_t *cfg) {
                          s_rtc_is_claimed = true;
                          bsp_nvs_set_u32("tb_claimed", 1);
                      },
                      NULL);
        // Wait up to 4 seconds for MQTT connection handshake
        app_mqtt_wait_connected(4000);
    }

    // 4. Claiming Flow vs Active Telemetry Screen
    if (!s_rtc_is_claimed && !s_debug_mode) {
        uint32_t nvs_claimed = 0;
        bsp_nvs_get_u32("tb_claimed", &nvs_claimed);
        s_rtc_is_claimed = (nvs_claimed == 1);
    }

    if (!s_rtc_is_claimed && !s_debug_mode) {
        ESP_LOGI(TAG, "Device unclaimed: Publishing claim token & showing Claiming Screen");
        if (app_mqtt_is_connected()) {
            app_mqtt_publish_claim_token(s_claim_key, CONFIG_CLAIM_DURATION_MS);
        }
        show_claiming_screen();

        // Wait on claiming screen until claimed or user advances
        int claim_wait = 0;
        while (!s_rtc_is_claimed && claim_wait < 180) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            claim_wait++;
        }
        s_rtc_is_claimed = true;
    }

    // 5. Build Active Telemetry UI
    create_active_telemetry_ui();

    // 6. Primary Sample, Publish, Display & Deep Sleep Cycle
    while (1) {
        const app_shared_config_t *shared = app_mqtt_get_shared_config();

        // Sample SHTC3 Environmental Sensor (native Kelvin)
        bsp_shtc3_data_t env;
        float temp_k = 293.15f; // Default 20.0 °C / 68.0 °F
        float rh_pct = 68.0f;
        if (bsp_shtc3_read(&env) == ESP_OK) {
            temp_k = env.temperature_k;
            rh_pct = env.humidity_percent;
        }

        // Read Battery ADC
        uint32_t vbat_mv = 0;
        bsp_battery_get_voltage(&vbat_mv, NULL);
        uint8_t battery_pct = bsp_battery_get_percentage();

        // Read RTC Time (HH:MM)
        bsp_rtc_datetime_t dt;
        char time_str[16] = "--:--";
        if (bsp_rtc_get_datetime(&dt) == ESP_OK) {
            snprintf(time_str, sizeof(time_str), "%02d:%02d", dt.hour, dt.minute);
        }

        // Network Status String
        char net_status_str[48];
        int rssi = -60;
        if (bsp_wifi_is_connected()) {
            bsp_wifi_get_rssi(&rssi);
            snprintf(net_status_str, sizeof(net_status_str), "Wi-Fi:%ddBm | MQTTS", rssi);

            // Synchronously publish telemetry to ThingsBoard (Wait for broker ACK before sleeping)
            if (app_mqtt_is_connected()) {
                app_mqtt_publish_telemetry_sync(temp_k, rh_pct, battery_pct, rssi, 3000);
            }
        } else {
            if (s_debug_mode) {
                snprintf(net_status_str, sizeof(net_status_str), "[DEBUG MODE] Live Test");
            } else {
                snprintf(net_status_str, sizeof(net_status_str), "! NO NETWORK / RETRY");
                s_network_failed = true;
            }
        }

        // Update Active Telemetry Dashboard (Preserved on e-Paper display across sleep)
        update_active_telemetry_ui(temp_k, rh_pct, vbat_mv, battery_pct, time_str, net_status_str);

        // Debug Bench Testing Guard: Keep awake continuously to evaluate screen, touch, sensors
        if (s_debug_mode) {
            ESP_LOGI(TAG, "[DEBUG MODE] Dashboard refreshed. Polling again in 3s (Deep sleep bypassed)...");
            vTaskDelay(pdMS_TO_TICKS(3000));
            continue;
        }

        // Network Failure Guard: Do NOT sleep if network failed so user can see status
        if (s_network_failed) {
            ESP_LOGW(TAG, "Network connection failure: Keeping device awake for diagnostic/pairing");
            vTaskDelay(pdMS_TO_TICKS(5000));
            // Retry Wi-Fi
            if (bsp_wifi_connect_from_nvs(4000) == ESP_OK) {
                s_network_failed = false;
            }
            continue;
        }

        // Enter Ultra-Low Power Deep Sleep (Power rails gated, display in sleep, screen image retained)
        int sleep_sec = (shared->sleep_interval_sec > 0) ? shared->sleep_interval_sec : 60;
        bsp_power_enter_deep_sleep(sleep_sec);
    }

    vTaskDelete(NULL);
}

/* =========================================================================
 * Master Application Entry Point
 * ========================================================================= */
extern "C" void app_main(void) 
{
    // 1. Increment boot counter (persists in RTC memory)
    s_rtc_boot_count++;

    // 2. Master Hardware Bringup (LVGL & EPD on Core 1)
    ESP_ERROR_CHECK(bsp_board_init());

    // 3. Power-Up Debug Mode Check: BOOT key held down at power-up OR Kconfig bypass flag
    #if defined(CONFIG_DEMO_DEBUG_BYPASS_PROV) && CONFIG_DEMO_DEBUG_BYPASS_PROV
    bool debug_flag_set = true;
    #else
    bool debug_flag_set = false;
    #endif

    if (debug_flag_set || (bsp_button_is_pressed(BSP_BUTTON_BOOT)) || (gpio_get_level(BSP_PIN_BUTTON_BOOT) == 0)) {
        s_debug_mode = true;
        s_rtc_is_claimed = true;
        ESP_LOGW(TAG, "****************************************************************");
        ESP_LOGW(TAG, "* [DEBUG MODE ENGAGED]                                         *");
        ESP_LOGW(TAG, "* BOOT key active at power-up -> Bypassing Provisioning/Claim  *");
        ESP_LOGW(TAG, "* Running Live Telemetry Dashboard & Continuous Bench Polling  *");
        ESP_LOGW(TAG, "****************************************************************");
    }

    // 3. Fetch Unique Device Name & Generate Cryptographic Claiming Key
    bsp_get_device_name(s_device_name, sizeof(s_device_name));
    app_claiming_get_active_key(s_claim_key, sizeof(s_claim_key));

    // 4. Register Buttons
    bsp_button_register_cb(BSP_BUTTON_BOOT,  BSP_BUTTON_EVENT_SINGLE_CLICK, on_boot_button,  NULL);
    bsp_button_register_cb(BSP_BUTTON_BOOT,  BSP_BUTTON_EVENT_LONG_PRESS,   on_boot_button,  NULL);
    bsp_button_register_cb(BSP_BUTTON_POWER, BSP_BUTTON_EVENT_SINGLE_CLICK, on_power_button, NULL);

    // 5. Mount Flash Partition for Zero-Copy Bitmap Assets (Space Cat on power-off)
    bsp_assets_init("storage", 'S', MMAP_STORAGE_FILES, MMAP_STORAGE_CHECKSUM);

    // 6. Play Chime only on initial cold boot (not timer wakeups)
    if (s_rtc_boot_count == 1) {
        play_audio_chime();
    }

    // 7. Spawn Networking, Telemetry & Deep Sleep Manager pinned to Core 0
    xTaskCreatePinnedToCore(
        network_telemetry_task,
        "net_telemetry",
        8192,
        NULL,
        3,
        NULL,
        0 // Pinned to Core 0
    );
}
