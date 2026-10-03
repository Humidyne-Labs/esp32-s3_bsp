/**
 * @file bsp.h
 * @brief Master Include Header for Waveshare ESP32-S3-ePaper-1.54 V2 BSP
 *
 * Umbrella header providing a consolidated, single-include point for all peripheral
 * drivers, hardware abstractions, error handling, versioning, and power management routines.
 *
 * Hardware Target:
 *  - Microcontroller: Espressif Systems ESP32-S3-PICO-1-N8R8
 *  - Target Board: Waveshare ESP32-S3 ePaper 1.54 V2
 *
 * @version 1.0.0
 * @attribution
 * - Architecture & Development: HUMIDYNE LABS / Humiditron
 * - AI Systems Co-Developer: Gemini (Google DeepMind)
 * - Hardware Target: Waveshare Electronics ESP32-S3 ePaper 1.54 V2
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_H
#define BSP_H

#include "bsp/bsp_version.h"
#include "bsp/bsp_err.h"
#include "bsp/pinout.h"
#include "bsp/bsp_power.h"
#include "bsp/bsp_i2c.h"
#include "bsp/bsp_sensors.h"
#include "bsp/bsp_sensor_cal.h"
#include "bsp/bsp_rtc.h"
#include "bsp/bsp_button.h"
#include "bsp/bsp_audio.h"
#include "bsp/bsp_sdcard.h"
#include "bsp/bsp_display.h"
#include "bsp/bsp_lvgl.h"
#include "bsp/bsp_nvs.h"
#include "bsp/bsp_wifi.h"
#include "bsp/bsp_rtc_mem.h"
#include "bsp/bsp_splash.h"
#include "bsp/bsp_time.h"
#include "bsp/bsp_prov.h"
#include "bsp/bsp_ota.h"
#if CONFIG_BSP_ENABLE_THINGSBOARD
#include "bsp/bsp_tb.h"
#endif
#include "bsp/bsp_lifecycle.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Modular Hardware Initialization Configuration
 */
typedef struct {
    bool  init_power;    ///< Hold LDO power rail HIGH and calibrate ADC battery monitor (Default: true)
    bool  init_i2c;      ///< Initialize shared I2C bus at 400kHz with mutex protection (Default: true)
    bool  init_sensors;  ///< Initialize Sensirion SHTC3 environmental sensor (Default: true)
    bool  init_rtc;      ///< Initialize PCF85063A hardware real-time clock (Default: true)
    bool  init_buttons;  ///< Initialize debounced interrupt handlers for BOOT and POWER keys (Default: true)
    bool  init_audio;    ///< Initialize ES8311 I2S audio codec & NS4168 amp (Default: true)
    float audio_volume;  ///< Initial audio volume 0-100 (Default: 80.0)
    bool  init_sdcard;   ///< Mount MicroSD card over SDMMC FATFS (Default: false)
    bool  init_display;  ///< Initialize SSD1681 1.54" SPI e-Paper display (Default: true)
    bool  init_nvs;      ///< Initialize non-volatile flash storage (Default: true)
    bool  start_lvgl;    ///< Spawn LVGL v9 FreeRTOS render task pinned to Core 1 (Default: true)
} bsp_config_t;

/**
 * @brief Default Hardware Initialization Configuration Macro
 */
#define BSP_CONFIG_DEFAULT() { \
    .init_power   = true,      \
    .init_i2c     = true,      \
    .init_sensors = true,      \
    .init_rtc     = true,      \
    .init_buttons = true,      \
    .init_audio   = true,      \
    .audio_volume = 80.0f,     \
    .init_sdcard  = false,     \
    .init_display = true,      \
    .init_nvs     = true,      \
    .start_lvgl   = true       \
}

/**
 * @brief Comprehensive Board Initialization
 *
 * Applies default configuration, enables power rails, configures I2C, RTC, sensors,
 * buttons, display driver, and starts the LVGL v9 rendering task pinned to Core 1.
 *
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_board_init(void);

/**
 * @brief Dynamic Hardware Initialization by Mode (FULL, FAST)
 *
 * - BSP_INIT_MODE_FULL: Cold boot, full peripheral startup, display + LVGL.
 * - BSP_INIT_MODE_FAST: Wake boot, fast display ready without clear.
 *
 * @param[in] mode Target bsp_init_mode_t
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_init_mode(bsp_init_mode_t mode);

/**
 * @brief Custom Board Initialization
 *
 * @param[in] config Pointer to custom bsp_config_t struct
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_board_init_with_config(const bsp_config_t *config);

/**
 * @brief Initialize all GPIO output pins (power latch, audio PA, display power rail, LED)
 *
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_init_io(void);

/**
 * @brief Retrieve Unique Hardware Device ID string from MAC address (e.g. "ESP32S3-70041D3B")
 *
 * @param[out] buf Destination buffer
 * @param[in] max_len Buffer length
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_get_device_id(char *buf, size_t max_len);

/**
 * @brief Retrieve Human-Readable Device Name string (e.g. "HumidOS-1D3B")
 *
 * @param[out] buf Destination buffer
 * @param[in] max_len Buffer length
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_get_device_name(char *buf, size_t max_len);

/**
 * @brief Base57 character set excluding visually ambiguous characters (0, O, o, 1, l, I)
 */
#define BSP_CHARSET_UNAMBIGUOUS "abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ23456789"

/**
 * @brief Generate a cryptographically random, unambiguous key string (excluding 0, O, o, 1, l, I)
 *
 * @param[out] buf Destination buffer
 * @param[in] len Key length (excluding null terminator)
 * @param[in] charset Custom character set (or NULL for BSP_CHARSET_UNAMBIGUOUS)
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_generate_unambiguous_key(char *buf, size_t len, const char *charset);

/**
 * @brief Perform Clean System Shutdown
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
void bsp_system_shutdown(void);

/**
 * @brief Enter Ultra-Low Power Deep Sleep Mode
 *
 * @param[in] sleep_sec Duration in seconds (0 for button wakeup only)
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
void bsp_system_deep_sleep(uint32_t sleep_sec);

/**
 * @brief Generic Millisecond Delay Helper
 *
 * Suspends execution for the specified duration in milliseconds.
 * Uses FreeRTOS task delay if the scheduler is running, or hardware ROM delay on early boot.
 * Allows delaying execution without including FreeRTOS header files in application code.
 *
 * @param[in] ms Delay duration in milliseconds
 * @details Memory ownership: none. Behavior: Blocking / Task Yield. Thread safety: Thread-safe.
 */
void bsp_delay_ms(uint32_t ms);

/**
 * @brief Generic Microsecond Delay Helper
 *
 * Pauses execution for the specified duration in microseconds using high-resolution timer.
 * Does not require including FreeRTOS headers.
 *
 * @param[in] us Delay duration in microseconds
 * @details Memory ownership: none. Behavior: Busy-wait delay. Thread safety: Thread-safe.
 */
void bsp_delay_us(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif /* BSP_H */
