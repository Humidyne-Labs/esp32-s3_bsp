#include <stdio.h>
#include <stdbool.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "bsp/bsp_power.h"

static const char *TAG = "bsp_power";

static adc_oneshot_unit_handle_t s_adc_handle = NULL;
static adc_cali_handle_t s_adc_cali_handle = NULL;
static bool s_led_state = false;

esp_err_t bsp_power_init(void)
{
    /* Configure PWR_KEY and USER_LED */
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << BSP_GPIO_PWR_KEY) | (1ULL << BSP_GPIO_USER_LED),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure power GPIOs: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Keep board powered on by default */
    bsp_power_hold();
    bsp_led_set(false);

    /* Initialize ADC1 Channel 0 for Battery monitoring (GPIO1) */
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    ret = adc_oneshot_new_unit(&init_config, &s_adc_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init ADC1 unit: %s", esp_err_to_name(ret));
        return ret;
    }

    adc_oneshot_chan_cfg_t chan_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ret = adc_oneshot_config_channel(s_adc_handle, ADC_CHANNEL_0, &chan_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to config ADC1 channel 0: %s", esp_err_to_name(ret));
        return ret;
    }

#if CONFIG_IDF_TARGET_ESP32S3
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .chan = ADC_CHANNEL_0,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    adc_cali_create_scheme_curve_fitting(&cali_config, &s_adc_cali_handle);
#endif

    ESP_LOGI(TAG, "Power management and Battery ADC initialized");
    return ESP_OK;
}

void bsp_power_hold(void)
{
    gpio_set_level((gpio_num_t)BSP_GPIO_PWR_KEY, 1);
}

void bsp_power_off(void)
{
    gpio_set_level((gpio_num_t)BSP_GPIO_PWR_KEY, 0);
}

void bsp_led_set(bool enable)
{
    s_led_state = enable;
    gpio_set_level((gpio_num_t)BSP_GPIO_USER_LED, enable ? 1 : 0);
}

void bsp_led_toggle(void)
{
    bsp_led_set(!s_led_state);
}

esp_err_t bsp_battery_get_voltage(uint32_t *voltage_mv, int *raw_adc)
{
    if (s_adc_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    int raw = 0;
    esp_err_t ret = adc_oneshot_read(s_adc_handle, ADC_CHANNEL_0, &raw);
    if (ret != ESP_OK) {
        return ret;
    }

    if (raw_adc != NULL) {
        *raw_adc = raw;
    }

    if (voltage_mv != NULL) {
        int voltage = 0;
        if (s_adc_cali_handle != NULL) {
            adc_cali_raw_to_voltage(s_adc_cali_handle, raw, &voltage);
            /* Battery voltage divider: 2:1 ratio */
            *voltage_mv = (uint32_t)voltage * 2;
        } else {
            /* Fallback estimation if calibration unavailable */
            *voltage_mv = ((uint32_t)raw * 3300 * 2) / 4095;
        }
    }

    return ESP_OK;
}

uint8_t bsp_battery_get_percentage(void)
{
    uint32_t v_mv = 0;
    if (bsp_battery_get_voltage(&v_mv, NULL) != ESP_OK) {
        return 0;
    }

    /* Li-Po / Li-Ion voltage range: 3300mV (0%) to 4200mV (100%) */
    if (v_mv >= 4200) return 100;
    if (v_mv <= 3300) return 0;
    return (uint8_t)(((v_mv - 3300) * 100) / (4200 - 3300));
}
