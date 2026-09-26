/**
 * @file app_claiming.c
 * @brief ThingsBoard Device Claiming Key Generator implementation
 * 
 * @attribution
 * - Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_random.h"
#include "app_claiming.h"
#include "app_secrets.h"

static const char *TAG = "app_claiming";

/* Character set: Uppercase A-Z and digits 0-9 (36 characters total) */
static const char CHARSET[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
#define CHARSET_SIZE (sizeof(CHARSET) - 1)

static char s_cached_claim_key[16] = {0};

esp_err_t app_claiming_generate_key(char *out_key, size_t max_len, size_t key_len)
{
    if (out_key == NULL || max_len == 0) return ESP_ERR_INVALID_ARG;

    /* Enforce 4 to 8 character length requirement */
    if (key_len < 4) key_len = 4;
    if (key_len > 8) key_len = 8;
    if (max_len <= key_len) return ESP_ERR_INVALID_SIZE;

    uint8_t random_bytes[8];
    esp_fill_random(random_bytes, key_len);

    for (size_t i = 0; i < key_len; i++) {
        out_key[i] = CHARSET[random_bytes[i] % CHARSET_SIZE];
    }
    out_key[key_len] = '\0';

    ESP_LOGI(TAG, "Generated %zu-character Device Claiming Key: %s", key_len, out_key);
    return ESP_OK;
}

esp_err_t app_claiming_get_active_key(char *out_key, size_t max_len)
{
    if (out_key == NULL || max_len < 9) return ESP_ERR_INVALID_ARG;

    if (strlen(s_cached_claim_key) == 0) {
        esp_err_t ret = app_claiming_generate_key(s_cached_claim_key, sizeof(s_cached_claim_key), CONFIG_CLAIM_KEY_LENGTH);
        if (ret != ESP_OK) return ret;
    }

    strncpy(out_key, s_cached_claim_key, max_len - 1);
    out_key[max_len - 1] = '\0';
    return ESP_OK;
}

esp_err_t app_claiming_generate_new_key(char *out_key, size_t max_len)
{
    s_cached_claim_key[0] = '\0';
    return app_claiming_get_active_key(out_key, max_len);
}
