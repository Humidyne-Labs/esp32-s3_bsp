idf_component_register(
    SRCS 
        "main.cpp"
        "app_claiming.c"
        "app_ble_prov.c"
        "app_time_sync.c"
        "app_mqtt.c"
    INCLUDE_DIRS 
        "."
    PRIV_REQUIRES 
        esp_mmap_assets 
        esp_lv_fs 
        esp32-s3_bsp 
        lvgl 
        esp_codec_dev
        mqtt
        esp_netif
        esp_event
        esp_wifi
        wifi_provisioning
        bt
        nvs_flash
        app_update
        esp_https_ota
        esp_http_client
        json
)

# Pack assets and specify file extension(s)
spiffs_create_partition_assets(storage ../spiflash_assets
    FLASH_IN_PROJECT
    MMAP_FILE_SUPPORT_FORMAT ".png"
    MMAP_SUPPORT_RAW
    MMAP_RAW_COLOR_FORMAT "I1" # Options: RGB565, RGB888, ARGB8888, AUTO, etc.
)
