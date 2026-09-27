/**
 * @file bsp_audio.c
 * @brief ES8311 I2S Audio Codec & NS4168 Class-D Mono Amplifier Driver Implementation
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics (https://www.waveshare.com)
 * - Microcontroller: Espressif Systems ESP32-S3 (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2s_std.h"
#include "esp_log.h"
#include "esp_codec_dev.h"
#include "esp_codec_dev_defaults.h"
#include "bsp/pinout.h"
#include "bsp/bsp_i2c.h"
#include "bsp/bsp_audio.h"

static const char *TAG = "bsp_audio";

static i2s_chan_handle_t      s_tx_chan      = NULL;
static esp_codec_dev_handle_t s_codec        = NULL;
static bool                   s_audio_inited = false;

void bsp_audio_power_enable(bool enable)
{
    /* GPIO 42 controls the audio power rail MOSFET (Active-LOW: 0 = Power ON, 1 = Power OFF) */
    gpio_set_level(BSP_PIN_PA_EN, enable ? 0 : 1);
}

esp_err_t bsp_audio_init(void)
{
    if (s_audio_inited) return ESP_OK;

    /* 1. Power ON the audio domain and allow rail to settle */
    bsp_audio_power_enable(true);
    vTaskDelay(pdMS_TO_TICKS(50));

    /* 2. Initialize I2S Channels (TX Output Channel as Primary Master) */
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.auto_clear        = true;

    esp_err_t ret = i2s_new_channel(&chan_cfg, &s_tx_chan, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2S TX channel: %s", esp_err_to_name(ret));
        return ret;
    }

    /* 3. Configure Mono I2S Timing & Clocking */
    i2s_std_config_t std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = BSP_PIN_I2S_MCLK,
            .bclk = BSP_PIN_I2S_SCLK,
            .ws   = BSP_PIN_I2S_LRCK,
            .dout = BSP_PIN_I2S_DSDIN,
            .din  = BSP_PIN_I2S_ASDOUT,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };

    /* ES8311 Mono DAC expects audio data on the Left Slot */
    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;

    ret = i2s_channel_init_std_mode(s_tx_chan, &std_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init I2S TX std mode: %s", esp_err_to_name(ret));
        return ret;
    }
    i2s_channel_enable(s_tx_chan);

    /* 4. Setup ES8311 Codec Control & Data Interfaces */
    audio_codec_i2c_cfg_t i2c_cfg = {
        .port       = I2C_NUM_0,
        .addr       = ES8311_CODEC_DEFAULT_ADDR,
        .bus_handle = bsp_i2c_get_handle(),
    };
    if (i2c_cfg.bus_handle == NULL) {
        ESP_LOGE(TAG, "Shared I2C bus is not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    audio_codec_i2s_cfg_t i2s_cfg = {
        .port      = I2S_NUM_0,
        .tx_handle = s_tx_chan,
        .rx_handle = NULL,
    };
    const audio_codec_data_if_t *data_if = audio_codec_new_i2s_data(&i2s_cfg);
    const audio_codec_ctrl_if_t *ctrl_if = audio_codec_new_i2c_ctrl(&i2c_cfg);
    const audio_codec_gpio_if_t *gpio_if = audio_codec_new_gpio();
    if (data_if == NULL || ctrl_if == NULL || gpio_if == NULL) {
        ESP_LOGE(TAG, "Failed to create codec interface abstractions");
        return ESP_ERR_NO_MEM;
    }

    /* 5. Instantiate ES8311 Driver with Mono Speaker & PA on GPIO 46 */
    es8311_codec_cfg_t codec_cfg = {
        .codec_mode      = ESP_CODEC_DEV_WORK_MODE_DAC,
        .ctrl_if         = ctrl_if,
        .gpio_if         = gpio_if,
        .pa_pin          = BSP_PIN_PA_CTRL, //configured by es8311_codec_new as output
        .use_mclk        = true,
        .hw_gain.pa_gain = 6.0f,
    };
    const audio_codec_if_t *codec_if = es8311_codec_new(&codec_cfg);
    if (codec_if == NULL) {
        ESP_LOGE(TAG, "Failed to probe and create ES8311 codec over I2C");
        return ESP_FAIL;
    }

    esp_codec_dev_cfg_t dev_cfg = {
        .codec_if = codec_if,
        .data_if  = data_if,
        .dev_type = ESP_CODEC_DEV_TYPE_OUT,
    };
    s_codec = esp_codec_dev_new(&dev_cfg);
    if (s_codec == NULL) {
        ESP_LOGE(TAG, "Failed to create ES8311 codec device");
        return ESP_FAIL;
    }

    /* 6. Configure Mono Sample Attributes */
    esp_codec_dev_sample_info_t sample_info = {
        .sample_rate     = 16000,
        .channel         = 1,      // Mono Channel
        .bits_per_sample = 16,     // 16-bit PCM
    };
    ret = esp_codec_dev_open(s_codec, &sample_info);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open ES8311: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Keep DAC output muted on startup to prevent pop / phantom beeps */
    esp_codec_dev_set_out_vol(s_codec, 0.0f);
    esp_codec_dev_set_out_mute(s_codec, true);

    s_audio_inited = true;
    ESP_LOGI(TAG, "ES8311 mono audio subsystem initialized successfully (muted)");
    return ESP_OK;
}

esp_err_t bsp_audio_set_volume(float volume)
{
    if (volume < 0.0f)   volume = 0.0f;
    if (volume > 100.0f) volume = 100.0f;

    if (!s_audio_inited || s_codec == NULL) {
        esp_err_t ret = bsp_audio_init();
        if (ret != ESP_OK) return ret;
    }

    esp_err_t ret = esp_codec_dev_set_out_vol(s_codec, volume);
    if (ret == ESP_OK) {
        ret = esp_codec_dev_set_out_mute(s_codec, volume <= 0.0f);
    }
    return ret;
}

esp_err_t bsp_audio_play(const void *data, size_t len, size_t *bytes_written)
{
    if (data == NULL || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!s_audio_inited || s_codec == NULL) {
        esp_err_t ret = bsp_audio_init();
        if (ret != ESP_OK) return ret;
    }

    const uint8_t *cursor = (const uint8_t *)data;
    size_t remaining     = len;
    size_t total_written = 0;
    while (remaining > 0) {
        size_t chunk_len = remaining > 256 ? 256 : remaining;
        esp_err_t ret = esp_codec_dev_write(s_codec, (void *)cursor, chunk_len);
        if (ret != ESP_OK) {
            if (bytes_written != NULL) {
                *bytes_written = total_written;
            }
            return ret;
        }
        cursor        += chunk_len;
        remaining     -= chunk_len;
        total_written += chunk_len;
    }

    if (bytes_written != NULL) {
        *bytes_written = total_written;
    }
    return ESP_OK;
}

esp_err_t bsp_audio_stop(void)
{
    if (s_codec != NULL) {
        return esp_codec_dev_set_out_mute(s_codec, true);
    }
    return ESP_OK;
}

esp_err_t bsp_audio_play_tone(uint32_t freq_hz, uint32_t duration_ms, float volume_pct)
{
    if (freq_hz == 0 || duration_ms == 0) return ESP_OK;

    if (!s_audio_inited || s_codec == NULL) {
        esp_err_t ret = bsp_audio_init();
        if (ret != ESP_OK) return ret;
    }

    if (volume_pct <= 0.0f) {
        bsp_audio_stop();
        vTaskDelay(pdMS_TO_TICKS(duration_ms));
        return ESP_OK;
    }

    bsp_audio_set_volume(volume_pct);
    esp_codec_dev_set_out_mute(s_codec, false);

    const uint32_t sample_rate = 16000;
    size_t total_samples = (sample_rate * duration_ms) / 1000;
    size_t ramp_samples = (sample_rate * 5) / 1000; // 5ms attack & decay ramp
    if (ramp_samples > total_samples / 2) {
        ramp_samples = total_samples / 2;
    }

    int16_t sample_buffer[256];
    size_t samples_generated = 0;
    float phase = 0.0f;
    float phase_increment = (2.0f * 3.14159265f * (float)freq_hz) / (float)sample_rate;

    while (samples_generated < total_samples) {
        size_t chunk = (total_samples - samples_generated > 256) ? 256 : (total_samples - samples_generated);
        for (size_t i = 0; i < chunk; i++) {
            size_t idx = samples_generated + i;
            float gain = 1.0f;
            if (idx < ramp_samples && ramp_samples > 0) {
                gain = (float)idx / (float)ramp_samples;
            } else if (idx >= total_samples - ramp_samples && ramp_samples > 0) {
                gain = (float)(total_samples - idx) / (float)ramp_samples;
            }
            sample_buffer[i] = (int16_t)(sinf(phase) * 16000.0f * gain);
            phase += phase_increment;
            if (phase >= 2.0f * 3.14159265f) phase -= 2.0f * 3.14159265f;
        }
        bsp_audio_play(sample_buffer, chunk * sizeof(int16_t), NULL);
        samples_generated += chunk;
    }

    // Flush DMA pipeline with silence samples to prevent cutting off trailing waveform
    memset(sample_buffer, 0, sizeof(sample_buffer));
    bsp_audio_play(sample_buffer, sizeof(sample_buffer), NULL);
    bsp_audio_play(sample_buffer, sizeof(sample_buffer), NULL);

    // Allow hardware DMA to finish playing silence before muting
    vTaskDelay(pdMS_TO_TICKS(35));

    bsp_audio_stop();
    return ESP_OK;
}