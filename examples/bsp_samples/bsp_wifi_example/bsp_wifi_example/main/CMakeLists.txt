idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        esp32-s3_bsp
        esp_driver_gpio
        esp_timer
        freertos
        log
        nvs_flash
        lvgl
)
