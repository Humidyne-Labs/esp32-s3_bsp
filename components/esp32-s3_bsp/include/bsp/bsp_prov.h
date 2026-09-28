/**
 * @file bsp_prov.h
 * @brief Turnkey BLE GATT Wi-Fi Provisioning & QR Code Generator
 * 
 * Features:
 *  - BLE GATT Wi-Fi Provisioning with automatic device name / PoP generation
 *  - Standard QR Code payload builder for mobile / Chrome web dashboards
 *  - Direct LVGL QR Code Widget Renderer on 200x200 1-bit E-Paper Display
 *  - UI Status Callbacks (Start, Credentials Received, Success, Failure)
 * 
 * Hardware Target:
 *  - Microcontroller: Espressif Systems ESP32-S3-PICO-1-N8R8
 *  - Target Board: Waveshare ESP32-S3 ePaper 1.54 V2
 * 
 * @attribution
 * - BSP Architecture: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_PROV_H
#define BSP_PROV_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Provisioning Event Types
 */
typedef enum {
    BSP_PROV_EVENT_STARTED        = 0, /*!< BLE advertising started                    */
    BSP_PROV_EVENT_CRED_RECEIVED  = 1, /*!< Wi-Fi SSID / Password received from client */
    BSP_PROV_EVENT_CRED_SUCCESS   = 2, /*!< Station successfully associated            */
    BSP_PROV_EVENT_CRED_FAILED    = 3, /*!< Station association failed                 */
    BSP_PROV_EVENT_FINISHED       = 4, /*!< Provisioning ended, BLE de-initialized     */
} bsp_prov_event_t;

/**
 * @brief Provisioning Status Callback
 */
typedef void (*bsp_prov_event_cb_t)(bsp_prov_event_t event, void *event_data, void *user_data);

/**
 * @brief Start BLE GATT Wi-Fi Provisioning Service
 * 
 * @param custom_service_name Optional custom name (pass NULL for default "PROV_ESP32S3-XXXX")
 * @param pop Optional Proof-of-Possession string (pass NULL for open pairing)
 * @param cb Event callback function pointer
 * @param user_data Optional user context
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_prov_start(const char *custom_service_name, const char *pop, bsp_prov_event_cb_t cb, void *user_data);

/**
 * @brief Check if Provisioning is currently active
 * 
 * @return true if BLE is advertising/provisioning
 */
bool bsp_prov_is_running(void);

/**
 * @brief Stop Provisioning Service and release Bluetooth memory
 */
void bsp_prov_stop(void);

/**
 * @brief Generate JSON QR Code Payload string for standard Espressif Provisioning apps
 * 
 * @param pop Proof-of-Possession PIN (or NULL)
 * @param dest Destination string buffer
 * @param max_len Buffer size
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_prov_get_qr_payload(const char *pop, char *dest, size_t max_len);

/**
 * @brief Render Provisioning QR Code directly on LVGL Screen / Canvas
 * 
 * Renders a high-contrast 1-bit QR code centered on screen (e.g. 140x140 px).
 * 
 * @param parent Parent LVGL object (e.g. lv_screen_active())
 * @param size Dimension in pixels (e.g. 140)
 * @param pop Proof-of-Possession PIN (or NULL)
 * @return lv_obj_t* Pointer to created QR code widget or NULL
 */
lv_obj_t *bsp_prov_render_qr_code(lv_obj_t *parent, int32_t size, const char *pop);

#ifdef __cplusplus
}
#endif

#endif /* BSP_PROV_H */
