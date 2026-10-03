/**
 * @file main.c
 * @brief Minimalist Hardware Verification Example for Waveshare ESP32-S3-ePaper-1.54 V2
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "bsp/pinout.h"

static const char *TAG = "BSP_EXAMPLE";

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing Waveshare ESP32-S3-ePaper-1.54 V2 Hardware...");

    // 1. Power Hold Latch: Must be driven HIGH to maintain power rail
    ESP_ERROR_CHECK(gpio_reset_pin(BSP_PIN_POWER_HOLD));
    ESP_ERROR_CHECK(gpio_set_direction(BSP_PIN_POWER_HOLD, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_level(BSP_PIN_POWER_HOLD, 1));
    ESP_LOGI(TAG, "Power Hold Latch (GPIO %d) set HIGH.", BSP_PIN_POWER_HOLD);

    // 2. Status LED: Configure as output (Active Low: 0=ON, 1=OFF)
    ESP_ERROR_CHECK(gpio_reset_pin(BSP_PIN_LED_STATUS));
    ESP_ERROR_CHECK(gpio_set_direction(BSP_PIN_LED_STATUS, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_level(BSP_PIN_LED_STATUS, 1));

    // 3. Tactile User Buttons: Configure with pull-ups (Active Low)
    ESP_ERROR_CHECK(gpio_reset_pin(BSP_PIN_BUTTON_BOOT));
    ESP_ERROR_CHECK(gpio_set_direction(BSP_PIN_BUTTON_BOOT, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_pull_mode(BSP_PIN_BUTTON_BOOT, GPIO_PULLUP_ONLY));

    ESP_ERROR_CHECK(gpio_reset_pin(BSP_PIN_BUTTON_POWER));
    ESP_ERROR_CHECK(gpio_set_direction(BSP_PIN_BUTTON_POWER, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_pull_mode(BSP_PIN_BUTTON_POWER, GPIO_PULLUP_ONLY));

    // 4. Battery ADC: Setup ADC1
    adc_oneshot_unit_handle_t adc_handle;
    adc_oneshot_unit_init_cfg_t init_cfg = { 
        .unit_id = ADC_UNIT_1 
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &adc_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten    = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_3, &chan_cfg));

    // 5. Hardware Verification Loop
    for (int i = 0; i < 5; i++) {
        // Blink Status LED
        ESP_ERROR_CHECK(gpio_set_level(BSP_PIN_LED_STATUS, 0)); // ON
        vTaskDelay(pdMS_TO_TICKS(200));
        ESP_ERROR_CHECK(gpio_set_level(BSP_PIN_LED_STATUS, 1)); // OFF
        vTaskDelay(pdMS_TO_TICKS(200));

        // Read Battery Sensing ADC
        int raw_adc = 0;
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_CHANNEL_3, &raw_adc));

        int boot_key = gpio_get_level(BSP_PIN_BUTTON_BOOT);
        int power_key = gpio_get_level(BSP_PIN_BUTTON_POWER);

        // Voltage calculation: 1:2 divider, 12-bit ADC, ~3.3V range
        float voltage = ((float)raw_adc / 4095.0f) * 3.3f * 2.0f;
        ESP_LOGI(TAG, "Battery ADC Raw=%d, Estimated Voltage=%.2fV | BOOT=%d, PWR=%d",
                 raw_adc, (double)voltage, boot_key, power_key);
    }

    ESP_LOGI(TAG, "Hardware verification complete.");

    // Cleanup ADC resources
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc_handle));
}