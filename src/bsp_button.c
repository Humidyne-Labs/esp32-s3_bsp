/**
 * @file bsp_button.c
 * @brief button lib
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics (https://www.waveshare.com)
 * - Microcontroller: Espressif Systems ESP32-S3 (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#include "driver/gpio.h"
#include "esp_log.h"
#include "bsp/bsp_button.h"
#include "bsp/pinout.h"

static const char *TAG = "bsp_button";
static bool s_button_inited = false;

static gpio_num_t button_gpio(bsp_button_t button)
{
    return button == BSP_BUTTON_BOOT ?
           (gpio_num_t)BSP_GPIO_BOOT_KEY :
           (gpio_num_t)BSP_GPIO_BAT_KEY;
}

esp_err_t bsp_button_init(void)
{
    if (s_button_inited) {
        return ESP_OK;
    }

    gpio_config_t config = {
        .pin_bit_mask = (1ULL << BSP_GPIO_BOOT_KEY) | (1ULL << BSP_GPIO_BAT_KEY),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t ret = gpio_config(&config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure button GPIOs: %s", esp_err_to_name(ret));
        return ret;
    }

    s_button_inited = true;
    ESP_LOGI(TAG, "Buttons initialized (BOOT GPIO%d, POWER GPIO%d)",
             BSP_GPIO_BOOT_KEY, BSP_GPIO_BAT_KEY);
    return ESP_OK;
}

bool bsp_button_is_pressed(bsp_button_t button)
{
    if (button >= BSP_BUTTON_COUNT) {
        return false;
    }
    if (!s_button_inited && bsp_button_init() != ESP_OK) {
        return false;
    }
    return gpio_get_level(button_gpio(button)) == 0;
}
