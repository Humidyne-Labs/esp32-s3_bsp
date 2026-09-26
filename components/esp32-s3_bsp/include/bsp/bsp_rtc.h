/**
 * @file bsp_rtc.h
 * @brief PCF85063A Real-Time Clock BSP Driver
 * 
 * Hardware Target:
 *  - Device: NXP Semiconductors PCF85063A (I2C Address: 0x51)
 *  - Integrated 32.768 kHz oscillator with quartz crystal compensation
 *  - Ultra-low power timekeeping (< 0.22 µA at 3.3V)
 *  - Hardware Alarm & 1Hz Periodic Countdown Timer
 *  - Interrupt line: GPIO 5 (RTC_INT, Active Low)
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics
 * - Microcontroller: Espressif Systems ESP32-S3
 * - Original PCF85063A Component: Espressif / Waveshare (Apache-2.0)
 * - BSP Unification & Enhancements: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_RTC_H
#define BSP_RTC_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/gpio.h"
#include "bsp/pinout.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t year;    /*!< Year (2000 - 2099) */
    uint8_t  month;   /*!< Month (1 - 12) */
    uint8_t  day;     /*!< Day of month (1 - 31) */
    uint8_t  weekday; /*!< Day of week (0 = Sunday, 1 = Monday, ... 6 = Saturday) */
    uint8_t  hour;    /*!< Hour (0 - 23, 24-hour mode) */
    uint8_t  minute;  /*!< Minute (0 - 59) */
    uint8_t  second;  /*!< Second (0 - 59) */
} bsp_rtc_datetime_t;

typedef struct {
    int8_t second;   /*!< 0 - 59, or -1 to ignore */
    int8_t minute;   /*!< 0 - 59, or -1 to ignore */
    int8_t hour;     /*!< 0 - 23, or -1 to ignore */
    int8_t day;      /*!< 1 - 31, or -1 to ignore */
    int8_t weekday;  /*!< 0 - 6,  or -1 to ignore */
} bsp_rtc_alarm_t;

typedef enum {
    BSP_RTC_OFFSET_MODE_2_HOURS  = 0, /*!< 4.340 ppm/step (Low power) */
    BSP_RTC_OFFSET_MODE_4_MIN    = 1  /*!< 4.069 ppm/step (Fast correction) */
} bsp_rtc_offset_mode_t;

/**
 * @brief Initialize the PCF85063A RTC and configure the INT GPIO.
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
 * @brief Disable CLKOUT square wave output on PCF85063A (COF = 0x07).
 * 
 * Ensures external CLKOUT output pin is high-impedance to eliminate noise and save power.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_rtc_disable_clkout(void);

/**
 * @brief Stop the PCF85063A 32.768 kHz quartz crystal oscillator.
 * 
 * Sets STOP=1 in Control_1 register to halt the oscillator and divider chain.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_rtc_stop_oscillator(void);

/**
 * @brief Start/Resume the PCF85063A 32.768 kHz quartz crystal oscillator.
 * 
 * Sets STOP=0 in Control_1 register to resume active quartz timekeeping.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_rtc_start_oscillator(void);

/**
 * @brief Configure ESP32-S3 sleep wakeup source from the RTC INT line (GPIO 5).
 * @param deep_sleep true for Deep Sleep (EXT1), false for Light Sleep (GPIO wakeup).
 */
esp_err_t bsp_rtc_enable_wakeup(bool deep_sleep);

#ifdef __cplusplus
}
#endif

#endif /* BSP_RTC_H */

