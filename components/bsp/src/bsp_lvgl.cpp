#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "bsp/bsp_display.h"
#include "bsp/bsp_touch.h"
#include "bsp/bsp_lvgl.h"

static const char *TAG = "bsp_lvgl";

static lv_display_t *s_lv_display = NULL;
static lv_indev_t *s_lv_touch_indev = NULL;

static void lvgl_display_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint16_t width = (area->x2 - area->x1 + 1);
    uint16_t height = (area->y2 - area->y1 + 1);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint16_t px_x = area->x1 + x;
            uint16_t px_y = area->y1 + y;
            uint8_t pixel_val = px_map[y * width + x];
            /* LVGL monomode / monochrome mapping */
            bsp_display_draw_pixel(px_x, px_y, (pixel_val > 127) ? BSP_DISPLAY_COLOR_WHITE : BSP_DISPLAY_COLOR_BLACK);
        }
    }

    bsp_display_flush();
    lv_display_flush_ready(disp);
}

static void lvgl_touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    uint16_t touch_x = 0;
    uint16_t touch_y = 0;

    bool touched = bsp_touch_read(&touch_x, &touch_y);
    if (touched) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = touch_x;
        data->point.y = touch_y;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

esp_err_t bsp_lvgl_init(void)
{
    /* Initialize display and touch hardware */
    esp_err_t ret = bsp_display_init();
    if (ret != ESP_OK) return ret;

    ret = bsp_touch_init();
    if (ret != ESP_OK) return ret;

    /* Initialize LVGL core library */
    lv_init();

    /* Create LVGL v9 Display */
    s_lv_display = lv_display_create(BSP_DISPLAY_WIDTH, BSP_DISPLAY_HEIGHT);
    if (s_lv_display == NULL) {
        ESP_LOGE(TAG, "Failed to create LVGL display");
        return ESP_FAIL;
    }

    /* Allocate buffer for LVGL rendering */
    size_t buf_size = BSP_DISPLAY_WIDTH * BSP_DISPLAY_HEIGHT;
    uint8_t *buf1 = (uint8_t *)malloc(buf_size);
    assert(buf1 != NULL);

    lv_display_set_buffers(s_lv_display, buf1, NULL, buf_size, LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(s_lv_display, lvgl_display_flush_cb);

    /* Create LVGL Touch Input device */
    s_lv_touch_indev = lv_indev_create();
    if (s_lv_touch_indev != NULL) {
        lv_indev_set_type(s_lv_touch_indev, LV_INDEV_TYPE_POINTER);
        lv_indev_set_read_cb(s_lv_touch_indev, lvgl_touch_read_cb);
    }

    ESP_LOGI(TAG, "LVGL v9 port initialized successfully");
    return ESP_OK;
}

void bsp_lvgl_port_task(void *pvParameters)
{
    ESP_LOGI(TAG, "LVGL task started");
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
        lv_timer_handler();
    }
}
