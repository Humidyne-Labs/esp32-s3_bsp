/**
 * @file bsp_tb.c
 * @brief ThingsBoard IoT Framework Implementation (MQTTS, RPC, Telemetry, OTA)
 *
 * @version 1.0.0
 * @attribution
 * - Architecture & Development: HUMIDYNE LABS / Humiditron
 * - AI Systems Co-Developer: Gemini (Google DeepMind)
 * - Hardware Target: Waveshare Electronics ESP32-S3 ePaper 1.54 V2
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_crt_bundle.h"
#include "cJSON.h"
#include "bsp/bsp.h"
#include "bsp/bsp_tb.h"
#include "bsp/bsp_ota.h"
#include "bsp/bsp_wifi.h"

static const char *TAG = "bsp_tb";

static esp_mqtt_client_handle_t s_mqtt_client  = NULL;
static EventGroupHandle_t       s_mqtt_events  = NULL;
static bsp_tb_config_t          s_tb_cfg       = {0};

#define MQTT_CONNECTED_BIT  BIT0
#define MQTT_PUB_ACK_BIT    BIT1

static void handle_rpc_message(const char *topic, const char *data, int data_len)
{
    // Topic: v1/devices/me/rpc/request/{requestId}
    const char *prefix = "v1/devices/me/rpc/request/";
    size_t prefix_len = strlen(prefix);
    if (strncmp(topic, prefix, prefix_len) != 0) return;

    char request_id[64] = {0};
    strlcpy(request_id, topic + prefix_len, sizeof(request_id));

    char *json_buf = (char *)malloc(data_len + 1);
    if (!json_buf) return;
    memcpy(json_buf, data, data_len);
    json_buf[data_len] = '\0';

    cJSON *root = cJSON_Parse(json_buf);
    if (root != NULL) {
        cJSON *method_item = cJSON_GetObjectItem(root, "method");
        cJSON *params_item = cJSON_GetObjectItem(root, "params");

        const char *method     = method_item ? method_item->valuestring            : "unknown";
        char       *params_str = params_item ? cJSON_PrintUnformatted(params_item) : NULL;

        ESP_LOGI(TAG, "Incoming RPC [%s] Method: %s", request_id, method);

        if (s_tb_cfg.rpc_cb) {
            s_tb_cfg.rpc_cb(request_id, method, params_str ? params_str : "{}", s_tb_cfg.user_data);
        }

        if (params_str) free(params_str);
        cJSON_Delete(root);
    }
    free(json_buf);
}

static void handle_attributes_message(const char *data, int data_len)
{
    char *json_buf = (char *)malloc(data_len + 1);
    if (!json_buf) return;
    memcpy(json_buf, data, data_len);
    json_buf[data_len] = '\0';

    ESP_LOGI(TAG, "Incoming Shared Attributes payload (%d bytes)", data_len);

    cJSON *root = cJSON_Parse(json_buf);
    if (root != NULL) {
        // Check for ThingsBoard Firmware OTA URL attribute
        cJSON *fw_url = cJSON_GetObjectItem(root, "fw_url");

        if (fw_url && cJSON_IsString(fw_url) && strlen(fw_url->valuestring) > 0) {
            ESP_LOGI(TAG, "OTA Firmware trigger received! URL: %s", fw_url->valuestring);
            bsp_tb_report_ota_state("DOWNLOADING", NULL);
            esp_err_t ota_ret = bsp_ota_from_url(fw_url->valuestring, s_tb_cfg.ota_cb, s_tb_cfg.user_data);
            if (ota_ret == ESP_OK) {
                bsp_tb_report_ota_state("SUCCESS", NULL);
                vTaskDelay(pdMS_TO_TICKS(1000));
                esp_restart();
            } else {
                bsp_tb_report_ota_state("FAILED", "HTTPS OTA Download Error");
            }
        }

        // Check for Alarm / Notification payload
        cJSON *alarm = cJSON_GetObjectItem(root, "alarm");
        if (!alarm) alarm = cJSON_GetObjectItem(root, "alarms");
        if (alarm && s_tb_cfg.alarm_cb) {
            char *alarm_str = cJSON_PrintUnformatted(alarm);
            if (alarm_str) {
                s_tb_cfg.alarm_cb(alarm_str, s_tb_cfg.user_data);
                free(alarm_str);
            }
        }

        if (s_tb_cfg.attr_cb) {
            s_tb_cfg.attr_cb(json_buf, s_tb_cfg.user_data);
        }
        cJSON_Delete(root);
    }
    free(json_buf);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "ThingsBoard MQTTS Connected");
        xEventGroupSetBits(s_mqtt_events, MQTT_CONNECTED_BIT);

        // Subscribe to Server RPC Commands & Shared Attributes
        esp_mqtt_client_subscribe(s_mqtt_client, "v1/devices/me/rpc/request/+"        , 1);
        esp_mqtt_client_subscribe(s_mqtt_client, "v1/devices/me/attributes"           , 1);
        esp_mqtt_client_subscribe(s_mqtt_client, "v1/devices/me/attributes/response/+", 1);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "ThingsBoard MQTTS Disconnected");
        xEventGroupClearBits(s_mqtt_events, MQTT_CONNECTED_BIT);
        break;

    case MQTT_EVENT_PUBLISHED:
        xEventGroupSetBits(s_mqtt_events, MQTT_PUB_ACK_BIT);
        break;

    case MQTT_EVENT_DATA: {
        char topic_buf[64] = {0};
        int topic_len = (event->topic_len < (int)sizeof(topic_buf) - 1) ? event->topic_len : (int)sizeof(topic_buf) - 1;
        memcpy(topic_buf, event->topic, topic_len);

        if (strstr(topic_buf, "v1/devices/me/rpc/request/")) {
            handle_rpc_message(topic_buf, event->data, event->data_len);
        } else if (strstr(topic_buf, "v1/devices/me/attributes")) {
            handle_attributes_message(event->data, event->data_len);
        }
        break;
    }

    default:
        break;
    }
}

esp_err_t bsp_tb_init(const bsp_tb_config_t *config)
{
    if (config == NULL || config->broker_uri == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    s_tb_cfg = *config;

    if (s_mqtt_events == NULL) {
        s_mqtt_events = xEventGroupCreate();
    }

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
                .uri = config->broker_uri,
            },
            .verification = {
                .crt_bundle_attach = (config->ca_cert_pem == NULL) ? esp_crt_bundle_attach : NULL,
                .certificate       = config->ca_cert_pem,
            },
        },
        .credentials = {
            .username = config->access_token,
        },
        .task = {
            .priority = 5,
            .stack_size = 6144,
        },
        .network = {
            .timeout_ms = 10000,
        },
    };

    if (s_mqtt_client != NULL) {
        esp_mqtt_client_destroy(s_mqtt_client);
    }

    s_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (s_mqtt_client == NULL) {
        ESP_LOGE(TAG, "Failed to create ThingsBoard MQTT client handle");
        return ESP_FAIL;
    }

    esp_mqtt_client_register_event(s_mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_err_t ret = esp_mqtt_client_start(s_mqtt_client);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start ThingsBoard MQTT client: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t bsp_tb_wait_connected(uint32_t timeout_ms)
{
    if (s_mqtt_events == NULL) return ESP_ERR_INVALID_STATE;
    EventBits_t bits = xEventGroupWaitBits(s_mqtt_events, MQTT_CONNECTED_BIT, pdFALSE, pdTRUE, pdMS_TO_TICKS(timeout_ms));
    return (bits & MQTT_CONNECTED_BIT) ? ESP_OK : ESP_ERR_TIMEOUT;
}

bool bsp_tb_is_connected(void)
{
    if (s_mqtt_events == NULL) return false;
    return (xEventGroupGetBits(s_mqtt_events) & MQTT_CONNECTED_BIT) != 0;
}

esp_err_t bsp_tb_send_custom_telemetry(const char *json_str, bool sync)
{
    if (s_mqtt_client == NULL || json_str == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    int qos = sync ? 1 : 0;
    if (sync) {
        xEventGroupClearBits(s_mqtt_events, MQTT_PUB_ACK_BIT);
    }

    int msg_id = esp_mqtt_client_publish(s_mqtt_client, "v1/devices/me/telemetry", json_str, 0, qos, 0);
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Failed to publish telemetry to ThingsBoard");
        return ESP_FAIL;
    }

    if (sync) {
        EventBits_t bits = xEventGroupWaitBits(s_mqtt_events, MQTT_PUB_ACK_BIT, pdTRUE, pdTRUE, pdMS_TO_TICKS(5000));
        if ((bits & MQTT_PUB_ACK_BIT) == 0) {
            ESP_LOGW(TAG, "Telemetry ACK timeout (QoS 1)");
            return ESP_ERR_TIMEOUT;
        }
    }

    return ESP_OK;
}

static cJSON *bsp_tb_entries_to_json(const bsp_tb_entry_t *entries, size_t count)
{
    if (entries == NULL || count == 0) return NULL;
    cJSON *root = cJSON_CreateObject();
    if (!root) return NULL;

    for (size_t i = 0; i < count; i++) {
        const bsp_tb_entry_t *e = &entries[i];
        if (e->key == NULL) continue;

        switch (e->type) {
        case BSP_TB_VAL_INT:
            cJSON_AddNumberToObject(root, e->key, (double)e->val.i_val);
            break;
        case BSP_TB_VAL_FLOAT:
            cJSON_AddNumberToObject(root, e->key, (double)e->val.f_val);
            break;
        case BSP_TB_VAL_DOUBLE:
            cJSON_AddNumberToObject(root, e->key, e->val.d_val);
            break;
        case BSP_TB_VAL_BOOL:
            cJSON_AddBoolToObject(root, e->key, e->val.b_val);
            break;
        case BSP_TB_VAL_STRING:
            cJSON_AddStringToObject(root, e->key, e->val.s_val ? e->val.s_val : "");
            break;
        default:
            break;
        }
    }
    return root;
}

esp_err_t bsp_tb_send_telemetry_entries(const bsp_tb_entry_t *entries, size_t count, bool sync, uint32_t timeout_ms)
{
    cJSON *root = bsp_tb_entries_to_json(entries, count);
    if (!root) return ESP_ERR_INVALID_ARG;

    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!json_str) return ESP_ERR_NO_MEM;

    esp_err_t ret = bsp_tb_send_custom_telemetry(json_str, sync);
    free(json_str);
    return ret;
}

esp_err_t bsp_tb_send_telemetry(float temp_k, float rh_pct, uint8_t battery_pct, int rssi_dbm)
{
    bsp_tb_entry_t entries[] = {
        { .key = "temp",    .type = BSP_TB_VAL_FLOAT, .val.f_val = temp_k      },
        { .key = "rh",      .type = BSP_TB_VAL_FLOAT, .val.f_val = rh_pct      },
        { .key = "battery", .type = BSP_TB_VAL_INT,   .val.i_val = battery_pct },
        { .key = "rssi",    .type = BSP_TB_VAL_INT,   .val.i_val = rssi_dbm    },
    };
    return bsp_tb_send_telemetry_entries(entries, 4, false, 0);
}

esp_err_t bsp_tb_send_telemetry_sync(float temp_k, float rh_pct, uint8_t battery_pct, int rssi_dbm, uint32_t timeout_ms)
{
    bsp_tb_entry_t entries[] = {
        { .key = "temp",    .type = BSP_TB_VAL_FLOAT, .val.f_val = temp_k },
        { .key = "rh",      .type = BSP_TB_VAL_FLOAT, .val.f_val = rh_pct },
        { .key = "battery", .type = BSP_TB_VAL_INT,   .val.i_val = battery_pct },
        { .key = "rssi",    .type = BSP_TB_VAL_INT,   .val.i_val = rssi_dbm },
    };
    return bsp_tb_send_telemetry_entries(entries, 4, true, timeout_ms);
}

esp_err_t bsp_tb_report_client_attributes_entries(const bsp_tb_entry_t *entries, size_t count)
{
    if (s_mqtt_client == NULL) return ESP_ERR_INVALID_STATE;
    cJSON *root = bsp_tb_entries_to_json(entries, count);
    if (!root) return ESP_ERR_INVALID_ARG;

    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!json_str) return ESP_ERR_NO_MEM;

    int msg_id = esp_mqtt_client_publish(s_mqtt_client, "v1/devices/me/attributes", json_str, 0, 1, 0);
    free(json_str);
    return (msg_id >= 0) ? ESP_OK : ESP_FAIL;
}

esp_err_t bsp_tb_report_client_attributes(void)
{
    if (s_mqtt_client == NULL) return ESP_ERR_INVALID_STATE;

    char dev_id[32] = {0};
    bsp_get_device_id(dev_id, sizeof(dev_id));

    char ip_str[32] = {0};
    bsp_wifi_get_ip_str(ip_str, sizeof(ip_str));

    uint32_t vbat_mv = 0;
    bsp_battery_get_voltage(&vbat_mv, NULL);

    bsp_tb_entry_t entries[] = {
        { .key = "bsp_ver",   .type = BSP_TB_VAL_STRING, .val.s_val = bsp_get_version() },
        { .key = "device_id", .type = BSP_TB_VAL_STRING, .val.s_val = dev_id            },
        { .key = "ip",        .type = BSP_TB_VAL_STRING, .val.s_val = ip_str            },
        { .key = "vbat_mv",   .type = BSP_TB_VAL_INT,    .val.i_val = vbat_mv           },
    };
    return bsp_tb_report_client_attributes_entries(entries, 4);
}

esp_err_t bsp_tb_request_shared_attributes_keys(const char **shared_keys, size_t count)
{
    if (s_mqtt_client == NULL) return ESP_ERR_INVALID_STATE;
    if (shared_keys == NULL || count == 0) {
        return bsp_tb_request_shared_attributes();
    }

    cJSON *root = cJSON_CreateObject();
    if (!root) return ESP_ERR_NO_MEM;

    char keys_buf[256] = {0};
    for (size_t i = 0; i < count; i++) {
        if (i > 0) strncat(keys_buf, ",", sizeof(keys_buf) - strlen(keys_buf) - 1);
        strncat(keys_buf, shared_keys[i], sizeof(keys_buf) - strlen(keys_buf) - 1);
    }
    cJSON_AddStringToObject(root, "sharedKeys", keys_buf);

    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!json_str) return ESP_ERR_NO_MEM;

    int msg_id = esp_mqtt_client_publish(s_mqtt_client, "v1/devices/me/attributes/request/1", json_str, 0, 1, 0);
    free(json_str);
    return (msg_id >= 0) ? ESP_OK : ESP_FAIL;
}

esp_err_t bsp_tb_request_shared_attributes(void)
{
    const char *default_keys[] = {"fw_title", "fw_version", "fw_url", "sleep_interval_sec"};
    return bsp_tb_request_shared_attributes_keys(default_keys, 4);
}

esp_err_t bsp_tb_send_rpc_response(const char *request_id, const char *response_json)
{
    if (s_mqtt_client == NULL || request_id == NULL) return ESP_ERR_INVALID_STATE;
    char topic[64] = {0};
    snprintf(topic, sizeof(topic), "v1/devices/me/rpc/response/%s", request_id);
    int msg_id = esp_mqtt_client_publish(s_mqtt_client, topic, response_json ? response_json : "{}", 0, 0, 0);
    return (msg_id >= 0) ? ESP_OK : ESP_FAIL;
}

esp_err_t bsp_tb_claim_device(const char *secret_key, uint32_t duration_ms)
{
    if (s_mqtt_client == NULL || secret_key == NULL) return ESP_ERR_INVALID_STATE;
    char payload[128] = {0};
    snprintf(payload, sizeof(payload), "{\"secretKey\":\"%s\",\"durationMs\":%lu}", secret_key, (unsigned long)duration_ms);
    int msg_id = esp_mqtt_client_publish(s_mqtt_client, "v1/devices/me/claim", payload, 0, 1, 0);
    return (msg_id >= 0) ? ESP_OK : ESP_FAIL;
}

esp_err_t bsp_tb_claim_device_auto(char *out_key, size_t key_len, uint32_t duration_ms)
{
    char key[8] = {0};
    bsp_generate_unambiguous_key(key, 6, NULL);
    if (out_key && key_len > 6) {
        strlcpy(out_key, key, key_len);
    }
    return bsp_tb_claim_device(key, duration_ms);
}

esp_err_t bsp_tb_report_ota_state(const char *state, const char *error_msg)
{
    if (s_mqtt_client == NULL || state == NULL) return ESP_ERR_INVALID_STATE;
    char payload[128] = {0};
    if (error_msg) {
        snprintf(payload, sizeof(payload), "{\"current_fw_state\":\"%s\",\"fw_error\":\"%s\"}", state, error_msg);
    } else {
        snprintf(payload, sizeof(payload), "{\"current_fw_state\":\"%s\"}", state);
    }
    return esp_mqtt_client_publish(s_mqtt_client, "v1/devices/me/telemetry", payload, 0, 1, 0) >= 0 ? ESP_OK : ESP_FAIL;
}

esp_err_t bsp_tb_disconnect(void)
{
    if (s_mqtt_client == NULL) return ESP_OK;
    esp_mqtt_client_stop(s_mqtt_client);
    esp_mqtt_client_destroy(s_mqtt_client);
    s_mqtt_client = NULL;
    return ESP_OK;
}
