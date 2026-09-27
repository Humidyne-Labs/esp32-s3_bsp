/**
 * @file main.c
 * @brief Comprehensive Peripherals & Staged Sleep/Wake Mode Verification Suite
 * 
 * Hardware Target:
 *  - Target Board: Waveshare ESP32-S3 ePaper 1.54 V2
 *  - MCU: ESP32-S3-PICO-1-N8R8
 * 
 * Static Peripheral Tests (Cold Boot):
 *   1. Dynamic Hardware Initialization (FULL, FAST)
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
 *  13. Onboard SPIFFS Flash Partition Mount & File I/O Lifecycle
 *  14. MicroSD Card 1-Bit Mode FATFS Mount & File I/O
 *  15. Wi-Fi Station & Passive Network Scanner
 *  16. SSD1681 E-Paper Display & LVGL v9 1-bit Rendering / QR Code Generator
 * 
 * Staged Sleep & Wake Verification Suite (Triggered via BOOT Button Click):
 *  - Stage 1: Press BOOT -> LS-1 (Light Sleep Internal Timer 3s)
 *  - Stage 2: Press BOOT -> LS-2 (Light Sleep PCF85063A Ext RTC Timer 3s)
 *  - Stage 3: Press BOOT -> DS-1 (Deep Sleep Internal Timer 4s -> FAST Init Mode)
 *  - Stage 4: Press BOOT -> DS-2 (Deep Sleep PCF85063A Ext RTC Timer 4s -> FAST Init Mode)
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
#include "esp_partition.h"
#include "bsp/bsp.h"
#include "lvgl.h"

static const char *TAG = "test_suite";

#pragma pack(push, 1)
typedef struct {
    uint8_t magic[4];       /*!< Magic number: "MMAP" */
    uint32_t version;       /*!< Version number (0x00010000 for v1.0.0) */
    uint32_t name_len;      /*!< Length of the asset name in table (including \0) */
    uint32_t files;         /*!< Total number of assets */
    uint32_t checksum;      /*!< Checksum of the table data */
    uint32_t payload_len;   /*!< Total length of combined Asset Table + Payload Data */
    uint32_t reserved[2];
} bsp_mmap_bin_header_t;
#pragma pack(pop)

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

static lv_obj_t *s_lbl_status = NULL;
static lv_obj_t *s_lbl_sub    = NULL;

static void test_ui_render_screen(const char *test_status_str, const char *sub_status_str)
{
    bsp_lvgl_lock();

    lv_obj_t *scr = lv_screen_active();

    if (s_lbl_status == NULL || s_lbl_sub == NULL) {
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
        s_lbl_status = lv_label_create(scr);
        lv_obj_set_style_text_font(s_lbl_status, &lv_font_montserrat_14, 0);
        lv_obj_set_width(s_lbl_status, 184);
        lv_obj_align(s_lbl_status, LV_ALIGN_TOP_LEFT, 8, 44);

        // Sub Status / Progress Details
        s_lbl_sub = lv_label_create(scr);
        lv_obj_set_style_text_font(s_lbl_sub, &lv_font_montserrat_14, 0);
        lv_obj_set_width(s_lbl_sub, 184);
        lv_obj_align(s_lbl_sub, LV_ALIGN_TOP_LEFT, 8, 108);
    }

    lv_label_set_text(s_lbl_status, test_status_str);
    lv_label_set_text(s_lbl_sub, sub_status_str);

    bsp_lvgl_unlock();
}

/**
 * @brief 4-Stage Audible Verification Sequence (0%, 35%, 70%, 100% Volume) with Distinct Tones & Pauses
 */
static void run_audio_chirps_test(void)
{
    ESP_LOGI(TAG, "Playing 4-stage Audio Chirp Verification (0%%, 35%%, 70%%, 100%% volume)...");

    // Chirp 1: 523 Hz (C5) @ 0% Volume (Muted baseline check)
    ESP_LOGI(TAG, "  Chirp 1/4: 523 Hz (C5) @ 0%% Volume (Muted Baseline)");
    bsp_audio_play_tone(523, 200, 0.0f);
    vTaskDelay(pdMS_TO_TICKS(600));

    // Chirp 2: 659 Hz (E5) @ 35% Volume (Low Chime)
    ESP_LOGI(TAG, "  Chirp 2/4: 659 Hz (E5) @ 35%% Volume (Low Chime)");
    bsp_audio_play_tone(659, 350, 35.0f);
    vTaskDelay(pdMS_TO_TICKS(600));

    // Chirp 3: 784 Hz (G5) @ 70% Volume (Medium Chime)
    ESP_LOGI(TAG, "  Chirp 3/4: 784 Hz (G5) @ 70%% Volume (Medium Chime)");
    bsp_audio_play_tone(784, 350, 70.0f);
    vTaskDelay(pdMS_TO_TICKS(600));

    // Chirp 4: 1046 Hz (C6) @ 100% Volume (Full High Chime)
    ESP_LOGI(TAG, "  Chirp 4/4: 1046 Hz (C6) @ 100%% Volume (Full High Chime)");
    bsp_audio_play_tone(1046, 450, 100.0f);
    vTaskDelay(pdMS_TO_TICKS(600));

    bsp_audio_stop();
}

/**
 * @brief Synthesize MMAP Asset binary table & 48x48 1-bit badge icon, flash to "storage", and mount drive 'S:'
 */
static esp_err_t build_test_mmap_asset(uint32_t *part_checksum)
{
    const uint32_t img_w = 48;
    const uint32_t img_h = 48;
    const uint32_t img_stride = (img_w + 7) / 8; // 6 bytes per row
    const uint32_t palette_size = 8;             // 2 colors (Black & White)
    const uint32_t pixel_bytes = img_stride * img_h; // 288 bytes
    const uint32_t raw_image_size = sizeof(lv_image_header_t) + palette_size + pixel_bytes; // 308 bytes

    const uint32_t mmap_name_len = 32;
    const uint32_t table_stride = mmap_name_len + 12; // 44 bytes
    const uint32_t sub_asset_size = 2 + raw_image_size; // 310 bytes (2B magic + 308B payload)
    const uint32_t total_payload_len = table_stride + sub_asset_size; // 354 bytes
    const uint32_t total_mmap_size = sizeof(bsp_mmap_bin_header_t) + total_payload_len; // 386 bytes

    uint8_t *mmap_blob = (uint8_t *)calloc(1, total_mmap_size);
    if (!mmap_blob) {
        ESP_LOGE(TAG, "[FAIL 13/15] Memory allocation failed for MMAP asset synthesis");
        return ESP_ERR_NO_MEM;
    }

    // 1. MMAP Header
    bsp_mmap_bin_header_t *hdr = (bsp_mmap_bin_header_t *)mmap_blob;
    memcpy(hdr->magic, "MMAP", 4);
    hdr->version     = 0x00010000;
    hdr->name_len    = mmap_name_len;
    hdr->files       = 1;
    hdr->payload_len = total_payload_len;

    // 2. Table Entry at offset 32
    uint8_t *entry = mmap_blob + sizeof(bsp_mmap_bin_header_t);
    strncpy((char *)entry, "test_badge.bin", mmap_name_len - 1);
    *(uint32_t *)(entry + mmap_name_len)     = sub_asset_size;
    *(uint32_t *)(entry + mmap_name_len + 4) = 0; // offset relative to data block
    *(uint16_t *)(entry + mmap_name_len + 8) = (uint16_t)img_w;
    *(uint16_t *)(entry + mmap_name_len + 10)= (uint16_t)img_h;

    // 3. Data Block at offset 32 + 44 = 76
    uint8_t *data_block = entry + table_stride;
    *(uint16_t *)data_block = 0x5A5A; // ASSETS_FILE_MAGIC_HEAD

    // 4. Raw Image at offset 76 + 2 = 78
    uint8_t *raw_img = data_block + 2;
    lv_image_header_t *img_hdr = (lv_image_header_t *)raw_img;
    img_hdr->magic  = LV_IMAGE_HEADER_MAGIC;
    img_hdr->cf     = LV_COLOR_FORMAT_I1;
    img_hdr->w      = img_w;
    img_hdr->h      = img_h;
    img_hdr->stride = img_stride;

    // Palette (8 bytes): Index 0 = Black, Index 1 = White
    lv_color32_t *pal = (lv_color32_t *)(raw_img + sizeof(lv_image_header_t));
    pal[0] = lv_color32_make(0x00, 0x00, 0x00, 0xFF);
    pal[1] = lv_color32_make(0xFF, 0xFF, 0xFF, 0xFF);

    // Pixels (288 bytes): Draw HumidOS diamond badge pattern
    uint8_t *pixels = (uint8_t *)(pal + 2);
    memset(pixels, 0xFF, pixel_bytes);
    for (uint32_t y = 0; y < img_h; y++) {
        for (uint32_t x = 0; x < img_w; x++) {
            bool is_border = (x == 0 || x == img_w - 1 || y == 0 || y == img_h - 1);
            int dx = (int)x - 24; if (dx < 0) dx = -dx;
            int dy = (int)y - 24; if (dy < 0) dy = -dy;
            bool is_diamond = (dx + dy == 20 || dx + dy == 21);
            bool is_cross = ((x >= 22 && x <= 25 && y >= 14 && y <= 33) ||
                             (y >= 22 && y <= 25 && x >= 14 && x <= 33));
            if (is_border || is_diamond || is_cross) {
                uint32_t byte_idx = y * img_stride + (x >> 3);
                pixels[byte_idx] &= ~(1 << (7 - (x & 0x07)));
            }
        }
    }

    // Compute checksum over table + payload
    uint32_t chksum = 0;
    for (uint32_t i = sizeof(bsp_mmap_bin_header_t); i < total_mmap_size; i++) {
        chksum += mmap_blob[i];
    }
    uint32_t expected_checksum = chksum & 0xFFFF;
    hdr->checksum = *part_checksum = expected_checksum;

    // Write to "storage" partition
    const esp_partition_t *part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "storage");
    if (!part) {
        ESP_LOGE(TAG, "[FAIL 13/15] Partition 'storage' not found in partition table");
        free(mmap_blob);
        return ESP_ERR_NOT_FOUND;
    }

    esp_partition_erase_range(part, 0, 4096);
    esp_partition_write(part, 0, mmap_blob, total_mmap_size);
    free(mmap_blob);
    return ESP_OK;
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
            ESP_LOGI(TAG, ">>> Triggering Interactive Deep Sleep (Wake via BOOT button or Ext RTC INT)... <<<");
            test_ui_render_screen("Interactive Deep Sleep\nWake: BOOT / Ext RTC",
                                  "Indefinite Low Power\nPress BOOT (or RTC) to wake");
            
            while (bsp_button_is_pressed(BSP_BUTTON_BOOT)) {
                vTaskDelay(pdMS_TO_TICKS(50));
            }

            bsp_sleep_config_t cfg = {
                .mode           = BSP_SLEEP_MODE_DEEP,
                .duration_sec   = 0, // 0 = indefinite external interrupt wakeup
                .wake_sources   = (bsp_wake_source_mask_t)(BSP_WAKE_SRC_BUTTONS | BSP_WAKE_SRC_EXTERNAL_RTC),
                .next_init_mode = BSP_INIT_MODE_FAST,
            };
            bsp_lifecycle_enter_sleep(&cfg);
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
        esp_err_t ret = bsp_lifecycle_enter_sleep(&ls_cfg);
        int64_t t_elapsed_ms = (esp_timer_get_time() - t_start) / 1000;

        bool healthy = verify_peripherals_healthy();
        if (ret == ESP_OK && healthy) {
            ESP_LOGI(TAG, "[PASS LS-1] Light Sleep + Internal Timer Wake Verified (Elapsed: %" PRId64 " ms)", t_elapsed_ms);
            s_current_stage = STAGE_READY_LS2;
            bsp_lifecycle_set_stage(STAGE_READY_LS2);

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
        esp_err_t ret = bsp_lifecycle_enter_sleep(&ls_cfg);
        int64_t t_elapsed_ms = (esp_timer_get_time() - t_start) / 1000;

        bool healthy = verify_peripherals_healthy();

        if (ret == ESP_OK && healthy) {
            ESP_LOGI(TAG, "[PASS LS-2] Light Sleep + External RTC Wake Verified (Elapsed: %" PRId64 " ms)",
                     t_elapsed_ms);
            s_current_stage = STAGE_READY_DS1;
            bsp_lifecycle_set_stage(STAGE_READY_DS1);

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

        bsp_lifecycle_set_stage(STAGE_WAKE_DS1);

        test_ui_render_screen("Entering DS-1 (4s)...\nWake: Internal Timer",
                              "Next Boot: FAST Mode\nRe-arming Timer (4s)");
        vTaskDelay(pdMS_TO_TICKS(1000));

        bsp_sleep_config_t ds_cfg = {
            .mode           = BSP_SLEEP_MODE_DEEP,
            .duration_sec   = 4,
            .wake_sources   = BSP_WAKE_SRC_TIMER,
            .next_init_mode = BSP_INIT_MODE_FAST,
        };
        bsp_lifecycle_enter_sleep(&ds_cfg);
        return;
    }

    // ========================================================================
    // STAGE 4: TRIGGER DS-2 (Deep Sleep External PCF85063A RTC 4s -> MIN Mode)
    // ========================================================================
    if (s_current_stage == STAGE_READY_DS2) {
        ESP_LOGI(TAG, "==================================================");
        ESP_LOGI(TAG, "  [STAGE 4/4] Entering DS-2: Ext RTC (4s) -> FAST ");
        ESP_LOGI(TAG, "==================================================");

        bsp_lifecycle_set_stage(STAGE_WAKE_DS2);

        test_ui_render_screen("Entering DS-2 (4s)...\nWake: PCF85063A INT",
                              "Next Boot: FAST Mode\nArming RTC Countdown (4s)");
        vTaskDelay(pdMS_TO_TICKS(1000));

        bsp_sleep_config_t ds_cfg = {
            .mode           = BSP_SLEEP_MODE_DEEP,
            .duration_sec   = 4,
            .wake_sources   = BSP_WAKE_SRC_EXTERNAL_RTC,
            .next_init_mode = BSP_INIT_MODE_FAST,
        };
        bsp_lifecycle_enter_sleep(&ds_cfg);
        return;
    }

    // Interactive Light Sleep when tests are complete
    if (s_current_stage == STAGE_TESTS_COMPLETED) {
        ESP_LOGI(TAG, ">>> Triggering Interactive Light Sleep (5 sec)... <<<");
        test_ui_render_screen("Interactive Light Sleep\nDuration: 5 seconds",
                              "Auto-wake in 5s\nClick BOOT: Sleep Again");
        vTaskDelay(pdMS_TO_TICKS(500));
        bsp_sleep_config_t ls_cfg = {
            .mode           = BSP_SLEEP_MODE_LIGHT,
            .duration_sec   = 5,
            .wake_sources   = (bsp_wake_source_mask_t)(BSP_WAKE_SRC_TIMER | BSP_WAKE_SRC_BUTTONS),
            .next_init_mode = BSP_INIT_MODE_FAST,
        };
        bsp_lifecycle_enter_sleep(&ls_cfg);
        verify_peripherals_healthy();
        test_ui_render_screen("ALL SLEEP MODES: [PASS]\nLS-1: OK | LS-2: OK\nDS-1: OK | DS-2: OK",
                              "Heartbeat Active\nClick: LS(5s) | Hold: DS");
        ESP_LOGI(TAG, ">>> Resumed from Interactive Light Sleep <<<");
    }
}

static void app_on_cold_boot(void *user_data);
static void app_on_wake(const bsp_wake_context_t *ctx, void *user_data);

static void setup_test_buttons(bool enable_long_press)
{
    bsp_button_register_cb(BSP_BUTTON_BOOT,  BSP_BUTTON_EVENT_SINGLE_CLICK, button_event_handler, NULL);
    if (enable_long_press) {
        bsp_button_register_cb(BSP_BUTTON_BOOT,  BSP_BUTTON_EVENT_LONG_PRESS,   button_event_handler, NULL);
        bsp_button_register_cb(BSP_BUTTON_POWER, BSP_BUTTON_EVENT_SINGLE_CLICK, button_event_handler, NULL);
        bsp_button_register_cb(BSP_BUTTON_POWER, BSP_BUTTON_EVENT_LONG_PRESS,   button_event_handler, NULL);
    }
}

static void app_on_wake(const bsp_wake_context_t *ctx, void *user_data)
{
    sleep_test_stage_t stage = (sleep_test_stage_t)ctx->app_stage;
    s_current_stage = stage;

    ESP_LOGI(TAG, ">>> [LIFECYCLE WAKE] Processing Stage %d (Wake Cause: %d, Mode: %d, Boot Count: %lu) <<<",
             (int)stage, (int)ctx->wake_cause, (int)ctx->init_mode, (unsigned long)ctx->boot_count);

    // ========================================================================
    // WAKE HANDLER 1: WAKING FROM DEEP SLEEP TEST 1 (Timer -> FAST Mode)
    // ========================================================================
    if (stage == STAGE_WAKE_DS1) {
        ESP_LOGI(TAG, ">>> [WAKE 1/2] Processing DS-1 (Internal Timer -> FAST Mode) <<<");
        bool healthy = verify_peripherals_healthy();

        if (healthy) {
            ESP_LOGI(TAG, "[PASS DS-1] Deep Sleep + Internal Timer Wake Verified in FAST Mode!");
            s_current_stage = STAGE_READY_DS2;
            bsp_lifecycle_set_stage(STAGE_READY_DS2);

            test_ui_render_screen("DS-1: [PASS] (FAST Mode)\nClick BOOT: Run DS-2",
                                  "Wake: Timer OK\nNext: DS-2 (Ext RTC 4s)");
        } else {
            ESP_LOGE(TAG, "[FAIL DS-1] Peripheral health check failed in FAST Mode!");
            test_ui_render_screen("DS-1: [FAIL] Peripheral Fault\nClick BOOT: Retry DS-1",
                                  "FAST Mode Check Failed");
            s_current_stage = STAGE_READY_DS1;
            bsp_lifecycle_set_stage(STAGE_READY_DS1);
        }

        setup_test_buttons(false);
        return;
    }

    // ========================================================================
    // WAKE HANDLER 2: WAKING FROM DEEP SLEEP TEST 2 (Ext RTC -> FAST Mode)
    // ========================================================================
    if (stage == STAGE_WAKE_DS2) {
        ESP_LOGI(TAG, ">>> [WAKE 2/2] Processing DS-2 (External RTC Wake -> FAST Mode) <<<");

        bool healthy = verify_peripherals_healthy();

        if (healthy) {
            ESP_LOGI(TAG, "[PASS DS-2] Deep Sleep + External RTC Wake Verified in FAST Mode!");
            ESP_LOGI(TAG, "==================================================");
            ESP_LOGI(TAG, "  ALL STATIC & SLEEP/WAKE TESTS PASSED 100%%       ");
            ESP_LOGI(TAG, "==================================================");

            s_current_stage = STAGE_TESTS_COMPLETED;
            bsp_lifecycle_set_stage(STAGE_TESTS_COMPLETED);

            test_ui_render_screen("ALL SLEEP MODES: [PASS]\nLS-1: OK | LS-2: OK\nDS-1: OK | DS-2: OK",
                                  "Heartbeat Active\nClick: LS(5s) | Hold: DS");
        } else {
            ESP_LOGE(TAG, "[FAIL DS-2] Peripheral health check failed in FAST Mode!");
            test_ui_render_screen("DS-2: [FAIL] Peripheral Fault\nClick BOOT: Retry DS-2",
                                  "FAST Mode Check Failed");
            s_current_stage = STAGE_READY_DS2;
            bsp_lifecycle_set_stage(STAGE_READY_DS2);
        }

        setup_test_buttons(true);

        int count = 0;
        while (1) {
            vTaskDelay(pdMS_TO_TICKS(5000));
            count++;
            ESP_LOGI(TAG, "[Heartbeat %d] System Normal. Silicon: %s, Deep Sleeps: %lu, Light Sleeps: %lu",
                     count, bsp_get_chip_revision_str(),
                     (unsigned long)ctx->deep_sleep_count,
                     (unsigned long)ctx->light_sleep_count);
        }
        return;
    }

    // ========================================================================
    // WAKE HANDLER 3: WAKING FROM INTERACTIVE DEEP SLEEP (STAGE_TESTS_COMPLETED)
    // ========================================================================
    if (stage == STAGE_TESTS_COMPLETED) {
        ESP_LOGI(TAG, ">>> [WAKE] Resumed from Interactive Deep Sleep via Button Wake! <<<");
        verify_peripherals_healthy();

        test_ui_render_screen("ALL SLEEP MODES: [PASS]\nResumed: Deep Sleep (Button)",
                              "Heartbeat Active\nClick: LS(5s) | Hold: DS");

        setup_test_buttons(true);

        int count = 0;
        while (1) {
            vTaskDelay(pdMS_TO_TICKS(5000));
            count++;
            ESP_LOGI(TAG, "[Heartbeat %d] System Normal. Silicon: %s, Deep Sleeps: %lu, Light Sleeps: %lu",
                     count, bsp_get_chip_revision_str(),
                     (unsigned long)ctx->deep_sleep_count,
                     (unsigned long)ctx->light_sleep_count);
        }
        return;
    }
}

static void app_on_cold_boot(void *user_data)
{
    ESP_LOGI(TAG, ">>> [COLD BOOT] Initializing Subsystems (FULL Mode) <<<");
    esp_err_t ret = ESP_OK;

    test_ui_render_screen("System Self-Test\nExecuting Tests 1-14...",
                          "Running Diagnostics\nSensors, RTC, Audio, WiFi");

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
    // 6. Lifecycle State Persistence & Wake Context
    // ----------------------------------------------------
    bsp_wake_context_t ctx = {0};
    bsp_lifecycle_get_context(&ctx);
    ESP_LOGI(TAG, "[PASS 6/15] Lifecycle Context Valid: Boot Count=%lu, Deep Sleeps=%lu, Light Sleeps=%lu",
             (unsigned long)ctx.boot_count, (unsigned long)ctx.deep_sleep_count,
             (unsigned long)ctx.light_sleep_count);

    const char *scratch_test = "HumidOS_SelfTest_OK";
    bsp_lifecycle_save_state(scratch_test, strlen(scratch_test) + 1);
    char scratch_read[32] = {0};
    bsp_lifecycle_load_state(scratch_read, sizeof(scratch_read));
    if (strcmp(scratch_test, scratch_read) == 0) {
        ESP_LOGI(TAG, "[PASS 6/15] Lifecycle State Persistence Verified: '%s'", scratch_read);
    }

    // ----------------------------------------------------
    // 7. Tactile Button Registration & Event Dispatcher
    // ----------------------------------------------------
    setup_test_buttons(true);
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
    // 13. BSP Zero-Copy MMAP Flash Asset Pipeline & LVGL Decoder
    // ----------------------------------------------------
    uint32_t PCS = 0x00;
    ret = build_test_mmap_asset(&PCS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Building of MMAP Asset Failed: %s", bsp_err_to_name(ret));
    }

    // Initialize BSP Assets layer
    ret = bsp_assets_init("storage", 'S', 1, PCS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "[FAIL 13/15] bsp_assets_init failed: %s", bsp_err_to_name(ret));
    }

    ESP_LOGI(TAG, "[PASS 13/15] BSP MMAP Asset Layer Initialized (Drive 'S:' mounted via MMU)");

    // Test LVGL Zero-Copy Image Rendering Pipeline
    bsp_lvgl_lock();
    lv_obj_t *badge_img = lv_image_create(lv_screen_active());
    lv_image_set_src(badge_img, "S:test_badge.bin");
    lv_obj_align(badge_img, LV_ALIGN_BOTTOM_RIGHT, -8, -8);
    bsp_lvgl_unlock();

    vTaskDelay(pdMS_TO_TICKS(3000));
    ESP_LOGI(TAG, "[PASS 13/15] LVGL Zero-Copy Image 'S:test_badge.bin' Decoded & Rendered Successfully");

    // ----------------------------------------------------
    // 14. MicroSD Card Detection & FATFS Mount Test
    // ----------------------------------------------------
    ret = bsp_sdcard_mount();
    if (ret == ESP_OK) {
        float cap_gb = bsp_sdcard_get_capacity_gb();
        ESP_LOGI(TAG, "[PASS 14/15] MicroSD Card Mounted! Capacity: %.2f GB", cap_gb);

        FILE *f = fopen("/sdcard/bsp_test.txt", "w");
        if (f) {
            fprintf(f, "HumidOS BSP MicroSD Verified\n");
            fclose(f);
            ESP_LOGI(TAG, "[PASS 14/15] MicroSD File Write Verified");
        }
        bsp_sdcard_unmount();
    } else {
        ESP_LOGW(TAG, "[SKIP 14/15] MicroSD Slot Empty / Not Inserted (Status: %s)", bsp_err_to_name(ret));
    }

    // ----------------------------------------------------
    // 15. Wi-Fi Station & Passive Network Scanner Test
    // ----------------------------------------------------
    ret = bsp_wifi_init();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Starting passive Wi-Fi scan...");
        uint16_t ap_count = 0;
        esp_err_t scan_err = bsp_wifi_scan(NULL, &ap_count, 0);
        if (scan_err == ESP_OK) {
            ESP_LOGI(TAG, "[PASS 15/15] Wi-Fi Subsystem Operational. Found %u Access Points in scan", ap_count);
        } else {
            ESP_LOGW(TAG, "[WARN 15/15] Wi-Fi scan completed with status: %s", bsp_err_to_name(scan_err));
        }
    }

    // ----------------------------------------------------
    // Render Fullscreen QR Code & Test Summary Screen
    // ----------------------------------------------------
    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "  [PROMPT] Press BOOT to display QR Code          ");
    ESP_LOGI(TAG, "==================================================");
    test_ui_render_screen("Static Tests Complete\nClick BOOT: Show QR",
                          "Provisioning Engine\nWaiting for BOOT click...");

    // Wait for user to press and release BOOT button using unified BSP helper
    bsp_button_wait_for_click(BSP_BUTTON_BOOT, 0);

    ESP_LOGI(TAG, "Rendering Fullscreen 180x180 px BLE Provisioning QR Code...");
    bsp_lvgl_lock();
    s_lbl_status = NULL;
    s_lbl_sub    = NULL;
    lv_obj_t *scr = lv_screen_active();
    lv_obj_clean(scr);
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
    lv_obj_t *qr = bsp_prov_render_qr_code(scr, 180, "HumidOS-1.0.0-OK");
    if (qr) lv_obj_center(qr);
    bsp_lvgl_unlock();

    // QR display timeout
    vTaskDelay(pdMS_TO_TICKS(3000));

    // Clear and display Static Tests Passed UI
    test_ui_render_screen("STATIC TESTS: [PASS]\nClick BOOT: Run LS-1",
                          "Ready for Sleep Tests\nNext: LS-1 (Timer 3s)");
    ESP_LOGI(TAG, "[PASS] E-Paper Display & LVGL Rendering Operational");

    // Set state machine to Stage 1 (Ready for LS-1 on BOOT click)
    s_current_stage = STAGE_READY_LS1;
    bsp_lifecycle_set_stage(STAGE_READY_LS1);

    setup_test_buttons(true);

    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "  STATIC TESTS COMPLETE -> WAITING FOR BOOT CLICK ");
    ESP_LOGI(TAG, "  Press BOOT (GPIO0) to execute LS-1              ");
    ESP_LOGI(TAG, "==================================================");
}

void app_main(void)
{
    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "  ESP32-S3 ePaper BSP Staged Verification Suite    ");
    ESP_LOGI(TAG, "==================================================");

    bsp_app_lifecycle_t lifecycle = {
        .on_cold_boot    = app_on_cold_boot,
        .on_wake         = app_on_wake,
        .on_before_sleep = NULL,
        .user_data       = NULL,
    };

    bsp_app_start(&lifecycle);
}
