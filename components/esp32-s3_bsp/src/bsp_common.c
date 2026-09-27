/**
 * @file bsp_common.c
 * @brief Master Board Support Package Initialization, Diagnostics & System Control
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics (https://www.waveshare.com)
 * - Microcontroller: Espressif Systems ESP32-S3 (https://www.espressif.com)
 * - BSP Implementation: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "esp_random.h"
#include "esp_chip_info.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "bsp/pinout.h"
#include "bsp/bsp.h"

static const char *TAG = "bsp_common";

const char *bsp_get_version(void)
{
    return BSP_VERSION_STRING;
}

uint32_t bsp_get_version_val(void)
{
    return BSP_CURRENT_VERSION;
}

esp_err_t bsp_get_chip_revision(uint32_t *major, uint32_t *minor)
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    if (major) *major = chip_info.revision / 100;
    if (minor) *minor = chip_info.revision % 100;
    return ESP_OK;
}

const char *bsp_get_chip_revision_str(void)
{
    static char rev_str[16] = {0};
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    snprintf(rev_str, sizeof(rev_str), "v%lu.%lu",
             (unsigned long)(chip_info.revision / 100),
             (unsigned long)(chip_info.revision % 100));
    return rev_str;
}

const char *bsp_err_to_name(esp_err_t err)
{
    switch (err) {
        case ESP_OK:                       return "BSP_OK / ESP_OK";
        case BSP_ERR_NOT_INITIALIZED:      return "BSP_ERR_NOT_INITIALIZED";
        case BSP_ERR_I2C_BUS_LOCKED:       return "BSP_ERR_I2C_BUS_LOCKED";
        case BSP_ERR_SENSOR_CRC_FAIL:      return "BSP_ERR_SENSOR_CRC_FAIL";
        case BSP_ERR_DISPLAY_BUSY_TIMEOUT: return "BSP_ERR_DISPLAY_BUSY_TIMEOUT";
        case BSP_ERR_AUDIO_NOT_READY:      return "BSP_ERR_AUDIO_NOT_READY";
        case BSP_ERR_SD_CARD_MOUNT:        return "BSP_ERR_SD_CARD_MOUNT";
        case BSP_ERR_WIFI_DISCONNECTED:    return "BSP_ERR_WIFI_DISCONNECTED";
        case BSP_ERR_OTA_VALIDATION:       return "BSP_ERR_OTA_VALIDATION";
        default:                           return esp_err_to_name(err);
    }
}

esp_err_t bsp_get_diagnostics(bsp_diag_info_t *diag)
{
    if (diag == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    diag->bsp_version            = bsp_get_version();
    diag->chip_model             = "ESP32-S3";
    diag->chip_revision_str      = bsp_get_chip_revision_str();
    diag->chip_revision          = chip_info.revision;
    diag->chip_cores             = chip_info.cores;
    diag->free_internal_heap     = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    diag->min_free_internal_heap = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    diag->free_psram_heap        = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    diag->uptime_seconds         = (uint32_t)(esp_timer_get_time() / 1000000ULL);
    uint32_t batt_mv             = 0;
    
	bsp_battery_get_voltage(&batt_mv, NULL);
	
    diag->battery_mv             = batt_mv;
    diag->battery_percentage     = (int8_t)bsp_battery_get_percentage();
    diag->power_rail_good        = (gpio_get_level((gpio_num_t)BSP_PIN_POWER_HOLD) == 1);
    diag->i2c_bus_healthy        = (bsp_i2c_probe(BSP_I2C_ADDR_SHTC3) == ESP_OK);
    diag->display_ready          = (bsp_display_get_buffer() != NULL);
    diag->wifi_connected         = bsp_wifi_is_connected();

    int rssi                     = 0;
    bsp_wifi_get_rssi(&rssi);
    diag->wifi_rssi              = (int8_t)rssi;

    return ESP_OK;
}

void bsp_diagnostics_dump(void)
{
    bsp_diag_info_t diag;
    if (bsp_get_diagnostics(&diag) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to retrieve system diagnostics");
        return;
    }

    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "  ESP32-S3 ePaper BSP System Diagnostics");
    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "  BSP Version:          %s", diag.bsp_version);
    ESP_LOGI(TAG, "  Silicon Chip:         %s (%s, %d Cores)", diag.chip_model, diag.chip_revision_str, diag.chip_cores);
    ESP_LOGI(TAG, "  Uptime:               %lu s", (unsigned long)diag.uptime_seconds);
    ESP_LOGI(TAG, "  Internal Heap Free:   %lu B (Min Free: %lu B)", 
             (unsigned long)diag.free_internal_heap, (unsigned long)diag.min_free_internal_heap);
    ESP_LOGI(TAG, "  PSRAM Free:           %lu B", (unsigned long)diag.free_psram_heap);
    ESP_LOGI(TAG, "  Battery:              %lu mV (%d%%)", (unsigned long)diag.battery_mv, diag.battery_percentage);
    ESP_LOGI(TAG, "  Power Rail Hold:      %s", diag.power_rail_good ? "ACTIVE (HIGH)" : "INACTIVE");
    ESP_LOGI(TAG, "  I2C Sensor Bus:       %s", diag.i2c_bus_healthy ? "HEALTHY" : "FAULT / UNRESPONSIVE");
    ESP_LOGI(TAG, "  Display Framebuffer:  %s", diag.display_ready ? "INITIALIZED" : "NOT INITIALIZED");
    ESP_LOGI(TAG, "  Wi-Fi Station:        %s (RSSI: %d dBm)", 
             diag.wifi_connected ? "CONNECTED" : "DISCONNECTED", diag.wifi_rssi);
    ESP_LOGI(TAG, "==================================================");
}

esp_err_t bsp_init_io(void)
{
    static bool s_io_inited = false;
    if (s_io_inited) return ESP_OK;

    // 1. Release all deep-sleep GPIO pad holds first
    gpio_deep_sleep_hold_dis();
    gpio_hold_dis((gpio_num_t)BSP_PIN_POWER_HOLD);
    gpio_hold_dis((gpio_num_t)BSP_PIN_EPD_3V3_EN);
    gpio_hold_dis((gpio_num_t)BSP_PIN_PA_EN);

    // 2. Pre-set output latch register levels BEFORE configuring direction to prevent glitches
    gpio_set_level((gpio_num_t)BSP_PIN_POWER_HOLD, 1);  // Latch onboard LDO power ON (Active HIGH)
    gpio_set_level((gpio_num_t)BSP_PIN_EPD_3V3_EN, 0);  // EPD & Sensor 3.3V Power Rail ON (Active LOW: 0=ON, 1=OFF)
    gpio_set_level((gpio_num_t)BSP_PIN_PA_EN,      1);  // Power Amp OFF by default (Active LOW: 1=OFF, 0=ON)
    gpio_set_level((gpio_num_t)BSP_PIN_LED_STATUS, 1);  // User Status LED OFF (Open-Drain Active LOW: 1=OFF, 0=ON)
    gpio_set_level((gpio_num_t)BSP_PIN_EPD_CS,     1);  // Display SPI CS Deselected (HIGH)
    gpio_set_level((gpio_num_t)BSP_PIN_EPD_DC,     1);  // Display Data/Command line default HIGH
    gpio_set_level((gpio_num_t)BSP_PIN_EPD_RST,    1);  // Display out of reset (HIGH)

    // Settle 3.3V power rails for sensors and pull-ups
    esp_rom_delay_us(25000);

    // 3. Configure all standard digital OUTPUT pins
    gpio_config_t out_cfg = {
        .pin_bit_mask = (1ULL << BSP_PIN_POWER_HOLD) |
                        (1ULL << BSP_PIN_PA_EN)      |
                        (1ULL << BSP_PIN_LED_STATUS) |
                        (1ULL << BSP_PIN_EPD_3V3_EN) |
                        (1ULL << BSP_PIN_EPD_RST)    |
                        (1ULL << BSP_PIN_EPD_DC)     |
                        (1ULL << BSP_PIN_EPD_CS),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    esp_err_t ret = gpio_config(&out_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure Output GPIOs: %s", esp_err_to_name(ret));
        return ret;
    }

    // Re-verify power latch is firmly latched HIGH
    gpio_set_level((gpio_num_t)BSP_PIN_POWER_HOLD, 1);

    // 4. Configure all INPUT pins (RTC INT) with pullups enabled
    gpio_config_t in_pullup_cfg = {
        .pin_bit_mask = (1ULL << BSP_PIN_RTC_INT),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    ret = gpio_config(&in_pullup_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure Pull-up Input GPIOs: %s", esp_err_to_name(ret));
        return ret;
    }

    // 5. Configure floating INPUT pins (EPD Busy line, Buttons with external pullups)
    gpio_config_t in_float_cfg = {
        .pin_bit_mask = (1ULL << BSP_PIN_EPD_BUSY)    |
                        (1ULL << BSP_PIN_BUTTON_BOOT) |
                        (1ULL << BSP_PIN_BUTTON_POWER),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    ret = gpio_config(&in_float_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure Input GPIOs: %s", esp_err_to_name(ret));
        return ret;
    }

    // 6. Give power rails time to ramp up and settle before communicating with I2C/SPI devices
    vTaskDelay(pdMS_TO_TICKS(50));

    s_io_inited = true;
    ESP_LOGI(TAG, "All board IO pins configured and latched to safe defaults");
    return ESP_OK;
}

esp_err_t bsp_board_init_with_config(const bsp_config_t *config)
{
    ESP_LOGI(TAG, "Initializing ESP32-S3 ePaper BSP Subsystems (v%s)...", bsp_get_version());

    bsp_config_t cfg = (config != NULL) ? *config : (bsp_config_t)BSP_CONFIG_DEFAULT();

    esp_err_t ret = bsp_init_io();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize base IO: %s", esp_err_to_name(ret));
        return ret;
    }

    if (cfg.init_power) {
        ret = bsp_power_init();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize power management: %s", esp_err_to_name(ret));
            return ret;
        }
    }

    if (cfg.init_buttons) {
        ret = bsp_button_init(NULL);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize buttons: %s", esp_err_to_name(ret));
            return ret;
        }
    }

    if (cfg.init_nvs) {
        ret = bsp_nvs_init();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize NVS flash: %s", esp_err_to_name(ret));
            return ret;
        }
    }

    if (cfg.init_i2c) {
        ret = bsp_i2c_init();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize shared I2C bus: %s", esp_err_to_name(ret));
            return ret;
        }
    }

    if (cfg.init_rtc) {
        ret = bsp_rtc_init();
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "RTC initialization returned: %s", esp_err_to_name(ret));
        }
    }

    if (cfg.init_sensors) {
        ret = bsp_sensors_init();
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "Sensors initialization returned: %s", esp_err_to_name(ret));
        }
    }

    if (cfg.init_audio) {
        ret = bsp_audio_init();
        if (ret == ESP_OK) {
            bsp_audio_set_volume(cfg.audio_volume);
        } else {
            ESP_LOGW(TAG, "Audio codec init returned: %s", esp_err_to_name(ret));
        }
    }

    if (cfg.init_sdcard) {
        ret = bsp_sdcard_mount();
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "MicroSD mount returned: %s", esp_err_to_name(ret));
        }
    }

    if (cfg.init_display && cfg.start_lvgl) {
        /* Pin LVGL UI & e-Paper render task to Core 1, leaving Core 0 for Wi-Fi/BLE/MQTT networking */
        ret = bsp_lvgl_start(5, 1);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to start LVGL display port");
            return ret;
        }
    } else if (cfg.init_display) {
        ret = bsp_display_init();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to init display: %s", esp_err_to_name(ret));
            return ret;
        }
    }

    char device_id[32] = {0};
    if (bsp_get_device_id(device_id, sizeof(device_id)) == ESP_OK) {
        ESP_LOGI(TAG, "Device Hardware Unique ID: %s", device_id);
    }

    ESP_LOGI(TAG, "ESP32-S3 ePaper Board initialized successfully");
    return ESP_OK;
}

esp_err_t bsp_init_mode(bsp_init_mode_t mode)
{
    bsp_rtc_mem_init();

    // Configure first flush mode: fast and min wake boots use partial refresh without screen blanking
    if (mode == BSP_INIT_MODE_FAST || mode == BSP_INIT_MODE_MIN) {
        bsp_lvgl_set_first_flush_mode(false);
    } else {
        bsp_lvgl_set_first_flush_mode(true);
    }

    bsp_config_t cfg = {
        .init_power   = true,
        .init_i2c     = true,
        .init_sensors = true,
        .init_rtc     = true,
        .init_buttons = true,
        .init_audio   = (mode == BSP_INIT_MODE_FULL),
        .audio_volume = 80.0f,
        .init_sdcard  = false,
        .init_display = true,
        .init_nvs     = true,
        .start_lvgl   = true,
    };

    bsp_rtc_state_t *rtc_st = bsp_rtc_mem_get_state();
    if (rtc_st && rtc_st->magic == BSP_RTC_MEM_MAGIC) {
        rtc_st->last_init_mode = (uint8_t)mode;
    }

    ESP_LOGI(TAG, "Executing Dynamic Init Profile: %s",
             (mode == BSP_INIT_MODE_FULL) ? "FULL (Cold Boot)" :
             (mode == BSP_INIT_MODE_FAST) ? "FAST (Wake Boot)" : "MIN (Lean Telemetry + Display)");

    return bsp_board_init_with_config(&cfg);
}

esp_err_t bsp_generate_unambiguous_key(char *buf, size_t len, const char *charset)
{
    if (buf == NULL || len == 0) return ESP_ERR_INVALID_ARG;
    if (charset == NULL) charset = BSP_CHARSET_UNAMBIGUOUS;
    size_t c_len = strlen(charset);
    if (c_len == 0) return ESP_ERR_INVALID_ARG;

    for (size_t i = 0; i < len; i++) {
        uint32_t r = esp_random();
        buf[i] = charset[r % c_len];
    }
    buf[len] = '\0';
    return ESP_OK;
}

esp_err_t bsp_board_init(void)
{
    bsp_rtc_mem_init();
    bsp_init_mode_t rec_mode = bsp_get_recommended_init_mode();
    return bsp_init_mode(rec_mode);
}

void bsp_system_shutdown(void)
{
    ESP_LOGI(TAG, "Initiating system shutdown sequence...");
    bsp_led_set(false);
    bsp_power_off();
}

void bsp_system_deep_sleep(uint32_t sleep_sec)
{
    bsp_power_enter_deep_sleep(sleep_sec);
}

esp_err_t bsp_get_device_id(char *buf, size_t max_len)
{
    if (buf == NULL || max_len < 16) return ESP_ERR_INVALID_ARG;

    uint8_t mac[6] = {0};
    esp_err_t ret = esp_efuse_mac_get_default(mac);
    if (ret != ESP_OK) {
        return ret;
    }

    snprintf(buf, max_len, "ESP32S3-%02X%02X%02X%02X", mac[2], mac[3], mac[4], mac[5]);
    return ESP_OK;
}

esp_err_t bsp_get_device_name(char *buf, size_t max_len)
{
    if (buf == NULL || max_len < 16) return ESP_ERR_INVALID_ARG;

    uint8_t mac[6] = {0};
    esp_err_t ret = esp_efuse_mac_get_default(mac);
    if (ret != ESP_OK) {
        return ret;
    }

    snprintf(buf, max_len, "HumidOS-%02X%02X", mac[4], mac[5]);
    return ESP_OK;
}