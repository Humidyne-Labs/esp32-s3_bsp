/**
 * @file main.c
 * @brief Standalone BSP Error Handling & System Diagnostics Reference Example
 *
 * Target: ESP32-S3 / Waveshare ESP32-S3 ePaper 1.54 V2
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "bsp/bsp_err.h"

static const char *TAG = "bsp_err_example";

void app_main(void)
{
    ESP_LOGI(TAG, "=== BSP Error Codes & System Diagnostics Test ===");

    /* 1. Query Silicon Revision numbers */
    uint32_t major = 0;
    uint32_t minor = 0;
    ESP_ERROR_CHECK(bsp_get_chip_revision(&major, &minor));
    ESP_LOGI(TAG, "Silicon Revision (Numeric): v%lu.%lu",
             (unsigned long)major, (unsigned long)minor);

    /* 2. Query Silicon Revision string representation */
    const char *rev_str = bsp_get_chip_revision_str();
    ESP_LOGI(TAG, "Silicon Revision (String): %s", rev_str != NULL ? rev_str : "UNKNOWN");

    /* 3. Validate BSP Error Code Translation */
    ESP_LOGI(TAG, "Testing BSP error code string translation:");
    ESP_LOGI(TAG, "  0x%04X -> %s", BSP_ERR_NOT_INITIALIZED,
             bsp_err_to_name(BSP_ERR_NOT_INITIALIZED));
    ESP_LOGI(TAG, "  0x%04X -> %s", BSP_ERR_I2C_BUS_LOCKED,
             bsp_err_to_name(BSP_ERR_I2C_BUS_LOCKED));
    ESP_LOGI(TAG, "  0x%04X -> %s", BSP_ERR_SENSOR_CRC_FAIL,
             bsp_err_to_name(BSP_ERR_SENSOR_CRC_FAIL));
    ESP_LOGI(TAG, "  0x%04X -> %s", BSP_ERR_DISPLAY_BUSY_TIMEOUT,
             bsp_err_to_name(BSP_ERR_DISPLAY_BUSY_TIMEOUT));
    ESP_LOGI(TAG, "  0x%04X -> %s", BSP_ERR_AUDIO_NOT_READY,
             bsp_err_to_name(BSP_ERR_AUDIO_NOT_READY));
    ESP_LOGI(TAG, "  0x%04X -> %s", BSP_ERR_SD_CARD_MOUNT,
             bsp_err_to_name(BSP_ERR_SD_CARD_MOUNT));
    ESP_LOGI(TAG, "  0x%04X -> %s", BSP_ERR_WIFI_DISCONNECTED,
             bsp_err_to_name(BSP_ERR_WIFI_DISCONNECTED));
    ESP_LOGI(TAG, "  0x%04X -> %s", BSP_ERR_OTA_VALIDATION,
             bsp_err_to_name(BSP_ERR_OTA_VALIDATION));

    /* 4. Acquire Diagnostic Information Snapshot */
    bsp_diag_info_t diag = {0};
    ESP_ERROR_CHECK(bsp_get_diagnostics(&diag));

    ESP_LOGI(TAG, "System Diagnostic Snapshot:");
    ESP_LOGI(TAG, "  BSP Version:          %s", diag.bsp_version ? diag.bsp_version : "N/A");
    ESP_LOGI(TAG, "  Chip Model:           %s", diag.chip_model ? diag.chip_model : "N/A");
    ESP_LOGI(TAG, "  Chip Revision:        %s (%u)",
             diag.chip_revision_str ? diag.chip_revision_str : "N/A",
             (unsigned int)diag.chip_revision);
    ESP_LOGI(TAG, "  CPU Cores:            %u", (unsigned int)diag.chip_cores);
    ESP_LOGI(TAG, "  Free Internal Heap:   %lu bytes", (unsigned long)diag.free_internal_heap);
    ESP_LOGI(TAG, "  Min Free Int Heap:    %lu bytes", (unsigned long)diag.min_free_internal_heap);
    ESP_LOGI(TAG, "  Free PSRAM Heap:      %lu bytes", (unsigned long)diag.free_psram_heap);
    ESP_LOGI(TAG, "  Uptime:               %lu s", (unsigned long)diag.uptime_seconds);
    ESP_LOGI(TAG, "  Battery:              %lu mV (%d%%)",
             (unsigned long)diag.battery_mv, (int)diag.battery_percentage);
    ESP_LOGI(TAG, "  Power Rail Good:      %s", diag.power_rail_good ? "YES" : "NO");
    ESP_LOGI(TAG, "  I2C Bus Healthy:      %s", diag.i2c_bus_healthy ? "YES" : "NO");
    ESP_LOGI(TAG, "  Display Ready:        %s", diag.display_ready ? "YES" : "NO");
    ESP_LOGI(TAG, "  Wi-Fi Connected:      %s", diag.wifi_connected ? "YES" : "NO");
    ESP_LOGI(TAG, "  Wi-Fi RSSI:           %d dBm", (int)diag.wifi_rssi);

    /* 5. Print Full Diagnostic Report via BSP Helper */
    ESP_LOGI(TAG, "Executing full diagnostics dump:");
    bsp_diagnostics_dump();

    ESP_LOGI(TAG, "=== BSP Diagnostics Test Completed Successfully ===");
}