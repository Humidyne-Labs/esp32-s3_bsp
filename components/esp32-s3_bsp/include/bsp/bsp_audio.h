/**
 * @file bsp_audio.h
 * @brief ES8311 I2S Audio Codec & NS4168 Class-D Mono Amplifier Driver
 * 
 * Hardware Target:
 *  - Codec: Everest Semi ES8311 (I2C Address: 0x18)
 *  - Power Amplifier: NS4168 (Power enable GPIO 42, Control GPIO 46)
 *  - I2S Pins: MCLK (GPIO 14), SCLK (GPIO 15), ASDOUT (GPIO 16), LRCK (GPIO 38), DSDIN (GPIO 45)
 *  - Sample Rates: 8 kHz to 48 kHz (16-bit Mono PCM)
 * 
 * @attribution
 * - Everest Semiconductor / Waveshare Electronics
 * - BSP Implementation: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_AUDIO_H
#define BSP_AUDIO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*bsp_audio_done_cb_t)(void *arg);

/**
 * @brief Control Power Rail for Audio Subsystem (GPIO 42)
 * 
 * @param enable true to power on audio domain (sets GPIO 42 LOW), false to power off (GPIO 42 HIGH)
 */
void bsp_audio_power_enable(bool enable);

/**
 * @brief Initialize I2S Master Channel & ES8311 Audio Codec
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_audio_init(void);

/**
 * @brief Play Raw PCM Audio Buffer
 * 
 * @param data Pointer to 16-bit PCM audio samples
 * @param len Size of data buffer in bytes
 * @param bytes_written Optional pointer to receive actual bytes transmitted
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_audio_play(const void *data, size_t len, size_t *bytes_written);

/**
 * @brief Set Software Audio Gain / Volume Scaling
 * 
 * @param volume Volume from 0.0 (mute) to 100.0 (maximum)
 * @return esp_err_t ESP_OK
 */
esp_err_t bsp_audio_set_volume(float volume);

/**
 * @brief Stop Active Audio Playback
 * 
 * @return esp_err_t ESP_OK
 */
esp_err_t bsp_audio_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_AUDIO_H */
