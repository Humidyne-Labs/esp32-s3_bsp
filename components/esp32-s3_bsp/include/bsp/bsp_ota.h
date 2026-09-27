/**
 * @file bsp_ota.h
 * @brief Generic Over-The-Air (OTA) Dual-Slot Firmware Update Subsystem
 * 
 * Hardware Target:
 *  - Microcontroller: Espressif Systems ESP32-S3-PICO-1-N8R8
 *  - Target Board: Waveshare ESP32-S3 ePaper 1.54 V2 (8MB Flash Dual OTA)
 * 
 * Features:
 *  - Dual-slot OTA management (ota_0 / ota_1 / otadata)
 *  - Chunked stream writing API for custom protocols (ThingsBoard, MQTT, BLE)
 *  - Direct HTTPS OTA downloader with progress callbacks and cert bundle validation
 *  - Firmware rollback and validation guard
 * 
 * @attribution
 * - BSP Architecture: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_OTA_H
#define BSP_OTA_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"
#include "esp_ota_ops.h"
#include "esp_app_desc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief OTA Operation Status
 */
typedef enum {
    BSP_OTA_STATUS_IDLE        = 0,
    BSP_OTA_STATUS_STARTING    = 1,
    BSP_OTA_STATUS_DOWNLOADING = 2,
    BSP_OTA_STATUS_VERIFYING   = 3,
    BSP_OTA_STATUS_SUCCESS     = 4,
    BSP_OTA_STATUS_FAILED      = 5,
} bsp_ota_status_t;

/**
 * @brief OTA Progress Callback Signature
 */
typedef void (*bsp_ota_progress_cb_t)(bsp_ota_status_t status, int progress_pct, const char *msg, void *user_data);

/**
 * @brief Begin a Chunked OTA Update Session
 * 
 * Selects next inactive OTA partition and initializes flash erase.
 * 
 * @param image_size Expected total binary size in bytes (or OTA_SIZE_UNKNOWN)
 * @param[out] out_handle Destination pointer to receive OTA session handle
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_ota_begin(size_t image_size, esp_ota_handle_t *out_handle);

/**
 * @brief Write a Data Chunk to the Active OTA Session
 * 
 * @param handle OTA session handle from bsp_ota_begin()
 * @param data Chunk buffer pointer
 * @param size Chunk length in bytes
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_ota_write(esp_ota_handle_t handle, const void *data, size_t size);

/**
 * @brief Finalize OTA Session, Validate Header, and Set Boot Partition
 * 
 * @param handle OTA session handle
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_ota_end(esp_ota_handle_t handle);

/**
 * @brief Abort an in-progress OTA Session
 * 
 * @param handle OTA session handle
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_ota_abort(esp_ota_handle_t handle);

/**
 * @brief Download and Flash Firmware directly from HTTPS URL
 * 
 * Uses system TLS certificate bundle and streams chunked download.
 * 
 * @param url HTTPS URL to firmware .bin image
 * @param cb Progress callback pointer
 * @param user_data Optional user context
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_ota_from_url(const char *url, bsp_ota_progress_cb_t cb, void *user_data);

/**
 * @brief Mark the currently running firmware partition as valid (prevents auto-rollback)
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_ota_mark_valid(void);

/**
 * @brief Mark current firmware invalid and trigger rollback to previous working slot
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_ota_rollback(void);

/**
 * @brief Retrieve application descriptor for the currently running image
 * 
 * @return const esp_app_desc_t* Pointer to app description (version, project name, compile time)
 */
const esp_app_desc_t *bsp_ota_get_app_desc(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_OTA_H */
