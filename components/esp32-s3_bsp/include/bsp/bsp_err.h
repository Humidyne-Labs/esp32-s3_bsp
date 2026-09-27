/**
 * @file bsp_err.h
 * @brief Board Support Package Error Codes & System Diagnostics API
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

#ifndef BSP_ERR_H
#define BSP_ERR_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Custom BSP Error Code Space (Base: 0x8000) */
#define BSP_ERR_BASE                    0x8000
#define BSP_ERR_NOT_INITIALIZED         (BSP_ERR_BASE + 1)  /*!< Requested subsystem is not initialized */
#define BSP_ERR_I2C_BUS_LOCKED          (BSP_ERR_BASE + 2)  /*!< Shared I2C bus mutex acquisition timed out */
#define BSP_ERR_SENSOR_CRC_FAIL         (BSP_ERR_BASE + 3)  /*!< SHTC3 sensor CRC checksum verification failed */
#define BSP_ERR_DISPLAY_BUSY_TIMEOUT    (BSP_ERR_BASE + 4)  /*!< E-Paper display busy signal timed out */
#define BSP_ERR_AUDIO_NOT_READY         (BSP_ERR_BASE + 5)  /*!< ES8311 codec or I2S channel not configured */
#define BSP_ERR_SD_CARD_MOUNT           (BSP_ERR_BASE + 6)  /*!< MicroSD card failed to mount filesystem */
#define BSP_ERR_WIFI_DISCONNECTED       (BSP_ERR_BASE + 7)  /*!< Wi-Fi interface is disconnected or down */
#define BSP_ERR_OTA_VALIDATION          (BSP_ERR_BASE + 8)  /*!< OTA firmware binary verification failed */

/**
 * @brief System Diagnostic Information Snapshot
 */
typedef struct {
    const char *bsp_version;            /*!< BSP SemVer version string */
    const char *chip_model;             /*!< MCU Silicon Model (e.g. "ESP32-S3") */
    const char *chip_revision_str;      /*!< MCU Silicon Revision string (e.g. "v0.2") */
    uint16_t   chip_revision;           /*!< MCU Silicon Revision (major * 100 + minor) */
    uint8_t    chip_cores;              /*!< MCU CPU Core count */
    uint32_t   free_internal_heap;      /*!< Free internal SRAM heap in bytes */
    uint32_t   min_free_internal_heap;  /*!< Minimum historical free internal SRAM in bytes */
    uint32_t   free_psram_heap;         /*!< Free external PSRAM in bytes */
    uint32_t   uptime_seconds;          /*!< Time elapsed since system boot in seconds */
    uint32_t   battery_mv;              /*!< Measured battery voltage in millivolts */
    int8_t     battery_percentage;      /*!< Calculated battery state of charge (0-100%) */
    bool       power_rail_good;         /*!< Power latch active state */
    bool       i2c_bus_healthy;         /*!< SHTC3 & RTC I2C response state */
    bool       display_ready;           /*!< Display controller SPI initialization state */
    bool       wifi_connected;          /*!< Wi-Fi station link state */
    int8_t     wifi_rssi;               /*!< Wi-Fi RSSI signal strength (dBm) */
} bsp_diag_info_t;

/**
 * @brief Get MCU Silicon Revision numbers (Major and Minor)
 * 
 * @param[out] major Major wafer revision
 * @param[out] minor Minor wafer revision
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_get_chip_revision(uint32_t *major, uint32_t *minor);

/**
 * @brief Get MCU Silicon Revision formatted string (e.g. "v0.2")
 * 
 * @return const char* Revision string
 */
const char *bsp_get_chip_revision_str(void);

/**
 * @brief Translate BSP error code to human-readable error name string
 * 
 * @param err Error code
 * @return const char* String representation of error
 */
const char *bsp_err_to_name(esp_err_t err);

/**
 * @brief Populate a runtime system diagnostics snapshot
 * 
 * @param diag Pointer to bsp_diag_info_t struct to populate
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_get_diagnostics(bsp_diag_info_t *diag);

/**
 * @brief Print formatted system diagnostic report to stdout/ESP_LOG
 */
void bsp_diagnostics_dump(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_ERR_H */
