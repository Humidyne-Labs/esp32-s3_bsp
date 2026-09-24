/**
 * @file bsp_power.c
 * @brief power lib
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics (https://www.waveshare.com)
 * - Microcontroller: Espressif Systems ESP32-S3 (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdbool.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "bsp/bsp_power.h"

static const char *TAG = "bsp_power";

static adc_oneshot_unit_handle_t s_adc_handle      = NULL;
static adc_cali_handle_t         s_adc_cali_handle = NULL;
static adc_channel_t             s_battery_channel;

#define BATTERY_DIVIDER_RATIO 2.0f
#define BATTERY_EMPTY_MV      3000
#define BATTERY_FULL_MV       4100

esp_err_t bsp_power_init(void)
{
    /* Keep board powered on by default */
    //bsp_power_hold();
    //bsp_led_set(false);

    adc_unit_t battery_unit;
    esp_err_t ret = adc_oneshot_io_to_channel(BSP_GPIO_BAT_ADC, &battery_unit, &s_battery_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Battery GPIO %d is not ADC-capable: %s", BSP_GPIO_BAT_ADC, esp_err_to_name(ret));
        return ret;
    }

    /* Initialize the ADC unit and channel selected by the battery GPIO. */
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = battery_unit,
    };
    ret = adc_oneshot_new_unit(&init_config, &s_adc_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init ADC1 unit: %s", esp_err_to_name(ret));
        return ret;
    }

    adc_oneshot_chan_cfg_t chan_config = {
        .atten    = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ret = adc_oneshot_config_channel(s_adc_handle, s_battery_channel, &chan_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to config battery ADC channel: %s", esp_err_to_name(ret));
        return ret;
    }

#if CONFIG_IDF_TARGET_ESP32S3
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id  = battery_unit,
        .chan     = s_battery_channel,
        .atten    = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ret = adc_cali_create_scheme_curve_fitting(&cali_config, &s_adc_cali_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "ADC calibration unavailable: %s", esp_err_to_name(ret));
        s_adc_cali_handle = NULL;
    }
#endif

    ESP_LOGI(TAG, "Power management and Battery ADC initialized");
    return ESP_OK;
}

esp_err_t bsp_battery_get_voltage(uint32_t *voltage_mv, int *raw_adc)
{
    if (s_adc_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    int raw = 0;
    esp_err_t ret = adc_oneshot_read(s_adc_handle, s_battery_channel, &raw);
    if (ret != ESP_OK) {
        return ret;
    }

    if (raw_adc != NULL) *raw_adc = raw;

    if (voltage_mv != NULL) {
        int voltage = 0;
        if (s_adc_cali_handle != NULL &&
            adc_cali_raw_to_voltage(s_adc_cali_handle, raw, &voltage) == ESP_OK) {
            *voltage_mv = (uint32_t)((float)voltage * BATTERY_DIVIDER_RATIO);
        } else {
            *voltage_mv = (uint32_t)((float)raw * 3300.0f * BATTERY_DIVIDER_RATIO / 4095.0f);
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

    if (v_mv >= BATTERY_FULL_MV)  return 100;
    if (v_mv <= BATTERY_EMPTY_MV) return 0;
    return (uint8_t)(((v_mv - BATTERY_EMPTY_MV) * 100) /
                     (BATTERY_FULL_MV - BATTERY_EMPTY_MV));
}
