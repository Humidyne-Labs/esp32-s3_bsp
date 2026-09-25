/**
 * @file bsp_wifi.h
 * @brief Wi-Fi Station Manager with RTC Fast Reconnect Session Caching (<400ms)
 * 
 * Performance Overview:
 *  - Standard Wi-Fi Station association requires full channel scanning (~2500 - 4500ms).
 *  - This module caches the last successful AP's BSSID, RF Channel, and Auth Mode in RTC Fast Memory.
 *  - On wake from Deep Sleep, it attempts a direct fast association to the cached channel and BSSID,
 *    reducing association time to under 400ms and saving significant battery energy.
 * 
 * @attribution
 * - Espressif Systems (ESP-IDF Wi-Fi Driver Architecture)
 * - BSP Implementation: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_WIFI_H
#define BSP_WIFI_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize Wi-Fi Subsystem in Station Mode
 * 
 * Sets up the default network interface (netif), initializes the L2 event loop,
 * and prepares the driver for connection.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_wifi_init(void);

/**
 * @brief Connect to Wi-Fi Network with Specified SSID and Password
 * 
 * Uses fast RTC cache if matching SSID was previously stored, otherwise performs full scan.
 * 
 * @param ssid Target Access Point SSID
 * @param password Network password (or empty string for open networks)
 * @param timeout_ms Maximum time to wait for IP assignment in milliseconds
 * @return esp_err_t ESP_OK on successful IP acquisition, or error code on timeout/failure
 */
esp_err_t bsp_wifi_connect(const char *ssid, const char *password, uint32_t timeout_ms);

/**
 * @brief Connect to Wi-Fi using Credentials Stored in NVS Flash
 * 
 * Reads "wifi_ssid" and "wifi_pass" from NVS. If missing or invalid, returns ESP_ERR_NVS_NOT_FOUND.
 * 
 * @param timeout_ms Maximum time to wait for IP assignment
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_wifi_connect_from_nvs(uint32_t timeout_ms);

/**
 * @brief Disconnect and Stop Wi-Fi Station
 * 
 * Cleanly terminates association and turns off RF circuitry prior to deep sleep.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_wifi_disconnect(void);

/**
 * @brief Check if Wi-Fi is Currently Connected and has Valid IP Address
 * 
 * @return true if connected with valid IP, false otherwise
 */
bool bsp_wifi_is_connected(void);

/**
 * @brief Retrieve Current Received Signal Strength Indicator (RSSI)
 * 
 * @param[out] out_rssi Pointer to receive RSSI value in dBm (e.g. -55 dBm)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_wifi_get_rssi(int *out_rssi);

/**
 * @brief Retrieve Formatted Local IPv4 Address String
 * 
 * @param[out] out_ip Destination character buffer
 * @param max_len Size of buffer (minimum 16 bytes recommended)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_wifi_get_ip_str(char *out_ip, size_t max_len);

/**
 * @brief Save New Wi-Fi Credentials into NVS Flash
 * 
 * @param ssid AP SSID string
 * @param password AP Password string
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_wifi_save_credentials(const char *ssid, const char *password);

/**
 * @brief Invalidate and Clear the RTC Fast Reconnect Session Cache
 */
void bsp_wifi_invalidate_fast_cache(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_WIFI_H */
