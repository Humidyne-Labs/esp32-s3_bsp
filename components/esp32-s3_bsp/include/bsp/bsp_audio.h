/**
 * @file bsp_audio.h
 * @brief MAX98357A I2S Class-D Mono Audio Amplifier Driver
 * 
 * Hardware Target:
 *  - Amplifier: Maxim Integrated MAX98357A (I2S input, Class-D PWM speaker driver)
 *  - Pinout: BCLK (GPIO 10), LRCK (GPIO 11), DOUT (GPIO 12)
 *  - Sample Rates: 8 kHz to 48 kHz (16-bit Mono/Stereo PCM)
 * 
 * @attribution
 * - Maxim Integrated / Analog Devices
 * - BSP Implementation: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_AUDIO_H
#define BSP_AUDIO_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*bsp_audio_done_cb_t)(void *arg);

/**
 * @brief Initialize I2S Master Channel for MAX98357A
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_audio_init(void);

/**
 * @brief Play Raw PCM Audio Buffer
 * 
 * @param pcm_data Pointer to 16-bit PCM audio samples
 * @param data_size Size of data buffer in bytes
 * @param callback Optional completion callback function (can be NULL)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_audio_play(const void *pcm_data, size_t data_size, bsp_audio_done_cb_t callback);

/**
 * @brief Set Software Audio Gain / Volume Scaling
 * 
 * @param volume_percent Volume from 0.0 (mute) to 100.0 (maximum)
 * @return esp_err_t ESP_OK
 */
esp_err_t bsp_audio_set_volume(float volume_percent);

/**
 * @brief Stop Active Audio Playback Immediately
 * 
 * @return esp_err_t ESP_OK
 */
esp_err_t bsp_audio_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_AUDIO_H */
