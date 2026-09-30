/**
 * @file main.c
 * @brief Standalone reference example for the PCF85063A Real-Time Clock BSP Driver
 * 
 * This example demonstrates:
 *  1. Initializing the PCF85063A RTC.
 *  2. Disabling CLKOUT to save power.
 *  3. Checking the oscillator status.
 *  4. Setting and reading back a specific date and time.
 *  5. Writing and reading the 8-bit general-purpose NVRAM.
 *  6. Configuring and verifying the 1Hz periodic countdown timer.
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "bsp/bsp_rtc.h"

static const char *TAG = "rtc_example";

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing PCF85063A RTC...");
    
    // 1. Initialize the RTC and underlying I2C bus
    ESP_ERROR_CHECK(bsp_rtc_init());

    // 2. Disable CLKOUT square wave output to save power and reduce noise
    ESP_ERROR_CHECK(bsp_rtc_disable_clkout());

    // 3. Check if the oscillator is running (integrity check)
    bool is_running = false;
    ESP_ERROR_CHECK(bsp_rtc_is_running(&is_running));
    ESP_LOGI(TAG, "RTC Oscillator running: %s", is_running ? "YES" : "NO (Power loss detected)");

    // 4. Set a known date and time (e.g., Monday, March 30, 2026, 12:00:00)
    bsp_rtc_datetime_t set_dt = {
        .year = 2026,
        .month = 3,
        .day = 30,
        .weekday = 1, // Monday
        .hour = 12,
        .minute = 0,
        .second = 0
    };
    
    ESP_LOGI(TAG, "Setting RTC time to 2026-03-30 12:00:00...");
    ESP_ERROR_CHECK(bsp_rtc_set_datetime(&set_dt));

    // Wait 2 seconds to let the RTC increment
    vTaskDelay(pdMS_TO_TICKS(2000));

    // Read back the date and time to verify incrementing
    bsp_rtc_datetime_t read_dt;
    ESP_ERROR_CHECK(bsp_rtc_get_datetime(&read_dt));
    ESP_LOGI(TAG, "Read RTC time: %04d-%02d-%02d %02d:%02d:%02d (Weekday: %d)",
             read_dt.year, read_dt.month, read_dt.day,
             read_dt.hour, read_dt.minute, read_dt.second, read_dt.weekday);

    // 5. Test the 8-bit general storage NVRAM (Register 0x03)
    uint8_t test_write_val = 0xA5;
    ESP_LOGI(TAG, "Writing 0x%02X to RTC NVRAM...", test_write_val);
    ESP_ERROR_CHECK(bsp_rtc_ram_write(test_write_val));

    uint8_t test_read_val = 0;
    ESP_ERROR_CHECK(bsp_rtc_ram_read(&test_read_val));
    ESP_LOGI(TAG, "Read from RTC NVRAM: 0x%02X", test_read_val);
    
    if (test_read_val == test_write_val) {
        ESP_LOGI(TAG, "NVRAM verification: SUCCESS");
    } else {
        ESP_LOGE(TAG, "NVRAM verification: FAILED");
    }

    // 6. Test the periodic countdown timer (Set for 3 seconds)
    ESP_LOGI(TAG, "Setting 3-second countdown timer...");
    ESP_ERROR_CHECK(bsp_rtc_set_countdown_timer(3));

    // Wait 4 seconds to ensure the timer expires
    ESP_LOGI(TAG, "Waiting 4 seconds for timer to expire...");
    vTaskDelay(pdMS_TO_TICKS(4000));

    // Read and clear the interrupt flags
    bool alarm_flag = false;
    bool timer_flag = false;
    ESP_ERROR_CHECK(bsp_rtc_get_and_clear_interrupts(&alarm_flag, &timer_flag));
    
    ESP_LOGI(TAG, "Interrupt Status - Alarm Flag: %s, Timer Flag: %s",
             alarm_flag ? "ACTIVE" : "INACTIVE",
             timer_flag ? "ACTIVE" : "INACTIVE");

    if (timer_flag) {
        ESP_LOGI(TAG, "Countdown timer test: SUCCESS");
    } else {
        ESP_LOGE(TAG, "Countdown timer test: FAILED (No interrupt flag detected)");
    }

    // Clean up countdown timer state
    ESP_ERROR_CHECK(bsp_rtc_clear_countdown_timer());
    
    ESP_LOGI(TAG, "RTC reference example completed.");
}