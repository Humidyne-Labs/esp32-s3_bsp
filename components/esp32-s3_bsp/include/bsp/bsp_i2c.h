/**
 * @file bsp_i2c.h
 * @brief Thread-Safe Shared I2C Master Bus Driver with Mutex Guarding
 * 
 * Hardware Target:
 *  - SDA: GPIO 47, SCL: GPIO 48
 *  - Clock Speed: 400 kHz (I2C Fast Mode)
 *  - Connected Slaves: SHTC3 (0x70), PCF85063A (0x51), FT6336 (0x38), ES8311 (0x18)
 * 
 * Concurrency Model:
 *  - Uses FreeRTOS recursive mutex guarding to allow safe concurrent access across multiple tasks.
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
#include <stddef.h>
#include "esp_err.h"
#include "driver/i2c_master.h"
#include "bsp/pinout.h"

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
 * @brief De-initialize Shared I2C Master Bus
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_i2c_deinit(void);

/**
 * @brief Get the underlying I2C master bus handle
 * 
 * @return i2c_master_bus_handle_t Bus handle or NULL
 */
i2c_master_bus_handle_t bsp_i2c_get_handle(void);

/**
 * @brief Add a device to the shared I2C master bus
 * 
 * @param dev_cfg Device configuration struct
 * @param[out] dev_handle Destination pointer to receive device handle
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_i2c_add_device(const i2c_device_config_t *dev_cfg, i2c_master_dev_handle_t *dev_handle);

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
 * @param data Data buffer to write (can be NULL if len == 0)
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
