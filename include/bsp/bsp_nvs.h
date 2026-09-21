/**
 * @file bsp_nvs.h
 * @brief nvs lib
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics (https://www.waveshare.com)
 * - Microcontroller: Espressif Systems ESP32-S3 (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_NVS_H
#define BSP_NVS_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize Non-Volatile Storage (NVS) partition
 * 
 * Handles partition initialization, formatting if corrupted, and mounting.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_nvs_init(void);

/**
 * @brief Store a string parameter in NVS (e.g., wifi_ssid, wifi_passkey)
 * 
 * @param key Parameter key string (max 15 characters)
 * @param value String value to save
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_nvs_set_str(const char *key, const char *value);

/**
 * @brief Retrieve a string parameter from NVS
 * 
 * @param key Parameter key string
 * @param buf Buffer to store retrieved string
 * @param max_len Size of destination buffer
 * @return esp_err_t ESP_OK on success, ESP_ERR_NVS_NOT_FOUND if missing
 */
esp_err_t bsp_nvs_get_str(const char *key, char *buf, size_t max_len);

/**
 * @brief Store a 32-bit integer parameter in NVS
 * 
 * @param key Parameter key string
 * @param value 32-bit integer value
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_nvs_set_u32(const char *key, uint32_t value);

/**
 * @brief Retrieve a 32-bit integer parameter from NVS
 * 
 * @param key Parameter key string
 * @param value Pointer to store 32-bit integer
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_nvs_get_u32(const char *key, uint32_t *value);

/**
 * @brief Erase a specific parameter key from NVS
 * 
 * @param key Key to erase
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_nvs_erase_key(const char *key);

#ifdef __cplusplus
}
#endif

#endif /* BSP_NVS_H */
