/**
 * @file bsp_time.c
 * @brief SNTP Synchronization, Timezone Management & RTC Bridge Implementation
 * 
 * @attribution
 * - BSP Architecture: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp/bsp_rtc.h"
#include "bsp/bsp_time.h"
#include "bsp/bsp_wifi.h"

static const char *TAG = "bsp_time";

static esp_timer_handle_t s_periodic_sync_timer = NULL;

esp_err_t bsp_time_set_timezone(const char *tz_str)
{
    if (tz_str == NULL || strlen(tz_str) == 0) {
        tz_str = "UTC";
    }
    setenv("TZ", tz_str, 1);
    tzset();
    ESP_LOGI(TAG, "System timezone set to: %s", tz_str);
    return ESP_OK;
}

esp_err_t bsp_time_sync_system_to_rtc(void)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    bsp_rtc_datetime_t rtc_dt = {
        .year    = (uint16_t)(timeinfo.tm_year + 1900),
        .month   = (uint8_t)(timeinfo.tm_mon + 1),
        .day     = (uint8_t)timeinfo.tm_mday,
        .weekday = (uint8_t)timeinfo.tm_wday,
        .hour    = (uint8_t)timeinfo.tm_hour,
        .minute  = (uint8_t)timeinfo.tm_min,
        .second  = (uint8_t)timeinfo.tm_sec,
    };

    esp_err_t ret = bsp_rtc_set_datetime(&rtc_dt);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Synchronized PCF85063A RTC from POSIX clock: %04d-%02d-%02d %02d:%02d:%02d",
                 rtc_dt.year, rtc_dt.month, rtc_dt.day, rtc_dt.hour, rtc_dt.minute, rtc_dt.second);
    } else {
        ESP_LOGE(TAG, "Failed to write PCF85063A RTC: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t bsp_time_sync_rtc_to_system(void)
{
    bsp_rtc_datetime_t rtc_dt;
    esp_err_t ret = bsp_rtc_get_datetime(&rtc_dt);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read PCF85063A RTC: %s", esp_err_to_name(ret));
        return ret;
    }

    struct tm tm_target = {
        .tm_sec   = rtc_dt.second,
        .tm_min   = rtc_dt.minute,
        .tm_hour  = rtc_dt.hour,
        .tm_mday  = rtc_dt.day,
        .tm_mon   = rtc_dt.month - 1,
        .tm_year  = rtc_dt.year - 1900,
        .tm_wday  = rtc_dt.weekday,
        .tm_isdst = -1,
    };

    time_t t = mktime(&tm_target);
    if (t == (time_t)-1) {
        ESP_LOGE(TAG, "Failed to convert RTC date to time_t");
        return ESP_FAIL;
    }

    struct timeval tv = { .tv_sec = t, .tv_usec = 0 };
    settimeofday(&tv, NULL);
    ESP_LOGI(TAG, "Synchronized ESP32 system clock from PCF85063A RTC: %04d-%02d-%02d %02d:%02d:%02d",
             rtc_dt.year, rtc_dt.month, rtc_dt.day, rtc_dt.hour, rtc_dt.minute, rtc_dt.second);
    return ESP_OK;
}

esp_err_t bsp_time_sntp_sync(uint32_t timeout_ms)
{
    if (!bsp_wifi_is_connected()) {
        ESP_LOGW(TAG, "Cannot sync SNTP: Wi-Fi is not connected");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Initializing SNTP client (pool.ntp.org)...");
    esp_sntp_stop();
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_setservername(1, "time.google.com");
    esp_sntp_init();

    TickType_t start_tick    = xTaskGetTickCount();
    TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms > 0 ? timeout_ms : 10000);

    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) {
        if ((xTaskGetTickCount() - start_tick) > timeout_ticks) {
            ESP_LOGW(TAG, "SNTP time synchronization timed out (%lu ms)", (unsigned long)timeout_ms);
            return ESP_ERR_TIMEOUT;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    ESP_LOGI(TAG, "SNTP time synchronization successful");
    bsp_time_sync_system_to_rtc();
    return ESP_OK;
}

esp_err_t bsp_time_get_formatted(bsp_time_format_t fmt, char *dest, size_t max_len)
{
    if (dest == NULL || max_len < 16) {
        return ESP_ERR_INVALID_ARG;
    }

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    switch (fmt) {
        case BSP_TIME_FMT_24H_SEC:
            strftime(dest, max_len, "%H:%M:%S", &timeinfo);
            break;
        case BSP_TIME_FMT_24H_MIN:
            strftime(dest, max_len, "%H:%M", &timeinfo);
            break;
        case BSP_TIME_FMT_12H_SEC:
            strftime(dest, max_len, "%I:%M:%S %p", &timeinfo);
            break;
        case BSP_TIME_FMT_12H_MIN:
            strftime(dest, max_len, "%I:%M %p", &timeinfo);
            break;
        default:
            strftime(dest, max_len, "%H:%M:%S", &timeinfo);
            break;
    }
    return ESP_OK;
}

esp_err_t bsp_time_get_date_formatted(bsp_date_format_t fmt, char *dest, size_t max_len)
{
    if (dest == NULL || max_len < 16) {
        return ESP_ERR_INVALID_ARG;
    }

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    switch (fmt) {
        case BSP_DATE_FMT_MM_DD_YY:
            strftime(dest, max_len, "%m/%d/%y", &timeinfo);
            break;
        case BSP_DATE_FMT_DOW:
            strftime(dest, max_len, "%A", &timeinfo);
            break;
        case BSP_DATE_FMT_MM_DD_YY_DOW:
            strftime(dest, max_len, "%m/%d/%y %A", &timeinfo);
            break;
        default:
            strftime(dest, max_len, "%m/%d/%y", &timeinfo);
            break;
    }
    return ESP_OK;
}

esp_err_t bsp_time_get_date_str(char *dest, size_t max_len)
{
    return bsp_time_get_date_formatted(BSP_DATE_FMT_MM_DD_YY, dest, max_len);
}

esp_err_t bsp_time_get_dow_str(char *dest, size_t max_len)
{
    return bsp_time_get_date_formatted(BSP_DATE_FMT_DOW, dest, max_len);
}

esp_err_t bsp_time_get_date_dow_str(char *dest, size_t max_len)
{
    return bsp_time_get_date_formatted(BSP_DATE_FMT_MM_DD_YY_DOW, dest, max_len);
}

static void periodic_sync_timer_cb(void *arg)
{
    ESP_LOGI(TAG, "Periodic SNTP 24h timer triggered");
    if (bsp_wifi_is_connected()) {
        bsp_time_sntp_sync(8000);
    }
}

esp_err_t bsp_time_start_periodic_sync(uint32_t interval_sec)
{
    if (interval_sec == 0) {
        interval_sec = 86400; // Default: 24 Hours
    }

    if (s_periodic_sync_timer != NULL) {
        esp_timer_stop(s_periodic_sync_timer);
        esp_timer_delete(s_periodic_sync_timer);
        s_periodic_sync_timer = NULL;
    }

    esp_timer_create_args_t timer_args = {
        .callback = periodic_sync_timer_cb,
        .name     = "bsp_sntp_sync_timer",
    };
    esp_err_t ret = esp_timer_create(&timer_args, &s_periodic_sync_timer);
    if (ret != ESP_OK) return ret;

    return esp_timer_start_periodic(s_periodic_sync_timer, (uint64_t)interval_sec * 1000000ULL);
}

void bsp_time_stop_periodic_sync(void)
{
    if (s_periodic_sync_timer != NULL) {
        esp_timer_stop(s_periodic_sync_timer);
        esp_timer_delete(s_periodic_sync_timer);
        s_periodic_sync_timer = NULL;
    }
}
