/**
 * @file mmap_lvgl_decoder.h
 * @brief A custom Decoder for LVGL.
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics (https://www.waveshare.com)
 * - Microcontroller: Espressif Systems ESP32-S3 (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */
 
#ifndef MMAP_LVGL_DECODER_H
#define MMAP_LVGL_DECODER_H

#include "esp_err.h"
#include "esp_mmap_assets.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t init_drive(const char* partition_label, const char drive_letter, int max_files, uint32_t checksum);

#ifdef __cplusplus
}
#endif

#endif /* MMAP_LVGL_DECODER_H */