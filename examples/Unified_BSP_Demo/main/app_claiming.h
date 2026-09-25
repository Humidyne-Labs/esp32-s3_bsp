/**
 * @file app_claiming.h
 * @brief ThingsBoard Device Claiming Key Generator & Manager
 * 
 * @attribution
 * - Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef APP_CLAIMING_H
#define APP_CLAIMING_H

#include <stddef.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Generate a cryptographically random claiming secret key
 * 
 * Generates an uppercase alphanumeric string (A-Z, 0-9) of requested length (6 to 8 characters).
 * 
 * @param out_key Destination buffer (must be at least length + 1 bytes)
 * @param max_len Size of out_key buffer
 * @param key_len Desired length of secret key (clamped to 6-8 chars)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t app_claiming_generate_key(char *out_key, size_t max_len, size_t key_len);

/**
 * @brief Get the currently active device claiming key (or generates one if none exists)
 * 
 * @param out_key Destination buffer (at least 9 bytes)
 * @param max_len Size of destination buffer
 * @return esp_err_t ESP_OK on success
 */
esp_err_t app_claiming_get_active_key(char *out_key, size_t max_len);

#ifdef __cplusplus
}
#endif

#endif /* APP_CLAIMING_H */
