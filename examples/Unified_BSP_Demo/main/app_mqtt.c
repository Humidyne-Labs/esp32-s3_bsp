/**
 * @file app_mqtt.c
 * @brief ThingsBoard Secure MQTTS Client with Auto-Provisioning, RPC, and Shared Attributes
 * 
 * @attribution
 * - ThingsBoard.io / Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_https_ota.h"
#include "esp_http_client.h"
#include "mqtt_client.h"
#include "esp_crt_bundle.h"
#include "cJSON.h"
#include "bsp/bsp.h"
#include "app_mqtt.h"
#include "app_secrets.h"

static const char *TAG = "app_mqtt";

static esp_mqtt_client_handle_t s_mqtt_client = NULL;
static bool s_is_connected = false;
static app_ota_status_cb_t s_ota_cb = NULL;
static app_shared_config_cb_t s_config_cb = NULL;
static app_rpc_handler_cb_t s_rpc_cb = NULL;

static EventGroupHandle_t s_mqtt_sync_evg = NULL;
#define MQTT_SYNC_CONNECTED_BIT BIT0
#define MQTT_SYNC_PUBLISHED_BIT BIT1
static int s_pending_msg_id = -1;

static app_shared_config_t s_shared_config = {
    .email_alerts_enabled = true,
    .auto_update_enabled  = true,
    .device_theme         = "LIGHT",
    .manual_ota_trigger   = false,
    .sleep_interval_sec   = 60,
    .sleep_interval_min   = 1,
    .sound_enabled        = false,
    .temp_unit            = "F",
    .alarm_thresholds     = {
        .rh_low_critical      = 62.0f,
        .rh_low_warning       = 65.0f,
        .rh_high_warning      = 73.0f,
        .rh_high_critical     = 76.0f,
        .temp_low_critical    = 287.59f, // 58 °F
        .temp_low_warning     = 290.93f, // 64 °F
        .temp_high_warning    = 295.37f, // 72 °F
        .temp_high_critical   = 297.04f, // 75 °F
        .battery_low_critical = 15,
        .battery_low_warning  = 25,
        .rh_hist              = 1.5f,
        .temp_hist            = 0.56f,
        .batt_hist            = 2,
    },
};

/* =========================================================================
 * ThingsBoard Remote OTA Task
 * ========================================================================= */
static void ota_task(void *pvParameters)
{
    char *fw_url = (char *)pvParameters;
    ESP_LOGI(TAG, "Starting ThingsBoard OTA Download from: %s", fw_url);

    if (s_ota_cb) s_ota_cb(APP_OTA_DOWNLOADING, 0, "Starting download...");
    app_mqtt_report_ota_state("DOWNLOADING", NULL);

    esp_http_client_config_t http_config = {
        .url = fw_url,
        .timeout_ms = 15000,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .keep_alive_enable = true,
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &http_config,
    };

    esp_https_ota_handle_t https_ota_handle = NULL;
    esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ESP HTTPS OTA Begin failed: %s", esp_err_to_name(err));
        app_mqtt_report_ota_state("FAILED", "OTA Begin Failed");
        if (s_ota_cb) s_ota_cb(APP_OTA_FAILED, 0, "OTA Begin Failed");
        free(fw_url);
        vTaskDelete(NULL);
        return;
    }

    int total_bytes = esp_https_ota_get_image_size(https_ota_handle);
    while (1) {
        err = esp_https_ota_perform(https_ota_handle);
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
            break;
        }
        int read_bytes = esp_https_ota_get_image_len_read(https_ota_handle);
        int progress = (total_bytes > 0) ? ((read_bytes * 100) / total_bytes) : 50;
        if (s_ota_cb) s_ota_cb(APP_OTA_DOWNLOADING, progress, "Downloading firmware...");
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    if (esp_https_ota_is_complete_data_received(https_ota_handle)) {
        err = esp_https_ota_finish(https_ota_handle);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "ThingsBoard OTA update successfully installed!");
            app_mqtt_report_ota_state("UPDATED", NULL);
            if (s_ota_cb) s_ota_cb(APP_OTA_SUCCESS, 100, "Rebooting into new firmware...");
            vTaskDelay(pdMS_TO_TICKS(2000));
            esp_restart();
        } else {
            ESP_LOGE(TAG, "OTA finish failed: %s", esp_err_to_name(err));
            app_mqtt_report_ota_state("FAILED", "Verification failed");
            if (s_ota_cb) s_ota_cb(APP_OTA_FAILED, 0, "Verification failed");
        }
    } else {
        ESP_LOGE(TAG, "Complete data not received");
        esp_https_ota_abort(https_ota_handle);
        app_mqtt_report_ota_state("FAILED", "Incomplete download");
        if (s_ota_cb) s_ota_cb(APP_OTA_FAILED, 0, "Incomplete download");
    }

    free(fw_url);
    vTaskDelete(NULL);
}

/* =========================================================================
 * JSON Parser: Shared Attributes & Alarm Thresholds
 * ========================================================================= */
static void parse_shared_attributes_json(cJSON *root)
{
    if (!root) return;

    cJSON *item = NULL;

    if ((item = cJSON_GetObjectItem(root, "email_alerts_enabled")) != NULL) {
        s_shared_config.email_alerts_enabled = cJSON_IsTrue(item);
    }
    if ((item = cJSON_GetObjectItem(root, "auto_update_enabled")) != NULL) {
        s_shared_config.auto_update_enabled = cJSON_IsTrue(item);
    }
    if ((item = cJSON_GetObjectItem(root, "device_theme")) != NULL && cJSON_IsString(item)) {
        strncpy(s_shared_config.device_theme, item->valuestring, sizeof(s_shared_config.device_theme) - 1);
    }
    if ((item = cJSON_GetObjectItem(root, "manual_ota_trigger")) != NULL) {
        s_shared_config.manual_ota_trigger = cJSON_IsTrue(item);
    }
    if ((item = cJSON_GetObjectItem(root, "sleep_interval_sec")) != NULL && cJSON_IsNumber(item)) {
        s_shared_config.sleep_interval_sec = item->valueint;
    }
    if ((item = cJSON_GetObjectItem(root, "sleep_interval_min")) != NULL && cJSON_IsNumber(item)) {
        s_shared_config.sleep_interval_min = item->valueint;
    }
    if ((item = cJSON_GetObjectItem(root, "sound_enabled")) != NULL) {
        s_shared_config.sound_enabled = cJSON_IsTrue(item);
    }
    if ((item = cJSON_GetObjectItem(root, "temp_unit")) != NULL && cJSON_IsString(item)) {
        strncpy(s_shared_config.temp_unit, item->valuestring, sizeof(s_shared_config.temp_unit) - 1);
    }

    // Alarm Thresholds Nested Object
    cJSON *alarms = cJSON_GetObjectItem(root, "alarm_thresholds");
    if (alarms && cJSON_IsObject(alarms)) {
        cJSON *v = NULL;
        if ((v = cJSON_GetObjectItem(alarms, "rhLowCritical")) != NULL)  s_shared_config.alarm_thresholds.rh_low_critical = (float)v->valuedouble;
        if ((v = cJSON_GetObjectItem(alarms, "rhLowWarning")) != NULL)   s_shared_config.alarm_thresholds.rh_low_warning = (float)v->valuedouble;
        if ((v = cJSON_GetObjectItem(alarms, "rhHighWarning")) != NULL)  s_shared_config.alarm_thresholds.rh_high_warning = (float)v->valuedouble;
        if ((v = cJSON_GetObjectItem(alarms, "rhHighCritical")) != NULL) s_shared_config.alarm_thresholds.rh_high_critical = (float)v->valuedouble;
        
        if ((v = cJSON_GetObjectItem(alarms, "tempLowCritical")) != NULL)  s_shared_config.alarm_thresholds.temp_low_critical = (float)v->valuedouble;
        if ((v = cJSON_GetObjectItem(alarms, "tempLowWarning")) != NULL)   s_shared_config.alarm_thresholds.temp_low_warning = (float)v->valuedouble;
        if ((v = cJSON_GetObjectItem(alarms, "tempHighWarning")) != NULL)  s_shared_config.alarm_thresholds.temp_high_warning = (float)v->valuedouble;
        if ((v = cJSON_GetObjectItem(alarms, "tempHighCritical")) != NULL) s_shared_config.alarm_thresholds.temp_high_critical = (float)v->valuedouble;

        if ((v = cJSON_GetObjectItem(alarms, "batteryLowCritical")) != NULL) s_shared_config.alarm_thresholds.battery_low_critical = v->valueint;
        if ((v = cJSON_GetObjectItem(alarms, "batteryLowWarning")) != NULL)  s_shared_config.alarm_thresholds.battery_low_warning = v->valueint;
        if ((v = cJSON_GetObjectItem(alarms, "rhHist")) != NULL)             s_shared_config.alarm_thresholds.rh_hist = (float)v->valuedouble;
        if ((v = cJSON_GetObjectItem(alarms, "tempHist")) != NULL)           s_shared_config.alarm_thresholds.temp_hist = (float)v->valuedouble;
        if ((v = cJSON_GetObjectItem(alarms, "battHist")) != NULL)           s_shared_config.alarm_thresholds.batt_hist = v->valueint;
    }

    ESP_LOGI(TAG, "Parsed Shared Attributes: Sleep=%ds, Unit=%s, Theme=%s",
             s_shared_config.sleep_interval_sec,
             s_shared_config.temp_unit,
             s_shared_config.device_theme);

    // OTA Trigger Check
    cJSON *fw_url = cJSON_GetObjectItem(root, "fw_url");
    if (fw_url && cJSON_IsString(fw_url) && strlen(fw_url->valuestring) > 0) {
        char *url_copy = strdup(fw_url->valuestring);
        xTaskCreate(ota_task, "tb_ota_task", 8192, url_copy, 5, NULL);
    }

    if (s_config_cb) {
        s_config_cb(&s_shared_config);
    }
}

/* =========================================================================
 * RPC Command Handler
 * ========================================================================= */
static void handle_rpc_request(const char *request_id, const char *payload_str, int len)
{
    cJSON *root = cJSON_ParseWithLength(payload_str, len);
    if (!root) return;

    cJSON *method_item = cJSON_GetObjectItem(root, "method");
    cJSON *params_item = cJSON_GetObjectItem(root, "params");
    const char *method = method_item ? method_item->valuestring : "";

    ESP_LOGI(TAG, "Incoming RPC Request ID: %s, Method: '%s'", request_id, method);

    if (strcmp(method, "setStatus") == 0) {
        const char *status_val = "active";
        if (params_item && cJSON_IsObject(params_item)) {
            cJSON *s = cJSON_GetObjectItem(params_item, "status");
            if (s && cJSON_IsString(s)) status_val = s->valuestring;
        }
        char resp[128];
        snprintf(resp, sizeof(resp), "{\"status\":\"success\",\"currentState\":\"%s\"}", status_val);
        app_mqtt_send_rpc_response(request_id, resp);
    } else {
        if (s_rpc_cb) {
            char *params_str = params_item ? cJSON_PrintUnformatted(params_item) : NULL;
            s_rpc_cb(request_id, method, params_str ? params_str : "{}");
            if (params_str) free(params_str);
        } else {
            app_mqtt_send_rpc_response(request_id, "{\"status\":\"error\",\"message\":\"Unknown method\"}");
        }
    }

    cJSON_Delete(root);
}

/* =========================================================================
 * MQTT Event Callback
 * ========================================================================= */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "Secure MQTTS Connected to ThingsBoard");
        s_is_connected = true;
        if (s_mqtt_sync_evg) {
            xEventGroupSetBits(s_mqtt_sync_evg, MQTT_SYNC_CONNECTED_BIT);
        }

        // 1. Subscribe to Live Shared Attribute Updates
        esp_mqtt_client_subscribe(s_mqtt_client, "v1/devices/me/attributes", 1);

        // 2. Subscribe to Attribute Query Responses
        esp_mqtt_client_subscribe(s_mqtt_client, "v1/devices/me/attributes/response/+", 1);

        // 3. Subscribe to Server RPC Requests
        esp_mqtt_client_subscribe(s_mqtt_client, "v1/devices/me/rpc/request/+", 1);

        // 4. Request existing shared attributes on boot
        app_mqtt_request_shared_attributes();
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "Disconnected from ThingsBoard Broker");
        s_is_connected = false;
        if (s_mqtt_sync_evg) {
            xEventGroupClearBits(s_mqtt_sync_evg, MQTT_SYNC_CONNECTED_BIT);
        }
        break;

    case MQTT_EVENT_PUBLISHED:
        if (event->msg_id == s_pending_msg_id && s_mqtt_sync_evg) {
            xEventGroupSetBits(s_mqtt_sync_evg, MQTT_SYNC_PUBLISHED_BIT);
        }
        break;

    case MQTT_EVENT_DATA: {
        char topic[64] = {0};
        int tlen = (event->topic_len < (int)sizeof(topic) - 1) ? event->topic_len : (int)sizeof(topic) - 1;
        strncpy(topic, event->topic, tlen);

        if (strcmp(topic, "v1/devices/me/attributes") == 0) {
            cJSON *root = cJSON_ParseWithLength(event->data, event->data_len);
            if (root) {
                parse_shared_attributes_json(root);
                cJSON_Delete(root);
            }
        } else if (strncmp(topic, "v1/devices/me/attributes/response/", 34) == 0) {
            cJSON *root = cJSON_ParseWithLength(event->data, event->data_len);
            if (root) {
                cJSON *shared = cJSON_GetObjectItem(root, "shared");
                parse_shared_attributes_json(shared ? shared : root);
                cJSON_Delete(root);
            }
        } else if (strncmp(topic, "v1/devices/me/rpc/request/", 26) == 0) {
            const char *request_id = topic + 26;
            handle_rpc_request(request_id, event->data, event->data_len);
        }
        break;
    }

    default:
        break;
    }
}

/* =========================================================================
 * ThingsBoard Device Auto-Provisioning Flow
 * ========================================================================= */
static EventGroupHandle_t s_prov_evg = NULL;
#define PROV_SUCCESS_BIT BIT0
#define PROV_FAIL_BIT    BIT1
static char s_prov_token[128] = {0};

static void prov_mqtt_event_handler(void *args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "[Auto-Prov] Connected to broker as 'provision'. Subscribing to /provision/response...");
        esp_mqtt_client_subscribe(event->client, "/provision/response", 1);

        char prov_payload[256];
        snprintf(prov_payload, sizeof(prov_payload),
                 "{\"deviceName\":\"%s\",\"provisionDeviceKey\":\"%s\",\"provisionDeviceSecret\":\"%s\"}",
                 (const char *)args, CONFIG_THINGSBOARD_PROVISION_KEY, CONFIG_THINGSBOARD_PROVISION_SECRET);

        esp_mqtt_client_publish(event->client, "/provision/request", prov_payload, 0, 1, 0);
        ESP_LOGI(TAG, "[Auto-Prov] Published provision request: %s", prov_payload);
        break;

    case MQTT_EVENT_DATA: {
        if (event->topic_len > 0 && strstr(event->topic, "/provision/response")) {
            cJSON *root = cJSON_ParseWithLength(event->data, event->data_len);
            if (root) {
                cJSON *status = cJSON_GetObjectItem(root, "status");
                cJSON *token = cJSON_GetObjectItem(root, "credentialsValue");
                if (status && strcmp(status->valuestring, "SUCCESS") == 0 && token && cJSON_IsString(token)) {
                    strncpy(s_prov_token, token->valuestring, sizeof(s_prov_token) - 1);
                    ESP_LOGI(TAG, "[Auto-Prov] SUCCESS! Received credentials token: %s", s_prov_token);
                    xEventGroupSetBits(s_prov_evg, PROV_SUCCESS_BIT);
                } else {
                    ESP_LOGE(TAG, "[Auto-Prov] Response error: %s", event->data);
                    xEventGroupSetBits(s_prov_evg, PROV_FAIL_BIT);
                }
                cJSON_Delete(root);
            }
        }
        break;
    }

    default:
        break;
    }
}

esp_err_t app_mqtt_auto_provision(const char *broker_uri,
                                  const char *device_name,
                                  const char *prov_key,
                                  const char *prov_secret,
                                  char *out_token,
                                  size_t max_len,
                                  uint32_t timeout_ms)
{
    if (!broker_uri || !device_name || !prov_key || !prov_secret || !out_token) {
        return ESP_ERR_INVALID_ARG;
    }

    s_prov_evg = xEventGroupCreate();
    memset(s_prov_token, 0, sizeof(s_prov_token));

    esp_mqtt_client_config_t prov_cfg = {
        .broker = {
            .address = {
                .uri = broker_uri,
            },
            .verification = {
                .crt_bundle_attach = esp_crt_bundle_attach,
                .skip_cert_common_name_check = true,
            },
        },
        .credentials = {
            .username = "provision",
        },
    };

    esp_mqtt_client_handle_t prov_client = esp_mqtt_client_init(&prov_cfg);
    if (!prov_client) return ESP_FAIL;

    esp_mqtt_client_register_event(prov_client, ESP_EVENT_ANY_ID, prov_mqtt_event_handler, (void *)device_name);
    esp_mqtt_client_start(prov_client);

    EventBits_t bits = xEventGroupWaitBits(s_prov_evg, PROV_SUCCESS_BIT | PROV_FAIL_BIT, pdFALSE, pdFALSE, pdMS_TO_TICKS(timeout_ms));
    
    esp_mqtt_client_stop(prov_client);
    esp_mqtt_client_destroy(prov_client);
    vEventGroupDelete(s_prov_evg);
    s_prov_evg = NULL;

    if (bits & PROV_SUCCESS_BIT) {
        strncpy(out_token, s_prov_token, max_len - 1);
        bsp_nvs_set_str("tb_token", s_prov_token);
        return ESP_OK;
    }

    return ESP_ERR_TIMEOUT;
}

/* =========================================================================
 * Public APIs
 * ========================================================================= */
esp_err_t app_mqtt_init(const char *broker_uri,
                        const char *access_token,
                        const char *ca_cert_pem,
                        app_ota_status_cb_t ota_cb,
                        app_shared_config_cb_t config_cb,
                        app_rpc_handler_cb_t rpc_cb)
{
    if (broker_uri == NULL) return ESP_ERR_INVALID_ARG;
    s_ota_cb    = ota_cb;
    s_config_cb = config_cb;
    s_rpc_cb    = rpc_cb;

    if (s_mqtt_sync_evg == NULL) {
        s_mqtt_sync_evg = xEventGroupCreate();
    }

    char final_token[128] = {0};

    // 1. Resolve Access Token
    if (access_token != NULL && strlen(access_token) > 0) {
        strncpy(final_token, access_token, sizeof(final_token) - 1);
    } else {
        // Check NVS
        if (bsp_nvs_get_str("tb_token", final_token, sizeof(final_token)) != ESP_OK) {
            ESP_LOGI(TAG, "No ThingsBoard token found. Initiating Auto-Provisioning...");
            char dev_name[32] = {0};
            bsp_get_device_name(dev_name, sizeof(dev_name));
            
            esp_err_t prov_ret = app_mqtt_auto_provision(broker_uri, dev_name,
                                                         CONFIG_THINGSBOARD_PROVISION_KEY,
                                                         CONFIG_THINGSBOARD_PROVISION_SECRET,
                                                         final_token, sizeof(final_token), 10000);
            if (prov_ret != ESP_OK) {
                ESP_LOGW(TAG, "Auto-provisioning failed or timed out. Proceeding with unauthenticated connection.");
            }
        }
    }

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
                .uri = broker_uri,
            },
            .verification = {
                .certificate = ca_cert_pem,
                .crt_bundle_attach = (ca_cert_pem == NULL) ? esp_crt_bundle_attach : NULL,
                .skip_cert_common_name_check = true,
            },
        },
        .credentials = {
            .username = (strlen(final_token) > 0) ? final_token : NULL,
        },
    };

    s_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (s_mqtt_client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize MQTT client handle");
        return ESP_FAIL;
    }

    esp_mqtt_client_register_event(s_mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    return esp_mqtt_client_start(s_mqtt_client);
}

esp_err_t app_mqtt_wait_connected(uint32_t timeout_ms)
{
    if (s_is_connected) return ESP_OK;
    if (s_mqtt_sync_evg == NULL) return ESP_ERR_INVALID_STATE;

    EventBits_t bits = xEventGroupWaitBits(
        s_mqtt_sync_evg,
        MQTT_SYNC_CONNECTED_BIT,
        pdFALSE,
        pdFALSE,
        pdMS_TO_TICKS(timeout_ms)
    );

    return (bits & MQTT_SYNC_CONNECTED_BIT) ? ESP_OK : ESP_ERR_TIMEOUT;
}

esp_err_t app_mqtt_publish_telemetry(float temp_k, float rh_pct, uint8_t battery_pct, int rssi_dbm)
{
    if (!s_is_connected || s_mqtt_client == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    char payload[180];
    snprintf(payload, sizeof(payload),
             "{\"temp\":%.2f,\"rh\":%.2f,\"battery\":%u,\"rssi\":%d}",
             temp_k, rh_pct, battery_pct, rssi_dbm);

    int msg_id = esp_mqtt_client_publish(s_mqtt_client, "v1/devices/me/telemetry", payload, 0, 1, 0);
    return (msg_id >= 0) ? ESP_OK : ESP_FAIL;
}

esp_err_t app_mqtt_publish_telemetry_sync(float temp_k, float rh_pct, uint8_t battery_pct, int rssi_dbm, uint32_t timeout_ms)
{
    if (!s_is_connected || s_mqtt_client == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    char payload[180];
    snprintf(payload, sizeof(payload),
             "{\"temp\":%.2f,\"rh\":%.2f,\"battery\":%u,\"rssi\":%d}",
             temp_k, rh_pct, battery_pct, rssi_dbm);

    if (s_mqtt_sync_evg) {
        xEventGroupClearBits(s_mqtt_sync_evg, MQTT_SYNC_PUBLISHED_BIT);
    }

    s_pending_msg_id = esp_mqtt_client_publish(s_mqtt_client, "v1/devices/me/telemetry", payload, 0, 1, 0);
    if (s_pending_msg_id < 0) {
        return ESP_FAIL;
    }

    if (s_mqtt_sync_evg) {
        EventBits_t bits = xEventGroupWaitBits(
            s_mqtt_sync_evg,
            MQTT_SYNC_PUBLISHED_BIT,
            pdTRUE,
            pdFALSE,
            pdMS_TO_TICKS(timeout_ms)
        );
        if (!(bits & MQTT_SYNC_PUBLISHED_BIT)) {
            ESP_LOGW(TAG, "Synchronous telemetry publish ACK timed out");
            return ESP_ERR_TIMEOUT;
        }
    }

    ESP_LOGI(TAG, "Telemetry published & acknowledged by ThingsBoard (QoS 1)");
    return ESP_OK;
}

esp_err_t app_mqtt_publish_claim_token(const char *secret_key, uint32_t duration_ms)
{
    if (!s_is_connected || s_mqtt_client == NULL || secret_key == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    char payload[128];
    snprintf(payload, sizeof(payload),
             "{\"secretKey\":\"%s\",\"durationMs\":%lu}",
             secret_key, (unsigned long)duration_ms);

    int msg_id = esp_mqtt_client_publish(s_mqtt_client, "v1/devices/me/claim", payload, 0, 1, 0);
    if (msg_id >= 0) {
        ESP_LOGI(TAG, "Published ThingsBoard Claiming Token -> %s", payload);
        return ESP_OK;
    }

    return ESP_FAIL;
}

esp_err_t app_mqtt_report_client_attributes(const char *fw_version,
                                            const char *device_name,
                                            const char *mac_address,
                                            const char *ssid,
                                            const char *ip_address,
                                            bool has_sd_card,
                                            bool audio_synced)
{
    if (!s_is_connected || s_mqtt_client == NULL) return ESP_ERR_INVALID_STATE;

    char payload[256];
    snprintf(payload, sizeof(payload),
             "{\"fw_version\":\"%s\",\"device_name\":\"%s\",\"mac_address\":\"%s\",\"ssid\":\"%s\",\"ip_address\":\"%s\",\"has_sd_card\":%s,\"audio_synced\":%s}",
             fw_version ? fw_version : "v1.0.4",
             device_name ? device_name : "HumidOS-Node",
             mac_address ? mac_address : "00:00:00:00:00:00",
             ssid ? ssid : "Unknown-AP",
             ip_address ? ip_address : "0.0.0.0",
             has_sd_card ? "true" : "false",
             audio_synced ? "true" : "false");

    int msg_id = esp_mqtt_client_publish(s_mqtt_client, "v1/devices/me/attributes", payload, 0, 1, 0);
    return (msg_id >= 0) ? ESP_OK : ESP_FAIL;
}

esp_err_t app_mqtt_request_shared_attributes(void)
{
    if (!s_is_connected || s_mqtt_client == NULL) return ESP_ERR_INVALID_STATE;

    const char *req_payload = "{\"sharedKeys\":\"sleep_interval_sec,sleep_interval_min,device_theme,sound_enabled,auto_update_enabled,manual_ota_trigger,temp_unit,alarm_thresholds\"}";
    int msg_id = esp_mqtt_client_publish(s_mqtt_client, "v1/devices/me/attributes/request/1", req_payload, 0, 1, 0);
    ESP_LOGI(TAG, "Requested Shared Attributes from ThingsBoard server");
    return (msg_id >= 0) ? ESP_OK : ESP_FAIL;
}

esp_err_t app_mqtt_send_rpc_response(const char *request_id, const char *response_json)
{
    if (!s_is_connected || s_mqtt_client == NULL || !request_id || !response_json) return ESP_ERR_INVALID_STATE;

    char topic[64];
    snprintf(topic, sizeof(topic), "v1/devices/me/rpc/response/%s", request_id);
    int msg_id = esp_mqtt_client_publish(s_mqtt_client, topic, response_json, 0, 1, 0);
    ESP_LOGI(TAG, "Sent RPC response to %s: %s", topic, response_json);
    return (msg_id >= 0) ? ESP_OK : ESP_FAIL;
}

const app_shared_config_t *app_mqtt_get_shared_config(void)
{
    return &s_shared_config;
}

esp_err_t app_mqtt_report_ota_state(const char *state, const char *error_msg)
{
    if (!s_is_connected || s_mqtt_client == NULL || state == NULL) return ESP_ERR_INVALID_STATE;

    char payload[160];
    if (error_msg) {
        snprintf(payload, sizeof(payload), "{\"fw_state\":\"%s\",\"fw_error\":\"%s\"}", state, error_msg);
    } else {
        snprintf(payload, sizeof(payload), "{\"fw_state\":\"%s\"}", state);
    }

    int msg_id = esp_mqtt_client_publish(s_mqtt_client, "v1/devices/me/telemetry", payload, 0, 1, 0);
    return (msg_id >= 0) ? ESP_OK : ESP_FAIL;
}

bool app_mqtt_is_connected(void)
{
    return s_is_connected;
}

esp_err_t app_mqtt_disconnect(void)
{
    if (s_mqtt_client == NULL) return ESP_OK;

    s_is_connected = false;
    esp_mqtt_client_stop(s_mqtt_client);
    return esp_mqtt_client_destroy(s_mqtt_client);
}
