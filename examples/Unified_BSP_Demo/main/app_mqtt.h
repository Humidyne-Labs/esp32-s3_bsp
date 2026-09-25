/**
 * @file app_mqtt.h
 * @brief ThingsBoard Secure MQTT (MQTTS) Client with Auto-Provisioning, Shared Attributes, Claiming, and OTA
 * 
 * @attribution
 * - ThingsBoard.io / Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef APP_MQTT_H
#define APP_MQTT_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ThingsBoard Alarm Thresholds Structure (All temperatures stored natively in Kelvin)
 */
typedef struct {
    float rh_low_critical;      /*!< Default: 62.0 % */
    float rh_low_warning;       /*!< Default: 65.0 % */
    float rh_high_warning;      /*!< Default: 73.0 % */
    float rh_high_critical;     /*!< Default: 76.0 % */
    float temp_low_critical;    /*!< Default: 287.59 K (58.0 °F) */
    float temp_low_warning;     /*!< Default: 290.93 K (64.0 °F) */
    float temp_high_warning;    /*!< Default: 295.37 K (72.0 °F) */
    float temp_high_critical;   /*!< Default: 297.04 K (75.0 °F) */
    int   battery_low_critical; /*!< Default: 15 % */
    int   battery_low_warning;  /*!< Default: 25 % */
    float rh_hist;              /*!< Default: 1.5 % */
    float temp_hist;            /*!< Default: 0.56 K */
    int   batt_hist;            /*!< Default: 2 % */
} app_alarm_thresholds_t;

/**
 * @brief ThingsBoard Device Shared Configuration Structure
 */
typedef struct {
    bool email_alerts_enabled;
    bool auto_update_enabled;
    char device_theme[16];      /*!< "LIGHT" or "DARK" */
    bool manual_ota_trigger;
    int  sleep_interval_sec;    /*!< Dynamic sleep duration in seconds */
    int  sleep_interval_min;    /*!< Sleep duration in minutes */
    bool sound_enabled;         /*!< Audible chime toggle */
    char temp_unit[4];          /*!< "F", "C", or "K" */
    app_alarm_thresholds_t alarm_thresholds;
} app_shared_config_t;

/**
 * @brief OTA Update Status
 */
typedef enum {
    APP_OTA_IDLE,
    APP_OTA_DOWNLOADING,
    APP_OTA_DOWNLOADED,
    APP_OTA_VERIFYING,
    APP_OTA_UPDATING,
    APP_OTA_FAILED,
    APP_OTA_SUCCESS
} app_ota_status_t;

typedef void (*app_ota_status_cb_t)(app_ota_status_t status, int progress_pct, const char *msg);
typedef void (*app_shared_config_cb_t)(const app_shared_config_t *config);
typedef void (*app_rpc_handler_cb_t)(const char *request_id, const char *method, const char *params_json);

/**
 * @brief Initialize secure MQTTS client connected to ThingsBoard broker
 * 
 * @param broker_uri Secure Broker URI (e.g. "mqtts://humid1.com:8883")
 * @param access_token Device access token (leave empty string or NULL to use auto-provisioning)
 * @param ca_cert_pem Optional CA Certificate in PEM format (NULL to use system cert bundle)
 * @param ota_cb Optional callback to receive OTA progress updates (can be NULL)
 * @param config_cb Optional callback when shared attributes are loaded or changed
 * @param rpc_cb Optional callback for incoming server RPC commands
 * @return esp_err_t ESP_OK on success
 */
esp_err_t app_mqtt_init(const char *broker_uri,
                        const char *access_token,
                        const char *ca_cert_pem,
                        app_ota_status_cb_t ota_cb,
                        app_shared_config_cb_t config_cb,
                        app_rpc_handler_cb_t rpc_cb);

/**
 * @brief Perform ThingsBoard Device Auto-Provisioning to acquire access token
 */
esp_err_t app_mqtt_auto_provision(const char *broker_uri,
                                  const char *device_name,
                                  const char *prov_key,
                                  const char *prov_secret,
                                  char *out_token,
                                  size_t max_len,
                                  uint32_t timeout_ms);

/**
 * @brief Publish sensor telemetry to ThingsBoard topic "v1/devices/me/telemetry"
 * 
 * Matches ThingsBoard Rule Chain schema: {"temp": Kelvin, "rh": %, "battery": %, "rssi": dBm}
 */
esp_err_t app_mqtt_publish_telemetry(float temp_k, float rh_pct, uint8_t battery_pct, int rssi_dbm);

/**
 * @brief Publish device claiming secret key to ThingsBoard topic "v1/devices/me/claim"
 */
esp_err_t app_mqtt_publish_claim_token(const char *secret_key, uint32_t duration_ms);

/**
 * @brief Report device client attributes to ThingsBoard topic "v1/devices/me/attributes"
 */
esp_err_t app_mqtt_report_client_attributes(const char *fw_version,
                                            const char *device_name,
                                            const char *mac_address,
                                            const char *ssid,
                                            const char *ip_address,
                                            bool has_sd_card,
                                            bool audio_synced);

/**
 * @brief Request shared configuration attributes from server (v1/devices/me/attributes/request/1)
 */
esp_err_t app_mqtt_request_shared_attributes(void);

/**
 * @brief Send RPC response back to ThingsBoard server (v1/devices/me/rpc/response/{requestId})
 */
esp_err_t app_mqtt_send_rpc_response(const char *request_id, const char *response_json);

/**
 * @brief Get currently loaded shared configuration struct
 */
const app_shared_config_t *app_mqtt_get_shared_config(void);

/**
 * @brief Report current OTA firmware state to ThingsBoard
 */
esp_err_t app_mqtt_report_ota_state(const char *state, const char *error_msg);

/**
 * @brief Check if MQTT client is currently connected to ThingsBoard broker
 */
bool app_mqtt_is_connected(void);

/**
 * @brief Disconnect and stop MQTT client
 */
esp_err_t app_mqtt_disconnect(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_MQTT_H */
