/**
 * @file app_time_sync.h
 * @brief SNTP Network Time Synchronization to PCF85063A RTC
 * 
 * @attribution
 * - Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef APP_TIME_SYNC_H
#define APP_TIME_SYNC_H

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Callback invoked when SNTP time sync completes
 */
typedef void (*app_time_sync_cb_t)(void);

/**
 * @brief Initialize SNTP background time synchronization
 * 
 * Automatically synchronizes with NTP servers (pool.ntp.org, time.google.com)
 * and updates the physical PCF85063A RTC hardware registers.
 * 
 * @param cb Optional completion callback (can be NULL)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t app_time_sync_init(app_time_sync_cb_t cb);

/**
 * @brief Check if time has been successfully synchronized since boot
 * 
 * @return true if synchronized, false otherwise
 */
bool app_time_sync_is_synced(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_TIME_SYNC_H */
