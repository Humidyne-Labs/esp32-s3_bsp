#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp/bsp_lvgl.h"

static const char *TAG = "example_lvgl";

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing LVGL graphics subsystem...");

    /* 1. Initialize LVGL drawing buffers and display port */
    ESP_ERROR_CHECK(bsp_lvgl_init());

    /* 2. Configure first flush to perform a full OTP refresh for clean cold start */
    bsp_lvgl_set_first_flush_mode(true);

    /* 3. Start LVGL FreeRTOS background task (Priority: 5, Core: 1) */
    ESP_LOGI(TAG, "Starting LVGL background task...");
    ESP_ERROR_CHECK(bsp_lvgl_start(5, 1));

    /* 4. Safely access LVGL within the reentrant mutex lock */
    if (bsp_lvgl_lock()) {
        ESP_LOGI(TAG, "LVGL mutex acquired successfully");
        bsp_lvgl_unlock();
        ESP_LOGI(TAG, "LVGL mutex released");
    } else {
        ESP_LOGE(TAG, "Failed to acquire LVGL mutex lock");
    }

    /* 5. Allow background task to execute briefly */
    vTaskDelay(pdMS_TO_TICKS(100));

    /* 6. Cleanly stop LVGL background task */
    ESP_LOGI(TAG, "Stopping LVGL background task...");
    ESP_ERROR_CHECK(bsp_lvgl_stop());

    ESP_LOGI(TAG, "LVGL example completed successfully");
}