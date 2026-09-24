/**
 * @file bsp.h
 * @brief general header
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics (https://www.waveshare.com)
 * - Microcontroller: Espressif Systems ESP32-S3 (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_H
#define BSP_H

#include "bsp/pinout.h"
#include "bsp/bsp_i2c.h"
#include "bsp/bsp_rtc.h"
#include "bsp/bsp_button.h"
#include "bsp/bsp_power.h"
#include "bsp/bsp_display.h"
#include "bsp/bsp_touch.h"
#include "bsp/bsp_sensors.h"
#include "bsp/bsp_audio.h"
#include "bsp/bsp_sdcard.h"
#include "bsp/bsp_lvgl.h"
#include "bsp/bsp_nvs.h"
#include "bsp/mmap_lvgl_decoder.h"

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
 * @brief init IO
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_init_io(void);

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

/**
 * @brief Keep power on (latch the battery control GPIO high)
 */
//void bsp_power_hold(void);

/**
 * @brief Turn power off (release the battery control GPIO low)
 */
//void bsp_power_off(void);

/**
 * @brief Set status LED state
 * 
 * @param enable true for ON, false for OFF
 */
void bsp_led_set(bool enable);

/**
 * @brief Toggle status LED
 */
void bsp_led_toggle(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_H */
