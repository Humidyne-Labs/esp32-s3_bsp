/**
 * @file bsp_audio.c
 * @brief Standalone ES8311 I2S Audio Codec & NS4168 Amp Driver (Direct Hardware Layer)
 *
 * Fully integrated with Espressif ES8311 reference initialization, stereo frame duplication,
 * and zero-heat deep power down.
 *
 * @version 3.6.0
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
#include "sdkconfig.h"
#include "bsp/pinout.h"
#include "bsp/bsp_i2c.h"
#include "bsp/bsp_splash.h"
#include "bsp/bsp_audio.h"

/* ====================================================================
 * Everest Semiconductor ES8311 Register Definitions (7-bit Addr: 0x18)
 * ==================================================================== */

#define ES8311_I2C_ADDR         BSP_I2C_ADDR_ES8311  /* 0x18 */

#define ES8311_REG_RESET        0x00  /* State Machine & Soft Resets                */
#define ES8311_REG_CLK_MGR1     0x01  /* MCLK source (Bit7:0=MCLK,1=BCLK), clk on   */
#define ES8311_REG_CLK_MGR2     0x02  /* Pre-divider & internal clock divide        */
#define ES8311_REG_CLK_MGR3     0x03  /* ADC clock divider / OSR                    */
#define ES8311_REG_CLK_MGR4     0x04  /* DAC digital clock divider / OSR            */
#define ES8311_REG_CLK_MGR5     0x05  /* DAC/ADC digital clock divider              */
#define ES8311_REG_CLK_MGR6     0x06  /* BCLK master clock divider                  */
#define ES8311_REG_CLK_MGR7     0x07  /* LRCK master clock divider bit 11:8         */
#define ES8311_REG_CLK_GATE     0x08  /* Digital clock gating / LRCK master bit 7:0 */
#define ES8311_REG_SDP_IN       0x09  /* DAC Serial Data Port (I2S format)          */
#define ES8311_REG_SDP_OUT      0x0A  /* ADC Serial Data Port (I2S format)          */
#define ES8311_REG_PWR_UP_A     0x0B  /* Power-Up Stage A delay                     */
#define ES8311_REG_SYSTEM_0C    0x0C  /* DAC channel select & routing               */
#define ES8311_REG_SYS_PWR_ANA  0x0D  /* Analog blocks, VREF, & VMID control        */
#define ES8311_REG_SYS_PGA_MOD  0x0E  /* PGA & ADC Modulator Power Down             */
#define ES8311_REG_SYS_LP_MOD   0x0F  /* Subsystem Low-Power overrides              */
#define ES8311_REG_SYS_BIAS     0x10  /* Analog bias current generator adjust       */
#define ES8311_REG_SYS_VMID     0x11  /* VMID buffer impedance & ramp               */
#define ES8311_REG_DAC_PWR      0x12  /* DAC analog power stage & output driver     */
#define ES8311_REG_SYS_PWR3     0x13  /* System internal reference bias             */
#define ES8311_REG_ADC_PWR      0x14  /* ADC analog power, PGA input select         */
#define ES8311_REG_ADC_GAIN     0x15  /* ADC PGA gain & ramp rate                   */
#define ES8311_REG_ADC_VOL      0x16  /* ADC gain scale                             */
#define ES8311_REG_ADC_ALC      0x17  /* ADC volume                                 */
#define ES8311_REG_ADC_HPF1     0x1B  /* ADC HPF stage 1                            */
#define ES8311_REG_ADC_HPF2     0x1C  /* ADC HPF stage 2 & equalizer                */
#define ES8311_REG_DAC_MUTE     0x31  /* DAC DSM/DEM mute, dither, phase inversion  */
#define ES8311_REG_DAC_VOL      0x32  /* DAC Digital Volume (0xBF = 0dB gain)       */
#define ES8311_REG_DAC_RAMP     0x37  /* DAC soft ramp rate & digital filter config */
#define ES8311_REG_GPIO_CFG     0x44  /* GPIO / I2C noise immunity configuration    */
#define ES8311_REG_GPIO_PULL    0x45  /* BCLK/LRCK internal pull-up control         */
#define ES8311_REG_CHIP_ID1     0xFD  /* Chip ID 1 (Expected: 0x83)                 */
#define ES8311_REG_CHIP_ID2     0xFE  /* Chip ID 2 (Expected: 0x11)                 */
#define ES8311_REG_CHIP_VER     0xFF  /* Silicon Version                            */

#define SAMPLE_CHUNK_FRAMES     256   /* Audio frames per chunk (each frame = 2 samples: L + R) */

static const char *TAG = "bsp_audio";

/* 256-point 16-bit single-period sine wave stored in flash. Peak amplitude = 30000 (~0 dBFS). */
static const int16_t SINE_LUT_256[256] = {
          0,    736,   1472,   2207,   2941,   3672,   4402,   5129,
       5853,   6573,   7289,   8001,   8709,   9410,  10107,  10797,
      11481,  12157,  12827,  13488,  14142,  14787,  15423,  16050,
      16667,  17274,  17871,  18457,  19032,  19595,  20147,  20686,
      21213,  21727,  22229,  22716,  23190,  23650,  24096,  24528,
      24944,  25346,  25732,  26103,  26458,  26797,  27120,  27426,
      27716,  27990,  28246,  28486,  28708,  28913,  29101,  29271,
      29424,  29558,  29675,  29774,  29856,  29919,  29964,  29991,
      30000,  29991,  29964,  29919,  29856,  29774,  29675,  29558,
      29424,  29271,  29101,  28913,  28708,  28486,  28246,  27990,
      27716,  27426,  27120,  26797,  26458,  26103,  25732,  25346,
      24944,  24528,  24096,  23650,  23190,  22716,  22229,  21727,
      21213,  20686,  20147,  19595,  19032,  18457,  17871,  17274,
      16667,  16050,  15423,  14787,  14142,  13488,  12827,  12157,
      11481,  10797,  10107,   9410,   8709,   8001,   7289,   6573,
       5853,   5129,   4402,   3672,   2941,   2207,   1472,    736,
          0,   -736,  -1472,  -2207,  -2941,  -3672,  -4402,  -5129,
      -5853,  -6573,  -7289,  -8001,  -8709,  -9410, -10107, -10797,
     -11481, -12157, -12827, -13488, -14142, -14787, -15423, -16050,
     -16667, -17274, -17871, -18457, -19032, -19595, -20147, -20686,
     -21213, -21727, -22229, -22716, -23190, -23650, -24096, -24528,
     -24944, -25346, -25732, -26103, -26458, -26797, -27120, -27426,
     -27716, -27990, -28246, -28486, -28708, -28913, -29101, -29271,
     -29424, -29558, -29675, -29774, -29856, -29919, -29964, -29991,
     -30000, -29991, -29964, -29919, -29856, -29774, -29675, -29558,
     -29424, -29271, -29101, -28913, -28708, -28486, -28246, -27990,
     -27716, -27426, -27120, -26797, -26458, -26103, -25732, -25346,
     -24944, -24528, -24096, -23650, -23190, -22716, -22229, -21727,
     -21213, -20686, -20147, -19595, -19032, -18457, -17871, -17274,
     -16667, -16050, -15423, -14787, -14142, -13488, -12827, -12157,
     -11481, -10797, -10107,  -9410,  -8709,  -8001,  -7289,  -6573,
      -5853,  -5129,  -4402,  -3672,  -2941,  -2207,  -1472,   -736
};

/* --- Driver State --- */
static i2s_chan_handle_t s_tx_chan          = NULL;
static bool              s_audio_inited     = false;
static bool              s_audio_in_standby = false;
static bool              s_low_power_mode   = false;

/* --- Forward Declarations --- */
static esp_err_t audio_play_tone_raw(uint32_t freq_hz, uint32_t duration_ms, float volume_pct, bool auto_standby);
static void bsp_audio_pa_ctrl_enable(bool enable);

static esp_err_t bsp_codec_write_reg(uint8_t reg, uint8_t val)
{
    uint8_t write_buf[2] = { reg, val };
    return bsp_i2c_write(ES8311_I2C_ADDR, write_buf, sizeof(write_buf));
}

static esp_err_t bsp_codec_read_reg(uint8_t reg, uint8_t *val)
{
    if (val == NULL) return ESP_ERR_INVALID_ARG;
    return bsp_i2c_read_reg(ES8311_I2C_ADDR, reg, val, 1);
}

static void _audio_codec_debug_reg_dump(void)
{
    uint8_t r00 = 0, r01 = 0, r02 = 0, r03 = 0, r04 = 0, r06 = 0, r08 = 0, r09 = 0, r0D = 0, r12 = 0, r31 = 0, r32 = 0, r44 = 0;
    bsp_codec_read_reg(0x00, &r00); bsp_codec_read_reg(0x01, &r01); bsp_codec_read_reg(0x02, &r02);
    bsp_codec_read_reg(0x03, &r03); bsp_codec_read_reg(0x04, &r04); bsp_codec_read_reg(0x06, &r06);
    bsp_codec_read_reg(0x08, &r08); bsp_codec_read_reg(0x09, &r09); bsp_codec_read_reg(0x0D, &r0D);
    bsp_codec_read_reg(0x12, &r12); bsp_codec_read_reg(0x31, &r31); bsp_codec_read_reg(0x32, &r32);
    bsp_codec_read_reg(0x44, &r44);
    ESP_LOGI(TAG, "ES8311 Regs: R00=0x%02X R01=0x%02X R02=0x%02X R03=0x%02X R04=0x%02X R06=0x%02X R08=0x%02X R09=0x%02X R0D=0x%02X R12=0x%02X R31=0x%02X R32=0x%02X R44=0x%02X",
             r00, r01, r02, r03, r04, r06, r08, r09, r0D, r12, r31, r32, r44);
}

void bsp_audio_power_enable(bool enable)
{
    /* Active-LOW power rail pin: 0 = Power ON, 1 = Power OFF.
     * Note: BSP_PIN_PA_EN rail should remain ON (0) during normal operation to prevent I2C bus clamping. */
    gpio_set_level((gpio_num_t)BSP_PIN_PA_EN, enable ? 0 : 1);
}

static void bsp_audio_pa_ctrl_enable(bool enable)
{
    /* Active-HIGH NS4168 PA control pin: 1 = Amp ENABLED, 0 = Amp MUTED/SHUTDOWN */
    gpio_set_level((gpio_num_t)BSP_PIN_PA_CTRL, enable ? 1 : 0);
}

/* --- Internal Core State Routines (Matching Official Reference Sequence) --- */

esp_err_t bsp_audio_init_dac_only(void)
{
    esp_err_t ret = ESP_OK;

    /* 1. Wake chip and set low-power analog baseline (es8311_open) */
    ret |= bsp_codec_write_reg(ES8311_REG_SYS_PWR_ANA, 0xFA);

    /* 2. Enhance ES8311 I2C noise immunity (written twice per silicon errata) */
    ret |= bsp_codec_write_reg(ES8311_REG_GPIO_CFG,    0x08);
    ret |= bsp_codec_write_reg(ES8311_REG_GPIO_CFG,    0x08);

    /* 3. Clock tree & initial analog reference biasing */
    ret |= bsp_codec_write_reg(ES8311_REG_CLK_MGR1,    0x30);
    ret |= bsp_codec_write_reg(ES8311_REG_CLK_MGR2,    0x00);
    ret |= bsp_codec_write_reg(ES8311_REG_CLK_MGR3,    0x10);
    ret |= bsp_codec_write_reg(ES8311_REG_ADC_VOL,     0x24); /* ADC gain scale */
    ret |= bsp_codec_write_reg(ES8311_REG_CLK_MGR4,    0x10);
    ret |= bsp_codec_write_reg(ES8311_REG_CLK_MGR5,    0x00);
    ret |= bsp_codec_write_reg(ES8311_REG_PWR_UP_A,    0x00);
    ret |= bsp_codec_write_reg(ES8311_REG_SYSTEM_0C,   0x00);
    ret |= bsp_codec_write_reg(ES8311_REG_SYS_BIAS,    0x1F); /* Reg 0x10: Analog bias generator adjust */
    ret |= bsp_codec_write_reg(ES8311_REG_SYS_VMID,    0x7F); /* Reg 0x11: Internal VMID buffer impedance */
    ret |= bsp_codec_write_reg(ES8311_REG_RESET,       0x80); /* CSM power on */

    /* 4. Slave mode & MCLK selection */
    ret |= bsp_codec_write_reg(ES8311_REG_RESET,       0x80); /* Slave mode (MSC = 0, CSM_ON = 1) */
    ret |= bsp_codec_write_reg(ES8311_REG_CLK_MGR1,    0x3F); /* Use MCLK pin (Bit 7 = 0), all clocks on */
    ret |= bsp_codec_write_reg(ES8311_REG_CLK_MGR6,    0x00); /* Normal SCLK polarity */

    ret |= bsp_codec_write_reg(ES8311_REG_SYS_PWR3,    0x10); /* Internal reference bias */
    ret |= bsp_codec_write_reg(ES8311_REG_ADC_HPF1,    0x0A);
    ret |= bsp_codec_write_reg(ES8311_REG_ADC_HPF2,    0x6A);
    ret |= bsp_codec_write_reg(ES8311_REG_GPIO_CFG,    0x58); /* Internal reference signal routing (ADCL + DACR) */

    /* 5. Configure Standard 16-bit 16kHz Philips I2S Sample Rate & Clock Dividers (coeff_div entry 74) */
    ret |= bsp_codec_write_reg(ES8311_REG_SDP_IN,      0x0C); /* 16-bit Philips I2S, SDP in unmute */
    ret |= bsp_codec_write_reg(ES8311_REG_SDP_OUT,     0x0C); /* 16-bit Philips I2S, SDP out unmute */
    ret |= bsp_codec_write_reg(ES8311_REG_CLK_MGR2,    0x00); /* Pre-divider = 1, Pre-multiplier = 1 */
    ret |= bsp_codec_write_reg(ES8311_REG_CLK_MGR5,    0x00); /* ADC/DAC clock dividers = 1 */
    ret |= bsp_codec_write_reg(ES8311_REG_CLK_MGR3,    0x10); /* Single speed, ADC OSR */
    ret |= bsp_codec_write_reg(ES8311_REG_CLK_MGR4,    0x20); /* DAC OSR = 0x20 for 16kHz Fs */
    ret |= bsp_codec_write_reg(ES8311_REG_CLK_MGR7,    0x00); /* Master LRCK divider high byte */
    ret |= bsp_codec_write_reg(ES8311_REG_CLK_GATE,    0xFF); /* Master LRCK divider low byte (0xFF) */
    ret |= bsp_codec_write_reg(ES8311_REG_CLK_MGR6,    0x03); /* BCLK divider = 4 (reg value 3) */

    /* 6. Power on DAC and Analog blocks (es8311_start) */
    ret |= bsp_codec_write_reg(ES8311_REG_RESET,       0x80); /* CSM power on */
    ret |= bsp_codec_write_reg(ES8311_REG_CLK_MGR1,    0x3F); /* MCLK pin, all clocks on */
    ret |= bsp_codec_write_reg(ES8311_REG_SDP_IN,      0x0C); /* Unmute SDP IN */
    ret |= bsp_codec_write_reg(ES8311_REG_SDP_OUT,     0x0C);
    ret |= bsp_codec_write_reg(ES8311_REG_ADC_ALC,     0xBF);
    ret |= bsp_codec_write_reg(ES8311_REG_SYS_PGA_MOD, 0x02); /* Analog blocks power on / VROI */
    ret |= bsp_codec_write_reg(ES8311_REG_DAC_PWR,     0x00); /* DAC analog power ON (PDN_DAC = 0) */
    ret |= bsp_codec_write_reg(ES8311_REG_ADC_PWR,     0x1A);
    ret |= bsp_codec_write_reg(ES8311_REG_SYS_PWR_ANA, 0x01); /* Analog reference & normal VMID */
    ret |= bsp_codec_write_reg(ES8311_REG_ADC_GAIN,    0x40);
    ret |= bsp_codec_write_reg(ES8311_REG_DAC_RAMP,    0x08);
    ret |= bsp_codec_write_reg(ES8311_REG_GPIO_PULL,   0x00);

    /* 7. Volume and Unmute */
    ret |= bsp_codec_write_reg(ES8311_REG_DAC_VOL,     0xBF); /* 0 dB nominal volume */
    ret |= bsp_codec_write_reg(ES8311_REG_DAC_MUTE,    0x00); /* Unmute DAC DSM/DEM */

    return ret;
}

esp_err_t bsp_audio_init(void)
{
    if (s_audio_inited && !s_audio_in_standby) {
        return ESP_OK;
    }
    if (s_audio_in_standby) {
        return bsp_audio_resume();
    }

    /* 1. Ensure GPIO direction is configured and pad holds released */
    gpio_hold_dis((gpio_num_t)BSP_PIN_PA_EN);
    gpio_hold_dis((gpio_num_t)BSP_PIN_PA_CTRL);
    gpio_set_direction((gpio_num_t)BSP_PIN_PA_EN, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)BSP_PIN_PA_CTRL, GPIO_MODE_OUTPUT);

    /* Power rail ON (Active-LOW: 0), PA initially muted (0) */
    bsp_audio_power_enable(true);
    bsp_audio_pa_ctrl_enable(false);
    vTaskDelay(pdMS_TO_TICKS(15));

    /* 2. Probe ES8311 silicon over I2C */
    uint8_t chip_id1 = 0, chip_id2 = 0;
    bsp_codec_read_reg(ES8311_REG_CHIP_ID1, &chip_id1);
    bsp_codec_read_reg(ES8311_REG_CHIP_ID2, &chip_id2);
    ESP_LOGI(TAG, "ES8311 Codec I2C Probe: ID=0x%02X%02X (Expected: 0x8311)", chip_id1, chip_id2);

    /* 3. Configure & Create I2S Master TX Channel (Stereo mode with dual-slot streaming) */
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.auto_clear        = true;
    chan_cfg.dma_desc_num      = 4;
    chan_cfg.dma_frame_num     = SAMPLE_CHUNK_FRAMES;

    esp_err_t ret = i2s_new_channel(&chan_cfg, &s_tx_chan, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2S TX channel: %s", esp_err_to_name(ret));
        return ret;
    }

    /* 4. Configure Standard 16-bit 16kHz Philips Mono I2S Timing & Clocking
     *    Standard Mono frame transmits 16-bit PCM on the Left slot mapped directly to ES8311 Mono DAC */
    i2s_std_config_t std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
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
    std_cfg.clk_cfg.mclk_multiple = I2S_MCLK_MULTIPLE_256; /* 16 kHz * 256 = 4.096 MHz */
    std_cfg.slot_cfg.slot_mask    = I2S_STD_SLOT_LEFT;     /* Left-channel audio slot mapped to Mono DAC */

    ret = i2s_channel_init_std_mode(s_tx_chan, &std_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init I2S TX std mode: %s", esp_err_to_name(ret));
        return ret;
    }
    i2s_channel_enable(s_tx_chan);

    /* 5. Configure ES8311 registers over I2C */
    ret = bsp_audio_init_dac_only();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize ES8311 DAC registers: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Log Register Verification Dump */
    //_audio_codec_debug_reg_dump();

    s_audio_inited     = true;
    s_audio_in_standby = false;

    /* 6. Set default volume and low power preferences */
#if defined(CONFIG_BSP_AUDIO_DEFAULT_VOLUME)
    bsp_audio_set_volume((float)CONFIG_BSP_AUDIO_DEFAULT_VOLUME);
#else
    bsp_audio_set_volume(80.0f);
#endif

#if defined(CONFIG_BSP_AUDIO_LOW_POWER_DEFAULT)
    bsp_audio_set_low_power_mode(CONFIG_BSP_AUDIO_LOW_POWER_DEFAULT);
#else
    bsp_audio_set_low_power_mode(true);
#endif

    ESP_LOGI(TAG, "Audio subsystem successfully initialized (Direct I2C/I2S, 16kHz MCLK=4.096MHz)");
    return ESP_OK;
}

esp_err_t bsp_audio_play(const void *data, size_t len, size_t *bytes_written)
{
    if (!s_audio_inited || s_tx_chan == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    if (data == NULL || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    if (s_audio_in_standby) {
        bsp_audio_resume();
    }

    return i2s_channel_write(s_tx_chan, data, len, bytes_written, portMAX_DELAY);
}

esp_err_t bsp_audio_set_volume(float volume)
{
    if (!s_audio_inited) return ESP_ERR_INVALID_STATE;

    if (volume < 0.0f)   volume = 0.0f;
    if (volume > 100.0f) volume = 100.0f;

    /* ES8311 Reg 0x32 volume: 0x00 is -95.5dB (Mute), 0xBF is 0dB (Unity).
     * Scale 1%..100% across -45dB (0x65) to 0dB (0xBF) for clear audible levels */
    uint8_t reg_val;
    if (volume <= 0.0f) {
        reg_val = 0x00;
    } else {
        reg_val = (uint8_t)(0x65 + (volume / 100.0f) * (0xBF - 0x65));
    }

    return bsp_codec_write_reg(ES8311_REG_DAC_VOL, reg_val);
}

esp_err_t bsp_audio_mute(bool enable_mute)
{
    if (!s_audio_inited) return ESP_ERR_INVALID_STATE;

    if (enable_mute) {
        bsp_audio_pa_ctrl_enable(false);
        bsp_codec_write_reg(ES8311_REG_DAC_MUTE, 0x60); /* DSM & DEM Soft Mute */
    } else {
        bsp_codec_write_reg(ES8311_REG_DAC_MUTE, 0x00);
        vTaskDelay(pdMS_TO_TICKS(5));
        bsp_audio_pa_ctrl_enable(true);
    }
    return ESP_OK;
}

esp_err_t bsp_audio_power_down(void)
{
    if (!s_audio_inited || s_audio_in_standby) {
        return ESP_OK;
    }

    /* 1. Mute external amplifier immediately */
    bsp_audio_pa_ctrl_enable(false);

    /* 2. Soft-mute DAC DSM */
    bsp_codec_write_reg(ES8311_REG_DAC_MUTE, 0x60);
    vTaskDelay(pdMS_TO_TICKS(5));

    /* 3. Full zero-heat silicon suspension per official es8311_suspend() */
    bsp_codec_write_reg(ES8311_REG_DAC_VOL,      0x00); /* Mute volume */
    bsp_codec_write_reg(ES8311_REG_ADC_ALC,      0x00);
    bsp_codec_write_reg(ES8311_REG_SYS_PGA_MOD,  0xFF); /* Power down PGA & ADC Modulator */
    bsp_codec_write_reg(ES8311_REG_DAC_PWR,      0x02); /* Power down DAC output stage */
    bsp_codec_write_reg(ES8311_REG_ADC_PWR,      0x00); /* Power down ADC */
    bsp_codec_write_reg(ES8311_REG_SYS_PWR_ANA,  0xFA);
    bsp_codec_write_reg(ES8311_REG_ADC_GAIN,     0x00);
    bsp_codec_write_reg(ES8311_REG_CLK_MGR2,     0x10);
    bsp_codec_write_reg(ES8311_REG_RESET,        0x00);
    bsp_codec_write_reg(ES8311_REG_RESET,        0x1F); /* Assert digital reset */
    bsp_codec_write_reg(ES8311_REG_CLK_MGR1,     0x30);
    bsp_codec_write_reg(ES8311_REG_CLK_MGR1,     0x00); /* Gate all clocks */
    bsp_codec_write_reg(ES8311_REG_GPIO_PULL,    0x00);
    bsp_codec_write_reg(ES8311_REG_SYS_PWR_ANA,  0xFC); /* Completely power down analog references & VMID */
    bsp_codec_write_reg(ES8311_REG_CLK_MGR2,     0x00);

    /* 4. Disable active I2S clock oscillation to stop digital switching dissipation */
    if (s_tx_chan != NULL) {
        i2s_channel_disable(s_tx_chan);
    }

    s_audio_in_standby = true;
    ESP_LOGD(TAG, "Audio subsystem placed in zero-heat deep standby (~0 uA)");
    return ESP_OK;
}

esp_err_t bsp_audio_standby(void)
{
    return bsp_audio_power_down();
}

esp_err_t bsp_audio_resume(void)
{
    if (!s_audio_inited) {
        return bsp_audio_init();
    }
    if (!s_audio_in_standby) {
        return ESP_OK;
    }

    /* 1. Restart I2S master clock lines */
    if (s_tx_chan != NULL) {
        i2s_channel_enable(s_tx_chan);
    }

    /* 2. Re-initialize codec registers */
    bsp_audio_init_dac_only();

    /* 3. Enable external PA */
    vTaskDelay(pdMS_TO_TICKS(10));
    bsp_audio_pa_ctrl_enable(true);

    s_audio_in_standby = false;
    return ESP_OK;
}

esp_err_t bsp_audio_stop(void)
{
    if (!s_audio_inited) return ESP_OK;
    return bsp_audio_power_down();
}

esp_err_t bsp_audio_set_low_power_mode(bool enable)
{
    s_low_power_mode = enable;
    if (s_audio_inited && !s_audio_in_standby) {
        uint8_t val = 0;
        if (bsp_codec_read_reg(ES8311_REG_SYS_LP_MOD, &val) != ESP_OK) {
            val = 0x00;
        }
        if (enable) {
            val |= (1 << 7); /* Enable LPDAC bit */
        } else {
            val &= ~(1 << 7);
        }
        return bsp_codec_write_reg(ES8311_REG_SYS_LP_MOD, val);
    }
    return ESP_OK;
}

/* --- Tone Synthesis & System Chimes --- */

static esp_err_t audio_play_tone_raw(uint32_t freq_hz, uint32_t duration_ms, float volume_pct, bool auto_standby)
{
    if (freq_hz == 0 || duration_ms == 0) return ESP_ERR_INVALID_ARG;

    esp_err_t ret = bsp_audio_init();
    if (ret != ESP_OK) return ret;

    if (s_audio_in_standby) {
        ret = bsp_audio_resume();
        if (ret != ESP_OK) return ret;
    }

    bsp_audio_set_volume(volume_pct);
    if (volume_pct > 0.0f) {
        bsp_audio_mute(false);
    } else {
        bsp_audio_mute(true);
    }

    const uint32_t sample_rate    = 16000;
    const uint32_t total_samples  = (sample_rate * duration_ms) / 1000;
    const uint32_t attack_samples = (sample_rate * 5) / 1000;
    const uint32_t decay_samples  = (sample_rate * 5) / 1000;

    uint32_t phase_acc = 0;
    const uint32_t phase_inc = (freq_hz * 65536) / sample_rate;

    /* Mono buffer: 1 int16_t sample per audio frame */
    int16_t chunk_buf[SAMPLE_CHUNK_FRAMES];
    size_t  frame_idx = 0;

    for (uint32_t i = 0; i < total_samples; i++) {
        uint8_t lut_idx = (phase_acc >> 8) & 0xFF;
        int32_t sample  = (int32_t)SINE_LUT_256[lut_idx]; /* Full scale 16-bit range (peak = 30000) */

        /* 5ms attack/decay anti-pop envelopes */
        if (i < attack_samples) {
            sample = (sample * (int32_t)i) / (int32_t)attack_samples;
        } else if (i > (total_samples - decay_samples)) {
            uint32_t rem = total_samples - i;
            sample = (sample * (int32_t)rem) / (int32_t)decay_samples;
        }

        chunk_buf[frame_idx++] = (int16_t)sample;
        phase_acc += phase_inc;

        if (frame_idx >= SAMPLE_CHUNK_FRAMES) {
            bsp_audio_play(chunk_buf, frame_idx * sizeof(int16_t), NULL);
            frame_idx = 0;
        }
    }

    if (frame_idx > 0) {
        bsp_audio_play(chunk_buf, frame_idx * sizeof(int16_t), NULL);
    }

    /* Wait for DMA buffer ring (4 x 256 frames = 64ms) to finish playing out */
    vTaskDelay(pdMS_TO_TICKS(100));

    if (auto_standby) {
        bsp_audio_stop();
    }
    return ESP_OK;
}

esp_err_t bsp_audio_play_tone(uint32_t freq_hz, uint32_t duration_ms, float volume_pct)
{
    return audio_play_tone_raw(freq_hz, duration_ms, volume_pct, true);
}

esp_err_t bsp_audio_play_chime(bsp_chime_type_t type)
{
    esp_err_t ret = ESP_OK;
    switch (type) {
        case BSP_CHIME_BOOT:
            audio_play_tone_raw(523,   90, 70.0f, false);
            audio_play_tone_raw(659,   90, 75.0f, false);
            audio_play_tone_raw(784,   90, 80.0f, false);
            audio_play_tone_raw(1046, 200, 85.0f, false);
            break;

        case BSP_CHIME_WAKE:
            audio_play_tone_raw(784,   80, 65.0f, false);
            audio_play_tone_raw(1046, 150, 75.0f, false);
            break;

        case BSP_CHIME_SLEEP:
            audio_play_tone_raw(1046, 120, 70.0f, false);
            audio_play_tone_raw(784,  120, 65.0f, false);
            audio_play_tone_raw(659,  220, 55.0f, false);
            break;

        case BSP_CHIME_SHUTDOWN:
            audio_play_tone_raw(784, 140, 70.0f, false);
            audio_play_tone_raw(659, 140, 65.0f, false);
            audio_play_tone_raw(523, 280, 60.0f, false);
            break;

        case BSP_CHIME_ALARM:
            for (int i = 0; i < 2; i++) {
                audio_play_tone_raw(1760, 100, 90.0f, false);
                audio_play_tone_raw(880,  100, 90.0f, false);
            }
            break;

        case BSP_CHIME_NOTIFY:
            audio_play_tone_raw(1046,  70, 70.0f, false);
            audio_play_tone_raw(1318, 120, 75.0f, false);
            break;

        case BSP_CHIME_EVENT:
            audio_play_tone_raw(1200, 35, 60.0f, false);
            break;

        default:
            ret = ESP_ERR_INVALID_ARG;
            break;
    }

    if (ret == ESP_OK) {
        /* Flush trailing pipeline and place codec into low-power standby */
        vTaskDelay(pdMS_TO_TICKS(100));
        bsp_audio_stop();
    }
    return ret;
}

static void bsp_audio_chime_wrapper_cb(bsp_chime_type_t type, void *user_data)
{
    (void)user_data;
    bsp_audio_play_chime(type);
}

esp_err_t bsp_audio_register_default_chimes(void)
{
    esp_err_t ret = ESP_OK;
    for (int i = 0; i < (int)BSP_CHIME_MAX; i++) {
        esp_err_t err = bsp_register_chime_cb((bsp_chime_type_t)i, bsp_audio_chime_wrapper_cb, NULL);
        if (err != ESP_OK) {
            ret = err;
        }
    }
    return ret;
}

esp_err_t bsp_audio_deinit(void)
{
    if (!s_audio_inited) {
        return ESP_OK;
    }

    bsp_audio_stop();

    if (s_tx_chan != NULL) {
        i2s_channel_disable(s_tx_chan);
        i2s_del_channel(s_tx_chan);
        s_tx_chan = NULL;
    }

    bsp_audio_pa_ctrl_enable(false);
    bsp_audio_power_enable(false);

    s_audio_inited     = false;
    s_audio_in_standby = false;
    ESP_LOGI(TAG, "Audio subsystem resources deallocated");
    return ESP_OK;
}