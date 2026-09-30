/**
 * @file main.c
 * @brief Unit example for shared I2C master bus driver
 */

#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp/bsp_i2c.h"

static const char *TAG = "example_i2c";

typedef struct {
    uint8_t addr;
    const char *name;
} i2c_target_device_t;

/* Known slave device addresses connected to shared I2C bus */
static const i2c_target_device_t known_devices[] = {
    { 0x18, "ES8311 Audio Codec" },
    { 0x38, "FT6336 Touch Controller" },
    { 0x51, "PCF85063A RTC" },
    { 0x70, "SHTC3 Temp/RH Sensor" }
};

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing shared I2C master bus...");
    ESP_ERROR_CHECK(bsp_i2c_init());

    /* Verify bus handle initialization */
    i2c_master_bus_handle_t bus_handle = bsp_i2c_get_handle();
    if (bus_handle == NULL) {
        ESP_LOGE(TAG, "Failed to retrieve valid I2C master bus handle");
        return;
    }
    ESP_LOGI(TAG, "I2C master bus handle retrieved successfully");

    /* Probe specific onboard slave devices */
    ESP_LOGI(TAG, "Probing target I2C devices...");
    size_t dev_count = sizeof(known_devices) / sizeof(known_devices[0]);

    for (size_t i = 0; i < dev_count; i++) {
        uint8_t addr = known_devices[i].addr;
        const char *name = known_devices[i].name;

        esp_err_t ret = bsp_i2c_probe(addr);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "  -> Found device at 0x%02X (%s)", addr, name);
        } else {
            ESP_LOGW(TAG, "  -> No response at 0x%02X (%s) [%s]", addr, name, esp_err_to_name(ret));
        }
    }

    /* Perform a test register read from PCF85063A RTC (0x51) if available */
    uint8_t rtc_addr = 0x51;
    esp_err_t ret = bsp_i2c_probe(rtc_addr);
    if (ret == ESP_OK) {
        uint8_t reg_val = 0;
        /* Register 0x00: Control_1 */
        ret = bsp_i2c_read_reg(rtc_addr, 0x00, &reg_val, 1);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "PCF85063A Control_1 reg (0x00): 0x%02X", reg_val);
        } else {
            ESP_LOGE(TAG, "Failed to read register from PCF85063A: %s", esp_err_to_name(ret));
        }
    }

    ESP_LOGI(TAG, "I2C peripheral verification complete.");
}