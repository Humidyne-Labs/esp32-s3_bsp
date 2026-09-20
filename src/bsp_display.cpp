#include <stdio.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "bsp/bsp_display.h"

static const char *TAG = "bsp_display";

static const uint8_t WF_Full_1IN54[159] = {
    0x80, 0x48, 0x40, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x40, 0x48, 0x80, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x80, 0x48, 0x40, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x40, 0x48, 0x80, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0xA,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
    0x8,  0x1,  0x0,  0x8,  0x1,  0x0,  0x2,  
    0xA,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
    0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x0,  0x0,  0x0,  
    0x22, 0x17, 0x41, 0x0,  0x32, 0x20
};

static const uint8_t WF_PARTIAL_1IN54[159] = {
    0x0, 0x40, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x80,0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x40,0x40, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0,  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0xF, 0x0,  0x0, 0x0, 0x0, 0x0, 0x0,
    0x1, 0x1,  0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0,  0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0,  0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0,  0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0,  0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0,  0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0,  0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0,  0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0,  0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0,  0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0,  0x0, 0x0, 0x0, 0x0, 0x0,
    0x22,0x22, 0x22,0x22,0x22,0x22,0x0, 0x0, 0x0,
    0x02,0x17, 0x41,0xB0,0x32,0x28
};

static spi_device_handle_t s_spi_handle = NULL;
static uint8_t *s_frame_buffer = NULL;

static void epd_set_cs(uint8_t level) { gpio_set_level((gpio_num_t)BSP_GPIO_EPD_CS, level); }
static void epd_set_dc(uint8_t level) { gpio_set_level((gpio_num_t)BSP_GPIO_EPD_DC, level); }
static void epd_set_rst(uint8_t level) { gpio_set_level((gpio_num_t)BSP_GPIO_EPD_RST, level); }

static void epd_read_busy(void)
{
    while (gpio_get_level((gpio_num_t)BSP_GPIO_EPD_BUSY) == 1) {
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

static void epd_send_byte(uint8_t data)
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;
    t.tx_buffer = &data;
    esp_err_t ret = spi_device_polling_transmit(s_spi_handle, &t);
    assert(ret == ESP_OK);
}

static void epd_send_cmd(uint8_t cmd)
{
    epd_set_dc(0);
    epd_set_cs(0);
    epd_send_byte(cmd);
    epd_set_cs(1);
}

static void epd_send_data(uint8_t data)
{
    epd_set_dc(1);
    epd_set_cs(0);
    epd_send_byte(data);
    epd_set_cs(1);
}

static void epd_write_bytes(const uint8_t *data, size_t len)
{
    epd_set_dc(1);
    epd_set_cs(0);
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8 * len;
    t.tx_buffer = data;
    esp_err_t ret = spi_device_polling_transmit(s_spi_handle, &t);
    assert(ret == ESP_OK);
    epd_set_cs(1);
}

static void epd_set_windows(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
    epd_send_cmd(0x44);
    epd_send_data((x_start >> 3) & 0xFF);
    epd_send_data((x_end >> 3) & 0xFF);

    epd_send_cmd(0x45);
    epd_send_data(y_start & 0xFF);
    epd_send_data((y_start >> 8) & 0xFF);
    epd_send_data(y_end & 0xFF);
    epd_send_data((y_end >> 8) & 0xFF);
}

static void epd_set_cursor(uint16_t x_start, uint16_t y_start)
{
    epd_send_cmd(0x4E);
    epd_send_data(x_start & 0xFF);

    epd_send_cmd(0x4F);
    epd_send_data(y_start & 0xFF);
    epd_send_data((y_start >> 8) & 0xFF);
}

static void epd_set_lut(const uint8_t *lut)
{
    epd_send_cmd(0x32);
    epd_write_bytes(lut, 153);
    epd_read_busy();

    epd_send_cmd(0x3F);
    epd_send_data(lut[153]);

    epd_send_cmd(0x03);
    epd_send_data(lut[154]);

    epd_send_cmd(0x04);
    epd_send_data(lut[155]);
    epd_send_data(lut[156]);
    epd_send_data(lut[157]);

    epd_send_cmd(0x2C);
    epd_send_data(lut[158]);
}

static void epd_turn_on_display(void)
{
    epd_send_cmd(0x22);
    epd_send_data(0xC7);
    epd_send_cmd(0x20);
    epd_read_busy();
}

static void epd_turn_on_display_part(void)
{
    epd_send_cmd(0x22);
    epd_send_data(0xCF);
    epd_send_cmd(0x20);
    epd_read_busy();
}

static void epd_write_frame(uint8_t command, uint8_t update_mode)
{
    epd_send_cmd(command);
    epd_write_bytes(s_frame_buffer, BSP_DISPLAY_BUFFER_SIZE);
    epd_send_cmd(0x22);
    epd_send_data(update_mode);
    epd_send_cmd(0x20);
    epd_read_busy();
}

esp_err_t bsp_display_init(void)
{
    if (s_frame_buffer == NULL) {
        s_frame_buffer = (uint8_t *)heap_caps_malloc(BSP_DISPLAY_BUFFER_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (s_frame_buffer == NULL) {
            s_frame_buffer = (uint8_t *)malloc(BSP_DISPLAY_BUFFER_SIZE);
        }
        assert(s_frame_buffer != NULL);
        memset(s_frame_buffer, 0xFF, BSP_DISPLAY_BUFFER_SIZE);
    }

    /* Configure GPIOs */
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << BSP_GPIO_EPD_3V3_EN) | (1ULL << BSP_GPIO_EPD_RST) |
                           (1ULL << BSP_GPIO_EPD_DC) | (1ULL << BSP_GPIO_EPD_CS);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << BSP_GPIO_EPD_BUSY);
    gpio_config(&io_conf);

    gpio_set_level((gpio_num_t)BSP_GPIO_EPD_3V3_EN, 0);
    epd_set_rst(1);

    /* Initialize SPI bus */
    if (s_spi_handle == NULL) {
        spi_bus_config_t buscfg = {};
        buscfg.mosi_io_num = BSP_GPIO_EPD_MOSI;
        buscfg.miso_io_num = -1;
        buscfg.sclk_io_num = BSP_GPIO_EPD_SCLK;
        buscfg.quadwp_io_num = -1;
        buscfg.quadhd_io_num = -1;
        buscfg.max_transfer_sz = BSP_DISPLAY_WIDTH * BSP_DISPLAY_HEIGHT;

        spi_device_interface_config_t devcfg = {};
        devcfg.mode = 0;
        devcfg.clock_speed_hz = 10 * 1000 * 1000;
        devcfg.spics_io_num = -1;
        devcfg.queue_size = 7;

        esp_err_t ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
        if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
            ESP_LOGE(TAG, "Failed to init SPI bus: %s", esp_err_to_name(ret));
            return ret;
        }

        ret = spi_bus_add_device(SPI2_HOST, &devcfg, &s_spi_handle);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to add SPI device: %s", esp_err_to_name(ret));
            return ret;
        }
    }

    /* HW Reset */
    epd_set_rst(1);
    vTaskDelay(pdMS_TO_TICKS(50));
    epd_set_rst(0);
    vTaskDelay(pdMS_TO_TICKS(20));
    epd_set_rst(1);
    vTaskDelay(pdMS_TO_TICKS(50));

    epd_read_busy();
    epd_send_cmd(0x12);
    epd_read_busy();

    epd_send_cmd(0x01);
    epd_send_data(0xC7);
    epd_send_data(0x00);
    epd_send_data(0x01);

    epd_send_cmd(0x11);
    epd_send_data(0x01);

    epd_set_windows(0, BSP_DISPLAY_WIDTH - 1, BSP_DISPLAY_HEIGHT - 1, 0);

    epd_send_cmd(0x3C);
    epd_send_data(0x01);

    epd_send_cmd(0x18);
    epd_send_data(0x80);

    epd_send_cmd(0x22);
    epd_send_data(0xB1);
    epd_send_cmd(0x20);

    epd_set_cursor(0, BSP_DISPLAY_HEIGHT - 1);
    epd_read_busy();

    epd_set_lut(WF_Full_1IN54);

    epd_write_frame(0x24, 0xC7);
    epd_send_cmd(0x26);
    epd_write_bytes(s_frame_buffer, BSP_DISPLAY_BUFFER_SIZE);
    bsp_display_init_partial();
    ESP_LOGI(TAG, "1.54 inch e-Paper display initialized");
    return ESP_OK;
}

esp_err_t bsp_display_init_partial(void)
{
    epd_set_rst(1);
    vTaskDelay(pdMS_TO_TICKS(50));
    epd_set_rst(0);
    vTaskDelay(pdMS_TO_TICKS(20));
    epd_set_rst(1);
    vTaskDelay(pdMS_TO_TICKS(50));

    epd_read_busy();
    epd_set_lut(WF_PARTIAL_1IN54);

    epd_send_cmd(0x37);
    for (int i = 0; i < 5; i++) epd_send_data(0x00);
    epd_send_data(0x40);
    for (int i = 0; i < 4; i++) epd_send_data(0x00);

    epd_send_cmd(0x3C);
    epd_send_data(0x80);

    epd_send_cmd(0x22);
    epd_send_data(0xC0);
    epd_send_cmd(0x20);
    epd_read_busy();

    ESP_LOGI(TAG, "Partial display refresh mode initialized");
    return ESP_OK;
}

void bsp_display_clear(void)
{
    if (s_frame_buffer != NULL) {
        memset(s_frame_buffer, 0xFF, BSP_DISPLAY_BUFFER_SIZE);
    }
}

void bsp_display_flush(void)
{
    if (s_frame_buffer == NULL) return;
    epd_write_frame(0x24, 0xC7);
}

void bsp_display_flush_partial(void)
{
    if (s_frame_buffer == NULL) return;
    epd_write_frame(0x24, 0xCF);
}

void bsp_display_draw_pixel(uint16_t x, uint16_t y, bsp_display_color_t color)
{
    if (x >= BSP_DISPLAY_WIDTH || y >= BSP_DISPLAY_HEIGHT || s_frame_buffer == NULL) {
        return;
    }
    uint16_t index = y * (BSP_DISPLAY_WIDTH / 8) + (x >> 3);
    uint8_t bit = 7 - (x & 0x07);
    if (color == BSP_DISPLAY_COLOR_WHITE) {
        s_frame_buffer[index] |= (1 << bit);
    } else {
        s_frame_buffer[index] &= ~(1 << bit);
    }
}

uint8_t *bsp_display_get_buffer(void)
{
    return s_frame_buffer;
}
