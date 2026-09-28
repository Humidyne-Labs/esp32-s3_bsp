/**
 * @file bsp_splash.h
 * @brief System UI Splash Screens & Audio Chime / Notification Callback Framework
 * 
 * Provides centralized callback registration and event dispatching for:
 *  - Boot Splash screen & chime (Cold boot)
 *  - Wake Splash screen & chime (Deep/Light sleep resume)
 *  - Sleep Splash screen & chime (Pre-sleep stand-down)
 *  - Shutdown Splash screen & chime (Power off / Space Cat)
 *  - Environmental / Alarm / Notification acoustic signals
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
    BSP_SPLASH_WAKE     = 1, /*!< Rendered upon resuming from sleep                   */
    BSP_SPLASH_SLEEP    = 2, /*!< Rendered prior to entering deep/light sleep         */
    BSP_SPLASH_SHUTDOWN = 3, /*!< Rendered prior to system power off (e.g. Space Cat) */
    BSP_SPLASH_MAX      = 4
} bsp_splash_type_t;

/**
 * @brief Audio Chime / Notification Event Types
 */
typedef enum {
    BSP_CHIME_BOOT     = 0, /*!< Bootup melodic chime                 */
    BSP_CHIME_WAKE     = 1, /*!< Wake from sleep acoustic cue         */
    BSP_CHIME_SLEEP    = 2, /*!< Sleep / stand-down descending tone   */
    BSP_CHIME_SHUTDOWN = 3, /*!< Power off tone                       */
    BSP_CHIME_ALARM    = 4, /*!< Critical telemetry / threshold alarm */
    BSP_CHIME_NOTIFY   = 5, /*!< General notification chirp           */
    BSP_CHIME_EVENT    = 6, /*!< User action / UI event click tone    */
    BSP_CHIME_MAX      = 7
} bsp_chime_type_t;

/**
 * @brief UI Splash Screen Callback Signature
 * 
 * @param type Splash event type
 * @param user_data User context pointer passed during registration
 */
typedef void (*bsp_splash_cb_t)(bsp_splash_type_t type, void *user_data);

/**
 * @brief Audio Chime Callback Signature
 * 
 * @param type Chime event type
 * @param user_data User context pointer passed during registration
 */
typedef void (*bsp_chime_cb_t)(bsp_chime_type_t type, void *user_data);

/**
 * @brief Register a callback for a specific UI Splash event
 * 
 * @param type Splash type (BOOT, WAKE, SLEEP, SHUTDOWN)
 * @param cb Callback function pointer
 * @param user_data Optional user context pointer
 * @return esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG on invalid type
 */
esp_err_t bsp_register_splash_cb(bsp_splash_type_t type, bsp_splash_cb_t cb, void *user_data);

/**
 * @brief Unregister a callback for a specific UI Splash event
 * 
 * @param type Splash type (BOOT, WAKE, SLEEP, SHUTDOWN)
 * @return esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG on invalid type
 */
esp_err_t bsp_unregister_splash_cb(bsp_splash_type_t type);

/**
 * @brief Register a callback for a specific Audio Chime event
 * 
 * @param type Chime type (BOOT, WAKE, SLEEP, SHUTDOWN, ALARM, NOTIFY, EVENT)
 * @param cb Callback function pointer
 * @param user_data Optional user context pointer
 * @return esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG on invalid type
 */
esp_err_t bsp_register_chime_cb(bsp_chime_type_t type, bsp_chime_cb_t cb, void *user_data);

/**
 * @brief Unregister a callback for a specific Audio Chime event
 * 
 * @param type Chime type (BOOT, WAKE, SLEEP, SHUTDOWN, ALARM, NOTIFY, EVENT)
 * @return esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG on invalid type
 */
esp_err_t bsp_unregister_chime_cb(bsp_chime_type_t type);

/**
 * @brief Check if a UI Splash callback is registered for a given event type
 * 
 * @param type Splash type
 * @return true if callback is registered, false otherwise
 */
bool bsp_has_splash_cb(bsp_splash_type_t type);

/**
 * @brief Check if an Audio Chime callback is registered for a given event type
 * 
 * @param type Chime type
 * @return true if callback is registered, false otherwise
 */
bool bsp_has_chime_cb(bsp_chime_type_t type);

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
