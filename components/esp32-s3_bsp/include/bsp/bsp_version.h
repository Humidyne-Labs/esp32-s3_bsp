/**
 * @file bsp_version.h
 * @brief Board Support Package Version Information & Release Tracking
 *
 * Hardware Target:
 *  - Microcontroller: Espressif Systems ESP32-S3-PICO-1-N8R8
 *  - Target Board: Waveshare ESP32-S3 ePaper 1.54 V2
 *
 * @version 1.0.0
 * @attribution
 * - Architecture & Development: HUMIDYNE LABS / Humiditron
 * - AI Systems Co-Developer: Gemini (Google DeepMind)
 * - Hardware Target: Waveshare Electronics ESP32-S3 ePaper 1.54 V2
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_VERSION_H
#define BSP_VERSION_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_VERSION_MAJOR   1
#define BSP_VERSION_MINOR   1
#define BSP_VERSION_PATCH   0
#define BSP_VERSION_STRING  "1.1.0"

#define BSP_VERSION_VAL(major, minor, patch) (((major) << 16) | ((minor) << 8) | (patch))
#define BSP_CURRENT_VERSION BSP_VERSION_VAL(BSP_VERSION_MAJOR, BSP_VERSION_MINOR, BSP_VERSION_PATCH)

/**
 * @brief Get BSP SemVer semantic version string
 *
 * @return const char* String formatted as "MAJOR.MINOR.PATCH"
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
const char *bsp_get_version(void);

/**
 * @brief Get BSP version as integer representation
 *
 * @return uint32_t Version encoded as (MAJOR << 16) | (MINOR << 8) | PATCH
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
uint32_t bsp_get_version_val(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_VERSION_H */
