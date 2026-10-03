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
 * @version 1.0.0
 * @attribution
 * - Architecture & Development: HUMIDYNE LABS / Humiditron
 * - AI Systems Co-Developer: Gemini (Google DeepMind)
 * - Hardware Target: Waveshare Electronics ESP32-S3 ePaper 1.54 V2
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
    BSP_PROV_EVENT_STARTED        = 0, ///< BLE advertising started
    BSP_PROV_EVENT_CRED_RECEIVED  = 1, ///< Wi-Fi SSID / Password received from client
    BSP_PROV_EVENT_CRED_SUCCESS   = 2, ///< Station successfully associated
    BSP_PROV_EVENT_CRED_FAILED    = 3, ///< Station association failed
    BSP_PROV_EVENT_FINISHED       = 4, ///< Provisioning ended, BLE de-initialized
} bsp_prov_event_t;

/**
 * @brief Provisioning Status Callback
 */
typedef void (*bsp_prov_event_cb_t)(bsp_prov_event_t event, void *event_data, void *user_data);

/**
 * @brief Start BLE GATT Wi-Fi Provisioning Service
 *
 * @param[in] custom_service_name Optional custom name (pass NULL for default "PROV_ESP32S3-XXXX")
 * @param[in] pop Optional Proof-of-Possession string (pass NULL for open pairing)
 * @param[in] cb Event callback function pointer
 * @param[in] user_data Optional user context
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_prov_start(const char *custom_service_name, const char *pop, bsp_prov_event_cb_t cb, void *user_data);

/**
 * @brief Check if Provisioning is currently active
 *
 * @return true if BLE is advertising/provisioning
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
bool bsp_prov_is_running(void);

/**
 * @brief Stop Provisioning Service and release Bluetooth memory
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
void bsp_prov_stop(void);

/**
 * @brief Generate JSON QR Code Payload string for standard Espressif Provisioning apps
 *
 * @param[in] pop Proof-of-Possession PIN (or NULL)
 * @param[out] dest Destination string buffer
 * @param[in] max_len Buffer size
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_prov_get_qr_payload(const char *pop, char *dest, size_t max_len);

/**
 * @brief Calculate optimal QR code canvas size for a given payload and constraints
 *
 * Calculates the smallest QR version required for the payload string, then multiplies by
 * an integer scale factor (pixels per module) to compute an exact whole-factor canvas size.
 * This guarantees 1:1, 2:2, 3:3, etc. integer pixel scaling without fractional distortion.
 *
 * @param[in] pop Proof-of-Possession PIN (or NULL)
 * @param[in] max_boundary Maximum display boundary size in pixels (pass <= 0 for default 180)
 * @param[in] min_scale Minimum allowed pixels per QR module (pass <= 0 for default 3)
 * @return int32_t Optimal whole-factor canvas pixel size
 */
int32_t bsp_prov_calc_qr_code_size(const char *pop, int32_t max_boundary, int32_t min_scale);

/**
 * @brief Generate raw 1-bit packed monochrome bitmap byte array for QR payload
 *
 * @param[in] pop Proof-of-Possession PIN (or NULL)
 * @param[in] max_boundary Target bounding dimension in pixels
 * @param[in] min_scale Minimum integer scale factor
 * @param[out] out_buf Destination buffer for 1-bit packed bitmap
 * @param[in] out_buf_size Size of out_buf in bytes
 * @param[out] out_size Output width/height in pixels
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_prov_get_qr_raw_bits(const char *pop, int32_t max_boundary, int32_t min_scale,
                                   uint8_t *out_buf, size_t out_buf_size, int32_t *out_size);

/**
 * @brief Render Provisioning QR Code directly on LVGL Screen / Canvas
 *
 * Renders a high-contrast 1-bit QR code centered on screen, automatically snapped to an exact whole-factor scale.
 *
 * @param[out] parent Parent LVGL object (e.g. lv_screen_active())
 * @param[in] size Dimension in pixels or max boundary (pass <= 0 for auto 180px max)
 * @param[in] pop Proof-of-Possession PIN (or NULL)
 * @return lv_obj_t* Pointer to created QR code widget or NULL
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
lv_obj_t *bsp_prov_render_qr_code(lv_obj_t *parent, int32_t size, const char *pop);

#ifdef __cplusplus
}
#endif

#endif /* BSP_PROV_H */
