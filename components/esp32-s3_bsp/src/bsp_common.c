/**
 * @file bsp_common.c
 * @brief Master Board Support Package Initialization & System Control
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "bsp/pinout.h"
#include "bsp/bsp.h"

static const char *TAG = "bsp_common";

esp_err_t bsp_init_io(void)
{
    static bool s_io_inited = false;
    if (s_io_inited) return ESP_OK;

    // 1. Release all deep-sleep GPIO pad holds first
    gpio_deep_sleep_hold_dis();
    gpio_hold_dis((gpio_num_t)BSP_PIN_POWER_HOLD);
    gpio_hold_dis((gpio_num_t)BSP_PIN_EPD_3V3_EN);
    gpio_hold_dis((gpio_num_t)BSP_PIN_PA_EN);
    gpio_hold_dis((gpio_num_t)BSP_PIN_TOUCH_RST);

    // 2. Pre-set output latch register levels BEFORE configuring direction to prevent glitches
    gpio_set_level((gpio_num_t)BSP_PIN_POWER_HOLD, 1);  // Latch onboard LDO power ON (Active HIGH)
    gpio_set_level((gpio_num_t)BSP_PIN_EPD_3V3_EN, 0);  // EPD & Sensor 3.3V Power Rail ON (Active LOW: 0=ON, 1=OFF)
    gpio_set_level((gpio_num_t)BSP_PIN_PA_EN,      1);  // Power Amp OFF by default (Active LOW: 1=OFF, 0=ON)
    //gpio_set_level((gpio_num_t)BSP_PIN_PA_CTRL,    0);  // Power Amp Muted (Active HIGH: 1=ON, 0=OFF)
    gpio_set_level((gpio_num_t)BSP_PIN_LED_STATUS, 1);  // User Status LED OFF (Open-Drain Active LOW: 1=OFF, 0=ON)
    gpio_set_level((gpio_num_t)BSP_PIN_EPD_CS,     1);  // Display SPI CS Deselected (HIGH)
    gpio_set_level((gpio_num_t)BSP_PIN_EPD_DC,     1);  // Display Data/Command line default HIGH
    gpio_set_level((gpio_num_t)BSP_PIN_EPD_RST,    1);  // Display out of reset (HIGH)
    gpio_set_level((gpio_num_t)BSP_PIN_TOUCH_RST,  1);  // Touch controller out of reset (HIGH)

    // Settle 3.3V power rails for sensors and pull-ups
    esp_rom_delay_us(25000);

    // 3. Configure all standard digital OUTPUT pins
    gpio_config_t out_cfg = {
        .pin_bit_mask = (1ULL << BSP_PIN_POWER_HOLD) |
                        (1ULL << BSP_PIN_PA_EN)      |
                        //(1ULL << BSP_PIN_PA_CTRL)    |
                        (1ULL << BSP_PIN_LED_STATUS) |
                        (1ULL << BSP_PIN_EPD_3V3_EN) |
                        (1ULL << BSP_PIN_EPD_RST)    |
                        (1ULL << BSP_PIN_EPD_DC)     |
                        (1ULL << BSP_PIN_EPD_CS)     |
                        (1ULL << BSP_PIN_TOUCH_RST),
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

    // 4. Configure all INPUT pins (Buttons, RTC INT, Touch INT) with pullups enabled
    gpio_config_t in_pullup_cfg = {
        .pin_bit_mask = (1ULL << BSP_PIN_RTC_INT)     |
                        (1ULL << BSP_PIN_TOUCH_INT),                        
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

    // 5. Configure floating INPUT pins (EPD Busy line)
    gpio_config_t in_float_cfg = {
        .pin_bit_mask = (1ULL << BSP_PIN_EPD_BUSY)    |
                        (1ULL << BSP_PIN_BUTTON_BOOT) | // Both Power and Boot have external pullups.
                        (1ULL << BSP_PIN_BUTTON_POWER),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    ret = gpio_config(&in_float_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure EPD Busy GPIO: %s", esp_err_to_name(ret));
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
    ESP_LOGI(TAG, "Initializing ESP32-S3 Touch ePaper BSP Subsystems...");

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