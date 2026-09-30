#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "bsp/bsp_audio.h"

static const char *TAG = "audio_example";

void app_main(void)
{
    ESP_LOGI(TAG, "Enabling audio power rail...");
    bsp_audio_power_enable(true);

    ESP_LOGI(TAG, "Initializing audio subsystem...");
    ESP_ERROR_CHECK(bsp_audio_init());

    ESP_LOGI(TAG, "Setting volume to 80%%...");
    ESP_ERROR_CHECK(bsp_audio_set_volume(80.0f));

    ESP_LOGI(TAG, "Registering default system chimes...");
    ESP_ERROR_CHECK(bsp_audio_register_default_chimes());

    ESP_LOGI(TAG, "Playing boot chime...");
    ESP_ERROR_CHECK(bsp_audio_play_chime(BSP_CHIME_BOOT));
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "Playing 440 Hz tone for 100 ms...");
    ESP_ERROR_CHECK(bsp_audio_play_tone(440, 100, 80.0f));
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "Playing raw PCM silence buffer...");
    int16_t pcm_samples[32] = {0};
    size_t bytes_written = 0;
    ESP_ERROR_CHECK(bsp_audio_play(pcm_samples, sizeof(pcm_samples), &bytes_written));

    ESP_LOGI(TAG, "Playing notification chime...");
    ESP_ERROR_CHECK(bsp_audio_play_chime(BSP_CHIME_NOTIFY));
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "Stopping audio and muting amplifier...");
    ESP_ERROR_CHECK(bsp_audio_stop());

    ESP_LOGI(TAG, "Disabling audio power rail...");
    bsp_audio_power_enable(false);

    ESP_LOGI(TAG, "Audio test complete.");
}