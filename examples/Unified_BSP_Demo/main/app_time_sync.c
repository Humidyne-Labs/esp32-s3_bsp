/**
 * @file app_time_sync.c
 * @brief SNTP Network Time Synchronization to PCF85063A RTC implementation
 * 
 * @attribution
 * - Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "esp_log.h"
#include "esp_sntp.h"
#include "bsp/bsp.h"
#include "app_time_sync.h"

static const char *TAG = "app_time_sync";

static app_time_sync_cb_t s_sync_cb   = NULL;
static bool               s_is_synced = false;

static void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "SNTP time synchronization event triggered");

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    /* Convert system time to PCF85063A RTC format */
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
        ESP_LOGI(TAG, "Committed new SNTP time to PCF85063A RTC: %04d-%02d-%02d %02d:%02d:%02d",
                 rtc_dt.year, rtc_dt.month, rtc_dt.day,
                 rtc_dt.hour, rtc_dt.minute, rtc_dt.second);
        s_is_synced = true;
    } else {
        ESP_LOGE(TAG, "Failed to commit time to RTC: %s", esp_err_to_name(ret));
    }

    if (s_sync_cb) {
        s_sync_cb();
    }
}

esp_err_t app_time_sync_init(app_time_sync_cb_t cb)
{
    s_sync_cb = cb;

    ESP_LOGI(TAG, "Initializing SNTP client...");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_setservername(1, "time.google.com");
    esp_sntp_setservername(2, "time.cloudflare.com");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    esp_sntp_init();

    return ESP_OK;
}

bool app_time_sync_is_synced(void)
{
    return s_is_synced;
}
