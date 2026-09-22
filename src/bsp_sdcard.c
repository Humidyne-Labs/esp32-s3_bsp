/**
 * @file bsp_sdcard.c
 * @brief sdcard lib
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics (https://www.waveshare.com)
 * - Microcontroller: Espressif Systems ESP32-S3 (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "esp_log.h"
#include "bsp/bsp_sdcard.h"

static const char *TAG = "bsp_sdcard";
static sdmmc_card_t *s_sd_card = NULL;

esp_err_t bsp_sdcard_mount(void)
{
    if (s_sd_card != NULL) {
        ESP_LOGW(TAG, "SD Card already mounted");
        return ESP_OK;
    }

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024,
    };

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.max_freq_khz = SDMMC_FREQ_DEFAULT; // 20MHz default for initial handshake
    host.flags = SDMMC_HOST_FLAG_1BIT;

    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 1;
    slot_config.clk = (gpio_num_t)BSP_GPIO_SD_CLK;
    slot_config.cmd = (gpio_num_t)BSP_GPIO_SD_MOSI;
    slot_config.d0  = (gpio_num_t)BSP_GPIO_SD_MISO;
    slot_config.cd  = SDMMC_SLOT_NO_CD; // No hardware Card Detect pin connected
    slot_config.wp  = SDMMC_SLOT_NO_WP;

    // Temporarily reduce logging from sdmmc stack so missing cards don't dump error traces
    esp_log_level_t prev_sdmmc_log = esp_log_level_get("sdmmc_common");
    esp_log_level_t prev_vfs_log   = esp_log_level_get("vfs_fat_sdmmc");
    esp_log_level_set("sdmmc_common", ESP_LOG_WARN);
    esp_log_level_set("vfs_fat_sdmmc", ESP_LOG_WARN);

    esp_err_t ret = esp_vfs_fat_sdmmc_mount(BSP_SDCARD_MOUNT_POINT, &host, &slot_config, &mount_config, &s_sd_card);

    // Restore logging levels
    esp_log_level_set("sdmmc_common", prev_sdmmc_log);
    esp_log_level_set("vfs_fat_sdmmc", prev_vfs_log);

    if (ret == ESP_ERR_TIMEOUT || ret == ESP_ERR_INVALID_RESPONSE) {
        ESP_LOGI(TAG, "No SD card detected in slot");
        s_sd_card = NULL;
        return ESP_ERR_NOT_FOUND;
    } else if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SD card (%s)", esp_err_to_name(ret));
        s_sd_card = NULL;
        return ret;
    }

    ESP_LOGI(TAG, "SD Card mounted at %s (Capacity: %.2f GB)", BSP_SDCARD_MOUNT_POINT, bsp_sdcard_get_capacity_gb());
    return ESP_OK;
}

esp_err_t bsp_sdcard_unmount(void)
{
    if (s_sd_card == NULL) return ESP_OK;

    esp_err_t ret = esp_vfs_fat_sdcard_unmount(BSP_SDCARD_MOUNT_POINT, s_sd_card);
    if (ret == ESP_OK) {
        s_sd_card = NULL;
        ESP_LOGI(TAG, "SD Card unmounted");
    }
    return ret;
}

float bsp_sdcard_get_capacity_gb(void)
{
    if (s_sd_card == NULL) return 0.0f;
    return (float)(s_sd_card->csd.capacity) / 2048.0f / 1024.0f;
}

bool bsp_sdcard_is_mounted(void)
{
    return (s_sd_card != NULL);
}