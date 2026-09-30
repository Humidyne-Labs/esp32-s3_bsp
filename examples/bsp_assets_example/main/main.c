#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "bsp/bsp_assets.h"

static const char *TAG = "assets_example";

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing Zero-Copy Flash Asset System...");

    /* Configuration for MMAP asset storage partition */
    const char *partition_label = "storage";
    const char drive_letter = 'S';
    const int max_files = 10;
    const uint32_t checksum = 0;

    /* Initialize zero-copy flash assets */
    esp_err_t ret = bsp_assets_init(partition_label, drive_letter, max_files, checksum);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount asset partition '%s' (err: %s)", 
                 partition_label, esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "Assets initialized successfully on drive '%c:'", drive_letter);

    /* Main loop */
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}