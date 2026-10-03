/**
 * @file main.cpp
 * @brief Production Boilerplate Template for Humid1 OS & ESP32-S3 BSP
 *
 * Demonstrates event-driven application lifecycle management dictated by bsp_lifecycle.c.
 * Hardware initialization, reset reason evaluation, wake context dispatching, and sleep/shutdown
 * hooks are handled automatically by bsp_app_start().
 *
 * @version 1.0.0
 * @attribution
 * - Architecture & Development: HUMIDYNE LABS / Humiditron
 * - AI Systems Co-Developer: Gemini (Google DeepMind)
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"
#include "bsp/bsp.h"

static const char *TAG = "boilerplate";

static lv_obj_t *lbl_header = NULL;
static lv_obj_t *lbl_batt   = NULL;
static lv_obj_t *lbl_env    = NULL;
static lv_obj_t *lbl_rtc    = NULL;
static lv_obj_t *lbl_status = NULL;

static void boilerplate_ui_init(void)
{
    if (bsp_lvgl_lock()) {
        lv_obj_t *scr = lv_screen_active();
        lv_obj_set_style_bg_color(scr, lv_color_white(), 0);

        lbl_header = lv_label_create(scr);
        lv_label_set_text(lbl_header, "HUMID1 OS - BOILERPLATE");
        lv_obj_align(lbl_header, LV_ALIGN_TOP_MID, 0, 10);

        lbl_batt = lv_label_create(scr);
        lv_label_set_text(lbl_batt, "Batt: --.- V (--%)");
        lv_obj_align(lbl_batt, LV_ALIGN_TOP_LEFT, 10, 45);

        lbl_env = lv_label_create(scr);
        lv_label_set_text(lbl_env, "Temp: --.- C | RH: --%");
        lv_obj_align(lbl_env, LV_ALIGN_TOP_LEFT, 10, 75);

        lbl_rtc = lv_label_create(scr);
        lv_label_set_text(lbl_rtc, "RTC: --:--:--");
        lv_obj_align(lbl_rtc, LV_ALIGN_TOP_LEFT, 10, 105);

        lbl_status = lv_label_create(scr);
        lv_label_set_text(lbl_status, "Status: RUNNING");
        lv_obj_align(lbl_status, LV_ALIGN_BOTTOM_LEFT, 10, -15);

        bsp_lvgl_unlock();
    }
}

static void boilerplate_ui_update(float batt_v, uint8_t batt_pct, float temp_c, float rh, const char *rtc_str)
{
    if (bsp_lvgl_lock()) {
        if (lbl_batt) {
            lv_label_set_text_fmt(lbl_batt, "Batt: %.2f V (%d%%)", (double)batt_v, batt_pct);
        }
        if (lbl_env) {
            lv_label_set_text_fmt(lbl_env, "Temp: %.1f C | RH: %.1f%%", (double)temp_c, (double)rh);
        }
        if (lbl_rtc && rtc_str) {
            lv_label_set_text_fmt(lbl_rtc, "RTC: %s", rtc_str);
        }
        bsp_lvgl_unlock();
    }
}

static void on_power_click(bsp_button_t btn, bsp_button_event_t evt, void *user_data)
{
    (void)btn;
    (void)evt;
    (void)user_data;
    ESP_LOGI(TAG, "Power button clicked -> Flushing display...");
    bsp_display_flush();
}

static void on_power_long_press(bsp_button_t btn, bsp_button_event_t evt, void *user_data)
{
    (void)btn;
    (void)evt;
    (void)user_data;
    ESP_LOGI(TAG, "Power button long-pressed -> Requesting power off...");
    bsp_power_off();
}

static void on_cold_boot(void *user_data)
{
    (void)user_data;
    ESP_LOGI(TAG, "Lifecycle Callback: Cold Boot initialized.");

    bsp_button_register_cb(BSP_BUTTON_POWER, BSP_BUTTON_EVENT_SINGLE_CLICK, on_power_click, NULL);
    bsp_button_register_cb(BSP_BUTTON_POWER, BSP_BUTTON_EVENT_LONG_PRESS, on_power_long_press, NULL);

    boilerplate_ui_init();

    ESP_LOGI(TAG, "Entering main telemetry sampling loop...");
    while (1) {
        bsp_shtc3_data_t sensor_data;
        esp_err_t s_err = bsp_shtc3_read(&sensor_data);

        uint32_t mv = 0;
        bsp_battery_get_voltage(&mv, NULL);
        float batt_v = (float)mv / 1000.0f;
        uint8_t batt_pct = bsp_battery_get_percentage();

        char time_str[32] = {0};
        bsp_time_get_formatted(BSP_TIME_FMT_24H_SEC, time_str, sizeof(time_str));

        float temp_c = (s_err == ESP_OK && sensor_data.valid) ? sensor_data.temperature_c : 0.0f;
        float rh     = (s_err == ESP_OK && sensor_data.valid) ? sensor_data.humidity_percent : 0.0f;

        boilerplate_ui_update(batt_v, batt_pct, temp_c, rh, time_str);

        bsp_delay_ms(5000);
    }
}

static void on_wake(const bsp_wake_context_t *ctx, void *user_data)
{
    (void)user_data;
    ESP_LOGI(TAG, "Lifecycle Callback: Resumed from sleep (cause: %d, boot count: %lu).",
             (int)ctx->wake_cause, (unsigned long)ctx->boot_count);
}

static void on_shutdown(void *user_data)
{
    (void)user_data;
    ESP_LOGI(TAG, "Lifecycle Callback: System shutting down.");
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Starting Humid1 OS Production Boilerplate v1.0.0...");

    bsp_app_lifecycle_t lifecycle = {
        .on_cold_boot    = on_cold_boot,
        .on_wake         = on_wake,
        .on_before_sleep = NULL,
        .on_shutdown     = on_shutdown,
        .user_data       = NULL,
    };

    ESP_ERROR_CHECK(bsp_app_start(&lifecycle));
}
