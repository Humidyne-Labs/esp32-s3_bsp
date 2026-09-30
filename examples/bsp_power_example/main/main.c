#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "bsp/pinout.h"
#include "bsp/bsp_rtc_mem.h"
#include "bsp/bsp_power.h"

static const char *TAG = "power_example";

static void my_shutdown_callback(void *user_data)
{
    ESP_LOGI(TAG, "Custom shutdown hook triggered!");
}

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing BSP Power Subsystem...");
    
    // 1. Initialize power subsystem and ADC
    ESP_ERROR_CHECK(bsp_power_init());

    // 2. Drive power latch HIGH to keep board powered on battery
    ESP_ERROR_CHECK(bsp_power_hold());

    // 3. Register optional system shutdown hook
    ESP_ERROR_CHECK(bsp_power_register_shutdown_cb(my_shutdown_callback, NULL));

    // 4. Test Status LED
    ESP_LOGI(TAG, "Testing Status LED output...");
    bsp_led_set(true);
    vTaskDelay(pdMS_TO_TICKS(500));
    bsp_led_toggle();
    vTaskDelay(pdMS_TO_TICKS(500));
    bsp_led_set(false);

    // 5. Read Reset Reason, Wakeup Cause, and Recommended Init Mode
    esp_reset_reason_t rst_reason = bsp_get_reset_reason();
    esp_sleep_wakeup_cause_t wake_cause = bsp_get_wakeup_cause();
    bsp_init_mode_t init_mode = bsp_get_recommended_init_mode();

    ESP_LOGI(TAG, "System Status -> Reset Reason: %d | Wake Cause: %d | Rec Init Mode: %d",
             (int)rst_reason, (int)wake_cause, (int)init_mode);

    // 6. Query Battery Voltage and Charge Level
    uint32_t vbat_mv = 0;
    uint32_t vbat_raw = 0;
    ESP_ERROR_CHECK(bsp_battery_get_voltage(&vbat_mv, &vbat_raw));

    uint8_t batt_pct = bsp_battery_get_percentage();
    bool is_low = bsp_battery_is_low(20);

    ESP_LOGI(TAG, "Battery Voltage: %lu mV (Raw ADC: %lu)",
             (unsigned long)vbat_mv, (unsigned long)vbat_raw);
    ESP_LOGI(TAG, "Battery State: %u%% %s",
             (unsigned int)batt_pct, is_low ? "[LOW BATTERY WARNING]" : "[OK]");

    // 7. Unregister shutdown callback before sleep demonstration
    ESP_ERROR_CHECK(bsp_power_unregister_shutdown_cb());

    // 8. Prepare and execute a 3-second sleep test
    bsp_sleep_config_t sleep_cfg = BSP_SLEEP_CONFIG_DEFAULT();
    sleep_cfg.duration_sec = 3;
    sleep_cfg.wake_sources = BSP_WAKE_SRC_TIMER;

    ESP_LOGI(TAG, "Entering sleep mode for %lu seconds...", (unsigned long)sleep_cfg.duration_sec);
    
    // Since the default mode is Deep Sleep, the chip will reset upon wake.
    // If configured for Light Sleep, execution would resume here.
    esp_err_t err = bsp_enter_sleep(&sleep_cfg);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Resumed execution from sleep successfully!");
    } else {
        ESP_LOGE(TAG, "Failed to enter sleep: %s", esp_err_to_name(err));
    }

    ESP_LOGI(TAG, "BSP Power demonstration finished.");
}