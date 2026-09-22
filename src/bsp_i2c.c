/**
 * @file bsp_i2c.c
 * @brief i2c lib
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics (https://www.waveshare.com)
 * - Microcontroller: Espressif Systems ESP32-S3 (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp/bsp_i2c.h"

static const char *TAG = "bsp_i2c";

static i2c_master_bus_handle_t s_i2c_bus_handle = NULL;
// Milliseconds timeout directly passed to ESP-IDF v5 driver
static const int I2C_TIMEOUT_MS = 50;

esp_err_t bsp_i2c_init(void)
{
    if (s_i2c_bus_handle != NULL) {
        return ESP_OK;
    }

    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = (gpio_num_t)BSP_GPIO_I2C_SCL,
        .sda_io_num = (gpio_num_t)BSP_GPIO_I2C_SDA,
        .glitch_ignore_cnt = 7,
        .flags = {
            .enable_internal_pullup = true,
        },
    };

    esp_err_t ret = i2c_new_master_bus(&bus_config, &s_i2c_bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init I2C master bus: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Shared I2C master bus initialized (SCL: %d, SDA: %d)", BSP_GPIO_I2C_SCL, BSP_GPIO_I2C_SDA);
    return ESP_OK;
}

esp_err_t bsp_i2c_deinit(void)
{
    if (s_i2c_bus_handle == NULL) {
        return ESP_OK;
    }

    esp_err_t ret = i2c_del_master_bus(s_i2c_bus_handle);
    if (ret == ESP_OK) {
        s_i2c_bus_handle = NULL;
        ESP_LOGI(TAG, "Shared I2C master bus deinitialized");
    }
    return ret;
}

i2c_master_bus_handle_t bsp_i2c_get_handle(void)
{
    return s_i2c_bus_handle;
}

esp_err_t bsp_i2c_add_device(const i2c_device_config_t *dev_cfg, i2c_master_dev_handle_t *dev_handle)
{
    if (s_i2c_bus_handle == NULL) {
        esp_err_t ret = bsp_i2c_init();
        if (ret != ESP_OK) return ret;
    }
    return i2c_master_bus_add_device(s_i2c_bus_handle, dev_cfg, dev_handle);
}

esp_err_t bsp_i2c_write_reg(i2c_master_dev_handle_t dev_handle, int reg, const uint8_t *buf, size_t len)
{
    if (dev_handle == NULL) return ESP_ERR_INVALID_ARG;

    if (reg < 0) {
        return i2c_master_transmit(dev_handle, buf, len, I2C_TIMEOUT_MS);
    } else {
        uint8_t *temp = (uint8_t *)malloc(len + 1);
        if (temp == NULL) return ESP_ERR_NO_MEM;
        temp[0] = (uint8_t)reg;
        if (len > 0 && buf != NULL) {
            memcpy(&temp[1], buf, len);
        }
        esp_err_t ret = i2c_master_transmit(dev_handle, temp, len + 1, I2C_TIMEOUT_MS);
        free(temp);
        return ret;
    }
}

esp_err_t bsp_i2c_read_reg(i2c_master_dev_handle_t dev_handle, int reg, uint8_t *buf, size_t len)
{
    if (dev_handle == NULL || buf == NULL) return ESP_ERR_INVALID_ARG;

    if (reg < 0) {
        return i2c_master_receive(dev_handle, buf, len, I2C_TIMEOUT_MS);
    } else {
        uint8_t reg_addr = (uint8_t)reg;
        return i2c_master_transmit_receive(dev_handle, &reg_addr, 1, buf, len, I2C_TIMEOUT_MS);
    }
}

esp_err_t bsp_i2c_write_read(i2c_master_dev_handle_t dev_handle, const uint8_t *write_buf, size_t write_len, uint8_t *read_buf, size_t read_len)
{
    if (dev_handle == NULL) return ESP_ERR_INVALID_ARG;
    return i2c_master_transmit_receive(dev_handle, write_buf, write_len, read_buf, read_len, I2C_TIMEOUT_MS);
}