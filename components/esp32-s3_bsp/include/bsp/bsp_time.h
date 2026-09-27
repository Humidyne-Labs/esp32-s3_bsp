/**
 * @file bsp_time.h
 * @brief SNTP Synchronization, Timezone Management, and Hardware RTC Bridge
 * 
 * Hardware Target:
 *  - NXP PCF85063A Hardware RTC (I2C: 0x51)
 *  - ESP32-S3 Internal High-Resolution POSIX Time (esp_timer / gettimeofday)
 * 
 * Features:
 *  - SNTP Network Time Synchronization via Wi-Fi
 *  - Automatic bidirectional sync between POSIX system time and PCF85063A
 *  - POSIX Timezone Configuration (e.g., "EST5EDT,M3.2.0,M11.1.0")
 *  - 4 Standardized Formatted Time String Generators:
 *      1. 24-Hour with Seconds ("HH:MM:SS")
 *      2. 24-Hour without Seconds ("HH:MM")
 *      3. 12-Hour with Seconds & AM/PM ("hh:mm:ss AM/PM")
 *      4. 12-Hour without Seconds & AM/PM ("hh:mm AM/PM")
 *  - Formatted Date String Generator: "MM/DD/YY DayOfWeek"
 * 
 * @attribution
 * - BSP Architecture: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_TIME_H
#define BSP_TIME_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Formatted Time Representation Modes
 */
typedef enum {
    BSP_TIME_FMT_24H_SEC = 0, /*!< "14:35:08" */
    BSP_TIME_FMT_24H_MIN = 1, /*!< "14:35" */
    BSP_TIME_FMT_12H_SEC = 2, /*!< "02:35:08 PM" */
    BSP_TIME_FMT_12H_MIN = 3, /*!< "02:35 PM" */
} bsp_time_format_t;

/**
 * @brief Formatted Date Representation Modes
 */
typedef enum {
    BSP_DATE_FMT_MM_DD_YY     = 0, /*!< "09/26/26" */
    BSP_DATE_FMT_DOW          = 1, /*!< "Saturday" */
    BSP_DATE_FMT_MM_DD_YY_DOW = 2, /*!< "09/26/26 Saturday" */
} bsp_date_format_t;

/**
 * @brief Configure System Timezone String
 * 
 * @param tz_str Standard POSIX Timezone string (e.g. "EST5EDT,M3.2.0,M11.1.0" or "UTC")
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_time_set_timezone(const char *tz_str);

/**
 * @brief Perform SNTP Network Time Synchronization
 * 
 * Connects to NTP server (pool.ntp.org), sets ESP32 system clock,
 * and synchronizes the external PCF85063A RTC hardware registers.
 * 
 * @param timeout_ms Maximum time to wait for NTP response
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_time_sntp_sync(uint32_t timeout_ms);

/**
 * @brief Synchronize internal ESP32 system time into external PCF85063A RTC
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_time_sync_system_to_rtc(void);

/**
 * @brief Synchronize external PCF85063A RTC hardware time into internal ESP32 system time
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_time_sync_rtc_to_system(void);

/**
 * @brief Format Current Local Time into Specified String Format
 * 
 * @param fmt Target bsp_time_format_t
 * @param dest Destination string buffer
 * @param max_len Buffer size
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_time_get_formatted(bsp_time_format_t fmt, char *dest, size_t max_len);

/**
 * @brief Format Current Local Date into Specified Date Format
 * 
 * @param fmt Target bsp_date_format_t
 * @param dest Destination string buffer
 * @param max_len Buffer size (minimum 24 bytes recommended)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_time_get_date_formatted(bsp_date_format_t fmt, char *dest, size_t max_len);

/**
 * @brief Format Current Date String as "MM/DD/YY" (e.g. "09/26/26")
 * 
 * @param dest Destination string buffer
 * @param max_len Buffer size (minimum 16 bytes recommended)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_time_get_date_str(char *dest, size_t max_len);

/**
 * @brief Format Current Day-of-Week String as "DayOfWeek" (e.g. "Saturday")
 * 
 * @param dest Destination string buffer
 * @param max_len Buffer size (minimum 16 bytes recommended)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_time_get_dow_str(char *dest, size_t max_len);

/**
 * @brief Format Current Date and Day-of-Week String as "MM/DD/YY DayOfWeek" (e.g. "09/26/26 Saturday")
 * 
 * @param dest Destination string buffer
 * @param max_len Buffer size (minimum 24 bytes recommended)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_time_get_date_dow_str(char *dest, size_t max_len);

/**
 * @brief Start Background Periodic SNTP Synchronization Timer (e.g., every 24 hours)
 * 
 * @param interval_sec Synchronization interval in seconds (Default: 86400 / 24h)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_time_start_periodic_sync(uint32_t interval_sec);

/**
 * @brief Stop Background Periodic SNTP Timer
 */
void bsp_time_stop_periodic_sync(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_TIME_H */
