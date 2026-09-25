/**
 * @file bsp_common.c
 * @brief general
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics (https://www.waveshare.com)
 * - Microcontroller: Espressif Systems ESP32-S3 (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_mac.h"
#include "bsp/bsp.h"

static const char *TAG = "bsp_common";

static bool s_led_state = false;

esp_err_t bsp_board_init_with_config(const bsp_config_t *config)
{
    ESP_LOGI(TAG, "Initializing ESP32-S3 Touch ePaper BSP...");

    bsp_config_t cfg = (config != NULL) ? *config : (bsp_config_t)BSP_CONFIG_DEFAULT();

    esp_err_t ret = bsp_init_io();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize select IO: %s", esp_err_to_name(ret));
        return ret;
    }

    if (cfg.init_power) {
        ret = bsp_power_init();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize power management");
            return ret;
        }
        bsp_power_hold();
    }

    if (cfg.init_buttons) {
        ret = bsp_button_init(NULL);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize buttons");
            return ret;
        }
    }

    if (cfg.init_nvs) {
        ret = bsp_nvs_init();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize NVS flash");
            return ret;
        }
    }

    if (cfg.init_i2c) {
        ret = bsp_i2c_init();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize shared I2C bus");
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
            ESP_LOGE(TAG, "Failed to init display");
            return ret;
        }
    }

    char device_id[32] = {0};
    if (bsp_get_device_id(device_id, sizeof(device_id)) == ESP_OK) {
        ESP_LOGI(TAG, "Device Hardware Unique ID: %s", device_id);
    }

    ESP_LOGI(TAG, "ESP32-S3 Touch ePaper Board initialized successfully");
    return ESP_OK;
}

esp_err_t bsp_board_init(void)
{
    bsp_config_t default_cfg = BSP_CONFIG_DEFAULT();
    return bsp_board_init_with_config(&default_cfg);
}

void bsp_system_shutdown(void)
{
    ESP_LOGI(TAG, "Initiating system shutdown sequence...");
    bsp_led_set(false);
    bsp_power_release();
}

void bsp_system_enter_deep_sleep(uint32_t sleep_sec)
{
    uint64_t button_mask = (1ULL << BSP_GPIO_BOOT) | (1ULL << BSP_GPIO_BAT_KEY);
    bsp_power_enter_deep_sleep(sleep_sec, true, button_mask);
}


esp_err_t bsp_init_io(void)
{
    gpio_config_t pwr_cfg = {
        .pin_bit_mask = (1ULL << BSP_GPIO_BAT_CTRL)    | 
                        (1ULL << BSP_GPIO_PA_EN)       |
                        //(1ULL << BSP_GPIO_PA_CTRL)     |
                        (1ULL << BSP_GPIO_USER_LED)    |
                        (1ULL << BSP_GPIO_EPD_3V3_EN),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    esp_err_t ret = gpio_config(&pwr_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure Power GPIOs: %s", esp_err_to_name(ret));
        return ret;
    }

    bsp_led_set(false);
    return ESP_OK;
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

void bsp_led_set(bool enable)
{
    s_led_state = enable;
    gpio_set_level((gpio_num_t)BSP_GPIO_USER_LED, enable ? 1 : 0);
}

void bsp_led_toggle(void)
{
    bsp_led_set(!s_led_state);
}