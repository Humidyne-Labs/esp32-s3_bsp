#ifndef BSP_AUDIO_H
#define BSP_AUDIO_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "bsp/pinout.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize ES8311 Codec I2C control, I2S audio interface, and PA control pins
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_audio_init(void);

/**
 * @brief Enable or disable audio power amplifier (PA_EN on GPIO48)
 * 
 * @param enable true to enable amplifier output, false to mute/disable
 */
void bsp_audio_pa_enable(bool enable);

/**
 * @brief Set speaker output volume
 * 
 * @param volume Volume percentage (0.0 to 100.0)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_audio_set_volume(float volume);

/**
 * @brief Play PCM audio sample buffer over I2S
 * 
 * @param data Pointer to raw PCM audio data
 * @param len Size of data in bytes
 * @param bytes_written Pointer to store actual bytes written
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_audio_play(const void *data, size_t len, size_t *bytes_written);

#ifdef __cplusplus
}
#endif

#endif /* BSP_AUDIO_H */
