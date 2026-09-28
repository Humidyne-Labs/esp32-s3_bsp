/**
 * @file bsp_rtc_mem.h
 * @brief Internal ESP32-S3 RTC Slow Memory Persistent Storage Wrapper
 * 
 * Provides structured, zero-wear persistent state across Deep Sleep cycles
 * using ESP32-S3 RTC Slow SRAM.
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

#ifndef BSP_RTC_MEM_H
#define BSP_RTC_MEM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_RTC_MEM_MAGIC 0x53335254  /*!< "S3RT" magic token */

/**
 * @brief Initialization Modes for Dynamic Hardware Configuration
 */
typedef enum {
    BSP_INIT_MODE_FULL = 0, /*!< Cold boot / all subsystems & LVGL initialized  */
    BSP_INIT_MODE_FAST = 1, /*!< Wake boot / fast display refresh (no clearing) */
} bsp_init_mode_t;

/**
 * @brief Sleep Execution Modes
 */
typedef enum {
    BSP_SLEEP_MODE_LIGHT = 0, /*!< Light sleep (RAM preserved, clock gated)           */
    BSP_SLEEP_MODE_DEEP  = 1, /*!< Deep sleep (Power down, RTC slow memory preserved) */
} bsp_sleep_mode_t;

/**
 * @brief Persistent RTC State Struct (Stored in RTC Slow Memory)
 */
typedef struct {
    uint32_t magic;                    /*!< Validation magic token                      */
    uint32_t boot_count;               /*!< Total system boot counter                   */
    uint32_t deep_sleep_count;         /*!< Total deep sleep cycles                     */
    uint32_t light_sleep_count;        /*!< Total light sleep cycles                    */
    uint8_t  last_init_mode;           /*!< Last executed bsp_init_mode_t               */
    uint8_t  next_init_mode;           /*!< Planned next bsp_init_mode_t on wake        */
    uint8_t  last_sleep_mode;          /*!< Last executed bsp_sleep_mode_t              */
    uint8_t  flags;                    /*!< System runtime status flags                 */
    uint32_t last_sleep_duration_sec;  /*!< Duration of previous sleep period           */
    uint32_t last_wake_epoch;          /*!< Epoch timestamp of previous wake event      */
    uint8_t  scratchpad[32];           /*!< Application telemetry / scratch data buffer */
} bsp_rtc_state_t;

/**
 * @brief Initialize or validate RTC Slow Memory state
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_rtc_mem_init(void);

/**
 * @brief Get direct pointer to RTC state struct
 * 
 * @return bsp_rtc_state_t* Pointer to RTC Slow Memory struct
 */
bsp_rtc_state_t *bsp_rtc_mem_get_state(void);

/**
 * @brief Get total boot count
 * 
 * @return uint32_t Boot count
 */
uint32_t bsp_rtc_mem_get_boot_count(void);

/**
 * @brief Set the desired initialization mode for the next wake cycle
 * 
 * @param mode Target init mode
 */
void bsp_rtc_mem_set_next_init_mode(bsp_init_mode_t mode);

/**
 * @brief Get the configured initialization mode for the current/next cycle
 * 
 * @return bsp_init_mode_t Init mode
 */
bsp_init_mode_t bsp_rtc_mem_get_next_init_mode(void);

/**
 * @brief Read arbitrary user scratchpad bytes from RTC memory
 * 
 * @param dest Destination buffer
 * @param len Number of bytes (max 32)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_rtc_mem_read_scratchpad(uint8_t *dest, size_t len);

/**
 * @brief Write arbitrary user scratchpad bytes to RTC memory
 * 
 * @param src Source buffer
 * @param len Number of bytes (max 32)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_rtc_mem_write_scratchpad(const uint8_t *src, size_t len);

/**
 * @brief Save 200x200 1-bit EPD frame buffer to RTC Slow Memory for partial refresh persistence
 * 
 * @param frame Pointer to 5000-byte frame buffer
 * @param len Size of buffer in bytes
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_rtc_mem_save_display_frame(const uint8_t *frame, size_t len);

/**
 * @brief Load 200x200 1-bit EPD frame buffer from RTC Slow Memory
 * 
 * @param dest Destination buffer (5000 bytes)
 * @param len Size of buffer in bytes
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_rtc_mem_load_display_frame(uint8_t *dest, size_t len);

/**
 * @brief Check if a valid display frame is stored in RTC Slow Memory
 * 
 * @return true if valid frame exists
 */
bool bsp_rtc_mem_has_display_frame(void);

/**
 * @brief Reset RTC memory structure to defaults
 */
void bsp_rtc_mem_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_RTC_MEM_H */
