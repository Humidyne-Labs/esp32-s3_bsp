#ifndef BSP_I2C_H
#define BSP_I2C_H

#include "esp_err.h"
#include "driver/i2c_master.h"
#include "bsp/pinout.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize shared I2C master bus for board peripherals
 * 
 * Configures GPIO6 (SCL) and GPIO7 (SDA) with internal pull-ups and glitch filter.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_i2c_init(void);

/**
 * @brief Deinitialize shared I2C master bus
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_i2c_deinit(void);

/**
 * @brief Get the initialized I2C master bus handle
 * 
 * @return i2c_master_bus_handle_t Handle or NULL if not initialized
 */
i2c_master_bus_handle_t bsp_i2c_get_handle(void);

/**
 * @brief Add a device to the shared I2C master bus
 * 
 * @param dev_cfg Device configuration
 * @param dev_handle Pointer to output device handle
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_i2c_add_device(const i2c_device_config_t *dev_cfg, i2c_master_dev_handle_t *dev_handle);

/**
 * @brief Write data bytes to a specific register on an I2C device
 * 
 * @param dev_handle Device handle
 * @param reg Register address, or -1 if writing without register offset
 * @param buf Data buffer
 * @param len Number of bytes to write
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_i2c_write_reg(i2c_master_dev_handle_t dev_handle, int reg, const uint8_t *buf, size_t len);

/**
 * @brief Read data bytes from a specific register on an I2C device
 * 
 * @param dev_handle Device handle
 * @param reg Register address, or -1 if reading without register offset
 * @param buf Buffer to store read data
 * @param len Number of bytes to read
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_i2c_read_reg(i2c_master_dev_handle_t dev_handle, int reg, uint8_t *buf, size_t len);

/**
 * @brief Transmit then receive data on an I2C device
 * 
 * @param dev_handle Device handle
 * @param write_buf Buffer to write
 * @param write_len Length of write buffer
 * @param read_buf Buffer to receive read data
 * @param read_len Length of read buffer
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_i2c_write_read(i2c_master_dev_handle_t dev_handle, const uint8_t *write_buf, size_t write_len, uint8_t *read_buf, size_t read_len);

#ifdef __cplusplus
}
#endif

#endif /* BSP_I2C_H */
