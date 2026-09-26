/**
 * @file bsp_i2c.c
 * @brief Thread-safe I2C driver implementation for ESP-IDF v5/v6 i2c_master
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics (https://www.waveshare.com)
 * - Microcontroller: Espressif Systems ESP32-S3 (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/i2c_master.h"
#include "bsp/bsp_i2c.h"
#include "bsp/pinout.h"

static const char *TAG = "bsp_i2c";

static i2c_master_bus_handle_t s_i2c_bus_handle = NULL;
static SemaphoreHandle_t       s_i2c_mutex      = NULL;
static const int               I2C_TIMEOUT_MS   = 100;

#define MAX_CACHED_DEVICES 8
typedef struct {
    uint8_t                 addr;
    i2c_master_dev_handle_t handle;
} cached_i2c_dev_t;

static cached_i2c_dev_t s_dev_cache[MAX_CACHED_DEVICES];
static size_t           s_dev_cache_count = 0;

static esp_err_t get_or_create_dev_handle(uint8_t addr, i2c_master_dev_handle_t *out_handle)
{
    if (s_i2c_bus_handle == NULL) {
        esp_err_t ret = bsp_i2c_init();
        if (ret != ESP_OK) return ret;
    }

    for (size_t i = 0; i < s_dev_cache_count; i++) {
        if (s_dev_cache[i].addr == addr) {
            *out_handle = s_dev_cache[i].handle;
            return ESP_OK;
        }
    }

    if (s_dev_cache_count >= MAX_CACHED_DEVICES) {
        ESP_LOGE(TAG, "I2C device cache full");
        return ESP_ERR_NO_MEM;
    }

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = addr,
        .scl_speed_hz    = 400000,
    };

    i2c_master_dev_handle_t new_handle = NULL;
    esp_err_t ret = i2c_master_bus_add_device(s_i2c_bus_handle, &dev_cfg, &new_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add I2C device 0x%02X: %s", addr, esp_err_to_name(ret));
        return ret;
    }

    s_dev_cache[s_dev_cache_count].addr   = addr;
    s_dev_cache[s_dev_cache_count].handle = new_handle;
    s_dev_cache_count++;

    *out_handle = new_handle;
    return ESP_OK;
}

esp_err_t bsp_i2c_init(void)
{
    if (s_i2c_bus_handle != NULL) {
        return ESP_OK;
    }

    if (s_i2c_mutex == NULL) {
        s_i2c_mutex = xSemaphoreCreateRecursiveMutex();
        if (s_i2c_mutex == NULL) {
            ESP_LOGE(TAG, "Failed to create I2C mutex");
            return ESP_ERR_NO_MEM;
        }
    }

    i2c_master_bus_config_t bus_config = {
        .i2c_port          = I2C_NUM_0,
        .sda_io_num        = (gpio_num_t)BSP_PIN_I2C_SDA,
        .scl_io_num        = (gpio_num_t)BSP_PIN_I2C_SCL,
        .clk_source        = I2C_CLK_SRC_DEFAULT,
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

    s_dev_cache_count = 0;
    ESP_LOGI(TAG, "Shared I2C master bus initialized (SDA: %d, SCL: %d @ 400kHz)",
             BSP_PIN_I2C_SDA, BSP_PIN_I2C_SCL);
    return ESP_OK;
}

esp_err_t bsp_i2c_deinit(void)
{
    if (s_i2c_bus_handle == NULL) {
        return ESP_OK;
    }

    if (s_i2c_mutex) {
        xSemaphoreTakeRecursive(s_i2c_mutex, portMAX_DELAY);
    }

    for (size_t i = 0; i < s_dev_cache_count; i++) {
        if (s_dev_cache[i].handle) {
            i2c_master_bus_rm_device(s_dev_cache[i].handle);
        }
    }
    s_dev_cache_count = 0;

    esp_err_t ret = i2c_del_master_bus(s_i2c_bus_handle);
    s_i2c_bus_handle = NULL;

    if (s_i2c_mutex) {
        xSemaphoreGiveRecursive(s_i2c_mutex);
    }

    ESP_LOGI(TAG, "Shared I2C master bus deinitialized");
    return ret;
}

i2c_master_bus_handle_t bsp_i2c_get_handle(void)
{
    if (s_i2c_bus_handle == NULL) {
        bsp_i2c_init();
    }
    return s_i2c_bus_handle;
}

esp_err_t bsp_i2c_add_device(const i2c_device_config_t *dev_cfg, i2c_master_dev_handle_t *dev_handle)
{
    if (dev_cfg == NULL || dev_handle == NULL) return ESP_ERR_INVALID_ARG;
    if (s_i2c_bus_handle == NULL) {
        esp_err_t ret = bsp_i2c_init();
        if (ret != ESP_OK) return ret;
    }
    return i2c_master_bus_add_device(s_i2c_bus_handle, dev_cfg, dev_handle);
}

esp_err_t bsp_i2c_write(uint8_t addr, const uint8_t *data, size_t len)
{
    if (data == NULL || len == 0) return ESP_ERR_INVALID_ARG;
    if (s_i2c_bus_handle == NULL) {
        esp_err_t err = bsp_i2c_init();
        if (err != ESP_OK) return err;
    }

    if (s_i2c_mutex) xSemaphoreTakeRecursive(s_i2c_mutex, portMAX_DELAY);

    i2c_master_dev_handle_t dev = NULL;
    esp_err_t ret = get_or_create_dev_handle(addr, &dev);
    if (ret == ESP_OK) {
        ret = i2c_master_transmit(dev, data, len, I2C_TIMEOUT_MS);
    }

    if (s_i2c_mutex) xSemaphoreGiveRecursive(s_i2c_mutex);
    return ret;
}

esp_err_t bsp_i2c_read(uint8_t addr, uint8_t *data, size_t len)
{
    if (data == NULL || len == 0) return ESP_ERR_INVALID_ARG;
    if (s_i2c_bus_handle == NULL) {
        esp_err_t err = bsp_i2c_init();
        if (err != ESP_OK) return err;
    }

    if (s_i2c_mutex) xSemaphoreTakeRecursive(s_i2c_mutex, portMAX_DELAY);

    i2c_master_dev_handle_t dev = NULL;
    esp_err_t ret = get_or_create_dev_handle(addr, &dev);
    if (ret == ESP_OK) {
        ret = i2c_master_receive(dev, data, len, I2C_TIMEOUT_MS);
    }

    if (s_i2c_mutex) xSemaphoreGiveRecursive(s_i2c_mutex);
    return ret;
}

esp_err_t bsp_i2c_write_reg(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len)
{
    if (s_i2c_bus_handle == NULL) {
        esp_err_t err = bsp_i2c_init();
        if (err != ESP_OK) return err;
    }

    uint8_t *buf = (uint8_t *)malloc(len + 1);
    if (!buf) return ESP_ERR_NO_MEM;
    buf[0] = reg;
    if (data && len > 0) {
        memcpy(&buf[1], data, len);
    }

    esp_err_t ret = bsp_i2c_write(addr, buf, len + 1);
    free(buf);
    return ret;
}

esp_err_t bsp_i2c_read_reg(uint8_t addr, uint8_t reg, uint8_t *data, size_t len)
{
    if (data == NULL || len == 0) return ESP_ERR_INVALID_ARG;
    if (s_i2c_bus_handle == NULL) {
        esp_err_t err = bsp_i2c_init();
        if (err != ESP_OK) return err;
    }

    if (s_i2c_mutex) xSemaphoreTakeRecursive(s_i2c_mutex, portMAX_DELAY);

    i2c_master_dev_handle_t dev = NULL;
    esp_err_t ret = get_or_create_dev_handle(addr, &dev);
    if (ret == ESP_OK) {
        ret = i2c_master_transmit_receive(dev, &reg, 1, data, len, I2C_TIMEOUT_MS);
    }

    if (s_i2c_mutex) xSemaphoreGiveRecursive(s_i2c_mutex);
    return ret;
}