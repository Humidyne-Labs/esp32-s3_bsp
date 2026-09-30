/**
 * @file bsp_splash.c
 * @brief System UI Splash Screens & Audio Chime Callback Dispatcher Implementation
 *
 * @attribution
 * - BSP Architecture: Humidyne Labs / Humiditron (2026)
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "bsp/bsp_splash.h"

static const char *TAG = "bsp_splash";

typedef struct {
    bsp_splash_cb_t cb; ///< cb value
    void            *user_data; ///< user_data value
} splash_entry_t;

typedef struct {
    bsp_chime_cb_t  cb; ///< cb value
    void            *user_data; ///< user_data value
} chime_entry_t;

static splash_entry_t s_splash_table[BSP_SPLASH_MAX] = {0};
static chime_entry_t  s_chime_table[BSP_CHIME_MAX]   = {0};

esp_err_t bsp_register_splash_cb(bsp_splash_type_t type, bsp_splash_cb_t cb, void *user_data)
{
    if (type >= BSP_SPLASH_MAX) {
        return ESP_ERR_INVALID_ARG;
    }
    s_splash_table[type].cb        = cb;
    s_splash_table[type].user_data = user_data;
    ESP_LOGD(TAG, "Registered Splash Callback for type %d", (int)type);
    return ESP_OK;
}

esp_err_t bsp_unregister_splash_cb(bsp_splash_type_t type)
{
    if (type >= BSP_SPLASH_MAX) {
        return ESP_ERR_INVALID_ARG;
    }
    s_splash_table[type].cb        = NULL;
    s_splash_table[type].user_data = NULL;
    return ESP_OK;
}

esp_err_t bsp_register_chime_cb(bsp_chime_type_t type, bsp_chime_cb_t cb, void *user_data)
{
    if (type >= BSP_CHIME_MAX) {
        return ESP_ERR_INVALID_ARG;
    }
    s_chime_table[type].cb        = cb;
    s_chime_table[type].user_data = user_data;
    ESP_LOGD(TAG, "Registered Chime Callback for type %d", (int)type);
    return ESP_OK;
}

esp_err_t bsp_unregister_chime_cb(bsp_chime_type_t type)
{
    if (type >= BSP_CHIME_MAX) {
        return ESP_ERR_INVALID_ARG;
    }
    s_chime_table[type].cb        = NULL;
    s_chime_table[type].user_data = NULL;
    return ESP_OK;
}

bool bsp_has_splash_cb(bsp_splash_type_t type)
{
    if (type >= BSP_SPLASH_MAX) {
        return false;
    }
    return (s_splash_table[type].cb != NULL);
}

bool bsp_has_chime_cb(bsp_chime_type_t type)
{
    if (type >= BSP_CHIME_MAX) {
        return false;
    }
    return (s_chime_table[type].cb != NULL);
}

esp_err_t bsp_trigger_splash(bsp_splash_type_t type)
{
    if (type >= BSP_SPLASH_MAX) {
        return ESP_ERR_INVALID_ARG;
    }
    if (s_splash_table[type].cb != NULL) {
        ESP_LOGI(TAG, "Triggering Splash Event %d...", (int)type);
        s_splash_table[type].cb(type, s_splash_table[type].user_data);
        return ESP_OK;
    }
    return ESP_ERR_NOT_FOUND;
}

esp_err_t bsp_trigger_chime(bsp_chime_type_t type)
{
    if (type >= BSP_CHIME_MAX) {
        return ESP_ERR_INVALID_ARG;
    }
    if (s_chime_table[type].cb != NULL) {
        ESP_LOGI(TAG, "Triggering Chime Event %d...", (int)type);
        s_chime_table[type].cb(type, s_chime_table[type].user_data);
        return ESP_OK;
    }
    return ESP_ERR_NOT_FOUND;
}
