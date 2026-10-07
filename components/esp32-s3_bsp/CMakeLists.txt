set(bsp_srcs
    "src/bsp_common.c"
    "src/bsp_i2c.c"
    "src/bsp_rtc.c"
    "src/bsp_button.c"
    "src/bsp_power.c"
    "src/bsp_display.cpp"
    "src/bsp_sensors.c"
    "src/bsp_sdcard.c"

    "src/bsp_audio.c"
    "src/bsp_lvgl.cpp"
    "src/bsp_nvs.c"
    "src/bsp_wifi.c"
    "src/bsp_rtc_mem.c"
    "src/bsp_splash.c"
    "src/bsp_time.c"
    "src/bsp_prov.c"
    "src/bsp_ota.c"
)

set(bsp_requires
    esp_driver_gpio
    esp_driver_i2c
    esp_driver_i2s
    esp_driver_spi
    esp_driver_sdmmc
    sdmmc
    esp_adc
    esp_timer
    nvs_flash
    fatfs
    log
    freertos
    esp_wifi
    esp_netif
    esp_event
    network_provisioning
    bt
    protocomm
    app_update
    esp_https_ota
    mbedtls
    esp-tls
    lvgl
    esp_mmap_assets
    esp_lv_fs
    esp_driver_tsens
)


if(CONFIG_BSP_ENABLE_THINGSBOARD)
    list(APPEND bsp_srcs "src/bsp_tb.c")
    list(APPEND bsp_requires mqtt cjson)
endif()

idf_component_register(
    SRCS ${bsp_srcs}
    INCLUDE_DIRS "include"
    REQUIRES ${bsp_requires}
)