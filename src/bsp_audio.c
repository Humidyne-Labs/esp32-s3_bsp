/**
 * @file bsp_audio.c
 * @brief audio lib
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2s_std.h"
#include "esp_log.h"
#include "esp_codec_dev.h"
#include "esp_codec_dev_defaults.h"
#include "bsp/bsp_i2c.h"
#include "bsp/bsp_audio.h"

static const char *TAG = "bsp_audio";

static i2s_chan_handle_t      s_tx_chan      = NULL;
static esp_codec_dev_handle_t s_codec        = NULL;
static bool                   s_audio_inited = false;

void bsp_audio_power_enable(bool enable)
{
    /* GPIO 42 controls the audio power rail MOSFET (Active-LOW: 0 = Power ON) */
    gpio_set_level((gpio_num_t)BSP_GPIO_PA_EN, enable ? 0 : 1);
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

    /* Create TX playback channel (Set &s_rx_chan to NULL if microphone recording is not required,
       which completely eliminates the slave warning and saves DMA memory) */
    esp_err_t ret = i2s_new_channel(&chan_cfg, &s_tx_chan, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2S TX channel: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Configure Mono I2S Timing & Clocking */
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = (gpio_num_t)BSP_GPIO_I2S_MCLK,
            .bclk = (gpio_num_t)BSP_GPIO_I2S_SCLK,
            .ws   = (gpio_num_t)BSP_GPIO_I2S_LRCK,
            .dout = (gpio_num_t)BSP_GPIO_I2S_DSIN,
            .din  = (gpio_num_t)BSP_GPIO_I2S_ASOUT,
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
        .port = I2C_NUM_0,
        .addr = ES8311_CODEC_DEFAULT_ADDR,
        .bus_handle = bsp_i2c_get_handle(),
    };
    if (i2c_cfg.bus_handle == NULL) {
        ESP_LOGE(TAG, "Shared I2C bus is not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    audio_codec_i2s_cfg_t i2s_cfg = {
        .port = I2S_NUM_0,
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
        .pa_pin          = BSP_GPIO_PA_CTRL, // GPIO 46
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
        .sample_rate = 16000,
        .channel = 1,              // Mono Channel
        .bits_per_sample = 16,     // 16-bit PCM
    };
    ret = esp_codec_dev_open(s_codec, &sample_info);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open ES8311: %s", esp_err_to_name(ret));
        return ret;
    }

    esp_codec_dev_set_out_vol(s_codec, 80.0f);

    s_audio_inited = true;
    ESP_LOGI(TAG, "ES8311 mono audio subsystem initialized successfully");
    return ESP_OK;
}

esp_err_t bsp_audio_set_volume(float volume)
{
    if (volume < 0.0f) volume = 0.0f;
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
    size_t remaining = len;
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
        cursor += chunk_len;
        remaining -= chunk_len;
        total_written += chunk_len;
    }

    if (bytes_written != NULL) {
        *bytes_written = total_written;
    }
    return ESP_OK;
}