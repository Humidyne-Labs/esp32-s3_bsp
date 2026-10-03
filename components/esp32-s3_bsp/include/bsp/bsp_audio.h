/**
 * @file bsp_audio.h
 * @brief ES8311 I2S Audio Codec, NS4168 Class-D Mono Amplifier Driver & Synthesized Chimes
 *
 * Hardware Target:
 *  - Codec: Everest Semi ES8311 (I2C Address: 0x18)
 *  - Power Amplifier: NS4168 (Power enable GPIO 42, Control GPIO 46)
 *  - I2S Pins: MCLK (GPIO 14), SCLK (GPIO 15), ASDOUT (GPIO 16), LRCK (GPIO 38), DSDIN (GPIO 45)
 *  - Sample Rates: 8 kHz to 48 kHz (16-bit Mono PCM, 16 kHz default)
 *
 * Features:
 *  - Zero-heap Direct Digital Synthesis (DDS) sine wave tone generator
 *  - Built-in multi-tone acoustic notification chimes for system events
 *  - Automated callback hook registration for boot, wake, sleep, shutdown, and alarms
 *  - Power-domain management and amplifier mute controls to prevent I2C bus clamping
 *
 * @version 1.0.0
 * @attribution
 * - Architecture & Development: HUMIDYNE LABS / Humiditron
 * - AI Systems Co-Developer: Gemini (Google DeepMind)
 * - Hardware Target: Waveshare Electronics ESP32-S3 ePaper 1.54 V2
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_AUDIO_H
#define BSP_AUDIO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"
#include "bsp/bsp_splash.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Audio playback completion callback type
 */
typedef void (*bsp_audio_done_cb_t)(void *arg);

/**
 * @brief Control Power Rail for Audio Subsystem (GPIO 42)
 *
 * @param[in] enable true to power on audio domain (sets GPIO 42 LOW), false to power off (GPIO 42 HIGH)
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
void bsp_audio_power_enable(bool enable);

/**
 * @brief Initialize I2S Master Channel & ES8311 Audio Codec
 *
 * Configures I2S0 master TX channel at 16 kHz 16-bit mono, initializes the ES8311 codec
 * via the shared I2C bus, enables the NS4168 power amplifier, and starts in muted state.
 *
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_audio_init(void);

/**
 * @brief Play Raw PCM Audio Buffer
 *
 * @param[in] data Pointer to 16-bit PCM audio samples
 * @param[in] len Size of data buffer in bytes
 * @param[out] bytes_written Optional pointer to receive actual bytes transmitted
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_audio_play(const void *data, size_t len, size_t *bytes_written);

/**
 * @brief Set Software Audio Gain / Volume Scaling
 *
 * @param[in] volume Volume level from 0.0 (mute) to 100.0 (maximum)
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_audio_set_volume(float volume);

/**
 * @brief Play Synthesized Sine Tone
 *
 * Generates a smooth sine wave tone using a 256-point lookup table with 5ms attack/decay
 * envelope ramps to eliminate acoustic popping.
 *
 * @param[in] freq_hz Frequency in Hertz (e.g. 440, 523, 1046)
 * @param[in] duration_ms Duration in milliseconds
 * @param[in] volume_pct Volume percentage from 0.0 (muted) to 100.0 (maximum)
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_audio_play_tone(uint32_t freq_hz, uint32_t duration_ms, float volume_pct);

/**
 * @brief Play Built-in Synthesized Acoustic System Chime / Notification Sound
 *
 * Synthesizes structured melodic tone sequences tailored for system events:
 *  - BSP_CHIME_BOOT: Ascending 4-tone melodic arpeggio (C5 -> E5 -> G5 -> C6)
 *  - BSP_CHIME_WAKE: Quick rising wake cue (G5 -> C6)
 *  - BSP_CHIME_SLEEP: Descending stand-down cadence (C6 -> G5 -> E5)
 *  - BSP_CHIME_SHUTDOWN: Warm descending shutdown tone (G5 -> E5 -> C5)
 *  - BSP_CHIME_ALARM: High-urgency alternating warning warble (1760 Hz / 880 Hz)
 *  - BSP_CHIME_NOTIFY: Dual-ping notification chirp (1046 Hz -> 1318 Hz)
 *  - BSP_CHIME_EVENT: Tactile click feedback blip (1200 Hz)
 *
 * @param[in] type Chime event type
 * @return esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG on unknown chime type
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_audio_play_chime(bsp_chime_type_t type);

/**
 * @brief Register Built-in Chimes with the System Notification Dispatcher
 *
 * Automatically connects all bsp_chime_type_t event types to bsp_audio_play_chime(),
 * enabling out-of-the-box acoustic feedback on boot, wake, sleep, shutdown, alarms, and UI clicks.
 *
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_audio_register_default_chimes(void);

/**
 * @brief Stop Active Audio Playback and Mute Amplifier
 *
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_audio_stop(void);

/**
 * @brief Enter Ultra-Low Power Standby Mode (~15 uA)
 *
 * Mutes power amplifier, powers down ES8311 analog/digital blocks via I2C,
 * and halts I2S output channel while leaving codec power rail powered (to prevent I2C bus clamping).
 *
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_audio_standby(void);

/**
 * @brief Resume ES8311 Codec from Standby Mode
 *
 * Restores ES8311 analog/digital power registers, configures low power mode,
 * re-enables I2S TX channel, and enables NS4168 power amplifier.
 *
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_audio_resume(void);

/**
 * @brief Enable or Disable ES8311 Low-Power Playback Mode (Reg 0x0F)
 *
 * Sets LPDAC and LPDACVRP bits in ES8311 Reg 0x0F to reduce active DAC current draw by ~30%.
 *
 * @param[in] enable true to enable low power playback mode, false for full power
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_audio_set_low_power_mode(bool enable);

/**
 * @brief Completely De-initialize Audio Subsystem and Release Resources
 *
 * Places ES8311 into standby, disables and deletes I2S channel, and frees codec device handle.
 *
 * @return esp_err_t ESP_OK on success
 * @details Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.
 */
esp_err_t bsp_audio_deinit(void);

#ifdef __cplusplus

}
#endif

#endif /* BSP_AUDIO_H */
