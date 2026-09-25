/**
 * @file bsp_i2c.h
 * @brief Thread-Safe Shared I2C Master Bus Driver with Mutex Guarding
 * 
 * Hardware Target:
 *  - SDA: GPIO 15, SCL: GPIO 20
 *  - Clock Speed: 400 kHz (I2C Fast Mode)
 *  - Connected Slaves: SHTC3 (0x70), PCF85063A (0x51), CST816S (0x15)
 * 
 * Concurrency Model:
 *  - Uses FreeRTOS mutex guarding to allow safe concurrent access across multiple tasks.
 * 
 * @attribution
 * - BSP Implementation: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_I2C_H
#define BSP_I2C_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize Shared I2C Master Bus and Create Mutex Guard
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_i2c_init(void);

/**
 * @brief Write Raw Bytes to an I2C Slave Device (Thread-Safe)
 * 
 * @param addr 7-bit slave device address
 * @param data Data buffer to transmit
 * @param len Number of bytes to transmit
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_i2c_write(uint8_t addr, const uint8_t *data, size_t len);

/**
 * @brief Read Raw Bytes from an I2C Slave Device (Thread-Safe)
 * 
 * @param addr 7-bit slave device address
 * @param[out] data Buffer to receive incoming bytes
 * @param len Number of bytes to read
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_i2c_read(uint8_t addr, uint8_t *data, size_t len);

/**
 * @brief Write Bytes to a Specific 8-bit Register on an I2C Slave (Thread-Safe)
 * 
 * @param addr 7-bit slave device address
 * @param reg 8-bit register address
 * @param data Data buffer to write
 * @param len Number of data bytes
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_i2c_write_reg(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len);

/**
 * @brief Read Bytes from a Specific 8-bit Register on an I2C Slave (Thread-Safe)
 * 
 * @param addr 7-bit slave device address
 * @param reg 8-bit register address
 * @param[out] data Buffer to receive data bytes
 * @param len Number of bytes to read
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_i2c_read_reg(uint8_t addr, uint8_t reg, uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* BSP_I2C_H */
