/**
 * @file bsp_nvs.c
 * @brief ESP32-S3 Touch ePaper BSP - Non-Volatile Storage (NVS) Parameter Helper Implementation
 * 
 * @copyright Copyright (c) 2026 Humidyne Labs / humid1-os-stage
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "bsp/bsp_nvs.h"

static const char *TAG = "bsp_nvs";
static const char *NVS_NAMESPACE = "board_config";
static bool s_nvs_inited = false;

esp_err_t bsp_nvs_init(void)
{
    if (s_nvs_inited) return ESP_OK;

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "Erasing and reinitializing NVS partition...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    if (ret == ESP_OK) {
        s_nvs_inited = true;
        ESP_LOGI(TAG, "NVS flash partition initialized successfully");
    } else {
        ESP_LOGE(TAG, "Failed to initialize NVS flash: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t bsp_nvs_set_str(const char *key, const char *value)
{
    if (key == NULL || value == NULL) return ESP_ERR_INVALID_ARG;
    esp_err_t ret = bsp_nvs_init();
    if (ret != ESP_OK) return ret;

    nvs_handle_t handle;
    ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK) return ret;

    ret = nvs_set_str(handle, key, value);
    if (ret == ESP_OK) {
        ret = nvs_commit(handle);
    }
    nvs_close(handle);
    return ret;
}

esp_err_t bsp_nvs_get_str(const char *key, char *buf, size_t max_len)
{
    if (key == NULL || buf == NULL || max_len == 0) return ESP_ERR_INVALID_ARG;
    esp_err_t ret = bsp_nvs_init();
    if (ret != ESP_OK) return ret;

    nvs_handle_t handle;
    ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (ret != ESP_OK) return ret;

    size_t required_size = max_len;
    ret = nvs_get_str(handle, key, buf, &required_size);
    nvs_close(handle);
    return ret;
}

esp_err_t bsp_nvs_set_u32(const char *key, uint32_t value)
{
    if (key == NULL) return ESP_ERR_INVALID_ARG;
    esp_err_t ret = bsp_nvs_init();
    if (ret != ESP_OK) return ret;

    nvs_handle_t handle;
    ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK) return ret;

    ret = nvs_set_u32(handle, key, value);
    if (ret == ESP_OK) {
        ret = nvs_commit(handle);
    }
    nvs_close(handle);
    return ret;
}

esp_err_t bsp_nvs_get_u32(const char *key, uint32_t *value)
{
    if (key == NULL || value == NULL) return ESP_ERR_INVALID_ARG;
    esp_err_t ret = bsp_nvs_init();
    if (ret != ESP_OK) return ret;

    nvs_handle_t handle;
    ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (ret != ESP_OK) return ret;

    ret = nvs_get_u32(handle, key, value);
    nvs_close(handle);
    return ret;
}

esp_err_t bsp_nvs_erase_key(const char *key)
{
    if (key == NULL) return ESP_ERR_INVALID_ARG;
    esp_err_t ret = bsp_nvs_init();
    if (ret != ESP_OK) return ret;

    nvs_handle_t handle;
    ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK) return ret;

    ret = nvs_erase_key(handle, key);
    if (ret == ESP_OK) {
        ret = nvs_commit(handle);
    }
    nvs_close(handle);
    return ret;
}
