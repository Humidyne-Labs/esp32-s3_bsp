/**
 * @file bsp_button.c
 * @brief Button driver state-machine with debounce, clicks, hold detection, and shutdown hooks.
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics (https://www.waveshare.com)
 * - Microcontroller: Espressif Systems ESP32-S3 (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#include "bsp/bsp_button.h"
#include "bsp/bsp_power.h"
#include "bsp/pinout.h"
#include "bsp/bsp.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "bsp_button";

#define TIMER_INTERVAL_MS 10

typedef enum {
    STATE_BOOT_WAIT_RELEASE = 0,
    STATE_IDLE,
    STATE_DEBOUNCE_PRESS,
    STATE_PRESSED,
    STATE_DEBOUNCE_RELEASE,
    STATE_WAIT_DOUBLE_CLICK,
} button_state_t;

typedef struct {
    bsp_button_cb_t cb;
    void *user_data;
} button_callback_entry_t;

typedef struct {
    gpio_num_t              gpio;
    button_state_t          state;
    uint32_t                press_start_tick;
    uint32_t                release_tick;
    uint32_t                stable_state_ticks;
    bool                    long_press_fired;
    button_callback_entry_t callbacks[BSP_BUTTON_EVENT_MAX];
} button_dev_t;

static button_dev_t        s_buttons[BSP_BUTTON_COUNT];
static bsp_button_config_t s_cfg;
static esp_timer_handle_t  s_timer_handle = NULL;
static bool                s_inited       = false;

// Shutdown hook storage
static bsp_power_off_cb_t s_shutdown_cb         = NULL;
static void               *s_shutdown_user_data = NULL;

static void fire_event(bsp_button_t btn, bsp_button_event_t event)
{
    if (s_buttons[btn].callbacks[event].cb) {
        s_buttons[btn].callbacks[event].cb(btn, event, s_buttons[btn].callbacks[event].user_data);
    }

    // Built-in auto power off action on power button long-press
    if (btn == BSP_BUTTON_POWER && event == BSP_BUTTON_EVENT_LONG_PRESS && s_cfg.auto_power_off_on_hold) {
        ESP_LOGW(TAG, "Power button long-press detected! Initiating power off...");
        bsp_power_off();
    }
}

static void button_timer_cb(void *arg)
{
    uint32_t now = (uint32_t)(esp_timer_get_time() / 1000ULL);

    for (int i = 0; i < BSP_BUTTON_COUNT; i++) {
        bsp_button_t btn     = (bsp_button_t)i;
        bool         is_down = (gpio_get_level(s_buttons[btn].gpio) == 0);

        switch (s_buttons[btn].state) {
            case STATE_BOOT_WAIT_RELEASE:
                if (!is_down) {
                    s_buttons[btn].state = STATE_IDLE;
                }
                break;

            case STATE_IDLE:
                if (is_down) {
                    s_buttons[btn].state = STATE_DEBOUNCE_PRESS;
                    s_buttons[btn].stable_state_ticks = now;
                }
                break;

            case STATE_DEBOUNCE_PRESS:
                if (is_down) {
                    if ((now - s_buttons[btn].stable_state_ticks) >= s_cfg.debounce_ms) {
                        s_buttons[btn].state = STATE_PRESSED;
                        s_buttons[btn].press_start_tick = now;
                        s_buttons[btn].long_press_fired = false;
                        fire_event(btn, BSP_BUTTON_EVENT_PRESS_DOWN);
                    }
                } else {
                    s_buttons[btn].state = STATE_IDLE;
                }
                break;

            case STATE_PRESSED:
                if (is_down) {
                    if (!s_buttons[btn].long_press_fired && 
                        (now - s_buttons[btn].press_start_tick) >= s_cfg.long_press_ms) {
                        s_buttons[btn].long_press_fired = true;
                        fire_event(btn, BSP_BUTTON_EVENT_LONG_PRESS);
                    }
                } else {
                    s_buttons[btn].state = STATE_DEBOUNCE_RELEASE;
                    s_buttons[btn].stable_state_ticks = now;
                }
                break;

            case STATE_DEBOUNCE_RELEASE:
                if (!is_down) {
                    if ((now - s_buttons[btn].stable_state_ticks) >= s_cfg.debounce_ms) {
                        fire_event(btn, BSP_BUTTON_EVENT_PRESS_UP);

                        if (s_buttons[btn].long_press_fired) {
                            s_buttons[btn].state = STATE_IDLE;
                        } else {
                            s_buttons[btn].state = STATE_WAIT_DOUBLE_CLICK;
                            s_buttons[btn].release_tick = now;
                        }
                    }
                } else {
                    s_buttons[btn].state = STATE_PRESSED;
                }
                break;

            case STATE_WAIT_DOUBLE_CLICK:
                if (is_down) {
                    s_buttons[btn].state = STATE_DEBOUNCE_PRESS;
                    s_buttons[btn].stable_state_ticks = now;
                    fire_event(btn, BSP_BUTTON_EVENT_DOUBLE_CLICK);
                } else if ((now - s_buttons[btn].release_tick) >= s_cfg.click_timeout_ms) {
                    fire_event(btn, BSP_BUTTON_EVENT_SINGLE_CLICK);
                    s_buttons[btn].state = STATE_IDLE;
                }
                break;
        }
    }
}

esp_err_t bsp_power_register_shutdown_cb(bsp_power_off_cb_t cb, void *user_data)
{
    s_shutdown_cb        = cb;
    s_shutdown_user_data = user_data;
    return ESP_OK;
}

void bsp_power_off(void)
{
    static bool s_shutting_down = false;
    if (s_shutting_down) return;
    s_shutting_down = true;

    ESP_LOGI(TAG, "Executing complete shutdown sequence...");

    // 1. Stop button debounce timer immediately so no further button events fire
    if (s_timer_handle) {
        esp_timer_stop(s_timer_handle);
    }

    // 2. Run user shutdown callback if registered (e.g. terminate tasks & render Space Cat)
    if (s_shutdown_cb) {
        ESP_LOGI(TAG, "Calling user shutdown callback...");
        s_shutdown_cb(s_shutdown_user_data);
    }

    // 3. Stop LVGL rendering background task
    bsp_lvgl_stop();

    // 4. Put display to deep sleep and cut display power rail
    bsp_display_deep_sleep();
    gpio_set_level((gpio_num_t)BSP_PIN_EPD_3V3_EN, 1);
    
    // 5. Mute and power off audio amp
    bsp_audio_stop();
    bsp_audio_power_enable(false);
    gpio_set_level((gpio_num_t)BSP_PIN_PA_CTRL, 0);
    gpio_set_level((gpio_num_t)BSP_PIN_PA_EN, 1);

    // 6. Turn off status LED
    bsp_led_set(false);

    // 7. Disconnect Wi-Fi
    bsp_wifi_disconnect();

    // 8. Wait until the user physically releases the power button so it doesn't immediately re-trigger
    while (gpio_get_level((gpio_num_t)BSP_PIN_BUTTON_POWER) == 0) {
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    vTaskDelay(pdMS_TO_TICKS(100));

    // 9. Drop power latch (GPIO 17 = 0) and release hardware hold
    ESP_LOGI(TAG, "De-asserting BAT_CTRL power latch (GPIO %d)...", BSP_PIN_POWER_HOLD);
    gpio_hold_dis((gpio_num_t)BSP_PIN_POWER_HOLD);
    gpio_set_level((gpio_num_t)BSP_PIN_POWER_HOLD, 0);
    vTaskDelay(pdMS_TO_TICKS(100));

    // 10. If still powered (e.g. USB cable attached), enter deep sleep with wake on POWER or BOOT
    //ESP_LOGI(TAG, "External power (USB) detected. Entering deep sleep with wakeup on POWER (GPIO %d) & BOOT (GPIO %d)...",
    //         BSP_PIN_BUTTON_POWER, BSP_PIN_BUTTON_BOOT);
    //esp_sleep_enable_ext1_wakeup_io((1ULL << BSP_PIN_BUTTON_POWER) | (1ULL << BSP_PIN_BUTTON_BOOT), ESP_EXT1_WAKEUP_ANY_LOW);
    //vTaskDelay(pdMS_TO_TICKS(50));
    //esp_deep_sleep_start();

    esp_restart(); // Restart, don't sleep if powered by VBUS.
}

esp_err_t bsp_button_init(const bsp_button_config_t *config)
{
    if (s_inited) {
        return ESP_OK;
    }

    if (config) {
        s_cfg = *config;
    } else {
        s_cfg.debounce_ms            = 20;
        s_cfg.click_timeout_ms       = 280;
        s_cfg.long_press_ms          = 2500;
        s_cfg.auto_power_off_on_hold = true;
    }

    // 1. Ensure master IO configuration is applied (Buttons on GPIO 0 & 18 with pullups)
    bsp_init_io();

    memset(s_buttons, 0, sizeof(s_buttons));
    s_buttons[BSP_BUTTON_BOOT].gpio  = BSP_PIN_BUTTON_BOOT;
    s_buttons[BSP_BUTTON_POWER].gpio = BSP_PIN_BUTTON_POWER;

    for (int i = 0; i < BSP_BUTTON_COUNT; i++) {
        bsp_button_t btn = (bsp_button_t)i;
        bool is_down = (gpio_get_level(s_buttons[btn].gpio) == 0);
        if (is_down) {
            // Button is actively held down during boot (e.g. power-on button press)
            // Wait until it is released before accepting user clicks
            s_buttons[btn].state = STATE_BOOT_WAIT_RELEASE;
        } else {
            s_buttons[btn].state = STATE_IDLE;
        }
    }

    const esp_timer_create_args_t timer_args = {
        .callback = &button_timer_cb,
        .name     = "bsp_btn_tmr"
    };
    esp_err_t ret = esp_timer_create(&timer_args, &s_timer_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create button timer: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_timer_start_periodic(s_timer_handle, TIMER_INTERVAL_MS * 1000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start button timer: %s", esp_err_to_name(ret));
        return ret;
    }

    s_inited = true;
    ESP_LOGI(TAG, "Buttons initialized (BOOT GPIO%d, POWER GPIO%d)",
             BSP_PIN_BUTTON_BOOT, BSP_PIN_BUTTON_POWER);
    return ESP_OK;
}

esp_err_t bsp_button_register_cb(bsp_button_t button, bsp_button_event_t event, bsp_button_cb_t cb, void *user_data)
{
    if (button >= BSP_BUTTON_COUNT || event >= BSP_BUTTON_EVENT_MAX) {
        return ESP_ERR_INVALID_ARG;
    }
    s_buttons[button].callbacks[event].cb = cb;
    s_buttons[button].callbacks[event].user_data = user_data;
    return ESP_OK;
}

esp_err_t bsp_button_unregister_cb(bsp_button_t button, bsp_button_event_t event)
{
    return bsp_button_register_cb(button, event, NULL, NULL);
}

bool bsp_button_is_pressed(bsp_button_t button)
{
    if (button >= BSP_BUTTON_COUNT) {
        return false;
    }
    return gpio_get_level(s_buttons[button].gpio) == 0;
}