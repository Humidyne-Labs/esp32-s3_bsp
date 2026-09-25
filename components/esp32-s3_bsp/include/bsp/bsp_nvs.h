/**
 * @file bsp_nvs.h
 * @brief Non-Volatile Storage (NVS) Flash Helper & Persistent Configuration Store
 * 
 * Provides thread-safe, structured key-value persistence in ESP32 SPI Flash for:
 *  - Provisioned Wi-Fi Credentials ("wifi_ssid", "wifi_pass")
 *  - ThingsBoard Tokens & Claim State ("tb_token", "tb_claimed")
 *  - Persistent Boot Counters ("boot_count")
 *  - Complete Factory Reset Wiping
 * 
 * @attribution
 * - Espressif Systems (NVS Flash Architecture)
 * - BSP Implementation: Humidyne Labs / Humiditron (2026)
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
 * @brief Initialize Non-Volatile Storage (NVS) Subsystem
 * 
 * Automatically recovers and re-initializes if the partition table is truncated or empty.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_nvs_init(void);

/**
 * @brief Store a String Value in NVS
 * 
 * @param key Key name (up to 15 characters)
 * @param value Null-terminated string value
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_nvs_set_str(const char *key, const char *value);

/**
 * @brief Retrieve a String Value from NVS
 * 
 * @param key Key name
 * @param[out] out_val Destination character buffer
 * @param max_len Size of buffer
 * @return esp_err_t ESP_OK on success, or ESP_ERR_NVS_NOT_FOUND if key doesn't exist
 */
esp_err_t bsp_nvs_get_str(const char *key, char *out_val, size_t max_len);

/**
 * @brief Store an Unsigned 32-bit Integer in NVS
 * 
 * @param key Key name
 * @param value 32-bit unsigned integer
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_nvs_set_u32(const char *key, uint32_t value);

/**
 * @brief Retrieve an Unsigned 32-bit Integer from NVS
 * 
 * @param key Key name
 * @param[out] out_val Pointer to receive 32-bit integer
 * @return esp_err_t ESP_OK on success, or ESP_ERR_NVS_NOT_FOUND if key doesn't exist
 */
esp_err_t bsp_nvs_get_u32(const char *key, uint32_t *out_val);

/**
 * @brief Clear Stored Wi-Fi SSID and Password from NVS
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_nvs_clear_wifi_credentials(void);

/**
 * @brief Perform Complete Factory Wipe of All NVS Keys in Namespace
 * 
 * Erases all configuration, claiming tokens, and boot history.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_nvs_wipe_all(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_NVS_H */
