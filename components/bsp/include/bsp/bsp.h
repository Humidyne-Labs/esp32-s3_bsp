/**
 * @file bsp.h
 * @brief ESP32-S3 Touch ePaper Board Support Package - Master Umbrella Header
 * 
 * Provides master initialization, unique device ID helpers, NVS parameters,
 * and umbrella access to all hardware drivers.
 * 
 * @copyright Copyright (c) 2026 Humidyne Labs / Humiditron
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_H
#define BSP_H

#include "bsp/pinout.h"
#include "bsp/bsp_i2c.h"
#include "bsp/bsp_power.h"
#include "bsp/bsp_display.h"
#include "bsp/bsp_touch.h"
#include "bsp/bsp_sensors.h"
#include "bsp/bsp_audio.h"
#include "bsp/bsp_sdcard.h"
#include "bsp/bsp_lvgl.h"
#include "bsp/bsp_nvs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize essential board hardware (Power hold, LED, NVS, shared I2C bus)
 * 
 * Call this function at the start of app_main() to bring up hardware defaults.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_board_init(void);

/**
 * @brief Retrieve unique chip device ID derived from ESP32-S3 eFuse factory MAC
 * 
 * Example output: "ESP32S3-70041D3B"
 * 
 * @param buf Destination buffer (at least 20 bytes)
 * @param max_len Buffer length
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_get_device_id(char *buf, size_t max_len);

/**
 * @brief Retrieve default Bluetooth/Device advertisement name with unique suffix
 * 
 * Example output: "HumidOS-70041D3B"
 * 
 * @param buf Destination buffer
 * @param max_len Buffer length
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_get_device_name(char *buf, size_t max_len);

#ifdef __cplusplus
}
#endif

#endif /* BSP_H */
