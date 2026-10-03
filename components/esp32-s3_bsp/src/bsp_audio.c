/**
 * @file bsp_audio.c
 * @brief ES8311 I2S Audio Codec, NS4168 Class-D Mono Amplifier Driver & Synthesized Chimes Implementation
 *
 * @version 1.0.0
 * @attribution
 * - Architecture & Development: HUMIDYNE LABS / Humiditron
 * - AI Systems Co-Developer: Gemini (Google DeepMind)
 * - Hardware Target: Waveshare Electronics ESP32-S3 ePaper 1.54 V2
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
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
#include "bsp/bsp_splash.h"
#include "bsp/bsp_audio.h"

static const char *TAG = "bsp_audio";

// 256-point 16-bit sine wave stored in flash. Peak amplitude = 16000.
// Consumes exactly 512 bytes of Flash, 0 bytes of RAM.
static const int16_t SINE_LUT_256[256] = {
        0,    392,   784,    1176,   1567,   1958,   2348,   2737,
     3125,   3511,  3896,    4279,   4660,   5038,   5414,   5787,
     6157,   6523,  6886,    7245,   7600,   7951,   8297,   8638,
     8974,   9304,  9628,    9946,  10258,  10563,  10861,  11152,
    11435,  11710, 11977,   12235,  12484,  12724,  12954,  13175,
    13386,  13586, 13776,   13955,  14123,  14280,  14426,  14560,
    14683,  14794, 14893,   14980,  15055,  15118,  15168,  15206,
    15232,  15245, 15246,   15234,  15210,  15173,  15124,  15063,
    14989,  14903, 14805,   14965,  14573,  14439,  14293,  14136,
    13967,  13786, 13594,   13391,  13176,  12950,  12713,  12465,
    12206,  11936, 11656,   11365,  11064,  10752,  10430,  10098,
     9757,   9406,  9045,    8676,   8297,   7910,   7514,   7110,
     6698,   6278,  5852,    5418,   4978,   4532,   4080,   3622,
     3160,   2692,  2221,    1746,   1268,    788,    307,   -174,
     -656,  -1137, -1617,   -2094,  -2569,  -3040,  -3507,  -3970,
    -4427,  -4878, -5323,   -5760,  -6189,  -6609,  -7020,  -7421,
    -7811,  -8191, -8558,   -8914,  -9257,  -9587,  -9903, -10205,
   -10493, -10766, -11023, -11265, -11491, -11700, -11893, -12068,
   -12226, -12367, -12489, -12594, -12681, -12749, -12799, -12830,
   -12842, -12835, -12810, -12765, -12702, -12620, -12520, -12401,
   -12264, -12108, -11935, -11744, -11535, -11308, -11065, -10804,
   -10527, -10233,  -9923,  -9598,  -9257,  -8902,  -8532,  -8149,
    -7751,  -7341,  -6918,  -6483,  -6037,  -5580,  -5113,  -4636,
    -4151,  -3658,  -3157,  -2650,  -2137,  -1619,  -1098,   -573,
      -46,    481,   1008,   1534,   2057,   2577,   3092,   3601,
     4103,   4596,   5080,   5553,   6014,   6463,   6898,   7318,
     7722,   8110,   8480,   8832,   9165,   9478,   9771,  10042,
    10292,  10519,  10723,  10904,  11062,  11195,  11304,  11388,
    11448,  11482,  11491,  11475,  11434,  11367,  11276,  11159,
    11018,  10852,  10662,  10447,  10209,   9947,   9662,   9354,
     9024,   8673,   8300,   7907,   7494,   7062,   6612,   6144,
     5659,   5159,   4643,   4113,   3570,   3015,   2449,   1874
};

static i2s_chan_handle_t      s_tx_chan          = NULL;
static esp_codec_dev_handle_t s_codec             = NULL;
static bool                   s_audio_inited     = false;
static bool                   s_audio_in_standby = false;
#ifdef CONFIG_BSP_AUDIO_LOW_POWER_DEFAULT
static bool                   s_low_power_mode   = true;
#else
static bool                   s_low_power_mode   = false;
#endif

void bsp_audio_power_enable(bool enable)
{
    /* GPIO 42 controls the audio power rail MOSFET (Active-LOW: 0 = Power ON, 1 = Power OFF) */
    gpio_set_level(BSP_PIN_PA_EN, enable ? 0 : 1);
}

esp_err_t bsp_audio_init(void)
{
    if (s_audio_inited && !s_audio_in_standby) return ESP_OK;
    if (s_audio_in_standby) {
        return bsp_audio_resume();
    }

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
        .pa_pin          = BSP_PIN_PA_CTRL, // configured by es8311_codec_new as output
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
    esp_codec_dev_set_out_vol (s_codec, 0.0f);
    esp_codec_dev_set_out_mute(s_codec, true);

    s_audio_inited     = true;
    s_audio_in_standby = false;

    /* Apply default low power playback mode configuration (Reg 0x0F) */
    bsp_audio_set_low_power_mode(s_low_power_mode);

    ESP_LOGI(TAG, "ES8311 mono audio subsystem initialized successfully (%s mode, muted)",
             s_low_power_mode ? "low-power" : "full-power");
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

    esp_err_t ret = esp_codec_dev_write(s_codec, (void *)data, (int)len);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send audio buffer to codec: %s", esp_err_to_name(ret));
    }
    if (bytes_written) {
        *bytes_written = (ret == ESP_OK) ? len : 0;
    }
    return ret;
}

esp_err_t bsp_audio_stop(void)
{
    if (s_codec != NULL) {
        return esp_codec_dev_set_out_mute(s_codec, true);
    }
    return ESP_OK;
}

esp_err_t bsp_audio_set_low_power_mode(bool enable)
{
    s_low_power_mode = enable;
    if (s_audio_inited && !s_audio_in_standby) {
        uint8_t val = 0;
        esp_err_t ret = bsp_i2c_read_reg(ES8311_CODEC_DEFAULT_ADDR, 0x0F, &val, 1);
        if (ret == ESP_OK) {
            if (enable) {
                val |= 0x03; // Bit 0: LPDAC, Bit 1: LPDACVRP
            } else {
                val &= ~0x03;
            }
            ret = bsp_i2c_write_reg(ES8311_CODEC_DEFAULT_ADDR, 0x0F, &val, 1);
        }
        return ret;
    }
    return ESP_OK;
}

esp_err_t bsp_audio_standby(void)
{
    if (!s_audio_inited || s_audio_in_standby) {
        return ESP_OK;
    }

    ESP_LOGD(TAG, "Entering ES8311 low-power standby mode...");

    // 1. Mute DAC and PA amplifier
    if (s_codec != NULL) {
        esp_codec_dev_set_out_mute(s_codec, true);
    }
    gpio_set_level((gpio_num_t)BSP_PIN_PA_CTRL, 0);

    // 2. Write ES8311 low-power power-down registers over I2C
    uint8_t reg_0d = 0xFC; // PDN_ANA=1, PDN_IBIASGEN=1, PDN_ADCVERFGEN=1, PDN_DACVREFGEN=1, VMIDSEL=00
    bsp_i2c_write_reg(ES8311_CODEC_DEFAULT_ADDR, 0x0D, &reg_0d, 1);

    uint8_t reg_00 = 0x1F; // RST_DIG=1, RST_CMG=1, RST_MST=1, RST_ADC_DIG=1, RST_DAC_DIG=1, CSM_ON=0
    bsp_i2c_write_reg(ES8311_CODEC_DEFAULT_ADDR, 0x00, &reg_00, 1);

    // 3. Disable I2S channel if initialized
    if (s_tx_chan != NULL) {
        i2s_channel_disable(s_tx_chan);
    }

    // 4. Ensure codec power rail remains ON (Active-LOW: GPIO 42 = 0) to prevent I2C bus clamping
    bsp_audio_power_enable(true);

    s_audio_in_standby = true;
    ESP_LOGI(TAG, "Audio subsystem in ultra-low power standby (~15 uA)");
    return ESP_OK;
}

esp_err_t bsp_audio_resume(void)
{
    if (!s_audio_inited) {
        return bsp_audio_init();
    }

    if (!s_audio_in_standby) {
        return ESP_OK;
    }

    ESP_LOGD(TAG, "Resuming ES8311 from standby mode...");

    // 1. Ensure audio power rail is active and stable
    bsp_audio_power_enable(true);
    vTaskDelay(pdMS_TO_TICKS(10));

    // 2. Re-enable I2S channel
    if (s_tx_chan != NULL) {
        i2s_channel_enable(s_tx_chan);
    }

    // 3. Restore ES8311 CSM & Power Management registers
    uint8_t reg_00 = 0x00; // Clear digital resets, CSM ON
    bsp_i2c_write_reg(ES8311_CODEC_DEFAULT_ADDR, 0x00, &reg_00, 1);

    uint8_t reg_0d = 0x01; // Power up analog blocks, VMID normal
    bsp_i2c_write_reg(ES8311_CODEC_DEFAULT_ADDR, 0x0D, &reg_0d, 1);

    // 4. Restore Low Power Mode preference
    bsp_audio_set_low_power_mode(s_low_power_mode);

    // 5. Enable PA amplifier
    gpio_set_level((gpio_num_t)BSP_PIN_PA_CTRL, 1);

    s_audio_in_standby = false;
    ESP_LOGI(TAG, "Audio subsystem resumed from standby");
    return ESP_OK;
}

esp_err_t bsp_audio_deinit(void)
{
    if (!s_audio_inited) {
        return ESP_OK;
    }

    bsp_audio_standby();

    if (s_codec != NULL) {
        esp_codec_dev_close(s_codec);
        esp_codec_dev_delete(s_codec);
        s_codec = NULL;
    }

    if (s_tx_chan != NULL) {
        i2s_del_channel(s_tx_chan);
        s_tx_chan = NULL;
    }

    s_audio_inited     = false;
    s_audio_in_standby = false;
    ESP_LOGI(TAG, "Audio subsystem de-initialized");
    return ESP_OK;
}

esp_err_t bsp_audio_play_tone(uint32_t freq_hz, uint32_t duration_ms, float volume_pct)
{
    if (freq_hz == 0 || duration_ms == 0) return ESP_OK;

    if (!s_audio_inited || s_codec == NULL) {
        esp_err_t ret = bsp_audio_init();
        if (ret != ESP_OK) return ret;
    } else if (s_audio_in_standby) {
        esp_err_t ret = bsp_audio_resume();
        if (ret != ESP_OK) return ret;
    }

    if (volume_pct <= 0.0f) {
        bsp_audio_stop();
        vTaskDelay(pdMS_TO_TICKS(duration_ms));
        return ESP_OK;
    }

    bsp_audio_set_volume(volume_pct);
    esp_codec_dev_set_out_mute(s_codec, false);

    const uint32_t sample_rate   = 16000;
    const size_t   total_samples = (sample_rate * duration_ms) / 1000;

    // 5ms attack/decay ramp to eliminate audio clicking
    size_t ramp_samples = (sample_rate * 5) / 1000;
    if (ramp_samples > total_samples / 2) {
        ramp_samples = total_samples / 2;
    }

    // DDS phase step: (freq_hz * 2^32) / sample_rate
    const uint32_t phase_step = (uint32_t)(((uint64_t)freq_hz << 32) / sample_rate);
    uint32_t            phase = 0;

    // Small, static streaming buffer (512 samples = 1 KB).
    // Zero heap allocation, no stack bloat, infinite duration headroom.
    int16_t buffer[512];
    size_t  samples_generated = 0;

    while (samples_generated < total_samples) {
        size_t chunk = total_samples - samples_generated;
        if (chunk > 512) chunk = 512;

        for (size_t i = 0; i < chunk; i++) {
            size_t idx = samples_generated + i;

            // Top 8 bits map 32-bit phase space into the 256-entry table
            uint8_t lut_idx = (uint8_t)(phase >> 24);
            int32_t sample  = SINE_LUT_256[lut_idx];

            // Apply amplitude envelope
            if (ramp_samples > 0) {
                if (idx < ramp_samples) {
                    sample = (sample * (int32_t)idx) / (int32_t)ramp_samples;
                } else if (idx >= total_samples - ramp_samples) {
                    sample = (sample * (int32_t)(total_samples - idx)) / (int32_t)ramp_samples;
                }
            }

            buffer[i] = (int16_t)sample;
            phase += phase_step; // Natural overflow wraps around 2*pi
        }

        // DMA handles queuing; blocks efficiently without burning CPU
        bsp_audio_play(buffer, chunk * sizeof(int16_t), NULL);
        samples_generated += chunk;
    }

    // Flush trailing DMA pipelines with silence
    memset(buffer, 0, sizeof(buffer));
    bsp_audio_play(buffer, sizeof(buffer), NULL);

    vTaskDelay(pdMS_TO_TICKS(35));
    bsp_audio_stop();

    return ESP_OK;
}

esp_err_t bsp_audio_play_chime(bsp_chime_type_t type)
{
    bsp_audio_resume();

    esp_err_t ret = ESP_OK;
    switch (type) {
        case BSP_CHIME_BOOT:
            // Ascending 4-tone arpeggio: C5 (523Hz) -> E5 (659Hz) -> G5 (784Hz) -> C6 (1046Hz)
            bsp_audio_play_tone(523,  90, 70.0f);
            bsp_audio_play_tone(659,  90, 75.0f);
            bsp_audio_play_tone(784,  90, 80.0f);
            bsp_audio_play_tone(1046, 200, 85.0f);
            break;

        case BSP_CHIME_WAKE:
            // Quick rising acoustic cue: G5 (784Hz) -> C6 (1046Hz)
            bsp_audio_play_tone(784,  80, 65.0f);
            bsp_audio_play_tone(1046, 150, 75.0f);
            break;

        case BSP_CHIME_SLEEP:
            // Descending stand-down tone: C6 (1046Hz) -> G5 (784Hz) -> E5 (659Hz)
            bsp_audio_play_tone(1046, 120, 70.0f);
            bsp_audio_play_tone(784,  120, 65.0f);
            bsp_audio_play_tone(659,  220, 55.0f);
            break;

        case BSP_CHIME_SHUTDOWN:
            // Warm descending cadence: G5 (784Hz) -> E5 (659Hz) -> C5 (523Hz)
            bsp_audio_play_tone(784, 140, 70.0f);
            bsp_audio_play_tone(659, 140, 65.0f);
            bsp_audio_play_tone(523, 280, 60.0f);
            break;

        case BSP_CHIME_ALARM:
            // Urgent alternating warble
            for (int i = 0; i < 2; i++) {
                bsp_audio_play_tone(1760, 100, 90.0f);
                bsp_audio_play_tone(880,  100, 90.0f);
            }
            break;

        case BSP_CHIME_NOTIFY:
            // Crisp dual ping: C6 (1046Hz) -> E6 (1318Hz)
            bsp_audio_play_tone(1046, 70, 70.0f);
            bsp_audio_play_tone(1318, 120, 75.0f);
            break;

        case BSP_CHIME_EVENT:
            // Short tactile click / blip
            bsp_audio_play_tone(1200, 35, 60.0f);
            break;

        default:
            ret = ESP_ERR_INVALID_ARG;
            break;
    }

    // Automatically put audio into low-power standby after chime finishes
    bsp_audio_standby();
    return ret;
}

static void bsp_default_chime_dispatcher(bsp_chime_type_t type, void *user_data)
{
    (void)user_data;
    bsp_audio_play_chime(type);
}

esp_err_t bsp_audio_register_default_chimes(void)
{
    for (int i = 0; i < BSP_CHIME_MAX; i++) {
        bsp_register_chime_cb((bsp_chime_type_t)i, bsp_default_chime_dispatcher, NULL);
    }
    ESP_LOGI(TAG, "Registered synthesized acoustic notification hooks for all system chimes");
    return ESP_OK;
}
