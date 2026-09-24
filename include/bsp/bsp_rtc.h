/**
 * @file bsp_rtc.h
 * @brief PCF85063A Real-Time Clock BSP Driver
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics
 * - Microcontroller: Espressif Systems ESP32-S3
 * - Original PCF85063A Component: Espressif / Waveshare (Apache-2.0)
 * - BSP Unification & Enhancements: Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/gpio.h"
#include "bsp/pinout.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BSP_GPIO_RTC_INT
#define BSP_GPIO_RTC_INT 5
#endif

typedef struct {
    uint16_t year;    // 2000 - 2099
    uint8_t  month;   // 1 - 12
    uint8_t  day;     // 1 - 31
    uint8_t  weekday; // 0 - 6 (0 = Sunday, 1 = Monday, ...)
    uint8_t  hour;    // 0 - 23 (24-hour mode)
    uint8_t  minute;  // 0 - 59
    uint8_t  second;  // 0 - 59
} bsp_rtc_datetime_t;

typedef struct {
    int8_t second;   // 0 - 59, or -1 to ignore
    int8_t minute;   // 0 - 59, or -1 to ignore
    int8_t hour;     // 0 - 23, or -1 to ignore
    int8_t day;      // 1 - 31, or -1 to ignore
    int8_t weekday;  // 0 - 6,  or -1 to ignore
} bsp_rtc_alarm_t;

typedef enum {
    BSP_RTC_OFFSET_MODE_2_HOURS  = 0, // 4.340 ppm/step (Low power)
    BSP_RTC_OFFSET_MODE_4_MIN    = 1  // 4.069 ppm/step (Fast correction)
} bsp_rtc_offset_mode_t;

/**
 * @brief Initialize the PCF85063A RTC and configure the INT GPIO.
 *        Performs an I2C bus recovery sequence if SDA is stuck.
 */
esp_err_t bsp_rtc_init(void);

/**
 * @brief Deinitialize RTC handle and release bus resources.
 */
esp_err_t bsp_rtc_deinit(void);

/**
 * @brief Perform a software reset on the PCF85063A (Command 0x58).
 */
esp_err_t bsp_rtc_software_reset(void);

/**
 * @brief Check if the oscillator is running and time integrity is guaranteed.
 * @param[out] is_running true if running, false if power loss/stopped (OS flag set).
 */
esp_err_t bsp_rtc_is_running(bool *is_running);

/**
 * @brief Read current date and time from the RTC in a single atomic transaction.
 */
esp_err_t bsp_rtc_get_datetime(bsp_rtc_datetime_t *datetime);

/**
 * @brief Set current date and time on the RTC and clear the OS (Oscillator Stop) flag.
 */
esp_err_t bsp_rtc_set_datetime(const bsp_rtc_datetime_t *datetime);

/**
 * @brief Configure the PCF85063A offset calibration register.
 * @param offset Signed step count (-64 to +63).
 * @param mode Correction interval mode (2 hours or 4 minutes).
 */
esp_err_t bsp_rtc_set_offset(int8_t offset, bsp_rtc_offset_mode_t mode);

/**
 * @brief Configure the PCF85063A hardware alarm.
 * @param alarm Target alarm struct (pass -1 to ignore any field).
 */
esp_err_t bsp_rtc_set_alarm(const bsp_rtc_alarm_t *alarm);

/**
 * @brief Disable and clear the RTC alarm.
 */
esp_err_t bsp_rtc_clear_alarm(void);

/**
 * @brief Configure the PCF85063A 1Hz periodic countdown timer.
 * @param seconds Countdown value (1 to 255 seconds).
 */
esp_err_t bsp_rtc_set_countdown_timer(uint8_t seconds);

/**
 * @brief Disable the countdown timer.
 */
esp_err_t bsp_rtc_clear_countdown_timer(void);

/**
 * @brief Read and clear interrupt flags (AF / TF) while preserving control registers.
 * @param[out] alarm_flag True if an alarm generated the interrupt.
 * @param[out] timer_flag True if the timer generated the interrupt.
 */
esp_err_t bsp_rtc_get_and_clear_interrupts(bool *alarm_flag, bool *timer_flag);

/**
 * @brief Configure ESP32-S3 sleep wakeup source from the RTC INT line (GPIO 5).
 * @param deep_sleep true for Deep Sleep (EXT0), false for Light Sleep (GPIO wakeup).
 */
esp_err_t bsp_rtc_enable_wakeup(bool deep_sleep);

#ifdef __cplusplus
}
#endif