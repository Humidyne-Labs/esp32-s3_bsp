/**
 * @file bsp_lifecycle.h
 * @brief High-Level Application Lifecycle, Sleep/Wake Dispatcher, Shutdown & State Engine
 * 
 * Provides an event-driven lifecycle framework for ESP32-S3 ePaper applications,
 * abstracting reset reason inspection, dynamic init mode selection, persistent
 * stage tracking, wake dispatching, sleep stand-down, and clean shutdown hooks into structured callbacks.
 * 
 * Hardware Target:
 *  - Microcontroller: Espressif Systems ESP32-S3-PICO-1-N8R8
 *  - Target Board: Waveshare ESP32-S3 ePaper 1.54 V2
 * 
 * @attribution
 * - BSP Architecture: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_LIFECYCLE_H
#define BSP_LIFECYCLE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"
#include "esp_sleep.h"
#include "bsp/bsp_rtc_mem.h"
#include "bsp/bsp_button.h"
#include "bsp/bsp_power.h"
#include "bsp/bsp_splash.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Structured Wake Context passed to application on_wake callback
 */
typedef struct {
    esp_reset_reason_t       reset_reason;       /*!< Reset reason (e.g. ESP_RST_DEEPSLEEP, ESP_RST_POWERON) */
    esp_sleep_wakeup_cause_t wake_cause;         /*!< Wakeup cause (e.g. EXT1, TIMER, GPIO)                  */
    uint64_t                 ext1_wakeup_pins;   /*!< GPIO mask of pins that triggered EXT1 wakeup           */
    bool                     woke_from_button;   /*!< True if wake was triggered by BOOT or POWER button     */
    bsp_button_t             wake_button;        /*!< Which button triggered wakeup (if button wake)         */
    bsp_init_mode_t          init_mode;          /*!< Initialization profile executed (FULL, FAST, MIN)      */
    uint32_t                 sleep_duration_sec; /*!< Configured sleep duration from previous cycle          */
    uint32_t                 boot_count;         /*!< Monotonic system boot count                            */
    uint32_t                 deep_sleep_count;   /*!< Total deep sleep cycles                                */
    uint32_t                 light_sleep_count;  /*!< Total light sleep cycles                               */
    uint8_t                  app_stage;          /*!< Persistent application stage code (from RTC memory)    */
    void                     *user_data;         /*!< User data pointer passed during lifecycle start        */
} bsp_wake_context_t;

/**
 * @brief Callback executed on cold boot (Power-On Reset, Brownout, Software Restart, etc.)
 */
typedef void (*bsp_cold_boot_cb_t)(void *user_data);

/**
 * @brief Callback executed on resume from Deep Sleep or Light Sleep
 */
typedef void (*bsp_wake_cb_t)(const bsp_wake_context_t *ctx, void *user_data);

/**
 * @brief Callback executed right before entering Deep or Light Sleep (for app cleanup / display badge)
 */
typedef void (*bsp_before_sleep_cb_t)(bsp_sleep_mode_t mode, uint32_t duration_sec, void *user_data);

/**
 * @brief Callback executed immediately prior to system power off / clean shutdown
 */
typedef void (*bsp_shutdown_cb_t)(void *user_data);

/**
 * @brief Comprehensive Application Lifecycle Configuration
 */
typedef struct {
    bsp_cold_boot_cb_t    on_cold_boot;    /*!< Handler for initial cold boot                */
    bsp_wake_cb_t         on_wake;         /*!< Handler for sleep wake events                */
    bsp_before_sleep_cb_t on_before_sleep; /*!< Hook called immediately prior to sleep entry */
    bsp_shutdown_cb_t     on_shutdown;     /*!< Hook called immediately prior to power off   */
    void                 *user_data;       /*!< Custom application context pointer           */
} bsp_app_lifecycle_t;

/**
 * @brief Start BSP Application Engine with Lifecycle Hooks
 * 
 * Automatically initializes RTC memory, inspects reset reason, selects the optimal
 * hardware initialization mode (FULL on cold boot, FAST/MIN on wake), populates the
 * wake context, and dispatches to on_cold_boot or on_wake.
 * 
 * @param lifecycle Pointer to bsp_app_lifecycle_t configuration
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_app_start(const bsp_app_lifecycle_t *lifecycle);

/**
 * @brief Retrieve current wake context
 * 
 * @param[out] ctx Pointer to bsp_wake_context_t destination
 * @return esp_err_t ESP_OK on success, ESP_ERR_INVALID_STATE if context is not available
 */
esp_err_t bsp_lifecycle_get_context(bsp_wake_context_t *ctx);

/**
 * @brief Set application stage index in RTC Slow Memory
 * 
 * @param stage Stage identifier (0..255)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_lifecycle_set_stage(uint8_t stage);

/**
 * @brief Get application stage index from RTC Slow Memory
 * 
 * @return uint8_t Current stage identifier
 */
uint8_t bsp_lifecycle_get_stage(void);

/**
 * @brief Save custom application state struct to RTC Slow Memory scratchpad (max 31 bytes)
 * 
 * @param data Source buffer
 * @param len Byte count (max 31)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_lifecycle_save_state(const void *data, size_t len);

/**
 * @brief Load custom application state struct from RTC Slow Memory scratchpad
 * 
 * @param[out] out_data Destination buffer
 * @param len Byte count (max 31)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_lifecycle_load_state(void *out_data, size_t len);

/**
 * @brief Enter sleep with automatic before_sleep lifecycle hook and splash/chime execution
 * 
 * @param config Sleep configuration (mode, duration, wake sources, next init mode)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_lifecycle_enter_sleep(const bsp_sleep_config_t *config);

/**
 * @brief Perform clean hardware power off and system shutdown
 * 
 * Executes registered on_shutdown callback, drops BAT_CTRL power latch, and stops peripherals.
 */
void bsp_lifecycle_power_off(void);

/**
 * @brief Internal lifecycle dispatcher hook invoked by bsp_power_off()
 */
void bsp_lifecycle_invoke_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_LIFECYCLE_H */
