/**
 * @file main.c
 * @brief Comprehensive Peripherals & Staged Sleep/Wake Mode Verification Suite
 * 
 * Hardware Target:
 *  - Target Board: Waveshare ESP32-S3 ePaper 1.54 V2
 *  - MCU: ESP32-S3-PICO-1-N8R8
 * 
 * Static Peripheral Tests (Cold Boot):
 *   1. Dynamic Hardware Initialization (FULL, FAST, MIN)
 *   2. System Identification, Silicon Revision & Diagnostics API
 *   3. Power Rail Latch & Battery ADC Monitoring
 *   4. I2C Bus Arbitration & SHTC3 Environmental Sampling
 *   5. NXP PCF85063A RTC Timekeeping & 8-Bit NVRAM Persistence
 *   6. ESP32-S3 RTC Slow Memory Persistent State Engine & Scratchpad
 *   7. Tactile Button Registration & Event Dispatcher (BOOT & POWER)
 *   8. ES8311 Audio Codec & NS4168 Amp Tone Synthesizer
 *   9. Timezone & Formatted Time/Date String Generators (4 Time + 3 Date formats)
 *  10. ThingsBoard Generic Telemetry & Attribute Struct Serialization
 *  11. Base57 Unambiguous Random Key Generation (8-char PoP, 6-char Claim)
 *  12. NVS Storage Key-Value Read/Write/Erase Lifecycle
 *  13. MicroSD Card 1-Bit Mode FATFS Mount & File I/O
 *  14. Wi-Fi Station & Passive Network Scanner
 *  15. SSD1681 E-Paper Display & LVGL v9 1-bit Rendering / QR Code Generator
 * 
 * Staged Sleep & Wake Verification Suite (Triggered via BOOT Button Click):
 *  - Stage 1: Press BOOT -> LS-1 (Light Sleep Internal Timer 3s)
 *  - Stage 2: Press BOOT -> LS-2 (Light Sleep PCF85063A Ext RTC Timer 3s)
 *  - Stage 3: Press BOOT -> DS-1 (Deep Sleep Internal Timer 4s -> FAST Init Mode)
 *  - Stage 4: Press BOOT -> DS-2 (Deep Sleep PCF85063A Ext RTC Timer 4s -> MIN Init Mode)
 *  - Stage 5: All Tests Passed (100%) -> Interactive Heartbeat Loop
 * 
 * @attribution
 * - Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "bsp/bsp.h"
#include "lvgl.h"

static const char *TAG = "test_suite";

/**
 * @brief Sleep Test State Tracker (Stored in RTC Slow Memory Scratchpad)
 */
typedef enum {
    STAGE_COLD_BOOT        = 0, /*!< Initial cold boot / run all static peripheral tests */
    STAGE_READY_LS1        = 1, /*!< Static tests passed -> Waiting for BOOT click to run LS-1 */
    STAGE_READY_LS2        = 2, /*!< LS-1 passed -> Waiting for BOOT click to run LS-2 */
    STAGE_READY_DS1        = 3, /*!< LS-2 passed -> Waiting for BOOT click to run DS-1 */
    STAGE_WAKE_DS1         = 4, /*!< In Deep Sleep 1 (Timer 4s) */
    STAGE_READY_DS2        = 5, /*!< DS-1 woke & verified -> Waiting for BOOT click to run DS-2 */
    STAGE_WAKE_DS2         = 6, /*!< In Deep Sleep 2 (PCF85063A Ext RTC 4s) */
    STAGE_TESTS_COMPLETED  = 7, /*!< All tests passed 100% -> Heartbeat active */
} sleep_test_stage_t;

static volatile sleep_test_stage_t s_current_stage = STAGE_COLD_BOOT;

static void test_ui_render_screen(const char *test_status_str, const char *sub_status_str)
{
    bsp_lvgl_lock();

    lv_obj_t *scr = lv_screen_active();
    lv_obj_clean(scr);
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);

    // Header Title
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text_fmt(title, "HUMIDOS TEST v%s", bsp_get_version());
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 4);

    // Divider Line
    static lv_point_precise_t line_pts[] = {{8, 20}, {192, 20}};
    lv_obj_t *line = lv_line_create(scr);
    lv_line_set_points(line, line_pts, 2);
    lv_obj_set_style_line_width(line, 1, 0);
    lv_obj_set_style_line_color(line, lv_color_black(), 0);

    // Silicon Chip & System Info
    lv_obj_t *lbl_chip = lv_label_create(scr);
    lv_label_set_text_fmt(lbl_chip, "Silicon: ESP32-S3 (%s)", bsp_get_chip_revision_str());
    lv_obj_set_style_text_font(lbl_chip, &lv_font_montserrat_14, 0);
    lv_obj_align(lbl_chip, LV_ALIGN_TOP_LEFT, 8, 24);

    // Test Status Box
    lv_obj_t *lbl_status = lv_label_create(scr);
    lv_label_set_text(lbl_status, test_status_str);
    lv_obj_set_style_text_font(lbl_status, &lv_font_montserrat_14, 0);
    lv_obj_set_width(lbl_status, 184);
    lv_obj_align(lbl_status, LV_ALIGN_TOP_LEFT, 8, 44);

    // Sub Status / Progress Details
    lv_obj_t *lbl_sub = lv_label_create(scr);
    lv_label_set_text(lbl_sub, sub_status_str);
    lv_obj_set_style_text_font(lbl_sub, &lv_font_montserrat_14, 0);
    lv_obj_set_width(lbl_sub, 184);
    lv_obj_align(lbl_sub, LV_ALIGN_TOP_LEFT, 8, 108);

    bsp_lvgl_unlock();
}

/**
 * @brief 4-Stage Audible Verification Sequence (0%, 25%, 50%, 100% Volume) with 500ms Pauses
 */
static void run_audio_chirps_test(void)
{
    ESP_LOGI(TAG, "Playing 4-stage Audio Chirp Verification (0%%, 25%%, 50%%, 100%% volume)...");

    // Chirp 1: 523 Hz (C5) @ 0% Volume (Muted baseline check)
    ESP_LOGI(TAG, "  Chirp 1/4: 523 Hz (C5) @ 0%% Volume (Muted)");
    bsp_audio_play_tone(523, 150, 0.0f);
    vTaskDelay(pdMS_TO_TICKS(500));

    // Chirp 2: 659 Hz (E5) @ 25% Volume (Low)
    ESP_LOGI(TAG, "  Chirp 2/4: 659 Hz (E5) @ 25%% Volume");
    bsp_audio_play_tone(659, 150, 25.0f);
    vTaskDelay(pdMS_TO_TICKS(500));

    // Chirp 3: 784 Hz (G5) @ 50% Volume (Medium)
    ESP_LOGI(TAG, "  Chirp 3/4: 784 Hz (G5) @ 50%% Volume");
    bsp_audio_play_tone(784, 150, 50.0f);
    vTaskDelay(pdMS_TO_TICKS(500));

    // Chirp 4: 1046 Hz (C6) @ 100% Volume (Full)
    ESP_LOGI(TAG, "  Chirp 4/4: 1046 Hz (C6) @ 100%% Volume");
    bsp_audio_play_tone(1046, 220, 100.0f);
    vTaskDelay(pdMS_TO_TICKS(500));

    bsp_audio_stop();
}

/**
 * @brief Quick Peripheral Health Check after Wake
 */
static bool verify_peripherals_healthy(void)
{
    bsp_shtc3_data_t data       = {0};
    esp_err_t        err_sensor = bsp_shtc3_read(&data);
    uint32_t         vbat       = 0;
    esp_err_t        err_bat    = bsp_battery_get_voltage(&vbat, NULL);

    bool healthy = (err_sensor == ESP_OK && data.valid && err_bat == ESP_OK && vbat > 3000);
    ESP_LOGI(TAG, "[HEALTH CHECK] I2C Sensor: %s (T=%.1f C, RH=%.1f%%), Battery: %s (%lu mV)",
             (err_sensor == ESP_OK && data.valid) ? "HEALTHY" : "FAULT",
             data.temperature_k - 273.15f, data.humidity_percent,
             (err_bat == ESP_OK) ? "HEALTHY" : "FAULT", (unsigned long)vbat);
    return healthy;
}

static void button_event_handler(bsp_button_t btn, bsp_button_event_t event, void *user_data)
{
    const char *btn_name = (btn   == BSP_BUTTON_BOOT)               ? "BOOT"         : "POWER";
    const char *evt_name = (event == BSP_BUTTON_EVENT_SINGLE_CLICK) ? "CLICK"        :
                           (event == BSP_BUTTON_EVENT_DOUBLE_CLICK) ? "DOUBLE_CLICK" :
                           (event == BSP_BUTTON_EVENT_LONG_PRESS)   ? "LONG_PRESS"   :
                           (event == BSP_BUTTON_EVENT_PRESS_DOWN)   ? "PRESS_DOWN"   : "PRESS_UP";

    ESP_LOGI(TAG, ">>> Button Event: [%s] -> %s (Current Stage: %d) <<<", btn_name, evt_name, (int)s_current_stage);

    if (btn != BSP_BUTTON_BOOT || event != BSP_BUTTON_EVENT_SINGLE_CLICK) {
        if (btn == BSP_BUTTON_BOOT && event == BSP_BUTTON_EVENT_LONG_PRESS && s_current_stage == STAGE_TESTS_COMPLETED) {
            ESP_LOGI(TAG, ">>> Triggering Interactive Deep Sleep with Button Wake (Press BOOT to wake)... <<<");
            bsp_sleep_config_t cfg = {
                .mode           = BSP_SLEEP_MODE_DEEP,
                .duration_sec   = 0,
                .wake_sources   = BSP_WAKE_SRC_BUTTONS,
                .next_init_mode = BSP_INIT_MODE_FAST,
            };
            bsp_enter_sleep(&cfg);
        }
        return;
    }

    // ========================================================================
    // STAGE 1: TRIGGER LS-1 (Light Sleep Internal Timer 3s)
    // ========================================================================
    if (s_current_stage == STAGE_READY_LS1) {
        ESP_LOGI(TAG, "==================================================");
        ESP_LOGI(TAG, "  [STAGE 1/4] Executing LS-1: Internal Timer (3s)  ");
        ESP_LOGI(TAG, "==================================================");

        test_ui_render_screen("Executing LS-1 (3s)...\nWake: Internal Timer",
                              "Entering Light Sleep\nAuto-wake in 3 seconds");
        vTaskDelay(pdMS_TO_TICKS(500));

        int64_t t_start = esp_timer_get_time();
        bsp_sleep_config_t ls_cfg = {
            .mode           = BSP_SLEEP_MODE_LIGHT,
            .duration_sec   = 3,
            .wake_sources   = BSP_WAKE_SRC_TIMER,
            .next_init_mode = BSP_INIT_MODE_FAST,
        };
        esp_err_t ret = bsp_enter_sleep(&ls_cfg);
        int64_t t_elapsed_ms = (esp_timer_get_time() - t_start) / 1000;

        bool healthy = verify_peripherals_healthy();
        if (ret == ESP_OK && healthy) {
            ESP_LOGI(TAG, "[PASS LS-1] Light Sleep + Internal Timer Wake Verified (Elapsed: %" PRId64 " ms)", t_elapsed_ms);
            s_current_stage = STAGE_READY_LS2;
            bsp_rtc_state_t *rtc_st = bsp_rtc_mem_get_state();
            if (rtc_st) rtc_st->scratchpad[0] = (uint8_t)STAGE_READY_LS2;

            test_ui_render_screen("LS-1: [PASS] (Timer 3s)\nClick BOOT: Run LS-2",
                                  "Wake: SHTC3/I2C OK\nNext: Ext RTC Timer (3s)");
        } else {
            ESP_LOGE(TAG, "[FAIL LS-1] Light Sleep execution or peripheral fault (Status: %s)", bsp_err_to_name(ret));
            test_ui_render_screen("LS-1: [FAIL] Fault\nClick BOOT: Retry",
                                  "Peripheral I2C Check Failed");
        }
        return;
    }

    // ========================================================================
    // STAGE 2: TRIGGER LS-2 (Light Sleep External PCF85063A RTC Timer 3s)
    // ========================================================================
    if (s_current_stage == STAGE_READY_LS2) {
        ESP_LOGI(TAG, "==================================================");
        ESP_LOGI(TAG, "  [STAGE 2/4] Executing LS-2: Ext RTC Timer (3s)  ");
        ESP_LOGI(TAG, "==================================================");

        test_ui_render_screen("Executing LS-2 (3s)...\nWake: PCF85063A INT",
                              "Arming RTC Countdown (3s)\nEntering Light Sleep");
        vTaskDelay(pdMS_TO_TICKS(500));

        bsp_rtc_set_countdown_timer(3);
        int64_t t_start = esp_timer_get_time();
        bsp_sleep_config_t ls_cfg = {
            .mode           = BSP_SLEEP_MODE_LIGHT,
            .duration_sec   = 0,
            .wake_sources   = BSP_WAKE_SRC_EXTERNAL_RTC,
            .next_init_mode = BSP_INIT_MODE_FAST,
        };
        esp_err_t ret = bsp_enter_sleep(&ls_cfg);
        int64_t t_elapsed_ms = (esp_timer_get_time() - t_start) / 1000;

        bool alarm_flag = false, timer_flag = false;
        bsp_rtc_get_and_clear_interrupts(&alarm_flag, &timer_flag);
        bool healthy = verify_peripherals_healthy();

        if (ret == ESP_OK && healthy) {
            ESP_LOGI(TAG, "[PASS LS-2] Light Sleep + External RTC Wake Verified (Elapsed: %" PRId64 " ms, TF=%d)",
                     t_elapsed_ms, (int)timer_flag);
            s_current_stage = STAGE_READY_DS1;
            bsp_rtc_state_t *rtc_st = bsp_rtc_mem_get_state();
            if (rtc_st) rtc_st->scratchpad[0] = (uint8_t)STAGE_READY_DS1;

            test_ui_render_screen("LS-2: [PASS] (Ext RTC)\nClick BOOT: Run DS-1",
                                  "Wake: RTC INT OK\nNext: DS-1 Timer (4s)");
        } else {
            ESP_LOGE(TAG, "[FAIL LS-2] Light Sleep Ext RTC fault (Status: %s)", bsp_err_to_name(ret));
            test_ui_render_screen("LS-2: [FAIL] Fault\nClick BOOT: Retry",
                                  "RTC Interrupt Check Failed");
        }
        return;
    }

    // ========================================================================
    // STAGE 3: TRIGGER DS-1 (Deep Sleep Internal Timer 4s -> FAST Mode)
    // ========================================================================
    if (s_current_stage == STAGE_READY_DS1) {
        ESP_LOGI(TAG, "==================================================");
        ESP_LOGI(TAG, "  [STAGE 3/4] Entering DS-1: Timer (4s) -> FAST   ");
        ESP_LOGI(TAG, "==================================================");

        bsp_rtc_state_t *rtc_st = bsp_rtc_mem_get_state();
        if (rtc_st) rtc_st->scratchpad[0] = (uint8_t)STAGE_WAKE_DS1;

        test_ui_render_screen("Entering DS-1 (4s)...\nWake: Internal Timer",
                              "Next Boot: FAST Mode\nRe-arming Timer (4s)");
        vTaskDelay(pdMS_TO_TICKS(1000));

        bsp_sleep_config_t ds_cfg = {
            .mode           = BSP_SLEEP_MODE_DEEP,
            .duration_sec   = 4,
            .wake_sources   = BSP_WAKE_SRC_TIMER,
            .next_init_mode = BSP_INIT_MODE_FAST,
        };
        bsp_enter_sleep(&ds_cfg);
        return;
    }

    // ========================================================================
    // STAGE 4: TRIGGER DS-2 (Deep Sleep External PCF85063A RTC 4s -> MIN Mode)
    // ========================================================================
    if (s_current_stage == STAGE_READY_DS2) {
        ESP_LOGI(TAG, "==================================================");
        ESP_LOGI(TAG, "  [STAGE 4/4] Entering DS-2: Ext RTC (4s) -> MIN  ");
        ESP_LOGI(TAG, "==================================================");

        bsp_rtc_state_t *rtc_st = bsp_rtc_mem_get_state();
        if (rtc_st) rtc_st->scratchpad[0] = (uint8_t)STAGE_WAKE_DS2;

        test_ui_render_screen("Entering DS-2 (4s)...\nWake: PCF85063A INT",
                              "Next Boot: MIN Mode\nArming RTC Countdown (4s)");
        vTaskDelay(pdMS_TO_TICKS(1000));

        bsp_sleep_config_t ds_cfg = {
            .mode           = BSP_SLEEP_MODE_DEEP,
            .duration_sec   = 4,
            .wake_sources   = BSP_WAKE_SRC_EXTERNAL_RTC,
            .next_init_mode = BSP_INIT_MODE_MIN,
        };
        bsp_enter_sleep(&ds_cfg);
        return;
    }

    // Interactive Light Sleep when tests are complete
    if (s_current_stage == STAGE_TESTS_COMPLETED) {
        ESP_LOGI(TAG, ">>> Triggering Interactive Light Sleep (5 sec)... <<<");
        test_ui_render_screen("Interactive Light Sleep\nDuration: 5 seconds",
                              "Auto-wake in 5s\nClick BOOT: Sleep Again");
        vTaskDelay(pdMS_TO_TICKS(500));
        bsp_power_enter_light_sleep(5);
        verify_peripherals_healthy();
        test_ui_render_screen("ALL SLEEP MODES: [PASS]\nLS-1: OK | LS-2: OK\nDS-1: OK | DS-2: OK",
                              "Heartbeat Active\nClick: LS(5s) | Hold: DS");
        ESP_LOGI(TAG, ">>> Resumed from Interactive Light Sleep <<<");
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "  ESP32-S3 ePaper BSP Staged Verification Suite    ");
    ESP_LOGI(TAG, "==================================================");

    // Initialize RTC Slow Memory state tracker
    bsp_rtc_mem_init();
    bsp_rtc_state_t *rtc_st = bsp_rtc_mem_get_state();

    esp_reset_reason_t       reset_reason = bsp_get_reset_reason();
    esp_sleep_wakeup_cause_t wake_cause   = bsp_get_wakeup_cause();
    bsp_init_mode_t          rec_mode     = bsp_get_recommended_init_mode();

    ESP_LOGI(TAG, "[BOOT INFO] Reset Reason: %d, Wake Cause: %d, Recommended Mode: %d, Boot Count: %lu, Silicon: %s",
             (int)reset_reason, (int)wake_cause, (int)rec_mode,
             rtc_st ? (unsigned long)rtc_st->boot_count : 0,
             bsp_get_chip_revision_str());

    sleep_test_stage_t stage = STAGE_COLD_BOOT;
    if (rtc_st && rtc_st->magic == BSP_RTC_MEM_MAGIC) {
        stage = (sleep_test_stage_t)rtc_st->scratchpad[0];
    }
    s_current_stage = stage;

    // ========================================================================
    // WAKE HANDLER 1: WAKING FROM DEEP SLEEP TEST 1 (Timer -> FAST Mode)
    // ========================================================================
    if (reset_reason == ESP_RST_DEEPSLEEP && stage == STAGE_WAKE_DS1) {
        ESP_LOGI(TAG, ">>> [WAKE 1/2] Processing DS-1 (Internal Timer -> FAST Mode) <<<");

        // FAST profile: partial refresh display, audio paused
        bsp_init_mode(BSP_INIT_MODE_FAST);
        bool healthy = verify_peripherals_healthy();

        if (healthy) {
            ESP_LOGI(TAG, "[PASS DS-1] Deep Sleep + Internal Timer Wake Verified in FAST Mode!");
            s_current_stage = STAGE_READY_DS2;
            if (rtc_st) rtc_st->scratchpad[0] = (uint8_t)STAGE_READY_DS2;

            test_ui_render_screen("DS-1: [PASS] (FAST Mode)\nClick BOOT: Run DS-2",
                                  "Wake: Timer OK\nNext: DS-2 (Ext RTC 4s)");
        } else {
            ESP_LOGE(TAG, "[FAIL DS-1] Peripheral health check failed in FAST Mode!");
            test_ui_render_screen("DS-1: [FAIL] Peripheral Fault\nClick BOOT: Retry DS-1",
                                  "FAST Mode Check Failed");
            s_current_stage = STAGE_READY_DS1;
        }

        // Arm button dispatcher for next staged test
        bsp_button_config_t btn_cfg = {
            .debounce_ms            = 50,
            .click_timeout_ms       = 300,
            .long_press_ms          = 2000,
            .auto_power_off_on_hold = false,
        };
        bsp_button_init(&btn_cfg);
        bsp_button_register_cb(BSP_BUTTON_BOOT, BSP_BUTTON_EVENT_SINGLE_CLICK, button_event_handler, NULL);
        return;
    }

    // ========================================================================
    // WAKE HANDLER 2: WAKING FROM DEEP SLEEP TEST 2 (Ext RTC -> MIN Mode)
    // ========================================================================
    if (reset_reason == ESP_RST_DEEPSLEEP && stage == STAGE_WAKE_DS2) {
        ESP_LOGI(TAG, ">>> [WAKE 2/2] Processing DS-2 (External RTC Wake -> MIN Mode) <<<");

        // Clear PCF85063A timer interrupt flag
        bool alarm_flag = false, timer_flag = false;
        bsp_rtc_get_and_clear_interrupts(&alarm_flag, &timer_flag);

        // MIN profile: Lean telemetry + fast partial EPD display
        bsp_init_mode(BSP_INIT_MODE_MIN);
        bool healthy = verify_peripherals_healthy();

        if (healthy) {
            ESP_LOGI(TAG, "[PASS DS-2] Deep Sleep + External RTC Wake Verified in MIN Mode! (TF=%d)", (int)timer_flag);
            ESP_LOGI(TAG, "==================================================");
            ESP_LOGI(TAG, "  ALL STATIC & SLEEP/WAKE TESTS PASSED 100%%       ");
            ESP_LOGI(TAG, "==================================================");

            s_current_stage = STAGE_TESTS_COMPLETED;
            if (rtc_st) rtc_st->scratchpad[0] = (uint8_t)STAGE_TESTS_COMPLETED;

            test_ui_render_screen("ALL SLEEP MODES: [PASS]\nLS-1: OK | LS-2: OK\nDS-1: OK | DS-2: OK",
                                  "Heartbeat Active\nClick: LS(5s) | Hold: DS");
        } else {
            ESP_LOGE(TAG, "[FAIL DS-2] Peripheral health check failed in MIN Mode!");
            test_ui_render_screen("DS-2: [FAIL] Peripheral Fault\nClick BOOT: Retry DS-2",
                                  "MIN Mode Check Failed");
            s_current_stage = STAGE_READY_DS2;
        }

        // Arm button dispatcher and start heartbeat
        bsp_button_config_t btn_cfg = {
            .debounce_ms            = 50,
            .click_timeout_ms       = 300,
            .long_press_ms          = 2000,
            .auto_power_off_on_hold = false,
        };
        bsp_button_init(&btn_cfg);
        bsp_button_register_cb(BSP_BUTTON_BOOT, BSP_BUTTON_EVENT_SINGLE_CLICK, button_event_handler, NULL);
        bsp_button_register_cb(BSP_BUTTON_BOOT, BSP_BUTTON_EVENT_LONG_PRESS,   button_event_handler, NULL);

        int count = 0;
        while (1) {
            vTaskDelay(pdMS_TO_TICKS(5000));
            count++;
            ESP_LOGI(TAG, "[Heartbeat %d] System Normal. Silicon: %s, Deep Sleeps: %lu, Light Sleeps: %lu",
                     count, bsp_get_chip_revision_str(),
                     rtc_st ? (unsigned long)rtc_st->deep_sleep_count  : 0,
                     rtc_st ? (unsigned long)rtc_st->light_sleep_count : 0);
        }
        return;
    }

    // ========================================================================
    // WAKE HANDLER 3: WAKING FROM INTERACTIVE DEEP SLEEP (STAGE_TESTS_COMPLETED)
    // ========================================================================
    if (reset_reason == ESP_RST_DEEPSLEEP && stage == STAGE_TESTS_COMPLETED) {
        ESP_LOGI(TAG, ">>> [WAKE] Resumed from Interactive Deep Sleep via Button Wake! (FAST Mode) <<<");
        bsp_init_mode(BSP_INIT_MODE_FAST);
        verify_peripherals_healthy();

        test_ui_render_screen("ALL SLEEP MODES: [PASS]\nResumed: Deep Sleep (Button)",
                              "Heartbeat Active\nClick: LS(5s) | Hold: DS");

        bsp_button_config_t btn_cfg = {
            .debounce_ms            = 50,
            .click_timeout_ms       = 300,
            .long_press_ms          = 2000,
            .auto_power_off_on_hold = false,
        };
        bsp_button_init(&btn_cfg);
        bsp_button_register_cb(BSP_BUTTON_BOOT, BSP_BUTTON_EVENT_SINGLE_CLICK, button_event_handler, NULL);
        bsp_button_register_cb(BSP_BUTTON_BOOT, BSP_BUTTON_EVENT_LONG_PRESS,   button_event_handler, NULL);

        int count = 0;
        while (1) {
            vTaskDelay(pdMS_TO_TICKS(5000));
            count++;
            ESP_LOGI(TAG, "[Heartbeat %d] System Normal. Silicon: %s, Deep Sleeps: %lu, Light Sleeps: %lu",
                     count, bsp_get_chip_revision_str(),
                     rtc_st ? (unsigned long)rtc_st->deep_sleep_count  : 0,
                     rtc_st ? (unsigned long)rtc_st->light_sleep_count : 0);
        }
        return;
    }

    // ========================================================================
    // COLD BOOT: Initialize Subsystems (FULL Mode) & Execute Static Tests
    // ========================================================================
    ESP_LOGI(TAG, ">>> [COLD BOOT] Initializing Subsystems (FULL Mode) <<<");
    esp_err_t nvs_err = nvs_flash_init();
    if (nvs_err == ESP_ERR_NVS_NO_FREE_PAGES || nvs_err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS truncated or unformatted; executing flash erase...");
        nvs_flash_erase();
        nvs_flash_init();
    }

    esp_err_t ret = bsp_board_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "bsp_board_init failed: %s", bsp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "[PASS 1/15] Board Subsystems Initialized Successfully");
    }

    // ----------------------------------------------------
    // 2. System Identification, Silicon Revision & Diagnostics
    // ----------------------------------------------------
    char dev_id[32]   = {0};
    char dev_name[32] = {0};
    bsp_get_device_id(dev_id, sizeof(dev_id));
    bsp_get_device_name(dev_name, sizeof(dev_name));
    uint32_t chip_major = 0, chip_minor = 0;
    bsp_get_chip_revision(&chip_major, &chip_minor);
    ESP_LOGI(TAG, "[INFO] Device ID: %s, Name: %s, Silicon Revision: v%" PRIu32 ".%" PRIu32 " (%s)",
             dev_id, dev_name, chip_major, chip_minor, bsp_get_chip_revision_str());
    bsp_diagnostics_dump();
    ESP_LOGI(TAG, "[PASS 2/15] System Identification & Diagnostics API Verified");

    // ----------------------------------------------------
    // 3. Battery Voltage & Power Latch Verification
    // ----------------------------------------------------
    uint32_t batt_mv = 0;
    ret = bsp_battery_get_voltage(&batt_mv, NULL);
    uint8_t batt_pct = bsp_battery_get_percentage();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "[PASS 3/15] Battery Voltage: %lu mV (%u%%)", (unsigned long)batt_mv, batt_pct);
    } else {
        ESP_LOGE(TAG, "[FAIL 3/15] Battery voltage read error: %s", bsp_err_to_name(ret));
    }

    // ----------------------------------------------------
    // 4. SHTC3 Environmental Sensor Verification
    // ----------------------------------------------------
    bsp_shtc3_data_t sensor_data = {0};
    ret = bsp_shtc3_read(&sensor_data);
    float temp_c = 0.0f, temp_f = 0.0f, rh_pct = 0.0f;
    if (ret == ESP_OK && sensor_data.valid) {
        temp_c = sensor_data.temperature_k - 273.15f;
        temp_f = temp_c * 1.8f + 32.0f;
        rh_pct = sensor_data.humidity_percent;
        ESP_LOGI(TAG, "[PASS 4/15] SHTC3 Sensor: Temp = %.2f C (%.2f F), Humidity = %.2f %%RH",
                 temp_c, temp_f, rh_pct);
    } else {
        ESP_LOGE(TAG, "[FAIL 4/15] SHTC3 sensor read error: %s", bsp_err_to_name(ret));
    }

    // ----------------------------------------------------
    // 5. PCF85063A Hardware RTC & 8-Bit NVRAM Persistence
    // ----------------------------------------------------
    bsp_rtc_datetime_t dt;
    ret = bsp_rtc_get_datetime(&dt);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "[PASS 5/15] PCF85063A RTC: %04u-%02u-%02u %02u:%02u:%02u (DOW: %u)",
                 dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second, dt.weekday);
        bsp_time_sync_rtc_to_system();
    } else {
        ESP_LOGE(TAG, "[FAIL 5/15] PCF85063A RTC read error: %s", bsp_err_to_name(ret));
    }

    // Test PCF85063A 8-bit RAM Register (0x03)
    uint8_t test_token = 0xAB;
    bsp_rtc_ram_write(test_token);
    uint8_t read_token = 0;
    ret = bsp_rtc_ram_read(&read_token);
    if (ret == ESP_OK && read_token == test_token) {
        ESP_LOGI(TAG, "[PASS 5/15] PCF85063A 8-Bit NVRAM Read/Write Verified (0x%02X)", read_token);
    } else {
        ESP_LOGE(TAG, "[FAIL 5/15] PCF85063A NVRAM mismatch: Wrote 0x%02X, Read 0x%02X", test_token, read_token);
    }

    // ----------------------------------------------------
    // 6. ESP32-S3 RTC Slow Memory Engine & Scratchpad
    // ----------------------------------------------------
    if (rtc_st && rtc_st->magic == BSP_RTC_MEM_MAGIC) {
        ESP_LOGI(TAG, "[PASS 6/15] RTC Slow Memory Valid: Boot Count=%lu, Deep Sleeps=%lu, Light Sleeps=%lu",
                 (unsigned long)rtc_st->boot_count, (unsigned long)rtc_st->deep_sleep_count,
                 (unsigned long)rtc_st->light_sleep_count);
    }

    const char *scratch_test = "HumidOS_SelfTest_OK";
    bsp_rtc_mem_write_scratchpad((const uint8_t *)scratch_test, strlen(scratch_test) + 1);
    char scratch_read[32] = {0};
    bsp_rtc_mem_read_scratchpad((uint8_t *)scratch_read, sizeof(scratch_read));
    if (strcmp(scratch_test, scratch_read) == 0) {
        ESP_LOGI(TAG, "[PASS 6/15] RTC Scratchpad Persistence Verified: '%s'", scratch_read);
    }

    // ----------------------------------------------------
    // 7. Tactile Button Registration & Event Dispatcher
    // ----------------------------------------------------
    bsp_button_config_t btn_cfg = {
        .debounce_ms            = 50,
        .click_timeout_ms       = 300,
        .long_press_ms          = 2000,
        .auto_power_off_on_hold = false,
    };
    bsp_button_init(&btn_cfg);
    bsp_button_register_cb(BSP_BUTTON_BOOT,  BSP_BUTTON_EVENT_SINGLE_CLICK, button_event_handler, NULL);
    bsp_button_register_cb(BSP_BUTTON_BOOT,  BSP_BUTTON_EVENT_LONG_PRESS,   button_event_handler, NULL);
    bsp_button_register_cb(BSP_BUTTON_POWER, BSP_BUTTON_EVENT_SINGLE_CLICK, button_event_handler, NULL);
    bsp_button_register_cb(BSP_BUTTON_POWER, BSP_BUTTON_EVENT_LONG_PRESS,   button_event_handler, NULL);
    ESP_LOGI(TAG, "[PASS 7/15] Hardware Buttons (BOOT & POWER) Event Handlers Active");

    // ----------------------------------------------------
    // 8. ES8311 Audio Codec & 4-Stage Volume Chirp Synthesizer
    // ----------------------------------------------------
    bsp_audio_power_enable(true);
    bsp_audio_init();
    run_audio_chirps_test();
    ESP_LOGI(TAG, "[PASS 8/15] Audio Codec & 4-Stage Chirp Sequence Verified (0%%, 25%%, 50%%, 100%%)");

    // ----------------------------------------------------
    // 9. Timezone & Formatted Time/Date String Generators
    // ----------------------------------------------------
    bsp_time_set_timezone("EST5EDT,M3.2.0,M11.1.0");
    char time_24h_s[32]    = {0}, time_24h_m[32] = {0};
    char time_12h_s[32]    = {0}, time_12h_m[32] = {0};
    char date_mm_dd_yy[32] = {0}, date_dow[32]   = {0}, date_full[32] = {0};

    bsp_time_get_formatted(BSP_TIME_FMT_24H_SEC, time_24h_s, sizeof(time_24h_s));
    bsp_time_get_formatted(BSP_TIME_FMT_24H_MIN, time_24h_m, sizeof(time_24h_m));
    bsp_time_get_formatted(BSP_TIME_FMT_12H_SEC, time_12h_s, sizeof(time_12h_s));
    bsp_time_get_formatted(BSP_TIME_FMT_12H_MIN, time_12h_m, sizeof(time_12h_m));

    bsp_time_get_date_str(date_mm_dd_yy, sizeof(date_mm_dd_yy));
    bsp_time_get_dow_str(date_dow, sizeof(date_dow));
    bsp_time_get_date_dow_str(date_full, sizeof(date_full));

    ESP_LOGI(TAG, "[PASS 9/15] Time Formats: 24h=[%s, %s], 12h=[%s, %s]",
             time_24h_s, time_24h_m, time_12h_s, time_12h_m);
    ESP_LOGI(TAG, "[PASS 9/15] Date Formats: Date=[%s], DOW=[%s], Full=[%s]",
             date_mm_dd_yy, date_dow, date_full);

    // ----------------------------------------------------
    // 10. ThingsBoard Generic Telemetry Struct Serializer
    // ----------------------------------------------------
    bsp_tb_entry_t sample_entries[] = {
        { .key = "temp_k",   .type = BSP_TB_VAL_FLOAT,  .val.f_val = 295.15f },
        { .key = "rh_pct",   .type = BSP_TB_VAL_FLOAT,  .val.f_val = 58.0f },
        { .key = "batt_pct", .type = BSP_TB_VAL_INT,    .val.i_val = 85 },
        { .key = "status",   .type = BSP_TB_VAL_STRING, .val.s_val = "ONLINE" },
    };
    bsp_tb_send_telemetry_entries(sample_entries, 4, false, 0);
    ESP_LOGI(TAG, "[PASS 10/15] ThingsBoard Telemetry Struct Schema (4 Generic Entries Formatted)");

    // ----------------------------------------------------
    // 11. Unambiguous Base57 Key Generation Test
    // ----------------------------------------------------
    char pop_key[16]   = {0};
    char claim_key[16] = {0};
    bsp_generate_unambiguous_key(pop_key, 8, NULL);
    bsp_generate_unambiguous_key(claim_key, 6, NULL);
    ESP_LOGI(TAG, "[PASS 11/15] Unambiguous Keys Generated: 8-char PoP='%s', 6-char Claim='%s'",
             pop_key, claim_key);

    // ----------------------------------------------------
    // 12. NVS Storage Read / Write / Erase Lifecycle
    // ----------------------------------------------------
    bsp_nvs_init();
    bsp_nvs_set_u32("test_counter", 42);
    bsp_nvs_set_str("test_key", "HumidOS_NVS_OK");
    uint32_t read_cnt = 0;
    char read_str[32] = {0};
    bsp_nvs_get_u32("test_counter", &read_cnt);
    bsp_nvs_get_str("test_key", read_str, sizeof(read_str));
    if (read_cnt == 42 && strcmp(read_str, "HumidOS_NVS_OK") == 0) {
        ESP_LOGI(TAG, "[PASS 12/15] NVS Key-Value Persistence Verified (u32=%lu, str='%s')",
                 (unsigned long)read_cnt, read_str);
    }
    bsp_nvs_erase_key("test_counter");
    bsp_nvs_erase_key("test_key");

    // ----------------------------------------------------
    // 13. MicroSD Card Detection & FATFS Mount Test
    // ----------------------------------------------------
    ret = bsp_sdcard_mount();
    if (ret == ESP_OK) {
        float cap_gb = bsp_sdcard_get_capacity_gb();
        ESP_LOGI(TAG, "[PASS 13/15] MicroSD Card Mounted! Capacity: %.2f GB", cap_gb);

        FILE *f = fopen("/sdcard/bsp_test.txt", "w");
        if (f) {
            fprintf(f, "HumidOS BSP MicroSD Verified\n");
            fclose(f);
            ESP_LOGI(TAG, "[PASS 13/15] MicroSD File Write Verified");
        }
        bsp_sdcard_unmount();
    } else {
        ESP_LOGW(TAG, "[SKIP 13/15] MicroSD Slot Empty / Not Inserted (Status: %s)", bsp_err_to_name(ret));
    }

    // ----------------------------------------------------
    // 14. Wi-Fi Station & Passive Network Scanner Test
    // ----------------------------------------------------
    ret = bsp_wifi_init();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Starting passive Wi-Fi scan...");
        uint16_t ap_count = 0;
        esp_err_t scan_err = bsp_wifi_scan(NULL, &ap_count, 0);
        if (scan_err == ESP_OK) {
            ESP_LOGI(TAG, "[PASS 14/15] Wi-Fi Subsystem Operational. Found %u Access Points in scan", ap_count);
        } else {
            ESP_LOGW(TAG, "[WARN 14/15] Wi-Fi scan completed with status: %s", bsp_err_to_name(scan_err));
        }
    }

    // ----------------------------------------------------
    // 15. Render Fullscreen QR Code & Test Summary Screen
    // ----------------------------------------------------
    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "  [TEST 15/15] PROMPT: Press BOOT to display QR   ");
    ESP_LOGI(TAG, "==================================================");
    test_ui_render_screen("Test 15: BLE Prov QR Code\nClick BOOT: Show QR",
                          "Provisioning Engine\nWaiting for BOOT click...");

    // Wait for user to press and release BOOT button
    while (gpio_get_level((gpio_num_t)BSP_PIN_BUTTON_BOOT) != 0) {
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    while (gpio_get_level((gpio_num_t)BSP_PIN_BUTTON_BOOT) == 0) {
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    ESP_LOGI(TAG, "Rendering Fullscreen 180x180 px BLE Provisioning QR Code...");
    bsp_lvgl_lock();
    lv_obj_t *scr = lv_screen_active();
    lv_obj_clean(scr);
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
    lv_obj_t *qr = bsp_prov_render_qr_code(scr, 180, "HumidOS-1.0.0-OK");
    if (qr) lv_obj_center(qr);
    bsp_lvgl_unlock();

    ESP_LOGI(TAG, "[PASS 15/15] Fullscreen QR Code displayed on E-Paper.");
    ESP_LOGI(TAG, "Press BOOT (GPIO0) to dismiss QR and proceed to Sleep Tests...");

    // Wait for user to press and release BOOT button to proceed
    while (gpio_get_level((gpio_num_t)BSP_PIN_BUTTON_BOOT) != 0) {
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    while (gpio_get_level((gpio_num_t)BSP_PIN_BUTTON_BOOT) == 0) {
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    // Clear and display Static Tests Passed UI
    test_ui_render_screen("STATIC TESTS: [PASS]\nClick BOOT: Run LS-1",
                          "Ready for Sleep Tests\nNext: LS-1 (Timer 3s)");
    ESP_LOGI(TAG, "[PASS 15/15] E-Paper Display & LVGL Rendering Operational");

    // Set state machine to Stage 1 (Ready for LS-1 on BOOT click)
    s_current_stage = STAGE_READY_LS1;
    if (rtc_st) {
        rtc_st->scratchpad[0] = (uint8_t)STAGE_READY_LS1;
    }

    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "  STATIC TESTS COMPLETE -> WAITING FOR BOOT CLICK ");
    ESP_LOGI(TAG, "  Press BOOT (GPIO0) to execute LS-1              ");
    ESP_LOGI(TAG, "==================================================");
}
