#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"
#include "esp_log.h"
#include "bsp/bsp_nvs.h"

static const char *TAG = "nvs_example";

typedef struct {
    uint16_t sample_interval_sec;
    uint8_t  threshold_humidity;
    uint8_t  flags;
} app_config_t;

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing NVS subsystem...");
    ESP_ERROR_CHECK(bsp_nvs_init());

    /* 1. Unsigned 32-bit Integer Read/Write */
    uint32_t boot_count = 0;
    esp_err_t err = bsp_nvs_get_u32("boot_count", &boot_count);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Previous boot count: %lu", (unsigned long)boot_count);
    } else {
        ESP_LOGW(TAG, "Key 'boot_count' not found, initializing to 0.");
        boot_count = 0;
    }

    boot_count++;
    ESP_ERROR_CHECK(bsp_nvs_set_u32("boot_count", boot_count));
    ESP_LOGI(TAG, "Saved updated boot count: %lu", (unsigned long)boot_count);

    /* 2. String Read/Write */
    const char *node_name = "HumidiTron-S3";
    char read_name_buf[32] = {0};

    ESP_LOGI(TAG, "Writing string key 'node_name': %s", node_name);
    ESP_ERROR_CHECK(bsp_nvs_set_str("node_name", node_name));

    ESP_ERROR_CHECK(bsp_nvs_get_str("node_name", read_name_buf, sizeof(read_name_buf)));
    ESP_LOGI(TAG, "Read back string 'node_name': %s", read_name_buf);

    /* 3. Binary Blob Read/Write */
    app_config_t cfg_write = {
        .sample_interval_sec = 60,
        .threshold_humidity  = 75,
        .flags               = 0x01,
    };
    app_config_t cfg_read = {0};
    size_t blob_len = sizeof(cfg_read);

    ESP_LOGI(TAG, "Writing binary blob 'app_cfg' (%u bytes)...", (unsigned int)sizeof(cfg_write));
    ESP_ERROR_CHECK(bsp_nvs_set_blob("app_cfg", &cfg_write, sizeof(cfg_write)));

    ESP_ERROR_CHECK(bsp_nvs_get_blob("app_cfg", &cfg_read, &blob_len));
    ESP_LOGI(TAG, "Read binary blob 'app_cfg' (%u bytes): interval=%u, threshold=%u, flags=0x%02X",
             (unsigned int)blob_len,
             cfg_read.sample_interval_sec,
             cfg_read.threshold_humidity,
             cfg_read.flags);

    /* 4. Erase Key */
    ESP_LOGI(TAG, "Erasing key 'node_name'...");
    ESP_ERROR_CHECK(bsp_nvs_erase_key("node_name"));

    err = bsp_nvs_get_str("node_name", read_name_buf, sizeof(read_name_buf));
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "Confirmed: key 'node_name' was erased.");
    } else {
        ESP_LOGE(TAG, "Key 'node_name' unexpectedly still present!");
    }

    /* 5. Clear Wi-Fi Credentials Helper */
    ESP_LOGI(TAG, "Clearing stored Wi-Fi credentials...");
    ESP_ERROR_CHECK(bsp_nvs_clear_wifi_credentials());

    ESP_LOGI(TAG, "NVS demonstration completed successfully.");
}