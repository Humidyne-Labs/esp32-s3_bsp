/**
 * @file app_ble_prov.h
 * @brief BLE GATT Wi-Fi Provisioning Manager for Web / Chrome Dashboard Pairing
 * 
 * @attribution
 * - Espressif Systems ESP-IDF wifi_provisioning
 * - Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef APP_BLE_PROV_H
#define APP_BLE_PROV_H

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Callback invoked when Wi-Fi provisioning successfully completes
 */
typedef void (*app_ble_prov_done_cb_t)(void);

/**
 * @brief Start BLE GATT Wi-Fi provisioning service
 * 
 * Advertises with name "PROV_ESP32S3-XXXXXXXX" for connection via Chrome Web Bluetooth
 * or standard Espressif BLE provisioning mobile/web clients.
 * 
 * @param service_name Device advertisement name (e.g. "PROV_ESP32S3-70041D3B")
 * @param pop Optional Proof-of-Possession PIN (NULL for open pairing)
 * @param done_cb Callback invoked once provisioning completes
 * @return esp_err_t ESP_OK on success
 */
esp_err_t app_ble_prov_start(const char *service_name, const char *pop, app_ble_prov_done_cb_t done_cb);

/**
 * @brief Check if BLE provisioning service is actively running
 * 
 * @return true if advertising/provisioning, false otherwise
 */
bool app_ble_prov_is_running(void);

/**
 * @brief Stop BLE provisioning service and free Bluetooth memory
 */
void app_ble_prov_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_BLE_PROV_H */
