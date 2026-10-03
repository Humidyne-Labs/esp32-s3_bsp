/**
 * @file main.c
 * @brief Corrected reference example for bsp_time.h peripheral usage.
 */

#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp/bsp.h"
#include "bsp/bsp_time.h"

static const char *TAG = "bsp_time_example";

void app_main(void)
{
    // 1. Initialize BSP
    ESP_ERROR_CHECK(bsp_board_init());

    // 2. Configure Timezone
    ESP_ERROR_CHECK(bsp_time_set_timezone("EST5EDT,M3.2.0,M11.1.0"));

    // 3. Attempt SNTP Sync
    ESP_LOGI(TAG, "Attempting SNTP synchronization...");
    esp_err_t ret = bsp_time_sntp_sync(10000);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "SNTP Sync successful.");
    } else {
        ESP_LOGW(TAG, "SNTP Sync failed: %s. Falling back to RTC hardware.", esp_err_to_name(ret));
        // Fallback: Sync system time from external RTC
        ESP_ERROR_CHECK(bsp_time_sync_rtc_to_system());
    }

    // 4. Demonstrate Formatted Time/Date String Generators
    char time_buf[32];
    char date_buf[32];

    // Get 12-hour format with seconds
    ESP_ERROR_CHECK(bsp_time_get_formatted(BSP_TIME_FMT_12H_SEC, time_buf, sizeof(time_buf)));
    
    // Get full date string (MM/DD/YY DayOfWeek) using the specific function from header
    ESP_ERROR_CHECK(bsp_time_get_date_dow_str(date_buf, sizeof(date_buf)));

    ESP_LOGI(TAG, "Current Time: %s", time_buf);
    ESP_LOGI(TAG, "Current Date: %s", date_buf);

    // 5. Start background periodic sync (every 24 hours)
    ESP_ERROR_CHECK(bsp_time_start_periodic_sync(86400));

    // 6. Idle loop
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(60000));
        
        // Use 24H format for heartbeat
        if (bsp_time_get_formatted(BSP_TIME_FMT_24H_SEC, time_buf, sizeof(time_buf)) == ESP_OK) {
            ESP_LOGI(TAG, "Heartbeat - Time: %s", time_buf);
        }
    }
}