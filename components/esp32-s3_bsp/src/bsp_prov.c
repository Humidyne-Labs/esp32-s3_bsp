/**
 * @file bsp_prov.c
 * @brief Turnkey BLE GATT Wi-Fi Provisioning & QR Code Generator Implementation
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "network_provisioning/manager.h"
#include "network_provisioning/scheme_ble.h"
#include "lvgl.h"
#include "libs/qrcode/qrcodegen.h"
#include "bsp/bsp.h"
#include "bsp/bsp_prov.h"
#include "bsp/bsp_wifi.h"

static const char *TAG = "bsp_prov";

static bool                 s_prov_running         = false;
static bsp_prov_event_cb_t  s_prov_cb              = NULL;
static void                 *s_prov_user_data      = NULL;
static char                 s_active_serv_name[48] = {0};

static void prov_event_handler(void *user_data, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base != NETWORK_PROV_EVENT) return;

    switch (event_id) {
    case NETWORK_PROV_START:
        ESP_LOGI(TAG, "BLE Provisioning service started. Service name: %s", s_active_serv_name);
        if (s_prov_cb) s_prov_cb(BSP_PROV_EVENT_STARTED, NULL, s_prov_user_data);
        break;

    case NETWORK_PROV_WIFI_CRED_RECV: {
        wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
        ESP_LOGI(TAG, "Received Wi-Fi credentials via BLE for SSID: %s", (const char *)wifi_sta_cfg->ssid);
        bsp_wifi_save_credentials((const char *)wifi_sta_cfg->ssid, (const char *)wifi_sta_cfg->password);
        if (s_prov_cb) s_prov_cb(BSP_PROV_EVENT_CRED_RECEIVED, wifi_sta_cfg, s_prov_user_data);
        break;
    }

    case NETWORK_PROV_WIFI_CRED_FAIL: {
        network_prov_wifi_sta_fail_reason_t *reason = (network_prov_wifi_sta_fail_reason_t *)event_data;
        ESP_LOGE(TAG, "Provisioning failed! Reason code: %d", (int)*reason);
        if (s_prov_cb) s_prov_cb(BSP_PROV_EVENT_CRED_FAILED, reason, s_prov_user_data);
        break;
    }

    case NETWORK_PROV_WIFI_CRED_SUCCESS:
        ESP_LOGI(TAG, "Provisioning credentials verified and connected successfully");
        if (s_prov_cb) s_prov_cb(BSP_PROV_EVENT_CRED_SUCCESS, NULL, s_prov_user_data);
        break;

    case NETWORK_PROV_END:
        ESP_LOGI(TAG, "Provisioning complete. Releasing Bluetooth resources...");
        network_prov_mgr_deinit();
        s_prov_running = false;
        if (s_prov_cb) s_prov_cb(BSP_PROV_EVENT_FINISHED, NULL, s_prov_user_data);
        break;

    default:
        break;
    }
}

esp_err_t bsp_prov_start(const char *custom_service_name, const char *pop, bsp_prov_event_cb_t cb, void *user_data)
{
    if (s_prov_running) {
        ESP_LOGW(TAG, "BLE provisioning is already active");
        return ESP_OK;
    }

    s_prov_cb        = cb;
    s_prov_user_data = user_data;

    esp_err_t ret = bsp_wifi_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Wi-Fi base: %s", esp_err_to_name(ret));
        return ret;
    }

    network_prov_mgr_config_t config = {
        .scheme               = network_prov_scheme_ble,
        .scheme_event_handler = NETWORK_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM,
        .app_event_handler    = NETWORK_PROV_EVENT_HANDLER_NONE,
    };

    ret = network_prov_mgr_init(config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init network_prov_mgr: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_event_handler_register(NETWORK_PROV_EVENT, ESP_EVENT_ANY_ID, &prov_event_handler, NULL);
    if (ret != ESP_OK) return ret;

    if (custom_service_name != NULL && strlen(custom_service_name) > 0) {
        strlcpy(s_active_serv_name, custom_service_name, sizeof(s_active_serv_name));
    } else {
        char dev_id[32] = {0};
        bsp_get_device_id(dev_id, sizeof(dev_id));
        snprintf(s_active_serv_name, sizeof(s_active_serv_name), "PROV_%s", dev_id);
    }

    network_prov_security_t security = (pop != NULL && strlen(pop) > 0) ? NETWORK_PROV_SECURITY_1 : NETWORK_PROV_SECURITY_0;

    ret = network_prov_mgr_start_provisioning(security, (const void *)pop, s_active_serv_name, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start BLE provisioning: %s", esp_err_to_name(ret));
        network_prov_mgr_deinit();
        return ret;
    }

    s_prov_running = true;
    ESP_LOGI(TAG, "Provisioning active: Name='%s', Security=%d", s_active_serv_name, (int)security);
    return ESP_OK;
}

bool bsp_prov_is_running(void)
{
    return s_prov_running;
}

void bsp_prov_stop(void)
{
    if (s_prov_running) {
        network_prov_mgr_stop_provisioning();
        network_prov_mgr_deinit();
        s_prov_running = false;
        ESP_LOGI(TAG, "Provisioning stopped");
    }
}

esp_err_t bsp_prov_get_qr_payload(const char *pop, char *dest, size_t max_len)
{
    if (dest == NULL || max_len < 64) {
        return ESP_ERR_INVALID_ARG;
    }

    char name[48] = {0};
    if (strlen(s_active_serv_name) > 0) {
        snprintf(name, sizeof(name), "%s", s_active_serv_name);
    } else {
        char dev_id[32] = {0};
        bsp_get_device_id(dev_id, sizeof(dev_id));
        snprintf(name, sizeof(name), "PROV_%s", dev_id);
    }

    if (pop != NULL && strlen(pop) > 0) {
        snprintf(dest, max_len, "{\"ver\":\"v1\",\"name\":\"%s\",\"pop\":\"%s\",\"transport\":\"ble\"}", name, pop);
    } else {
        snprintf(dest, max_len, "{\"ver\":\"v1\",\"name\":\"%s\",\"transport\":\"ble\"}", name);
    }

    return ESP_OK;
}

int32_t bsp_prov_calc_qr_code_size(const char *pop, int32_t max_boundary, int32_t min_scale)
{
    if (max_boundary <= 0) {
        max_boundary = 180;
    }
    if (min_scale <= 0) {
        min_scale = 3;
    }

    char payload[128] = {0};
    if (bsp_prov_get_qr_payload(pop, payload, sizeof(payload)) != ESP_OK) {
        return max_boundary;
    }

    enum qrcodegen_Ecc ecc = qrcodegen_Ecc_MEDIUM;
    uint8_t qr0[qrcodegen_BUFFER_LEN_FOR_VERSION(10)];
    uint8_t tempBuffer[qrcodegen_BUFFER_LEN_FOR_VERSION(10)];

    bool ok = qrcodegen_encodeText(payload, tempBuffer, qr0, ecc,
                                   qrcodegen_VERSION_MIN, 10,
                                   qrcodegen_Mask_AUTO, true);

    int base_modules = ok ? qrcodegen_getSize(qr0) : 33;

    // LVGL 9 QR code widget default includes 4 modules of quiet zone on each side (+8 modules total)
    int total_modules = base_modules + 8;

    // Calculate maximum whole integer scale factor (pixels per module) fitting in max_boundary
    int scale = max_boundary / total_modules;
    if (scale < min_scale) {
        scale = min_scale;
    }

    int32_t exact_size = total_modules * scale;
    ESP_LOGI(TAG, "QR Calc: Payload %u bytes -> Base %d modules (+8 quiet = %d total). Scale %dx -> Whole-factor size %ld px",
             (unsigned)strlen(payload), base_modules, total_modules, scale, (long)exact_size);

    return exact_size;
}

esp_err_t bsp_prov_get_qr_raw_bits(const char *pop, int32_t max_boundary, int32_t min_scale,
                                   uint8_t *out_buf, size_t out_buf_size, int32_t *out_size)
{
    if (out_buf == NULL || out_buf_size == 0 || out_size == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    int32_t target_boundary = (max_boundary <= 0) ? 180 : max_boundary;
    int32_t scale_min       = (min_scale <= 0) ? 3 : min_scale;

    char payload[128] = {0};
    esp_err_t ret = bsp_prov_get_qr_payload(pop, payload, sizeof(payload));
    if (ret != ESP_OK) return ret;

    enum qrcodegen_Ecc ecc = qrcodegen_Ecc_MEDIUM;
    uint8_t qr0[qrcodegen_BUFFER_LEN_FOR_VERSION(10)];
    uint8_t tempBuffer[qrcodegen_BUFFER_LEN_FOR_VERSION(10)];

    bool ok = qrcodegen_encodeText(payload, tempBuffer, qr0, ecc,
                                   qrcodegen_VERSION_MIN, 10,
                                   qrcodegen_Mask_AUTO, true);
    if (!ok) {
        ESP_LOGE(TAG, "Failed to encode QR payload with qrcodegen");
        return ESP_FAIL;
    }

    int qr_modules    = qrcodegen_getSize(qr0);
    int total_modules = qr_modules + 8; // 4 modules quiet zone on each side

    int scale = target_boundary / total_modules;
    if (scale < scale_min) {
        scale = scale_min;
    }

    int32_t snapped_size = total_modules * scale;
    int margin = (snapped_size - (qr_modules * scale)) / 2;

    uint32_t stride = (snapped_size + 7) / 8; // 1-bit packed byte stride
    size_t req_bytes = stride * snapped_size;

    if (out_buf_size < req_bytes) {
        ESP_LOGE(TAG, "Buffer size too small (%u < %u bytes)", (unsigned)out_buf_size, (unsigned)req_bytes);
        return ESP_ERR_NO_MEM;
    }

    *out_size = snapped_size;
    memset(out_buf, 0x00, out_buf_size); // 0 = White background (Palette 0)

    for (int qy = 0; qy < qr_modules; qy++) {
        for (int qx = 0; qx < qr_modules; qx++) {
            if (qrcodegen_getModule(qr0, qx, qy)) {
                int start_x = margin + (qx * scale);
                int start_y = margin + (qy * scale);

                for (int sy = 0; sy < scale; sy++) {
                    int py = start_y + sy;
                    uint8_t *row_bytes = out_buf + (py * stride);
                    for (int sx = 0; sx < scale; sx++) {
                        int px = start_x + sx;
                        row_bytes[px >> 3] |= (1 << (7 - (px & 7))); // Set bit 1 (Black module)
                    }
                }
            }
        }
    }

    return ESP_OK;
}

lv_obj_t *bsp_prov_render_qr_code(lv_obj_t *parent, int32_t size, const char *pop)
{
    if (parent == NULL) {
        return NULL;
    }

    int32_t target_boundary = (size <= 0) ? 180 : size;
    int32_t min_scale       = (size <= 0) ? 3 : 1;

    char payload[128] = {0};
    if (bsp_prov_get_qr_payload(pop, payload, sizeof(payload)) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to generate provisioning QR payload");
        return NULL;
    }

    // Compute optimal snapped pixel size for display boundary
    int32_t snapped_size = bsp_prov_calc_qr_code_size(pop, target_boundary, min_scale);

    // Create standard LVGL 9 QR Code Widget
    lv_obj_t *qr = lv_qrcode_create(parent);
    if (qr == NULL) {
        ESP_LOGE(TAG, "Failed to allocate LVGL QR Code widget");
        return NULL;
    }

    lv_qrcode_set_size(qr, snapped_size);
    lv_qrcode_set_dark_color(qr, lv_color_black());
    lv_qrcode_set_light_color(qr, lv_color_white());
    lv_qrcode_set_data(qr, payload);

    lv_obj_center(qr);
    lv_obj_invalidate(qr);

    ESP_LOGI(TAG, "Rendered standard LVGL QR code widget (%ldx%ld px)",
             (long)snapped_size, (long)snapped_size);
    return qr;
}
