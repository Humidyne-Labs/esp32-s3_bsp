/**
 * @file main.c
 * @brief Standalone reference example for the Sensirion SHTC3 I2C Environmental Sensor
 * 
 * This example demonstrates how to initialize the SHTC3 sensor, perform periodic
 * temperature and relative humidity measurements, validate the telemetry data,
 * and convert the native Kelvin temperature readings into Celsius and Fahrenheit.
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"

/* BSP Header */
#include "bsp/bsp_sensors.h"

static const char *TAG = "shtc3_example";

// Function Pointer Prototype
typedef esp_err_t (*shtc3_read_fn_t)(bsp_shtc3_data_t *out_data);

void app_main(void)
{
    ESP_LOGI(TAG, "Starting SHTC3 Environmental Sensor Reference Example");

    // Initialize the SHTC3 sensor (wakes, verifies ID, and puts to sleep)
    // Wrapped in ESP_ERROR_CHECK as required by the rules
    ESP_ERROR_CHECK(bsp_shtc3_init());

    ESP_LOGI(TAG, "SHTC3 sensor initialized successfully. Entering measurement loop...");

	bool use_low_power = true;     // Switch dynamically as needed
    bsp_shtc3_data_t sensor_data;
	
	// Assign function pointer based on the boolean flag
	shtc3_read_fn_t read_sensor = use_low_power ? bsp_shtc3_read_lp : bsp_shtc3_read;

    while (1) {
		// bsp_shtc3_read automatically handles wakeup, measurement, and sleep
        esp_err_t err = read_sensor(&sensor_data);
        
        if (err == ESP_OK) {
            if (sensor_data.valid) {
                // Convert native Kelvin to Celsius and Fahrenheit
                float temp_c = sensor_data.temperature_c;
                float temp_f = sensor_data.temperature_f;
				float temp_k = sensor_data.temperature_k;
                float rh     = sensor_data.humidity_percent;
				float dp_c   = sensor_data.dew_point_c;
				float dp_f   = sensor_data.dew_point_f;
				float dp_k   = sensor_data.dew_point_k;
				float abs_h  = sensor_data.absolute_humidity_g;

                ESP_LOGI(TAG, "--- Full Environmental Telemetry Spread ---");
                ESP_LOGI(TAG, "  Temperature  : %.2f K  (%.2f C / %.2f F)", (double)temp_k, (double)temp_c, (double)temp_f);
                ESP_LOGI(TAG, "  Humidity     : %.2f %% RH"               , (double)rh);
				ESP_LOGI(TAG, "  Dew Point    : %.2f K  (%.2f C / %.2f F)", (double)dp_k, (double)dp_c, (double)dp_f);
				ESP_LOGI(TAG, "  ABS Humidity : %.2f g/m^3"               , (double)abs_h);
				ESP_LOGI(TAG, "-------------------------------------------");
            } else {
                ESP_LOGW(TAG, "Sensor read completed, but CRC verification failed!");
            }
        } else {
            ESP_LOGE(TAG, "Failed to read from SHTC3 sensor: %s", esp_err_to_name(err));
        }

        // Delay for 2000ms (2 seconds) between readings
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}