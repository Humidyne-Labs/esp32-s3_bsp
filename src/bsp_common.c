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

esp_err_t bsp_board_init(void)
{
    ESP_LOGI(TAG, "Initializing ESP32-S3 Touch ePaper BSP...");

    esp_err_t ret = bsp_init_io();
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize select IO");
        return ret;
    }

    /* Initialize Buttons Flash */
    ret = bsp_button_init(NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize buttons");
        return ret;
    }

    /* Initialize NVS Flash */
    ret = bsp_nvs_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS flash");
        return ret;
    }

    /* Power Management & ADC */
    ret = bsp_power_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize power management");
        return ret;
    }

    /* Shared I2C Master Bus */
    ret = bsp_i2c_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize shared I2C bus");
        return ret;
    }

    char device_id[32] = {0};
    if (bsp_get_device_id(device_id, sizeof(device_id)) == ESP_OK) {
        ESP_LOGI(TAG, "Device Hardware Unique ID: %s", device_id);
    }

    ESP_LOGI(TAG, "ESP32-S3 Touch ePaper Board initialized successfully");
    return ESP_OK;
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