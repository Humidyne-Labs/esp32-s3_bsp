/**
 * @file bsp_power.c
 * @brief Power Control Latch, Status LED, Battery ADC, Shutdown Sequence & Dual Sleep Modes Implementation
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
#include "bsp/bsp_splash.h"
#include "bsp/bsp_lvgl.h"
#include "bsp/bsp_button.h"
#include "bsp/bsp_lifecycle.h"
#include "bsp/bsp.h"
#include "sdkconfig.h"

static const char *TAG = "bsp_power";

// ADC Subsystem Handles (ESP32-S3 GPIO 4 = ADC1_CHANNEL_3)
#define BSP_ADC_BATTERY_CHANNEL ADC_CHANNEL_3
#define CORRECTION_FACTOR_MV    0 //mV
#define BATTERY_LUT_SIZE        (sizeof(s_battery_ocv_lut) / sizeof(s_battery_ocv_lut[0]))

typedef struct {
    uint16_t voltage_mv; ///< voltage_mv value
    uint8_t  percentage; ///< percentage value
} battery_lut_point_t;

static adc_oneshot_unit_handle_t s_adc_handle          = NULL;
static adc_cali_handle_t         s_cali_handle         = NULL;
static bool                      s_calibrated          = false;
static bool                      s_led_state           = false;
static bool                      s_power_inited        = false;
static bsp_power_off_cb_t        s_shutdown_cb         = NULL;
static void                      *s_shutdown_user_data = NULL;

// Standard 3.7V Li-ion/LiPo discharge curve (rested OCV) sorted ascending
static const battery_lut_point_t s_battery_ocv_lut[] = {
    { 3300,   0 },
    { 3450,   3 },
    { 3600,   8 },
    { 3680,  15 },
    { 3720,  25 },
    { 3750,  35 },
    { 3780,  45 },
    { 3820,  55 },
    { 3870,  65 },
    { 3920,  75 },
    { 3980,  85 },
    { 4060,  92 },
    { 4200, 100 }
};

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
            .bitwidth = ADC_BITWIDTH_DEFAULT, // 12-bits
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
            .bitwidth = ADC_BITWIDTH_DEFAULT, // 12-bits
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

esp_err_t bsp_power_register_shutdown_cb(bsp_power_off_cb_t cb, void *user_data)
{
    s_shutdown_cb        = cb;
    s_shutdown_user_data = user_data;
    return ESP_OK;
}

esp_err_t bsp_power_unregister_shutdown_cb(void)
{
    s_shutdown_cb        = NULL;
    s_shutdown_user_data = NULL;
    return ESP_OK;
}

void bsp_power_off(void)
{
    static bool s_shutting_down = false;
    if (s_shutting_down) return;
    s_shutting_down = true;

    ESP_LOGI(TAG, "Executing complete system shutdown sequence...");

    // 1. Trigger shutdown splash screen and acoustic chime if registered
    bsp_trigger_splash(BSP_SPLASH_SHUTDOWN);
    bsp_trigger_chime(BSP_CHIME_SHUTDOWN);

    // 2. Run lifecycle on_shutdown callback if configured
    bsp_lifecycle_invoke_shutdown();

    // 3. Run user shutdown callback if registered
    if (s_shutdown_cb != NULL) {
        ESP_LOGI(TAG, "Invoking registered shutdown callback...");
        s_shutdown_cb(s_shutdown_user_data);
    }

    // 4. Stop button timers/polling immediately so no further button events fire
    bsp_button_stop();

    // 5. Stop LVGL rendering background task
    bsp_lvgl_stop();

    // 6. Put display into deep sleep and disable display power rail
    bsp_display_deep_sleep();
    gpio_set_level((gpio_num_t)BSP_PIN_EPD_3V3_EN, 1);

    // 7. Mute and power off audio subsystem
    bsp_audio_stop();
    bsp_audio_power_enable(false);
    gpio_set_level((gpio_num_t)BSP_PIN_PA_CTRL, 0);
    gpio_set_level((gpio_num_t)BSP_PIN_PA_EN,   1);

    // 8. Turn off status LED
    bsp_led_set(false);

    // 9. Disconnect Wi-Fi
    bsp_wifi_disconnect();

    // 10. Wait until user physically releases the power button so it doesn't immediately re-trigger
    while (gpio_get_level((gpio_num_t)BSP_PIN_BUTTON_POWER) == 0) {
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    vTaskDelay(pdMS_TO_TICKS(100));

    // 11. Drop power latch (GPIO 17 = 0) and release hardware hold
    ESP_LOGI(TAG, "De-asserting BAT_CTRL power latch (GPIO %d)...", BSP_PIN_POWER_HOLD);
    gpio_hold_dis((gpio_num_t)BSP_PIN_POWER_HOLD);
    gpio_set_level((gpio_num_t)BSP_PIN_POWER_HOLD, 0);
    vTaskDelay(pdMS_TO_TICKS(100));

    // If external power (USB / VBUS) is present, the board stays powered -> perform clean restart
    ESP_LOGI(TAG, "External power (USB) detected; restarting MCU");
    esp_restart();
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
        .bitwidth = ADC_BITWIDTH_DEFAULT,  // 12-bits
    };
    err = adc_oneshot_config_channel(s_adc_handle, BSP_ADC_BATTERY_CHANNEL, &chan_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure ADC channel: %s", esp_err_to_name(err));
        return err;
    }

    // 3. Initialize Factory Calibration Scheme
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

	/* Multisampling is no longer needed. */

	int raw_val = 0;
	esp_err_t err = adc_oneshot_read(s_adc_handle, BSP_ADC_BATTERY_CHANNEL, &raw_val);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "ADC read failed: %s", esp_err_to_name(err));
		return err;
	}

    if (out_raw != NULL) {
        *out_raw = (uint32_t)raw_val;
    }

    int voltage_mv = 0;
    if (s_calibrated && s_cali_handle != NULL) {
        adc_cali_raw_to_voltage(s_cali_handle, raw_val, &voltage_mv);
		// Display Calibrated Voltage in mV
		ESP_LOGI(TAG, "ADC Cali Voltage: %" PRIu32 " mV", voltage_mv);
    } else {
		// Fallback: 12-bit ADC with 12 dB attenuation (Vref = 1100 mV, k = 0.25 -> 4400 mV full scale)
        // Add 2047 before division for half-LSB integer rounding
        voltage_mv = (((uint32_t)raw_val * 4400) + 2047) / 4095;
		// Display Non Calibrated Voltage in mV
		ESP_LOGW(TAG, "[FALLBACK] Uncalibrated ADC Pin Voltage: %" PRIu32 " mV", voltage_mv);
    }

    // Compensate for 1:2 'external' resistor divider (R1=200k 1%, R2=200k 1% -> 2.0x factor)
    *out_mv = (uint32_t)(voltage_mv * 2) + CORRECTION_FACTOR_MV; // R_DIV has a 1% Tolerance.
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

    // 1. Boundary Clamping
    if (vbat_mv <= s_battery_ocv_lut[0].voltage_mv) {
        return s_battery_ocv_lut[0].percentage;
    }
    if (vbat_mv >= s_battery_ocv_lut[BATTERY_LUT_SIZE - 1].voltage_mv) {
        return s_battery_ocv_lut[BATTERY_LUT_SIZE - 1].percentage;
    }

    // 2. Linear Interpolation between adjacent points
    for (size_t i = 1; i < BATTERY_LUT_SIZE; i++) {
        if (vbat_mv < s_battery_ocv_lut[i].voltage_mv) {
            uint16_t v_low   = s_battery_ocv_lut[i - 1].voltage_mv;
            uint16_t v_high  = s_battery_ocv_lut[i].voltage_mv;
            uint8_t  p_low   = s_battery_ocv_lut[i - 1].percentage;
            uint8_t  p_high  = s_battery_ocv_lut[i].percentage;

            uint32_t delta_v = v_high - v_low;
            uint32_t delta_p = p_high - p_low;

            // Integer interpolation with half-step rounding (+ delta_v / 2)
            uint32_t interpolated = p_low + (((vbat_mv - v_low) * delta_p + (delta_v / 2)) / delta_v);
            return (uint8_t)interpolated;
        }
    }

    return 100;
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
