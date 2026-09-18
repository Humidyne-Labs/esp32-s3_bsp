#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "bsp/bsp_i2c.h"
#include "bsp/bsp_display.h"
#include "bsp/bsp_touch.h"

static const char *TAG = "bsp_touch";

static i2c_master_dev_handle_t s_touch_dev_handle = NULL;

esp_err_t bsp_touch_init(void)
{
    /* Perform HW reset */
    bsp_touch_reset();

    /* Add FT6336 to shared I2C master bus */
    i2c_device_config_t dev_cfg = {};
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address = BSP_TOUCH_I2C_ADDR;
    dev_cfg.scl_speed_hz = 400000;

    esp_err_t ret = bsp_i2c_add_device(&dev_cfg, &s_touch_dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add FT6336 device to I2C bus: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "FT6336 touch controller initialized");
    return ESP_OK;
}

void bsp_touch_reset(void)
{
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << BSP_GPIO_TOUCH_RST);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    gpio_set_level((gpio_num_t)BSP_GPIO_TOUCH_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level((gpio_num_t)BSP_GPIO_TOUCH_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level((gpio_num_t)BSP_GPIO_TOUCH_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
}

bool bsp_touch_read(uint16_t *x, uint16_t *y)
{
    if (s_touch_dev_handle == NULL || x == NULL || y == NULL) {
        return false;
    }

    uint8_t touch_count = 0;
    esp_err_t ret = bsp_i2c_read_reg(s_touch_dev_handle, 0x02, &touch_count, 1);
    if (ret != ESP_OK || touch_count == 0) {
        return false;
    }

    uint8_t buf[4] = {0};
    ret = bsp_i2c_read_reg(s_touch_dev_handle, 0x03, buf, 4);
    if (ret != ESP_OK) {
        return false;
    }

    uint16_t touch_x = (((uint16_t)buf[0] & 0x0F) << 8) | (uint16_t)buf[1];
    uint16_t touch_y = (((uint16_t)buf[2] & 0x0F) << 8) | (uint16_t)buf[3];

    /* Bound checking */
    if (touch_x >= BSP_DISPLAY_WIDTH) touch_x = BSP_DISPLAY_WIDTH - 1;
    if (touch_y >= BSP_DISPLAY_HEIGHT) touch_y = BSP_DISPLAY_HEIGHT - 1;

    *x = touch_x;
    *y = touch_y;
    return true;
}
