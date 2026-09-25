/**
 * @file bsp_nvs.c
 * @brief Non-Volatile Storage (NVS) Flash Helper & Persistent Configuration Store Implementation
 * 
 * @attribution
 * - Espressif Systems
 * - BSP Implementation: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "bsp/bsp_nvs.h"

static const char *TAG = "bsp_nvs";
#define BSP_NVS_NAMESPACE "humid_bsp"

static bool s_nvs_initialized = false;

esp_err_t bsp_nvs_init(void)
{
    if (s_nvs_initialized) return ESP_OK;

    ESP_LOGI(TAG, "Initializing NVS Flash Subsystem");

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition truncated or version mismatch; erasing and re-initializing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    if (err == ESP_OK) {
        s_nvs_initialized = true;
    } else {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(err));
    }

    return err;
}

esp_err_t bsp_nvs_set_str(const char *key, const char *value)
{
    if (key == NULL || value == NULL) return ESP_ERR_INVALID_ARG;
    if (!s_nvs_initialized) bsp_nvs_init();

    nvs_handle_t handle;
    esp_err_t err = nvs_open(BSP_NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    err = nvs_set_str(handle, key, value);
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }

    nvs_close(handle);
    return err;
}

esp_err_t bsp_nvs_get_str(const char *key, char *out_val, size_t max_len)
{
    if (key == NULL || out_val == NULL || max_len == 0) return ESP_ERR_INVALID_ARG;
    if (!s_nvs_initialized) bsp_nvs_init();

    nvs_handle_t handle;
    esp_err_t err = nvs_open(BSP_NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) return err;

    size_t required_size = max_len;
    err = nvs_get_str(handle, key, out_val, &required_size);

    nvs_close(handle);
    return err;
}

esp_err_t bsp_nvs_set_u32(const char *key, uint32_t value)
{
    if (key == NULL) return ESP_ERR_INVALID_ARG;
    if (!s_nvs_initialized) bsp_nvs_init();

    nvs_handle_t handle;
    esp_err_t err = nvs_open(BSP_NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    err = nvs_set_u32(handle, key, value);
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }

    nvs_close(handle);
    return err;
}

esp_err_t bsp_nvs_get_u32(const char *key, uint32_t *out_val)
{
    if (key == NULL || out_val == NULL) return ESP_ERR_INVALID_ARG;
    if (!s_nvs_initialized) bsp_nvs_init();

    nvs_handle_t handle;
    esp_err_t err = nvs_open(BSP_NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) return err;

    err = nvs_get_u32(handle, key, out_val);

    nvs_close(handle);
    return err;
}

esp_err_t bsp_nvs_clear_wifi_credentials(void)
{
    if (!s_nvs_initialized) bsp_nvs_init();

    nvs_handle_t handle;
    esp_err_t err = nvs_open(BSP_NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    nvs_erase_key(handle, "wifi_ssid");
    nvs_erase_key(handle, "wifi_pass");
    nvs_commit(handle);
    nvs_close(handle);

    ESP_LOGI(TAG, "Cleared stored Wi-Fi credentials from NVS");
    return ESP_OK;
}

esp_err_t bsp_nvs_wipe_all(void)
{
    if (!s_nvs_initialized) bsp_nvs_init();

    nvs_handle_t handle;
    esp_err_t err = nvs_open(BSP_NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    err = nvs_erase_all(handle);
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }

    nvs_close(handle);
    ESP_LOGW(TAG, "Completely erased all keys from NVS namespace '%s'", BSP_NVS_NAMESPACE);
    return err;
}
