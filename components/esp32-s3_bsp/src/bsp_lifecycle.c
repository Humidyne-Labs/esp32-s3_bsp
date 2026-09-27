/**
 * @file bsp_lifecycle.c
 * @brief High-Level Application Lifecycle, Sleep/Wake Dispatcher & State Engine
 * 
 * Hardware Target:
 *  - Microcontroller: Espressif Systems ESP32-S3-PICO-1-N8R8
 *  - Target Board: Waveshare ESP32-S3 ePaper 1.54 V2
 * 
 * @attribution
 * - BSP Architecture: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_sleep.h"
#include "bsp/pinout.h"
#include "bsp/bsp.h"
#include "bsp/bsp_lifecycle.h"

static const char *TAG = "bsp_lifecycle";

static bsp_app_lifecycle_t s_active_lifecycle = {0};
static bsp_wake_context_t  s_current_context  = {0};
static bool                s_context_valid    = false;

esp_err_t bsp_lifecycle_get_context(bsp_wake_context_t *ctx)
{
    if (ctx == NULL || !s_context_valid) {
        return ESP_ERR_INVALID_STATE;
    }
    *ctx = s_current_context;
    return ESP_OK;
}

esp_err_t bsp_lifecycle_set_stage(uint8_t stage)
{
    bsp_rtc_state_t *rtc_st = bsp_rtc_mem_get_state();
    if (rtc_st == NULL || rtc_st->magic != BSP_RTC_MEM_MAGIC) {
        return ESP_ERR_INVALID_STATE;
    }
    rtc_st->scratchpad[0] = stage;
    s_current_context.app_stage = stage;
    return ESP_OK;
}

uint8_t bsp_lifecycle_get_stage(void)
{
    bsp_rtc_state_t *rtc_st = bsp_rtc_mem_get_state();
    if (rtc_st && rtc_st->magic == BSP_RTC_MEM_MAGIC) {
        return rtc_st->scratchpad[0];
    }
    return 0;
}

esp_err_t bsp_lifecycle_save_state(const void *data, size_t len)
{
    if (data == NULL || len == 0 || len > 31) {
        return ESP_ERR_INVALID_ARG;
    }
    bsp_rtc_state_t *rtc_st = bsp_rtc_mem_get_state();
    if (rtc_st == NULL || rtc_st->magic != BSP_RTC_MEM_MAGIC) {
        return ESP_ERR_INVALID_STATE;
    }
    memcpy(&rtc_st->scratchpad[1], data, len);
    return ESP_OK;
}

esp_err_t bsp_lifecycle_load_state(void *out_data, size_t len)
{
    if (out_data == NULL || len == 0 || len > 31) {
        return ESP_ERR_INVALID_ARG;
    }
    bsp_rtc_state_t *rtc_st = bsp_rtc_mem_get_state();
    if (rtc_st == NULL || rtc_st->magic != BSP_RTC_MEM_MAGIC) {
        return ESP_ERR_INVALID_STATE;
    }
    memcpy(out_data, &rtc_st->scratchpad[1], len);
    return ESP_OK;
}

esp_err_t bsp_lifecycle_enter_sleep(const bsp_sleep_config_t *config)
{
    bsp_sleep_config_t cfg = (config != NULL) ? *config : (bsp_sleep_config_t)BSP_SLEEP_CONFIG_DEFAULT();

    // Trigger visual/audio stand-down cues if registered
    bsp_trigger_splash(BSP_SPLASH_SLEEP);
    bsp_trigger_chime(BSP_CHIME_SLEEP);

    if (s_active_lifecycle.on_before_sleep != NULL) {
        ESP_LOGD(TAG, "Invoking registered on_before_sleep lifecycle hook...");
        s_active_lifecycle.on_before_sleep(cfg.mode, cfg.duration_sec, s_active_lifecycle.user_data);
    }

    return bsp_enter_sleep(&cfg);
}

esp_err_t bsp_app_start(const bsp_app_lifecycle_t *lifecycle)
{
    if (lifecycle != NULL) {
        s_active_lifecycle = *lifecycle;
    }

    // 1. Initialize RTC Slow Memory persistence engine
    bsp_rtc_mem_init();
    bsp_rtc_state_t *rtc_st = bsp_rtc_mem_get_state();

    esp_reset_reason_t       reset_reason = bsp_get_reset_reason();
    esp_sleep_wakeup_cause_t wake_cause   = bsp_get_wakeup_cause();
    bsp_init_mode_t          init_mode    = bsp_get_recommended_init_mode();

    // 2. Populate structured Wake Context
    memset(&s_current_context, 0, sizeof(s_current_context));
    s_current_context.reset_reason       = reset_reason;
    s_current_context.wake_cause         = wake_cause;
    s_current_context.init_mode          = init_mode;
    s_current_context.user_data          = s_active_lifecycle.user_data;

    if (rtc_st && rtc_st->magic == BSP_RTC_MEM_MAGIC) {
        s_current_context.boot_count         = rtc_st->boot_count;
        s_current_context.deep_sleep_count   = rtc_st->deep_sleep_count;
        s_current_context.light_sleep_count  = rtc_st->light_sleep_count;
        s_current_context.sleep_duration_sec = rtc_st->last_sleep_duration_sec;
        s_current_context.app_stage          = rtc_st->scratchpad[0];
    }

    if (wake_cause == ESP_SLEEP_WAKEUP_EXT1) {
        uint64_t ext1_mask = esp_sleep_get_ext1_wakeup_status();
        s_current_context.ext1_wakeup_pins = ext1_mask;
        if (ext1_mask & (1ULL << BSP_PIN_BUTTON_BOOT)) {
            s_current_context.woke_from_button = true;
            s_current_context.wake_button      = BSP_BUTTON_BOOT;
        } else if (ext1_mask & (1ULL << BSP_PIN_BUTTON_POWER)) {
            s_current_context.woke_from_button = true;
            s_current_context.wake_button      = BSP_BUTTON_POWER;
        }
    }

    s_context_valid = true;

    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "  BSP Lifecycle Engine: %s Boot (Count: %lu)",
             (reset_reason == ESP_RST_DEEPSLEEP) ? "Wake" : "Cold",
             (unsigned long)s_current_context.boot_count);
    ESP_LOGI(TAG, "  Reset: %d, Cause: %d, Mode: %d, Stage: %u",
             (int)reset_reason, (int)wake_cause, (int)init_mode,
             s_current_context.app_stage);
    ESP_LOGI(TAG, "==================================================");

    // 3. Initialize hardware profile
    esp_err_t ret = bsp_init_mode(init_mode);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize hardware profile (%d): %s", (int)init_mode, bsp_err_to_name(ret));
        return ret;
    }

    // 4. Dispatch to application lifecycle callbacks & splash
    if (reset_reason == ESP_RST_DEEPSLEEP) {
        // Automatically clear PCF85063A countdown timer and alarm flags in BSP
        bsp_rtc_clear_countdown_timer();
        bsp_rtc_get_and_clear_interrupts(NULL, NULL);

        if (s_active_lifecycle.on_wake != NULL) {
            s_active_lifecycle.on_wake(&s_current_context, s_active_lifecycle.user_data);
        } else {
            ESP_LOGW(TAG, "No on_wake callback registered; system idle");
        }
    } else {
        // Cold boot: trigger boot splash & chime hooks if registered
        bsp_trigger_splash(BSP_SPLASH_BOOT);
        bsp_trigger_chime(BSP_CHIME_BOOT);

        if (s_active_lifecycle.on_cold_boot != NULL) {
            s_active_lifecycle.on_cold_boot(s_active_lifecycle.user_data);
        } else {
            ESP_LOGW(TAG, "No on_cold_boot callback registered; system idle");
        }
    }

    return ESP_OK;
}
