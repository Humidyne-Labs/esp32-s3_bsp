/**
 * @file bsp_sensor_cal.c
 * @brief Decoupled MCU-Guided Thermal & Humidity Calibration Subsystem Implementation
 *
 * @version 1.0.0
 * @attribution
 * - Architecture & Development: HUMIDYNE LABS / Humiditron
 * - AI Systems Co-Developer: Gemini (Google DeepMind)
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "esp_log.h"
#include "driver/temperature_sensor.h"
#include "bsp/bsp_sensors.h"
#include "bsp/bsp_sensor_cal.h"

static const char *TAG = "bsp_sensor_cal";

#define C_TO_F(c) (((c) * 1.8f) + 32.0f)
#define C_TO_K(c) ((c) + 273.15f)

static temperature_sensor_handle_t s_temp_sensor     = NULL;
static bool                        s_mcu_temp_inited = false;
static float                       s_die_temp_filt   = -999.0f;

#ifdef CONFIG_BSP_THERMAL_COMPENSATION_DEFAULT
static bool                        s_comp_enabled    = true;
#else
static bool                        s_comp_enabled    = false;
#endif

#ifdef CONFIG_BSP_THERMAL_DIVIDER_K_X1000
static float                       s_coupling_k      = (float)CONFIG_BSP_THERMAL_DIVIDER_K_X1000 / 1000.0f;
#else
static float                       s_coupling_k      = 0.380f;
#endif

#ifdef CONFIG_BSP_THERMAL_EMA_ALPHA_X1000
static float                       s_filter_alpha    = (float)CONFIG_BSP_THERMAL_EMA_ALPHA_X1000 / 1000.0f;
#else
static float                       s_filter_alpha    = 0.050f;
#endif

static float calc_dew_point(float temp_c, float rh)
{
    const float m  = 17.62f;
    const float tn = 243.12f;

    if (rh < 0.01f)  rh = 0.01f;
    if (rh > 100.0f) rh = 100.0f;

    float gamma = logf(rh / 100.0f) + ((m * temp_c) / (tn + temp_c));
    return (tn * gamma) / (m - gamma);
}

static float calc_absolute_humidity(float temp_c, float rh)
{
    const float m  = 17.62f;
    const float tn = 243.12f;
    const float a  = 6.112f;

    if (rh < 0.0f)   rh = 0.0f;
    if (rh > 100.0f) rh = 100.0f;

    float vapor_pressure = (rh / 100.0f) * a * expf((m * temp_c) / (tn + temp_c));
    return 216.7f * (vapor_pressure / (273.15f + temp_c));
}

esp_err_t bsp_mcu_temp_read(float *out_die_temp)
{
    if (out_die_temp == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!s_mcu_temp_inited || s_temp_sensor == NULL) {
        temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 80);
        esp_err_t ret = temperature_sensor_install(&temp_sensor_config, &s_temp_sensor);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to install ESP32-S3 internal TSENS driver: %s", esp_err_to_name(ret));
            return ret;
        }
        ret = temperature_sensor_enable(s_temp_sensor);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to enable ESP32-S3 internal TSENS driver: %s", esp_err_to_name(ret));
            return ret;
        }
        s_mcu_temp_inited = true;
        ESP_LOGI(TAG, "ESP32-S3 internal temperature sensor (TSENS) enabled");
    }

    return temperature_sensor_get_celsius(s_temp_sensor, out_die_temp);
}

esp_err_t bsp_sensor_cal_init(void)
{
    ESP_LOGI(TAG, "Initializing SHTC3 Thermal Calibration Subsystem (K=%.3f, Alpha=%.3f, State=%s)",
             s_coupling_k, s_filter_alpha, s_comp_enabled ? "ENABLED" : "BYPASSED");

    esp_err_t ret = bsp_shtc3_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize underlying SHTC3 hardware: %s", esp_err_to_name(ret));
        return ret;
    }

    float die_temp = 0.0f;
    ret = bsp_mcu_temp_read(&die_temp);
    if (ret == ESP_OK) {
        s_die_temp_filt = die_temp;
        ESP_LOGI(TAG, "Initial ESP32-S3 junction die temperature: %.2f °C", die_temp);
    } else {
        ESP_LOGW(TAG, "Failed initial MCU die temperature read: %s", esp_err_to_name(ret));
    }

    return ESP_OK;
}

esp_err_t bsp_sensor_cal_read(bsp_sensor_cal_data_t *out_data)
{
    if (out_data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    memset(out_data, 0, sizeof(bsp_sensor_cal_data_t));

    // 1. Read raw telemetry directly from Sensirion SHTC3 driver
    bsp_shtc3_data_t raw_shtc;
    esp_err_t ret = bsp_shtc3_read(&raw_shtc);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read raw SHTC3 sensor telemetry: %s", esp_err_to_name(ret));
        return ret;
    }

    out_data->raw_temperature_c    = raw_shtc.temperature_c;
    out_data->raw_humidity_percent = raw_shtc.humidity_percent;
    out_data->valid                = raw_shtc.valid;

    // 2. Query raw MCU die temperature and update EMA low-pass filter
    float raw_die_temp = 0.0f;
    if (bsp_mcu_temp_read(&raw_die_temp) == ESP_OK) {
        if (s_die_temp_filt < -100.0f) {
            s_die_temp_filt = raw_die_temp;
        } else {
            s_die_temp_filt = (s_filter_alpha * raw_die_temp) + ((1.0f - s_filter_alpha) * s_die_temp_filt);
        }
    }
    out_data->die_temp_c = s_die_temp_filt;

    // 3. Perform Thermal Compensation or Bypass
    if (s_comp_enabled && out_data->die_temp_c > -100.0f) {
        // Calculate MCU to SHTC3 thermal gradient
        float delta_die = out_data->die_temp_c - raw_shtc.temperature_c;
        if (delta_die < 0.0f) delta_die = 0.0f;

        // Two-node PCB thermal divider offset
        float offset = s_coupling_k * delta_die;
        float comp_t = raw_shtc.temperature_c - offset;

        // Magnus-Tetens vapor pressure relative humidity equalization
        const float m  = 17.62f;
        const float tn = 243.12f;

        float rh_raw_clamped = fmaxf(raw_shtc.humidity_percent, 0.01f);
        if (rh_raw_clamped > 100.0f) rh_raw_clamped = 100.0f;

        float gamma = logf(rh_raw_clamped / 100.0f) + ((m * raw_shtc.temperature_c) / (tn + raw_shtc.temperature_c));
        float vapor_pressure = 6.112f * expf(gamma);
        float sat_pressure_amb = 6.112f * expf((m * comp_t) / (tn + comp_t));

        float comp_rh = (vapor_pressure / sat_pressure_amb) * 100.0f;
        if (comp_rh > 100.0f) comp_rh = 100.0f;
        if (comp_rh < 0.0f)   comp_rh = 0.0f;

        out_data->temperature_c    = comp_t;
        out_data->humidity_percent = comp_rh;
        out_data->thermal_offset_c = offset;
        out_data->compensated      = true;
    } else {
        // Bypass / Deactivated: return 100% raw uncalibrated SHTC3 telemetry
        out_data->temperature_c    = raw_shtc.temperature_c;
        out_data->humidity_percent = raw_shtc.humidity_percent;
        out_data->thermal_offset_c = 0.0f;
        out_data->compensated      = false;
    }

    // 4. Compute Derived Metrics
    out_data->temperature_f       = C_TO_F(out_data->temperature_c);
    out_data->temperature_k       = C_TO_K(out_data->temperature_c);
    out_data->dew_point_c         = calc_dew_point(out_data->temperature_c, out_data->humidity_percent);
    out_data->dew_point_f         = C_TO_F(out_data->dew_point_c);
    out_data->dew_point_k         = C_TO_K(out_data->dew_point_c);
    out_data->absolute_humidity_g = calc_absolute_humidity(out_data->temperature_c, out_data->humidity_percent);

    return ESP_OK;
}

void bsp_sensor_cal_set_k(float k)
{
    if (k < 0.0f) k = 0.0f;
    s_coupling_k = k;
    ESP_LOGI(TAG, "Thermal coupling constant K set to %.3f", s_coupling_k);
}

float bsp_sensor_cal_get_k(void)
{
    return s_coupling_k;
}

void bsp_sensor_cal_set_alpha(float alpha)
{
    if (alpha < 0.001f) alpha = 0.001f;
    if (alpha > 1.0f)   alpha = 1.0f;
    s_filter_alpha = alpha;
    ESP_LOGI(TAG, "MCU die temp EMA filter alpha set to %.3f", s_filter_alpha);
}

float bsp_sensor_cal_get_alpha(void)
{
    return s_filter_alpha;
}

void bsp_sensor_cal_enable(bool enable)
{
    s_comp_enabled = enable;
    ESP_LOGI(TAG, "Dynamic MCU thermal compensation %s", s_comp_enabled ? "ENABLED" : "DISABLED (Bypassed)");
}

bool bsp_sensor_cal_is_enabled(void)
{
    return s_comp_enabled;
}
