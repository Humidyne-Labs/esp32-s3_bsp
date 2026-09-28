/**
 * @file bsp_power.c
 * @brief Power Control Latch, Status LED, Battery ADC, and Dual Sleep Modes Implementation
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
 * - BSP Architecture: Humidyne Labs / Humiditron (2026)
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
#include "bsp/bsp_sensors.h"
#include "bsp/bsp_power.h"
#include "bsp/bsp_rtc_mem.h"
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
    bsp_led_set(!s_led_state);
}

esp_err_t bsp_battery_get_voltage(uint32_t *out_mv, uint32_t *out_raw)
{
    if (out_mv == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!s_power_inited) {
        esp_err_t err = bsp_power_init();
        if (err != ESP_OK) return err;
    }

    // Multisample ADC for noise rejection (16 samples)
    const int SAMPLES   = 16;
    int       raw_accum = 0;
    int       raw_val   = 0;

    for (int i = 0; i < SAMPLES; i++) {
        esp_err_t err = adc_oneshot_read(s_adc_handle, BSP_ADC_BATTERY_CHANNEL, &raw_val);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "ADC read failed: %s", esp_err_to_name(err));
            return err;
        }
        raw_accum += raw_val;
    }
    int raw_avg = raw_accum / SAMPLES;

    if (out_raw != NULL) {
        *out_raw = (uint32_t)raw_avg;
    }

    int voltage_mv = 0;
    if (s_calibrated && s_cali_handle != NULL) {
        adc_cali_raw_to_voltage(s_cali_handle, raw_avg, &voltage_mv);
    } else {
        // Fallback: 12-bit ADC -> 3.3V VREF
        voltage_mv = (raw_avg * 3300) / 4095;
    }

    // Compensate for 1:2 resistor divider (R1=100k, R2=100k -> 2.0x factor)
    *out_mv = (uint32_t)(voltage_mv * 2);
    return ESP_OK;
}

uint8_t bsp_battery_get_percentage(void)
{
    uint32_t vbat_mv = 0;
    esp_err_t err = bsp_battery_get_voltage(&vbat_mv, NULL);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Unable to read battery voltage; returning 100%%");
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

esp_reset_reason_t bsp_get_reset_reason(void)
{
    return esp_reset_reason();
}

esp_sleep_wakeup_cause_t bsp_get_wakeup_cause(void)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    return esp_sleep_get_wakeup_cause();
#pragma GCC diagnostic pop
}

bsp_init_mode_t bsp_get_recommended_init_mode(void)
{
    esp_reset_reason_t rst = esp_reset_reason();
    if (rst != ESP_RST_DEEPSLEEP) {
        return BSP_INIT_MODE_FULL;
    }

    bsp_rtc_state_t *rtc_st = bsp_rtc_mem_get_state();
    if (rtc_st && rtc_st->magic == BSP_RTC_MEM_MAGIC) {
        return (bsp_init_mode_t)rtc_st->next_init_mode;
    }

    return BSP_INIT_MODE_FAST;
}

esp_err_t bsp_enter_sleep(const bsp_sleep_config_t *config)
{
    bsp_sleep_config_t cfg = (config != NULL) ? *config : (bsp_sleep_config_t)BSP_SLEEP_CONFIG_DEFAULT();

    // Store sleep state in RTC Slow Memory
    bsp_rtc_state_t *rtc_st = bsp_rtc_mem_get_state();
    if (rtc_st && rtc_st->magic == BSP_RTC_MEM_MAGIC) {
        rtc_st->last_sleep_mode         = (uint8_t)cfg.mode;
        rtc_st->next_init_mode          = (uint8_t)cfg.next_init_mode;
        rtc_st->last_sleep_duration_sec = cfg.duration_sec;
        if (cfg.mode == BSP_SLEEP_MODE_DEEP) {
            rtc_st->deep_sleep_count++;
        } else {
            rtc_st->light_sleep_count++;
        }
    }

    // 1. Turn off Status LED
    bsp_led_set(false);

    // 2. Disconnect Wi-Fi
    bsp_wifi_disconnect();

    // 3. Put SSD1681 e-Paper into ultra-low power deep sleep mode (<1 uA)
    bsp_display_deep_sleep();

    // 4. Put SHTC3 into low-power sleep
    bsp_shtc3_sleep();

    // 5. Mute Audio Power Amp without cutting codec power rail (prevents I2C bus clamping!)
    bsp_audio_stop();
    gpio_set_level((gpio_num_t)BSP_PIN_PA_CTRL, 0); // Mute amplifier
    gpio_set_level((gpio_num_t)BSP_PIN_PA_EN, 0);   // Keep codec rail ON (Active LOW) to prevent clamping SDA/SCL
    gpio_hold_en((gpio_num_t)BSP_PIN_PA_EN);

    // 6. Configure Wakeup Triggers
    uint64_t ext1_pin_mask = 0;
    if (cfg.wake_sources & BSP_WAKE_SRC_BUTTONS) {
        ext1_pin_mask |= (1ULL << BSP_PIN_BUTTON_BOOT) | (1ULL << BSP_PIN_BUTTON_POWER);
    }

    if (cfg.wake_sources & BSP_WAKE_SRC_EXTERNAL_RTC) {
        // Clear any pending RTC interrupt flags so RTC_INT pin is HIGH before arming
        bool alarm_flag = false, timer_flag = false;
        bsp_rtc_get_and_clear_interrupts(&alarm_flag, &timer_flag);

        bsp_rtc_disable_clkout();
        if (cfg.duration_sec > 0) {
            if (cfg.duration_sec <= 255) {
                bsp_rtc_set_countdown_timer((uint8_t)cfg.duration_sec);
            } else {
                bsp_rtc_datetime_t now;
                if (bsp_rtc_get_datetime(&now) == ESP_OK) {
                    uint32_t total_sec  = (uint32_t)now.hour * 3600 + (uint32_t)now.minute * 60 + now.second + cfg.duration_sec;
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
                    bsp_rtc_set_alarm(&alarm);
                } else {
                    esp_sleep_enable_timer_wakeup((uint64_t)cfg.duration_sec * 1000000ULL);
                }
            }
        }
        gpio_pullup_en((gpio_num_t)BSP_PIN_RTC_INT);
        gpio_pulldown_dis((gpio_num_t)BSP_PIN_RTC_INT);
        ext1_pin_mask |= (1ULL << BSP_PIN_RTC_INT);
    }

    // Wait for user to release buttons before entering sleep so the current press doesn't instantly wake the MCU!
    if (cfg.wake_sources & BSP_WAKE_SRC_BUTTONS) {
        while (gpio_get_level((gpio_num_t)BSP_PIN_BUTTON_BOOT)  == 0 ||
               gpio_get_level((gpio_num_t)BSP_PIN_BUTTON_POWER) == 0) {
            vTaskDelay(pdMS_TO_TICKS(50));
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    if (cfg.wake_sources & BSP_WAKE_SRC_TIMER) {
        if (cfg.duration_sec > 0) {
            esp_sleep_enable_timer_wakeup((uint64_t)cfg.duration_sec * 1000000ULL);
        }
    }

    if (cfg.mode == BSP_SLEEP_MODE_LIGHT) {
        if (cfg.wake_sources & BSP_WAKE_SRC_BUTTONS) {
            gpio_wakeup_enable((gpio_num_t)BSP_PIN_BUTTON_BOOT,  GPIO_INTR_LOW_LEVEL);
            gpio_wakeup_enable((gpio_num_t)BSP_PIN_BUTTON_POWER, GPIO_INTR_LOW_LEVEL);
        }
        if (cfg.wake_sources & BSP_WAKE_SRC_EXTERNAL_RTC) {
            gpio_wakeup_enable((gpio_num_t)BSP_PIN_RTC_INT, GPIO_INTR_LOW_LEVEL);
        }
        if (ext1_pin_mask != 0) {
            esp_sleep_enable_gpio_wakeup();
        }

        if (cfg.duration_sec > 0) {
            ESP_LOGI(TAG, "Entering Light Sleep for %lu seconds...", (unsigned long)cfg.duration_sec);
        } else {
            ESP_LOGI(TAG, "Entering Light Sleep (Indefinite / External Wakeup Triggers)...");
        }
        fflush(stdout);
        vTaskDelay(pdMS_TO_TICKS(50));

        esp_err_t ret = esp_light_sleep_start();

        if (cfg.wake_sources & BSP_WAKE_SRC_BUTTONS) {
            gpio_wakeup_disable((gpio_num_t)BSP_PIN_BUTTON_BOOT);
            gpio_wakeup_disable((gpio_num_t)BSP_PIN_BUTTON_POWER);
        }
        if (cfg.wake_sources & BSP_WAKE_SRC_EXTERNAL_RTC) {
            gpio_wakeup_disable((gpio_num_t)BSP_PIN_RTC_INT);
            bsp_rtc_clear_countdown_timer();
            bsp_rtc_get_and_clear_interrupts(NULL, NULL);
        }
        bsp_shtc3_wakeup();
        ESP_LOGI(TAG, "Resumed from Light Sleep");
        return ret;
    }

    // DEEP SLEEP
    if (ext1_pin_mask != 0) {
        if (cfg.wake_sources & BSP_WAKE_SRC_BUTTONS) {
            gpio_pullup_en   ((gpio_num_t)BSP_PIN_BUTTON_BOOT);
            gpio_pulldown_dis((gpio_num_t)BSP_PIN_BUTTON_BOOT);
            gpio_hold_en     ((gpio_num_t)BSP_PIN_BUTTON_BOOT);

            gpio_pullup_en   ((gpio_num_t)BSP_PIN_BUTTON_POWER);
            gpio_pulldown_dis((gpio_num_t)BSP_PIN_BUTTON_POWER);
            gpio_hold_en     ((gpio_num_t)BSP_PIN_BUTTON_POWER);
        }
        if (cfg.wake_sources & BSP_WAKE_SRC_EXTERNAL_RTC) {
            gpio_pullup_en   ((gpio_num_t)BSP_PIN_RTC_INT);
            gpio_pulldown_dis((gpio_num_t)BSP_PIN_RTC_INT);
            gpio_hold_en     ((gpio_num_t)BSP_PIN_RTC_INT);
        }
        esp_sleep_enable_ext1_wakeup_io(ext1_pin_mask, ESP_EXT1_WAKEUP_ANY_LOW);
    }
    if (cfg.duration_sec > 0) {
        ESP_LOGI(TAG, "Entering Deep Sleep for %lu seconds...", (unsigned long)cfg.duration_sec);
    } else {
        ESP_LOGI(TAG, "Entering Deep Sleep (Indefinite / External Wakeup Triggers: Buttons & RTC INT)...");
    }

    // Maintain EPD & Sensor 3.3V Power Rail
    gpio_set_level((gpio_num_t)BSP_PIN_EPD_3V3_EN, 0);
    gpio_hold_en  ((gpio_num_t)BSP_PIN_EPD_3V3_EN);

    // Maintain EPD Control Lines (CS=HIGH, RST=HIGH, DC=HIGH) so SSD1681 does not see floating / reset state
    gpio_set_level((gpio_num_t)BSP_PIN_EPD_RST, 1);
    gpio_hold_en  ((gpio_num_t)BSP_PIN_EPD_RST);
    gpio_set_level((gpio_num_t)BSP_PIN_EPD_CS,  1);
    gpio_hold_en  ((gpio_num_t)BSP_PIN_EPD_CS);
    gpio_set_level((gpio_num_t)BSP_PIN_EPD_DC,  1);
    gpio_hold_en  ((gpio_num_t)BSP_PIN_EPD_DC);

    // Maintain Audio Codec Rail to prevent I2C clamping
    gpio_set_level((gpio_num_t)BSP_PIN_PA_EN, 0);
    gpio_hold_en  ((gpio_num_t)BSP_PIN_PA_EN);

    // Hold Battery LDO Power Latch
    gpio_set_level((gpio_num_t)BSP_PIN_POWER_HOLD, 1);
    gpio_hold_en  ((gpio_num_t)BSP_PIN_POWER_HOLD);
    gpio_deep_sleep_hold_en();

    fflush(stdout);
    vTaskDelay(pdMS_TO_TICKS(50));
    esp_deep_sleep_start();
    return ESP_OK;
}
