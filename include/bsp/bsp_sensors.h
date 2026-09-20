/**
 * @file bsp_sensors.h
 * @brief ESP32-S3 ePaper BSP - Onboard Environmental Sensors Driver (SHTC3)
 * 
 * Provides native temperature in Kelvin (K) and relative humidity (%) readings
 * from the SHTC3 sensor.
 * 
 * @note Based on Sensirion SHTC3 driver specifications & Waveshare ePaper sample library.
 * @copyright Copyright (c) 2026 Humidyne Labs / Humiditron
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_SENSORS_H
#define BSP_SENSORS_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_SHTC3_I2C_ADDR      (0x70)

typedef enum {
    BSP_SHTC3_POWER_MODE_NORMAL = 0,
    BSP_SHTC3_POWER_MODE_LOW,
} bsp_shtc3_power_mode_t;

typedef struct {
    float temperature_k;       /**< Native Temperature in Kelvin (K) */
    float humidity_percent;    /**< Relative Humidity (%) */
} bsp_shtc3_data_t;

/**
 * @brief Initialize SHTC3 temperature and humidity sensor over shared I2C bus
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_shtc3_init(void);

/**
 * @brief Select the SHTC3 measurement power mode.
 *
 * This setting applies to subsequent measurements and may be changed before
 * or after initialization.
 */
esp_err_t bsp_shtc3_set_power_mode(bsp_shtc3_power_mode_t mode);

/**
 * @brief Read current temperature (natively in Kelvin) and humidity from SHTC3
 * 
 * @param data Pointer to output struct receiving Kelvin temperature and humidity
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_shtc3_read(bsp_shtc3_data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* BSP_SENSORS_H */
