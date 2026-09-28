/**
 * @file bsp_splash.h
 * @brief System UI Splash Screens & Audio Chime / Notification Callback Framework
 * 
 * Provides centralized callback registration for:
 *  - Boot Splash screen & chime
 *  - Sleep Splash screen & chime
 *  - Shutdown Splash screen & chime
 *  - Environmental / Alarm notification signals
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

#ifndef BSP_SPLASH_H
#define BSP_SPLASH_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief UI Splash Screen Types
 */
typedef enum {
    BSP_SPLASH_BOOT     = 0, /*!< Rendered upon system cold boot                      */
    BSP_SPLASH_SLEEP    = 1, /*!< Rendered prior to entering deep/light sleep         */
    BSP_SPLASH_SHUTDOWN = 2, /*!< Rendered prior to system power off (e.g. Space Cat) */
    BSP_SPLASH_MAX      = 3
} bsp_splash_type_t;

/**
 * @brief Audio Chime / Signal Event Types
 */
typedef enum {
    BSP_CHIME_BOOT     = 0, /*!< Bootup melodic chime                 */
    BSP_CHIME_SLEEP    = 1, /*!< Sleep / stand-down tone              */
    BSP_CHIME_SHUTDOWN = 2, /*!< Power off tone                       */
    BSP_CHIME_ALARM    = 3, /*!< Critical telemetry / threshold alarm */
    BSP_CHIME_NOTIFY   = 4, /*!< General notification chirp           */
    BSP_CHIME_MAX      = 5
} bsp_chime_type_t;

/**
 * @brief UI Splash Screen Callback Signature
 */
typedef void (*bsp_splash_cb_t)(bsp_splash_type_t type, void *user_data);

/**
 * @brief Audio Chime Callback Signature
 */
typedef void (*bsp_chime_cb_t)(bsp_chime_type_t type, void *user_data);

/**
 * @brief Register a callback for a specific UI Splash event
 * 
 * @param type Splash type (BOOT, SLEEP, SHUTDOWN)
 * @param cb Callback function pointer
 * @param user_data Optional user context pointer
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_register_splash_cb(bsp_splash_type_t type, bsp_splash_cb_t cb, void *user_data);

/**
 * @brief Register a callback for a specific Audio Chime event
 * 
 * @param type Chime type (BOOT, SLEEP, SHUTDOWN, ALARM, NOTIFY)
 * @param cb Callback function pointer
 * @param user_data Optional user context pointer
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_register_chime_cb(bsp_chime_type_t type, bsp_chime_cb_t cb, void *user_data);

/**
 * @brief Trigger the registered UI Splash callback
 * 
 * @param type Splash type to invoke
 * @return esp_err_t ESP_OK if callback was executed, ESP_ERR_NOT_FOUND if no callback registered
 */
esp_err_t bsp_trigger_splash(bsp_splash_type_t type);

/**
 * @brief Trigger the registered Audio Chime callback
 * 
 * @param type Chime type to invoke
 * @return esp_err_t ESP_OK if callback was executed, ESP_ERR_NOT_FOUND if no callback registered
 */
esp_err_t bsp_trigger_chime(bsp_chime_type_t type);

#ifdef __cplusplus
}
#endif

#endif /* BSP_SPLASH_H */
