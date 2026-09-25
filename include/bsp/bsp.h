/**
 * @file bsp.h
 * @brief Master Include Header for Waveshare ESP32-S3-Touch-ePaper-1.54 V2 BSP
 * 
 * This umbrella header provides a consolidated, single-include point for all peripheral
 * drivers, hardware abstractions, and system power management routines provided by the
 * Board Support Package (BSP).
 * 
 * Architecture Overview:
 *  - Core 1 is reserved for LVGL v9 graphics rendering, SPI display bit-blitting, and tactile buttons.
 *  - Core 0 is reserved for Wi-Fi Station connectivity, BLE GATT provisioning, and ThingsBoard MQTTS.
 *  - Shared I2C Bus is protected by a FreeRTOS recursive mutex to prevent bus collisions.
 * 
 * Hardware Target:
 *  - Microcontroller: Espressif Systems ESP32-S3 (Xtensa Dual-Core LX7)
 *  - Target Board: Waveshare ESP32-S3-Touch-ePaper-1.54 V2
 * 
 * @attribution
 * - Microcontroller Architecture: Espressif Systems (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_H
#define BSP_H

#include "bsp/pinout.h"
#include "bsp/bsp_power.h"
#include "bsp/bsp_i2c.h"
#include "bsp/bsp_sensors.h"
#include "bsp/bsp_rtc.h"
#include "bsp/bsp_button.h"
#include "bsp/bsp_audio.h"
#include "bsp/bsp_sdcard.h"
#include "bsp/bsp_touch.h"
#include "bsp/bsp_display.h"
#include "bsp/bsp_lvgl.h"
#include "bsp/bsp_nvs.h"
#include "bsp/bsp_wifi.h"
#include "bsp/bsp_assets.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Modular Hardware Initialization Configuration
 * 
 * Allows granular control over which hardware subsystems are initialized during boot.
 */
typedef struct {
    bool init_power;    /*!< Hold LDO power rail HIGH and calibrate ADC battery monitor (Default: true) */
    bool init_i2c;      /*!< Initialize shared I2C bus at 400kHz with mutex protection (Default: true) */
    bool init_sensors;  /*!< Initialize Sensirion SHTC3 environmental sensor (Default: true) */
    bool init_rtc;      /*!< Initialize PCF85063A hardware real-time clock (Default: true) */
    bool init_buttons;  /*!< Initialize debounced interrupt handlers for BOOT and POWER keys (Default: true) */
    bool init_audio;    /*!< Initialize MAX98357A I2S audio amplifier (Default: true) */
    bool init_sdcard;   /*!< Mount MicroSD card over SPI FATFS (Default: false) */
    bool init_touch;    /*!< Initialize CST816S capacitive touch controller (Default: true) */
    bool init_display;  /*!< Initialize SSD1681 1.54" SPI e-Paper display (Default: true) */
    bool init_nvs;      /*!< Initialize non-volatile flash storage (Default: true) */
    bool start_lvgl;    /*!< Spawn LVGL v9 FreeRTOS render task pinned to Core 1 (Default: true) */
} bsp_config_t;

/**
 * @brief Default Hardware Initialization Configuration Macro
 * 
 * Initializes all core peripherals, holding the power latch and launching LVGL on Core 1.
 */
#define BSP_CONFIG_DEFAULT() { \
    .init_power   = true,      \
    .init_i2c     = true,      \
    .init_sensors = true,      \
    .init_rtc     = true,      \
    .init_buttons = true,      \
    .init_audio   = true,      \
    .init_sdcard  = false,     \
    .init_touch   = true,      \
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
 * @return esp_err_t ESP_OK on success, or specific error code on failure
 */
esp_err_t bsp_board_init(void);

/**
 * @brief Custom Granular Board Initialization
 * 
 * Allows selective initialization of peripherals via the bsp_config_t struct.
 * 
 * @param cfg Subsystem enablement flags
 * @return esp_err_t ESP_OK on success, or specific error code on failure
 */
esp_err_t bsp_board_init_custom(const bsp_config_t *cfg);

/**
 * @brief Retrieve Unique Hardware Device ID string
 * 
 * Generates an 8-character unique uppercase hexadecimal string derived from the ESP32-S3's
 * factory eFuse MAC address (e.g., "70041D3B").
 * 
 * @param out_id Destination character buffer
 * @param max_len Size of buffer (minimum 9 bytes recommended)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_get_device_id(char *out_id, size_t max_len);

/**
 * @brief Retrieve Human-Readable Device Name string
 * 
 * Formats a device name using the standard prefix and unique ID (e.g. "HumidOS-70041D3B").
 * 
 * @param out_name Destination character buffer
 * @param max_len Size of buffer (minimum 24 bytes recommended)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_get_device_name(char *out_name, size_t max_len);

/**
 * @brief Perform Clean System Shutdown
 * 
 * Flushes all pending storage/display buffers, releases the hardware power latch (GPIO 2),
 * and enters low-power standby until the user presses the POWER key.
 */
void bsp_system_shutdown(void);

/**
 * @brief Enter Ultra-Low Power Deep Sleep Mode
 * 
 * Preserves the bi-stable e-Paper display contents with zero power draw, disables
 * high-speed clocks and radios, and configures RTC wakeup timer and button GPIOs.
 * 
 * @param sleep_seconds Duration to sleep in seconds (0 for indefinite button wakeup)
 */
void bsp_system_deep_sleep(uint32_t sleep_seconds);

#ifdef __cplusplus
}
#endif

#endif /* BSP_H */
