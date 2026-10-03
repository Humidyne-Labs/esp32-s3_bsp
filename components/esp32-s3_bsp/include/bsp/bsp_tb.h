/**
 * @file bsp_tb.h
 * @brief ThingsBoard IoT Framework Wrapper (Telemetry, RPC, Attributes, Claiming, OTA)
 *
 * Architecture:
 *  - Secure MQTTS Client with System TLS Certificate Bundle
 *  - Telemetry Publishing (Asynchronous & Synchronous QoS 1 for Deep Sleep)
 *  - Server-Side RPC Command Dispatcher with user callbacks
 *  - Shared Configuration Attributes Synchronization
 *  - Client Attributes Reporting (Firmware version, MAC, IP, battery state)
 *  - Device Claiming Token Workflow
 *  - Seamless Dual-Slot ThingsBoard OTA Updates
 *  - Core 0 Affinity: Keeps all networking off Core 1 UI / rendering pipeline
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

#ifndef BSP_TB_H
#define BSP_TB_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"
#include "bsp/bsp_ota.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ThingsBoard Server RPC Command Handler Callback
 *
 * @param request_id Server RPC request ID string
 * @param method RPC method name (e.g. "getValue", "setLed", "reboot")
 * @param params_json JSON string of parameters
 * @param user_data Optional user context
 */
typedef void (*bsp_tb_rpc_cb_t)(const char *request_id, const char *method, const char *params_json, void *user_data);

/**
 * @brief ThingsBoard Shared Attributes Update Callback
 *
 * @param json_payload JSON payload string containing updated shared attributes
 * @param user_data Optional user context
 */
typedef void (*bsp_tb_attr_cb_t)(const char *json_payload, void *user_data);

/**
 * @brief ThingsBoard Alarm / Notification Callback
 *
 * @param alarm_json JSON payload string containing alarm data
 * @param user_data Optional user context
 */
typedef void (*bsp_tb_alarm_cb_t)(const char *alarm_json, void *user_data);

/**
 * @brief ThingsBoard Telemetry / Attribute Value Types
 */
typedef enum {
    BSP_TB_VAL_INT = 0,    ///< 64-bit Signed Integer (int64_t)
    BSP_TB_VAL_FLOAT,      ///< Single-Precision Float
    BSP_TB_VAL_DOUBLE,     ///< Double-Precision Float
    BSP_TB_VAL_BOOL,       ///< Boolean (true/false)
    BSP_TB_VAL_STRING,     ///< Null-terminated String
} bsp_tb_val_type_t;

/**
 * @brief ThingsBoard Generic Telemetry / Attribute Key-Value Entry
 */
typedef struct {
    const char        *key;     ///< Attribute/Telemetry key name
    bsp_tb_val_type_t  type;    ///< Value type
    union {
        int64_t     i_val; ///< i_val value
        float       f_val; ///< f_val value
        double      d_val; ///< d_val value
        bool        b_val; ///< b_val value
        const char *s_val; ///< s_val value
    } val;
} bsp_tb_entry_t;

/**
 * @brief ThingsBoard Client Configuration Struct
 */
typedef struct {
    const char            *broker_uri;     ///< MQTTS Broker URI (e.g. "mqtts://thingsboard.cloud:8883")
    const char            *access_token;   ///< Device access token (or NULL if using client claiming)
    const char            *ca_cert_pem;    ///< Optional custom CA certificate (NULL uses system cert bundle)
    bsp_tb_rpc_cb_t       rpc_cb;          ///< RPC command handler callback
    bsp_tb_attr_cb_t      attr_cb;         ///< Shared attributes update callback
    bsp_tb_alarm_cb_t     alarm_cb;        ///< Alarm / notification callback
    bsp_ota_progress_cb_t ota_cb;          ///< OTA progress callback
    void                  *user_data;      ///< User context pointer
} bsp_tb_config_t;

/**
 * @brief Initialize ThingsBoard MQTTS Client Engine (Pinned to Core 0)
 *
 * @param[in] config Pointer to bsp_tb_config_t struct
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_tb_init(const bsp_tb_config_t *config);

/**
 * @brief Wait until ThingsBoard MQTT connection is established
 *
 * @param[in] timeout_ms Maximum time to wait in milliseconds
 * @return esp_err_t ESP_OK if connected, ESP_ERR_TIMEOUT on timeout
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_tb_wait_connected(uint32_t timeout_ms);

/**
 * @brief Check if ThingsBoard client is currently connected
 *
 * @return true if connected to broker
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
bool bsp_tb_is_connected(void);

/**
 * @brief Publish Configurable Array of Telemetry Key-Value Entries
 *
 * @param[in] entries Pointer to array of bsp_tb_entry_t items
 * @param[in] count Number of entries in array
 * @param[in] sync true to wait for QoS 1 broker ACK, false for async QoS 0
 * @param[in] timeout_ms Maximum time to wait for ACK if sync is true
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_tb_send_telemetry_entries(const bsp_tb_entry_t *entries, size_t count, bool sync, uint32_t timeout_ms);

/**
 * @brief Publish Environmental Telemetry JSON to ThingsBoard (Async QoS 0)
 *
 * Convenience wrapper for: {"temp": Kelvin, "rh": %, "battery": %, "rssi": dBm}
 *
 * @param[in] temp_k Temperature in Kelvin
 * @param[in] rh_pct Relative Humidity percentage
 * @param[in] battery_pct Battery state of charge (0-100%)
 * @param[in] rssi_dbm Wi-Fi RSSI in dBm
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_tb_send_telemetry(float temp_k, float rh_pct, uint8_t battery_pct, int rssi_dbm);

/**
 * @brief Synchronously Publish Telemetry and Wait for Broker ACK (QoS 1)
 *
 * Critical for Deep Sleep: Guarantees delivery before powering down radios!
 *
 * @param[in] temp_k Temperature in Kelvin
 * @param[in] rh_pct Relative Humidity percentage
 * @param[in] battery_pct Battery %
 * @param[in] rssi_dbm Wi-Fi RSSI
 * @param[in] timeout_ms Max wait time for ACK
 * @return esp_err_t ESP_OK on confirmed receipt
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_tb_send_telemetry_sync(float temp_k, float rh_pct, uint8_t battery_pct, int rssi_dbm, uint32_t timeout_ms);

/**
 * @brief Publish Arbitrary JSON Telemetry String
 *
 * @param[in] json_str Formatted JSON string (e.g. "{\"pressure\":1013.25}")
 * @param[in] sync true for QoS 1 synchronous delivery, false for QoS 0 async
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_tb_send_custom_telemetry(const char *json_str, bool sync);

/**
 * @brief Report Custom Client Attributes Key-Value Array to ThingsBoard
 *
 * @param[in] entries Array of key-value attributes
 * @param[in] count Number of entries
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_tb_report_client_attributes_entries(const bsp_tb_entry_t *entries, size_t count);

/**
 * @brief Report Standard Client Attributes to ThingsBoard
 *
 * Reports FW Version, Device ID, IP, MAC, Battery Voltage, and Uptime.
 *
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_tb_report_client_attributes(void);

/**
 * @brief Request Specific Shared Attributes from ThingsBoard Server
 *
 * @param[in] shared_keys Array of attribute key names (or NULL for default OTA/sleep keys)
 * @param[in] count Number of keys in array
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_tb_request_shared_attributes_keys(const char **shared_keys, size_t count);

/**
 * @brief Request Standard Shared Attributes from ThingsBoard Server
 *
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_tb_request_shared_attributes(void);

/**
 * @brief Send RPC Response Back to ThingsBoard Server
 *
 * @param[in] request_id Request ID passed to RPC callback
 * @param[in] response_json JSON response string (e.g. "{\"success\":true}")
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_tb_send_rpc_response(const char *request_id, const char *response_json);

/**
 * @brief Publish Device Claiming Token (v1/devices/me/claim)
 *
 * @param[in] secret_key User claiming secret (or NULL to auto-generate 6-character key)
 * @param[in] duration_ms Claim validity duration in milliseconds
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_tb_claim_device(const char *secret_key, uint32_t duration_ms);

/**
 * @brief Generate a 6-Character Unambiguous Claiming Token and Publish to ThingsBoard
 *
 * @param[out] out_key Destination buffer to receive generated key (at least 8 bytes)
 * @param[in] key_len Buffer length
 * @param[in] duration_ms Claim duration in ms (e.g. 180000 / 3 min)
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_tb_claim_device_auto(char *out_key, size_t key_len, uint32_t duration_ms);

/**
 * @brief Report Current OTA Firmware State to ThingsBoard
 *
 * @param[in] state OTA status string ("DOWNLOADING", "DOWNLOADED", "UPDATING", "SUCCESS", "FAILED")
 * @param[in] error_msg Optional error message (NULL for success)
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_tb_report_ota_state(const char *state, const char *error_msg);

/**
 * @brief Disconnect and Stop ThingsBoard Client
 *
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_tb_disconnect(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_TB_H */
