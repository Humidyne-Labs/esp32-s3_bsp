/**
 * @file bsp_button.c
 * @brief Button driver state-machine with debounce, clicks, hold detection, and power off trigger.
 *
 * @version 1.0.0
 * @attribution
 * - Architecture & Development: HUMIDYNE LABS / Humiditron
 * - AI Systems Co-Developer: Gemini (Google DeepMind)
 * - Hardware Target: Waveshare Electronics ESP32-S3 ePaper 1.54 V2
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
    STATE_BOOT_WAIT_RELEASE = 0, ///< STATE_BOOT_WAIT_RELEASE value
    STATE_IDLE, ///< STATE_IDLE value
    STATE_DEBOUNCE_PRESS, ///< STATE_DEBOUNCE_PRESS value
    STATE_PRESSED, ///< STATE_PRESSED value
    STATE_DEBOUNCE_RELEASE, ///< STATE_DEBOUNCE_RELEASE value
    STATE_WAIT_DOUBLE_CLICK, ///< STATE_WAIT_DOUBLE_CLICK value
} button_state_t;

typedef struct {
    bsp_button_cb_t cb; ///< cb value
    void *user_data; ///< user_data value
} button_callback_entry_t;

typedef struct {
    gpio_num_t              gpio; ///< gpio value
    button_state_t          state; ///< state value
    uint32_t                press_start_tick; ///< press_start_tick value
    uint32_t                release_tick; ///< release_tick value
    uint32_t                stable_state_ticks; ///< stable_state_ticks value
    bool                    long_press_fired; ///< long_press_fired value
    button_callback_entry_t callbacks[BSP_BUTTON_EVENT_MAX]; ///< callbacks[BSP_BUTTON_EVENT_MAX] value
} button_dev_t;

static button_dev_t        s_buttons[BSP_BUTTON_COUNT];
static bsp_button_config_t s_cfg;
static esp_timer_handle_t  s_timer_handle = NULL;
static bool                s_inited       = false;

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
                    s_buttons[btn].state              = STATE_DEBOUNCE_PRESS;
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
                    s_buttons[btn].state              = STATE_DEBOUNCE_RELEASE;
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
                            s_buttons[btn].state        = STATE_WAIT_DOUBLE_CLICK;
                            s_buttons[btn].release_tick = now;
                        }
                    }
                } else {
                    s_buttons[btn].state = STATE_PRESSED;
                }
                break;

            case STATE_WAIT_DOUBLE_CLICK:
                if (is_down) {
                    s_buttons[btn].state              = STATE_DEBOUNCE_PRESS;
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

esp_err_t bsp_button_stop(void)
{
    if (s_timer_handle != NULL) {
        esp_timer_stop(s_timer_handle);
    }
    return ESP_OK;
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
    s_buttons[button].callbacks[event].cb        = cb;
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

esp_err_t bsp_button_wait_for_click(bsp_button_t button, uint32_t timeout_ms)
{
    if (button >= BSP_BUTTON_COUNT) {
        return ESP_ERR_INVALID_ARG;
    }

    gpio_num_t gpio = (button == BSP_BUTTON_BOOT) ?
                      (gpio_num_t)BSP_PIN_BUTTON_BOOT :
                      (gpio_num_t)BSP_PIN_BUTTON_POWER;

    int64_t start_us   = esp_timer_get_time();
    int64_t timeout_us = (timeout_ms > 0) ? ((int64_t)timeout_ms * 1000LL) : -1LL;

    // 1. Wait for button press (Active Low -> level 0)
    while (gpio_get_level(gpio) != 0) {
        if (timeout_us > 0 && (esp_timer_get_time() - start_us) > timeout_us) {
            return ESP_ERR_TIMEOUT;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    // Debounce press
    vTaskDelay(pdMS_TO_TICKS(30));

    // 2. Wait for button release (Active Low -> level 1)
    while (gpio_get_level(gpio) == 0) {
        if (timeout_us > 0 && (esp_timer_get_time() - start_us) > timeout_us) {
            return ESP_ERR_TIMEOUT;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    // Debounce release
    vTaskDelay(pdMS_TO_TICKS(30));
    return ESP_OK;
}
