/**
 * @file bsp_power.c
 * @brief Power Control Latch, Status LED, and Battery Voltage ADC Monitor Implementation
 * 
 * Circuit Architecture:
 *  - Power Latch: GPIO 17 (BAT_CTRL) is driven HIGH to turn on the onboard LDO gate and hold power.
 *  - Battery Voltage Divider:
 *      VBAT ────[ R1: 100kΩ ]────┬────[ R2: 100kΩ ]──── GND
 *                                │
 *                           GPIO 4 (ADC1_CH3)
 *      V_ADC = VBAT * (R2 / (R1 + R2)) = VBAT / 2
 *      VBAT  = V_ADC * 2.0
 * 
 * @attribution
 * - Circuit Design: Waveshare Electronics
 * - BSP Unification: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "bsp/pinout.h"
#include "bsp/bsp_display.h"
#include "bsp/bsp_audio.h"
#include "bsp/bsp_wifi.h"
#include "bsp/bsp_rtc.h"
#include "bsp/bsp_power.h"
#include "bsp/bsp.h"
#include "sdkconfig.h"

static const char *TAG = "bsp_power";

// ADC Subsystem Handles (ESP32-S3 GPIO 4 = ADC1_CHANNEL_3)
#define BSP_ADC_BATTERY_CHANNEL ADC_CHANNEL_3

static adc_oneshot_unit_handle_t s_adc_handle   = NULL;
static adc_cali_handle_t         s_cali_handle  = NULL;
static bool                      s_calibrated   = false;
static bool                      s_led_state    = false;
static bool                      s_power_inited = false;

/* =========================================================================
 * Internal Calibration Helper
 * ========================================================================= */
static bool init_adc_calibration(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    esp_err_t ret   = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGD(TAG, "Attempting Curve Fitting ADC calibration");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, out_handle);
        if (ret == ESP_OK) calibrated = true;
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGD(TAG, "Attempting Line Fitting ADC calibration");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, out_handle);
        if (ret == ESP_OK) calibrated = true;
    }
#endif

    if (calibrated) {
        ESP_LOGI(TAG, "ADC Calibration curve successfully registered");
    } else {
        ESP_LOGW(TAG, "ADC Calibration scheme not supported; using raw uncalibrated estimation");
    }

    return calibrated;
}

/* =========================================================================
 * Public Power & Battery APIs
 * ========================================================================= */
esp_err_t bsp_power_hold(void)
{
    return gpio_set_level(BSP_PIN_POWER_HOLD, 1);
}

esp_err_t bsp_power_release(void)
{
    ESP_LOGI(TAG, "Releasing power hold latch (GPIO %d)", BSP_PIN_POWER_HOLD);
    return gpio_set_level(BSP_PIN_POWER_HOLD, 0);
}

esp_err_t bsp_power_init(void)
{
    if (s_power_inited) return ESP_OK;

    ESP_LOGI(TAG, "Initializing Battery ADC Monitor (GPIO %d / ADC1_CH3)", BSP_PIN_BATTERY_ADC);

    // 1. Ensure master IO configuration is applied
    bsp_init_io();

    // 2. Configure ADC1 Channel 3 (GPIO 4) for Battery Sensing
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id  = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    esp_err_t err = adc_oneshot_new_unit(&init_config, &s_adc_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create ADC oneshot unit: %s", esp_err_to_name(err));
        return err;
    }

    adc_oneshot_chan_cfg_t chan_config = {
        .atten    = ADC_ATTEN_DB_12,       // 0 - 3.1V sensing range
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    err = adc_oneshot_config_channel(s_adc_handle, BSP_ADC_BATTERY_CHANNEL, &chan_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure ADC channel: %s", esp_err_to_name(err));
        return err;
    }

    // 4. Initialize Factory Calibration Scheme
    s_calibrated   = init_adc_calibration(ADC_UNIT_1, BSP_ADC_BATTERY_CHANNEL, ADC_ATTEN_DB_12, &s_cali_handle);
    s_power_inited = true;

    return ESP_OK;
}

void bsp_led_set(bool state)
{
    s_led_state = state;
    // Open-Drain Active LOW: 0 = LED ON, 1 = LED OFF
    gpio_set_level(BSP_PIN_LED_STATUS, state ? 0 : 1);
}

void bsp_led_toggle(void)
{
    s_led_state = !s_led_state;
    // Open-Drain Active LOW: 0 = LED ON, 1 = LED OFF
    gpio_set_level(BSP_PIN_LED_STATUS, s_led_state ? 0 : 1);
}

esp_err_t bsp_battery_get_voltage(uint32_t *out_mv, uint32_t *out_raw)
{
    if (!s_power_inited || s_adc_handle == NULL) {
        esp_err_t err = bsp_power_init();
        if (err != ESP_OK) return err;
    }

    int raw_val = 0;
    // Multi-sampling average (8 samples for noise reduction)
    int samples = 8;
    int raw_sum = 0;
    for (int i = 0; i < samples; i++) {
        int r = 0;
        adc_oneshot_read(s_adc_handle, BSP_ADC_BATTERY_CHANNEL, &r);
        raw_sum += r;
    }
    raw_val = raw_sum / samples;

    if (out_raw) *out_raw = (uint32_t)raw_val;

    int voltage_mv = 0;
    if (s_calibrated && s_cali_handle) {
        adc_cali_raw_to_voltage(s_cali_handle, raw_val, &voltage_mv);
    } else {
        // Fallback linear calculation for 12-bit ADC @ 3.3V full-scale
        voltage_mv = (raw_val * 3300) / 4095;
    }

    // Multiply by 2.0 to compensate for 1:2 resistor divider (100k + 100k)
    uint32_t bat_mv = (uint32_t)(voltage_mv * 2);

    if (out_mv) *out_mv = bat_mv;
    return ESP_OK;
}

uint8_t bsp_battery_get_percentage(void)
{
    uint32_t vbat_mv = 0;
    if (bsp_battery_get_voltage(&vbat_mv, NULL) != ESP_OK) {
        return 100;
    }

    // Standard 3.7V LiPo discharge curve approximation
    // Full charge: 4200 mV (100%), Nominal: 3700 mV (~50%), Cutoff: 3300 mV (0%)
    if (vbat_mv >= 4200) return 100;
    if (vbat_mv <= 3300) return 0;

    if (vbat_mv > 3850) {
        // 3850mV - 4200mV -> 60% to 100%
        return (uint8_t)(60 + ((vbat_mv - 3850) * 40) / (4200 - 3850));
    } else if (vbat_mv > 3650) {
        // 3650mV - 3850mV -> 20% to 60%
        return (uint8_t)(20 + ((vbat_mv - 3650) * 40) / (3850 - 3650));
    } else {
        // 3300mV - 3650mV -> 0% to 20%
        return (uint8_t)(((vbat_mv - 3300) * 20) / (3650 - 3300));
    }
}

bool bsp_battery_is_low(uint8_t threshold_pct)
{
    return (bsp_battery_get_percentage() <= threshold_pct);
}

esp_err_t bsp_power_enter_deep_sleep(uint32_t duration_sec)
{
    ESP_LOGI(TAG, "Preparing deep sleep power isolation (%lu seconds)...", (unsigned long)duration_sec);

    // 1. Turn off Status LED (Active LOW: 1 = OFF)
    bsp_led_set(false);

    // 2. Gate Audio Power Amp (GPIO 42 = HIGH / 1 -> Cut NS4168 PA power)
    bsp_audio_power_enable(false);
    gpio_hold_en(BSP_PIN_PA_EN);

    // 3. Put SSD1681 e-Paper into ultra-low power deep sleep mode (<1 uA)
    bsp_display_deep_sleep();
    // Cut EPD 3.3V Power Rail (Active LOW: GPIO 6 = 1 -> Rail OFF)
    gpio_set_level(BSP_PIN_EPD_3V3_EN, 1);
    gpio_hold_en(BSP_PIN_EPD_3V3_EN);

    // 4. Put Touch Controller in reset / low-power sleep
    gpio_set_level(BSP_PIN_TOUCH_RST, 0);
    gpio_hold_en(BSP_PIN_TOUCH_RST);

    // 5. Disconnect Wi-Fi and Bluetooth Radios
    bsp_wifi_disconnect();

    // 6. Hold Battery LDO Power Latch (GPIO 17 = HIGH) across deep sleep
    gpio_set_level(BSP_PIN_POWER_HOLD, 1);
    gpio_hold_en(BSP_PIN_POWER_HOLD);
    gpio_deep_sleep_hold_en();

    // 7. Configure Wakeup Sources based on Kconfig selection:
#if defined(CONFIG_BSP_RTC_WAKEUP_EXTERNAL_PCF85063)
    if (duration_sec > 0) {
        if (duration_sec <= 255) {
            ESP_LOGI(TAG, "Arming PCF85063A external RTC countdown timer for %lu seconds", (unsigned long)duration_sec);
            bsp_rtc_set_countdown_timer((uint8_t)duration_sec);
        } else {
            // For longer durations exceeding 255s countdown timer, read current time and arm RTC alarm
            bsp_rtc_datetime_t now;
            if (bsp_rtc_get_datetime(&now) == ESP_OK) {
                // Calculate future alarm
                uint32_t total_sec = (uint32_t)now.hour * 3600 + (uint32_t)now.minute * 60 + now.second + duration_sec;
                uint32_t target_sec = total_sec % 60;
                uint32_t target_min = (total_sec / 60) % 60;
                uint32_t target_hr  = (total_sec / 3600) % 24;

                bsp_rtc_alarm_t alarm = {
                    .second  = (int8_t)target_sec,
                    .minute  = (int8_t)target_min,
                    .hour    = (int8_t)target_hr,
                    .day     = -1,
                    .weekday = -1,
                };
                ESP_LOGI(TAG, "Arming PCF85063A alarm for %02d:%02d:%02d (%lu seconds sleep)",
                         (int)target_hr, (int)target_min, (int)target_sec, (unsigned long)duration_sec);
                bsp_rtc_set_alarm(&alarm);
            } else {
                // Fallback to internal timer if RTC read failed
                ESP_LOGW(TAG, "RTC read failed; falling back to internal timer wakeup");
                esp_sleep_enable_timer_wakeup((uint64_t)duration_sec * 1000000ULL);
            }
        }
    }
#else
    if (duration_sec > 0) {
        ESP_LOGI(TAG, "Arming ESP32-S3 internal RTC timer wakeup for %lu seconds", (unsigned long)duration_sec);
        esp_sleep_enable_timer_wakeup((uint64_t)duration_sec * 1000000ULL);
    }
#endif

    // Wakeup on GPIO 0 (Tactile BOOT Key) or GPIO 5 (External RTC Interrupt)
    uint64_t ext1_pin_mask = (1ULL << BSP_PIN_BUTTON_BOOT) | (1ULL << BSP_PIN_RTC_INT);
    gpio_pullup_en((gpio_num_t)BSP_PIN_RTC_INT);
    gpio_pullup_en((gpio_num_t)BSP_PIN_BUTTON_BOOT);
    esp_sleep_enable_ext1_wakeup_io(ext1_pin_mask, ESP_EXT1_WAKEUP_ANY_LOW);

    ESP_LOGI(TAG, "Power rails isolated. Starting ESP32-S3 deep sleep now");
    vTaskDelay(pdMS_TO_TICKS(50));
    esp_deep_sleep_start();
    return ESP_OK;
}
