/**
 * @file bsp_ota.c
 * @brief Generic Over-The-Air (OTA) Dual-Slot Firmware Update Implementation
 * 
 * @attribution
 * - Espressif Systems ESP-IDF esp_ota_ops / esp_https_ota
 * - BSP Architecture: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_https_ota.h"
#include "esp_crt_bundle.h"
#include "bsp/bsp_ota.h"

static const char *TAG = "bsp_ota";

static const esp_partition_t *s_update_partition = NULL;

esp_err_t bsp_ota_begin(size_t image_size, esp_ota_handle_t *out_handle)
{
    if (out_handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    const esp_partition_t *running = esp_ota_get_running_partition();
    s_update_partition = esp_ota_get_next_update_partition(NULL);

    if (s_update_partition == NULL) {
        ESP_LOGE(TAG, "No valid OTA partition found for writing");
        return ESP_ERR_NOT_FOUND;
    }

    ESP_LOGI(TAG, "Starting OTA update. Running: %s (0x%08lX), Target: %s (0x%08lX)",
             running->label, (unsigned long)running->address,
             s_update_partition->label, (unsigned long)s_update_partition->address);

    esp_err_t ret = esp_ota_begin(s_update_partition, image_size, out_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t bsp_ota_write(esp_ota_handle_t handle, const void *data, size_t size)
{
    if (data == NULL || size == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    return esp_ota_write(handle, data, size);
}

esp_err_t bsp_ota_end(esp_ota_handle_t handle)
{
    esp_err_t ret = esp_ota_end(handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_end validation failed: %s", esp_err_to_name(ret));
        return ret;
    }

    if (s_update_partition != NULL) {
        ret = esp_ota_set_boot_partition(s_update_partition);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "OTA update complete! Next boot partition set to: %s", s_update_partition->label);
        } else {
            ESP_LOGE(TAG, "Failed to set boot partition: %s", esp_err_to_name(ret));
        }
    }
    return ret;
}

esp_err_t bsp_ota_abort(esp_ota_handle_t handle)
{
    ESP_LOGW(TAG, "Aborting in-progress OTA update session");
    return esp_ota_abort(handle);
}

esp_err_t bsp_ota_from_url(const char *url, bsp_ota_progress_cb_t cb, void *user_data)
{
    if (url == NULL || strlen(url) == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Initiating HTTPS OTA download from: %s", url);
    if (cb) cb(BSP_OTA_STATUS_STARTING, 0, "Connecting to firmware server...", user_data);

    esp_http_client_config_t http_config = {
        .url               = url,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .timeout_ms        = 15000,
        .keep_alive_enable = true,
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &http_config,
    };

    esp_https_ota_handle_t https_ota_handle = NULL;
    esp_err_t ret = esp_https_ota_begin(&ota_config, &https_ota_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_https_ota_begin failed: %s", esp_err_to_name(ret));
        if (cb) cb(BSP_OTA_STATUS_FAILED, 0, "Failed to start HTTPS OTA session", user_data);
        return ret;
    }

    if (cb) cb(BSP_OTA_STATUS_DOWNLOADING, 0, "Downloading firmware binary...", user_data);

    int total_bytes = esp_https_ota_get_image_size(https_ota_handle);
    int read_bytes  = 0;

    while (1) {
        ret = esp_https_ota_perform(https_ota_handle);
        if (ret != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
            break;
        }

        read_bytes = esp_https_ota_get_image_len_read(https_ota_handle);
        int progress = (total_bytes > 0) ? (read_bytes * 100) / total_bytes : 0;
        if (cb) cb(BSP_OTA_STATUS_DOWNLOADING, progress, "Writing to flash...", user_data);
    }

    if (esp_https_ota_is_complete_data_received(https_ota_handle)) {
        if (cb) cb(BSP_OTA_STATUS_VERIFYING, 100, "Verifying image signature...", user_data);
        ret = esp_https_ota_finish(https_ota_handle);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "HTTPS OTA successfully completed and verified");
            if (cb) cb(BSP_OTA_STATUS_SUCCESS, 100, "Firmware update successful", user_data);
            return ESP_OK;
        }
    } else {
        esp_https_ota_abort(https_ota_handle);
    }

    ESP_LOGE(TAG, "HTTPS OTA failed: %s", esp_err_to_name(ret));
    if (cb) cb(BSP_OTA_STATUS_FAILED, 0, "OTA Download / Verification Failed", user_data);
    return ret;
}

esp_err_t bsp_ota_mark_valid(void)
{
    esp_err_t ret = esp_ota_mark_app_valid_cancel_rollback();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Running firmware partition confirmed valid (Rollback cancelled)");
    }
    return ret;
}

esp_err_t bsp_ota_rollback(void)
{
    ESP_LOGW(TAG, "Triggering firmware rollback to previous partition and rebooting!");
    return esp_ota_mark_app_invalid_rollback_and_reboot();
}

const esp_app_desc_t *bsp_ota_get_app_desc(void)
{
    return esp_app_get_description();
}
