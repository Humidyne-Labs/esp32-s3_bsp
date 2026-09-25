/**
 * @file bsp_rtc.h
 * @brief NXP PCF85063A Real-Time Clock (RTC) Driver
 * 
 * Hardware Target:
 *  - Device: NXP Semiconductors PCF85063A (I2C Address: 0x51)
 *  - Integrated 32.768 kHz oscillator with quartz crystal compensation
 *  - Ultra-low power timekeeping (< 0.22 µA at 3.3V)
 * 
 * Data Encoding:
 *  - Internal registers use Binary Coded Decimal (BCD).
 *  - Driver handles bidirectional BCD to Binary conversion.
 * 
 * @attribution
 * - NXP Semiconductors (PCF85063A Datasheet Rev. 7 - 2018)
 * - BSP Implementation: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_RTC_H
#define BSP_RTC_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calendar Date & Time Representation
 */
typedef struct {
    uint16_t year;    /*!< Year (e.g. 2026) */
    uint8_t  month;   /*!< Month (1 - 12) */
    uint8_t  day;     /*!< Day of month (1 - 31) */
    uint8_t  weekday; /*!< Day of week (0 = Sunday, 1 = Monday, ... 6 = Saturday) */
    uint8_t  hour;    /*!< Hour (0 - 23, 24-hour format) */
    uint8_t  minute;  /*!< Minute (0 - 59) */
    uint8_t  second;  /*!< Second (0 - 59) */
} bsp_rtc_datetime_t;

/**
 * @brief Initialize PCF85063A Real-Time Clock
 * 
 * Configures 24-hour mode, clears stop bits, and checks oscillator integrity.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_rtc_init(void);

/**
 * @brief Read Current Date and Time from RTC
 * 
 * @param[out] out_dt Destination struct to receive calendar time
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_rtc_get_datetime(bsp_rtc_datetime_t *out_dt);

/**
 * @brief Set Current Date and Time in RTC
 * 
 * @param in_dt Struct containing calendar time to program
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_rtc_set_datetime(const bsp_rtc_datetime_t *in_dt);

#ifdef __cplusplus
}
#endif

#endif /* BSP_RTC_H */
