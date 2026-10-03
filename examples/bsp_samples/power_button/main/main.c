/**
 * @file main.c
 * @brief PMIC Latch, Power Button Callbacks & Battery ADC Monitor Sample
 *
 * @version 1.0.0
 * @attribution
 * - Architecture & Development: HUMIDYNE LABS / Humiditron
 * - AI Systems Co-Developer: Gemini (Google DeepMind)
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "bsp/bsp.h"

static const char *TAG = "sample_power_button";

static void on_power_click(void *arg)
{
    ESP_LOGI(TAG, "Power button clicked!");
}

static void on_power_long_press(void *arg)
{
    ESP_LOGI(TAG, "Power button long pressed! Initiating clean shutdown...");
    bsp_power_off();
}

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing Humid1 OS Power & Button Sample...");
    ESP_ERROR_CHECK(bsp_init());

    bsp_button_register_callback(BSP_BUTTON_POWER, BSP_BUTTON_CLICK, on_power_click, NULL);
    bsp_button_register_callback(BSP_BUTTON_POWER, BSP_BUTTON_LONG_PRESS, on_power_long_press, NULL);

    while (1) {
        float batt_v = bsp_power_get_battery_voltage();
        uint8_t batt_pct = bsp_power_get_battery_percentage();

        ESP_LOGI(TAG, "Battery: %.2f V (%d%%)", (double)batt_v, batt_pct);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
