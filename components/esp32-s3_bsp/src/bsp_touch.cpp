/**
 * @file bsp_touch.cpp
 * @brief FocalTech FT6336 Capacitive Touch Controller Driver Implementation
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics (https://www.waveshare.com)
 * - Microcontroller: Espressif Systems ESP32-S3 (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "bsp/pinout.h"
#include "bsp/bsp_i2c.h"
#include "bsp/bsp_display.h"
#include "bsp/bsp_touch.h"

static const char *TAG = "bsp_touch";
static bool s_touch_inited = false;

void bsp_touch_reset(void)
{
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << BSP_PIN_TOUCH_RST);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    gpio_set_level(BSP_PIN_TOUCH_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(BSP_PIN_TOUCH_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(BSP_PIN_TOUCH_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(50));
}

esp_err_t bsp_touch_init(void)
{
    if (s_touch_inited) return ESP_OK;

    // 1. Perform Hardware Reset
    bsp_touch_reset();

    // 2. Configure INT Pin (GPIO 21)
    gpio_config_t int_cfg = {};
    int_cfg.pin_bit_mask = (1ULL << BSP_PIN_TOUCH_INT);
    int_cfg.mode = GPIO_MODE_INPUT;
    int_cfg.pull_up_en = GPIO_PULLUP_ENABLE;
    int_cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    int_cfg.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&int_cfg);

    // 3. Probe FT6336 on I2C address 0x38 (Chip ID Register 0xA8)
    uint8_t chip_id = 0;
    esp_err_t ret = bsp_i2c_read_reg(BSP_I2C_ADDR_FT6336, 0xA8, &chip_id, 1);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "FT6336 touch controller detected (Chip ID: 0x%02X)", chip_id);
    } else {
        ESP_LOGW(TAG, "FT6336 touch probe returned: %s", esp_err_to_name(ret));
    }

    s_touch_inited = true;
    return ESP_OK;
}

bool bsp_touch_read(uint16_t *x, uint16_t *y)
{
    if (x == NULL || y == NULL) {
        return false;
    }

    // Register 0x02: TD_STATUS (Number of touch points)
    uint8_t touch_count = 0;
    esp_err_t ret = bsp_i2c_read_reg(BSP_I2C_ADDR_FT6336, 0x02, &touch_count, 1);
    touch_count &= 0x0F;
    if (ret != ESP_OK || touch_count == 0) {
        return false;
    }

    // Register 0x03 to 0x06: Touch 1 coordinates [XH, XL, YH, YL]
    uint8_t buf[4] = {0};
    ret = bsp_i2c_read_reg(BSP_I2C_ADDR_FT6336, 0x03, buf, 4);
    if (ret != ESP_OK) {
        return false;
    }

    uint16_t touch_x = (((uint16_t)buf[0] & 0x0F) << 8) | (uint16_t)buf[1];
    uint16_t touch_y = (((uint16_t)buf[2] & 0x0F) << 8) | (uint16_t)buf[3];

    // Bound checking against 200x200 display resolution
    if (touch_x >= BSP_DISPLAY_WIDTH)  touch_x = BSP_DISPLAY_WIDTH - 1;
    if (touch_y >= BSP_DISPLAY_HEIGHT) touch_y = BSP_DISPLAY_HEIGHT - 1;

    *x = touch_x;
    *y = touch_y;
    return true;
}

esp_err_t bsp_touch_sleep(void)
{
    uint8_t sleep_cmd = 0x03; // Power mode: Hibernate
    return bsp_i2c_write_reg(BSP_I2C_ADDR_FT6336, 0xA5, &sleep_cmd, 1);
}
