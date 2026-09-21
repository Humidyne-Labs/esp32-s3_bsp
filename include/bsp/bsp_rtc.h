/**
 * @file bsp_rtc.h
 * @brief rtc controller lib
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics (https://www.waveshare.com)
 * - Microcontroller: Espressif Systems ESP32-S3 (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_RTC_H
#define BSP_RTC_H

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t weekday;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} bsp_rtc_datetime_t;

esp_err_t bsp_rtc_init(void);
esp_err_t bsp_rtc_get_datetime(bsp_rtc_datetime_t *datetime);
esp_err_t bsp_rtc_set_datetime(const bsp_rtc_datetime_t *datetime);

#ifdef __cplusplus
}
#endif

#endif /* BSP_RTC_H */
