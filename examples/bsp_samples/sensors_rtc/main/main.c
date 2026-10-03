/**
 * @file main.c
 * @brief Sensirion SHTC3 Sensor & PCF85063 RTC Sample
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

static const char *TAG = "sample_sensors_rtc";

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing Humid1 OS Sensors & RTC Sample...");
    ESP_ERROR_CHECK(bsp_init());

    while (1) {
        bsp_shtc3_data_t sensor_data;
        esp_err_t err = bsp_shtc3_read(&sensor_data);

        bsp_rtc_time_t rtc_time;
        esp_err_t rtc_err = bsp_rtc_get_time(&rtc_time);

        if (err == ESP_OK && sensor_data.valid) {
            ESP_LOGI(TAG, "Temp: %.2f °C | Humidity: %.2f %%", 
                     (double)sensor_data.temperature_c, 
                     (double)sensor_data.humidity_percent);
        } else {
            ESP_LOGW(TAG, "Failed to read SHTC3 sensor data");
        }

        if (rtc_err == ESP_OK) {
            ESP_LOGI(TAG, "RTC Time: %04d-%02d-%02d %02d:%02d:%02d",
                     rtc_time.year, rtc_time.month, rtc_time.day,
                     rtc_time.hour, rtc_time.minute, rtc_time.second);
        } else {
            ESP_LOGW(TAG, "Failed to read RTC time");
        }

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
