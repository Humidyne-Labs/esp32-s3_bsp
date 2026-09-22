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
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t weekday;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} bsp_rtc_datetime_t;

typedef struct {
    int8_t second;   // 0-59, or -1 to ignore
    int8_t minute;   // 0-59, or -1 to ignore
    int8_t hour;     // 0-23, or -1 to ignore
    int8_t day;      // 1-31, or -1 to ignore
    int8_t weekday;  // 0-6,  or -1 to ignore
} bsp_rtc_alarm_t;

esp_err_t bsp_rtc_init(void);
esp_err_t bsp_rtc_deinit(void);

esp_err_t bsp_rtc_get_datetime(bsp_rtc_datetime_t *datetime);
esp_err_t bsp_rtc_set_datetime(const bsp_rtc_datetime_t *datetime);

/**
 * @brief Configure the PCF85063A hardware alarm
 * @param alarm Target alarm struct (pass -1 to ignore any field)
 */
esp_err_t bsp_rtc_set_alarm(const bsp_rtc_alarm_t *alarm);
esp_err_t bsp_rtc_clear_alarm(void);

/**
 * @brief Configure the PCF85063A 1Hz periodic countdown timer
 * @param seconds Duration in seconds (1 to 255)
 */
esp_err_t bsp_rtc_set_countdown_timer(uint8_t seconds);
esp_err_t bsp_rtc_clear_countdown_timer(void);

/**
 * @brief Read and clear interrupt status flags (AF / TF)
 * @param alarm_flag True if an alarm triggered the interrupt
 * @param timer_flag True if the countdown timer triggered the interrupt
 */
esp_err_t bsp_rtc_get_and_clear_interrupts(bool *alarm_flag, bool *timer_flag);

/**
 * @brief Enable ESP32-S3 sleep wakeup from PCF85063A INT (GPIO 5)
 * @param deep_sleep true = configure EXT0 for Deep Sleep; false = Light Sleep GPIO wakeup
 */
esp_err_t bsp_rtc_enable_wakeup(bool deep_sleep);

#ifdef __cplusplus
}
#endif

#endif /* BSP_RTC_H */

/*
    // Example: Wake up every 60 seconds from Deep Sleep
    void enter_timed_deep_sleep(void)
    {
        bsp_rtc_init();

        // Set 60-second hardware timer on PCF85063A
        bsp_rtc_set_countdown_timer(60);

        // Put display into ultra-low power sleep (<1uA)
        bsp_display_deep_sleep();

        // Arm ESP32-S3 to wake on GPIO 5 going LOW
        bsp_rtc_enable_wakeup(true);

        ESP_LOGI("APP", "Entering deep sleep...");
        esp_deep_sleep_start();
    }
*/