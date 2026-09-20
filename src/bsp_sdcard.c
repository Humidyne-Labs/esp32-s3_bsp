#include <stdio.h>
#include <string.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "esp_log.h"
#include "bsp/bsp_sdcard.h"

/* SDMMC one-wire setup and capacity calculation adapted from
 * .port_bsp_org/port_sdcard.cpp. */

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
    host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;

    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 1;
    slot_config.clk = (gpio_num_t)BSP_GPIO_SD_CLK;
    slot_config.cmd = (gpio_num_t)BSP_GPIO_SD_MOSI;
    slot_config.d0 = (gpio_num_t)BSP_GPIO_SD_MISO;

    esp_err_t ret = esp_vfs_fat_sdmmc_mount(BSP_SDCARD_MOUNT_POINT, &host, &slot_config, &mount_config, &s_sd_card);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SD card VFS: %s", esp_err_to_name(ret));
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
        ESP_LOGI(TAG, "SD Card unmounted successfully");
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
