#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "bsp/bsp_ota.h"

static const char *TAG = "ota_example";

/**
 * @brief Progress callback demonstrating HTTPS OTA event notifications
 */
static void ota_progress_cb(bsp_ota_status_t status, int progress_pct, const char *msg, void *user_data)
{
    (void)user_data;
    ESP_LOGI(TAG, "OTA Status: %d, Progress: %d%%, Msg: %s",
             (int)status, progress_pct, msg ? msg : "");
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== BSP OTA Subsystem Example ===");

    // 1. Retrieve and log running application descriptor
    const esp_app_desc_t *app_desc = bsp_ota_get_app_desc();
    if (app_desc != NULL) {
        ESP_LOGI(TAG, "Project: %s, Version: %s, IDF: %s",
                 app_desc->project_name, app_desc->version, app_desc->idf_ver);
    }

    // 2. Mark current running slot valid to prevent rollback
    esp_err_t ret = bsp_ota_mark_valid();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Running firmware marked as valid");
    } else {
        ESP_LOGW(TAG, "bsp_ota_mark_valid status: %s", esp_err_to_name(ret));
    }

    // 3. Demonstrate chunked OTA session lifecycle (Begin -> Write -> Abort)
    // Use a small size (4096 bytes) to avoid erasing the whole partition
    const uint8_t mock_chunk[64] = {0};
    esp_ota_handle_t ota_handle = 0;

    ret = bsp_ota_begin(sizeof(mock_chunk), &ota_handle);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "OTA session opened, handle: %u", (unsigned int)ota_handle);

        ret = bsp_ota_write(ota_handle, mock_chunk, sizeof(mock_chunk));
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Wrote %u bytes of mock payload", (unsigned int)sizeof(mock_chunk));
        }

        // Abort the session cleanly
        ESP_ERROR_CHECK(bsp_ota_abort(ota_handle));
        ESP_LOGI(TAG, "OTA session aborted successfully");
    } else {
        ESP_LOGW(TAG, "bsp_ota_begin returned: %s", esp_err_to_name(ret));
    }

    // 4. Test callback invocation directly
    ota_progress_cb(BSP_OTA_STATUS_IDLE, 0, "Subsystem ready", NULL);

    ESP_LOGI(TAG, "OTA example completed.");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}