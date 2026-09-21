/**
 * @file bsp_power.h
 * @brief power controller lib
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics (https://www.waveshare.com)
 * - Microcontroller: Espressif Systems ESP32-S3 (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_POWER_H
#define BSP_POWER_H

#include "esp_err.h"
#include "bsp/pinout.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize power management GPIOs and battery ADC
 * 
 * Configures battery control, status LED, and the battery ADC GPIO.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_power_init(void);

/**
 * @brief Keep power on (latch the battery control GPIO high)
 */
void bsp_power_hold(void);

/**
 * @brief Turn power off (release the battery control GPIO low)
 */
void bsp_power_off(void);

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

/**
 * @brief Read battery voltage in millivolts
 * 
 * @param voltage_mv Pointer to store calculated voltage in mV
 * @param raw_adc Pointer to store raw ADC reading (optional, can be NULL)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_battery_get_voltage(uint32_t *voltage_mv, int *raw_adc);

/**
 * @brief Read battery charge percentage (0 - 100%)
 * 
 * @return uint8_t Battery level percentage
 */
uint8_t bsp_battery_get_percentage(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_POWER_H */
