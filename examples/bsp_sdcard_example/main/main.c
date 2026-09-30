/**
 * @file main.c
 * @brief Unit-level verification example for bsp_sdcard.h
 */

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp/bsp_sdcard.h"
#include "bsp/pinout.h"

static const char *TAG = "sdcard_unit_test";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting SD Card unit test...");

    // 1. Attempt to mount the SD card
    esp_err_t ret = bsp_sdcard_mount();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SD card: %s", esp_err_to_name(ret));
        // If card is missing or failed to mount, we cannot proceed with the test
        return;
    }

    // 2. Verify mount status and display capacity
    if (bsp_sdcard_is_mounted()) {
        float capacity = bsp_sdcard_get_capacity_gb();
        ESP_LOGI(TAG, "SD Card mounted successfully. Capacity: %.2f GB", capacity);

        // 3. Perform a simple file write and read test
        const char *file_path = BSP_SDCARD_MOUNT_POINT "/test.txt";
        ESP_LOGI(TAG, "Writing to file: %s", file_path);
        
        FILE *f = fopen(file_path, "w");
        if (f == NULL) {
            ESP_LOGE(TAG, "Failed to open file for writing");
        } else {
            fprintf(f, "BSP SDCard Unit Test Passed!\n");
            fclose(f);
            ESP_LOGI(TAG, "File written successfully.");
        }

        // Read back the written file to verify integrity
        ESP_LOGI(TAG, "Reading from file: %s", file_path);
        f = fopen(file_path, "r");
        if (f == NULL) {
            ESP_LOGE(TAG, "Failed to open file for reading");
        } else {
            char line[64];
            if (fgets(line, sizeof(line), f) != NULL) {
                ESP_LOGI(TAG, "Read content: %s", line);
            } else {
                ESP_LOGE(TAG, "Failed to read content from file");
            }
            fclose(f);
        }
    } else {
        ESP_LOGE(TAG, "SD Card reported as not mounted despite successful mount call.");
    }

    // 4. Unmount and cleanup
    ESP_LOGI(TAG, "Unmounting SD card...");
    ESP_ERROR_CHECK(bsp_sdcard_unmount());

    if (!bsp_sdcard_is_mounted()) {
        ESP_LOGI(TAG, "SD Card unmounted successfully. Test complete.");
    } else {
        ESP_LOGE(TAG, "Failed to unmount SD card properly.");
    }

    // Idle loop
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}