#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp/bsp_button.h"

static const char *TAG = "example_button";

static void button_event_cb(bsp_button_t btn, bsp_button_event_t evt, void *user_data)
{
    (void)user_data;
    const char *btn_name = (btn == BSP_BUTTON_BOOT) ? "BOOT" : "POWER";
    const char *evt_name = "UNKNOWN";

    switch (evt) {
        case BSP_BUTTON_EVENT_PRESS_DOWN:
            evt_name = "PRESS_DOWN";
            break;
        case BSP_BUTTON_EVENT_PRESS_UP:
            evt_name = "PRESS_UP";
            break;
        case BSP_BUTTON_EVENT_SINGLE_CLICK:
            evt_name = "SINGLE_CLICK";
            break;
        case BSP_BUTTON_EVENT_DOUBLE_CLICK:
            evt_name = "DOUBLE_CLICK";
            break;
        case BSP_BUTTON_EVENT_LONG_PRESS:
            evt_name = "LONG_PRESS";
            break;
        default:
            break;
    }

    ESP_LOGI(TAG, "Button Event: [%s] -> %s", btn_name, evt_name);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing hardware button subsystem...");

    const bsp_button_config_t btn_cfg = {
        .debounce_ms            = 20,
        .click_timeout_ms       = 280,
        .long_press_ms          = 2000,
        .auto_power_off_on_hold = false,
    };

    ESP_ERROR_CHECK(bsp_button_init(&btn_cfg));

    // Register event callbacks for BOOT button
    ESP_ERROR_CHECK(bsp_button_register_cb(BSP_BUTTON_BOOT, BSP_BUTTON_EVENT_SINGLE_CLICK, button_event_cb, NULL));
    ESP_ERROR_CHECK(bsp_button_register_cb(BSP_BUTTON_BOOT, BSP_BUTTON_EVENT_DOUBLE_CLICK, button_event_cb, NULL));
    ESP_ERROR_CHECK(bsp_button_register_cb(BSP_BUTTON_BOOT, BSP_BUTTON_EVENT_LONG_PRESS,   button_event_cb, NULL));

    // Register event callbacks for POWER button
    ESP_ERROR_CHECK(bsp_button_register_cb(BSP_BUTTON_POWER, BSP_BUTTON_EVENT_SINGLE_CLICK, button_event_cb, NULL));
    ESP_ERROR_CHECK(bsp_button_register_cb(BSP_BUTTON_POWER, BSP_BUTTON_EVENT_DOUBLE_CLICK, button_event_cb, NULL));
    ESP_ERROR_CHECK(bsp_button_register_cb(BSP_BUTTON_POWER, BSP_BUTTON_EVENT_LONG_PRESS,   button_event_cb, NULL));

    ESP_LOGI(TAG, "Testing synchronous click wait on BOOT button (100ms timeout)...");
    esp_err_t err = bsp_button_wait_for_click(BSP_BUTTON_BOOT, 100);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "BOOT button clicked!");
    } else if (err == ESP_ERR_TIMEOUT) {
        ESP_LOGI(TAG, "Wait for click timed out as expected.");
    } else {
        ESP_ERROR_CHECK(err);
    }

    ESP_LOGI(TAG, "Polling button states...");
    for (int i = 0; i < 5; i++) {
        bool boot_pressed  = bsp_button_is_pressed(BSP_BUTTON_BOOT);
        bool power_pressed = bsp_button_is_pressed(BSP_BUTTON_POWER);

        ESP_LOGI(TAG, "Poll %d: BOOT=%s, POWER=%s",
                 i,
                 boot_pressed  ? "PRESSED" : "RELEASED",
                 power_pressed ? "PRESSED" : "RELEASED");

        vTaskDelay(pdMS_TO_TICKS(200));
    }

    // Clean up callbacks
    ESP_ERROR_CHECK(bsp_button_unregister_cb(BSP_BUTTON_BOOT, BSP_BUTTON_EVENT_SINGLE_CLICK));
    ESP_ERROR_CHECK(bsp_button_unregister_cb(BSP_BUTTON_BOOT, BSP_BUTTON_EVENT_DOUBLE_CLICK));
    ESP_ERROR_CHECK(bsp_button_unregister_cb(BSP_BUTTON_BOOT, BSP_BUTTON_EVENT_LONG_PRESS));

    ESP_ERROR_CHECK(bsp_button_unregister_cb(BSP_BUTTON_POWER, BSP_BUTTON_EVENT_SINGLE_CLICK));
    ESP_ERROR_CHECK(bsp_button_unregister_cb(BSP_BUTTON_POWER, BSP_BUTTON_EVENT_DOUBLE_CLICK));
    ESP_ERROR_CHECK(bsp_button_unregister_cb(BSP_BUTTON_POWER, BSP_BUTTON_EVENT_LONG_PRESS));

    // Stop button timer and polling
    ESP_ERROR_CHECK(bsp_button_stop());
    ESP_LOGI(TAG, "Button subsystem stopped successfully.");
}