#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2s_std.h"
#include "esp_log.h"
#include "bsp/bsp_i2c.h"
#include "bsp/bsp_audio.h"

static const char *TAG = "bsp_audio";

static i2s_chan_handle_t s_tx_chan = NULL;
static bool s_audio_inited = false;

esp_err_t bsp_audio_init(void)
{
    if (s_audio_inited) return ESP_OK;

    /* Configure Power Amp GPIO pins (PA_CTRL GPIO47, PA_EN GPIO48) */
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << BSP_GPIO_PA_CTRL) | (1ULL << BSP_GPIO_PA_EN),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);

    /* Turn off PA by default */
    bsp_audio_pa_enable(false);
    gpio_set_level((gpio_num_t)BSP_GPIO_PA_CTRL, 1);

    /* Initialize I2S Master TX channel */
    i2s_chan_config_t chan_cfg = I2S_CHAN_CONFIG_DEFAULT(I2S_NUM_0, I2S_ROLE_MASTER);
    esp_err_t ret = i2s_new_channel(&chan_cfg, &s_tx_chan, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2S TX channel: %s", esp_err_to_name(ret));
        return ret;
    }

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
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

    ret = i2s_channel_init_std_mode(s_tx_chan, &std_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init I2S std mode: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = i2s_channel_enable(s_tx_chan);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable I2S channel: %s", esp_err_to_name(ret));
        return ret;
    }

    s_audio_inited = true;
    ESP_LOGI(TAG, "Audio hardware & I2S driver initialized");
    return ESP_OK;
}

void bsp_audio_pa_enable(bool enable)
{
    gpio_set_level((gpio_num_t)BSP_GPIO_PA_EN, enable ? 1 : 0);
}

esp_err_t bsp_audio_set_volume(float volume)
{
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 100.0f) volume = 100.0f;

    bsp_audio_pa_enable(volume > 0.0f);
    ESP_LOGI(TAG, "Audio volume set to %.1f%%", volume);
    return ESP_OK;
}

esp_err_t bsp_audio_play(const void *data, size_t len, size_t *bytes_written)
{
    if (!s_audio_inited || s_tx_chan == NULL) {
        esp_err_t ret = bsp_audio_init();
        if (ret != ESP_OK) return ret;
    }

    bsp_audio_pa_enable(true);
    esp_err_t ret = i2s_channel_write(s_tx_chan, data, len, bytes_written, pdMS_TO_TICKS(1000));
    return ret;
}
