/**
 * @file bsp_display.cpp
 * @brief lvgl backend
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics (https://www.waveshare.com)
 * - Microcontroller: Espressif Systems ESP32-S3 (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include <algorithm>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "bsp/bsp_display.h"

static const char *TAG = "bsp_display";

#define EPD_FULL_REFRESH_TIMEOUT_MS    10000
#define EPD_PARTIAL_REFRESH_TIMEOUT_MS 3000

// SSD1681 Factory-calibrated 159-byte Partial LUT (Datasheet Figure 6-6 layout)
static const uint8_t WF_PARTIAL_1IN54[159] = {
    // 000..011: LUT0 (Black -> Black)
    0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // 012..023: LUT1 (Black -> White)
    0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // 024..035: LUT2 (White -> Black)
    0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // 036..047: LUT3 (White -> White)
    0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // 048..059: LUT4 (VCOM Modulation)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // 060..143: Timing Groups 0 to 11 (TP, SR, RP)
    0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Group 0 (TP0A = 15 frames)
    0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, // Group 1
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Group 2
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Group 3
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Group 4
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Group 5
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Group 6
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Group 7
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Group 8
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Group 9
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Group 10
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Group 11
    // 144..149: Frame Rates (FR0..FR11 = 50Hz) & 150..152: XON
    0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00,
    // 153..158: EOPT (0x3F), Gate (0x03), Source (0x04), VCOM (0x2C)
    0x02, 0x17, 0x41, 0xB0, 0x32, 0x28
};

static spi_device_handle_t s_spi_handle = NULL;
static uint8_t *s_frame_buffer          = NULL;
static uint8_t *s_prev_frame_buffer     = NULL;
static uint32_t s_partial_refresh_count = 0;

static inline void epd_set_cs (uint8_t level) { gpio_set_level((gpio_num_t)BSP_GPIO_EPD_CS,  level); }
static inline void epd_set_dc (uint8_t level) { gpio_set_level((gpio_num_t)BSP_GPIO_EPD_DC,  level); }
static inline void epd_set_rst(uint8_t level) { gpio_set_level((gpio_num_t)BSP_GPIO_EPD_RST, level); }

esp_err_t bsp_display_wait_busy(uint32_t timeout_ms)
{
    vTaskDelay(pdMS_TO_TICKS(10));
    TickType_t start_tick = xTaskGetTickCount();
    TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);

    while (gpio_get_level((gpio_num_t)BSP_GPIO_EPD_BUSY) == 1) {
        if ((xTaskGetTickCount() - start_tick) > timeout_ticks) {
            ESP_LOGE(TAG, "Busy pin wait timeout (%lu ms)", (unsigned long)timeout_ms);
            return ESP_ERR_TIMEOUT;
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    return ESP_OK;
}

static void epd_send_byte(uint8_t data)
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;
    t.tx_buffer = &data;
    spi_device_polling_transmit(s_spi_handle, &t);
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
    if (len == 0 || data == NULL) return;
    epd_set_dc(1);
    epd_set_cs(0);
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8 * len;
    t.tx_buffer = data;
    spi_device_polling_transmit(s_spi_handle, &t);
    epd_set_cs(1);
}

static void epd_set_windows(uint16_t x_start_byte, uint16_t y_start, uint16_t x_end_byte, uint16_t y_end)
{
    epd_send_cmd(0x44);
    epd_send_data(x_start_byte & 0xFF);
    epd_send_data(x_end_byte & 0xFF);

    epd_send_cmd(0x45);
    epd_send_data(y_start & 0xFF);
    epd_send_data((y_start >> 8) & 0xFF);
    epd_send_data(y_end & 0xFF);
    epd_send_data((y_end >> 8) & 0xFF);
}

static void epd_set_cursor(uint16_t x_start_byte, uint16_t y_start)
{
    epd_send_cmd(0x4E);
    epd_send_data(x_start_byte & 0xFF);

    epd_send_cmd(0x4F);
    epd_send_data(y_start & 0xFF);
    epd_send_data((y_start >> 8) & 0xFF);
}

void epd_load_custom_lut(const uint8_t *lut_buffer)
{
    if (lut_buffer == NULL) return;

    epd_send_cmd(0x32);
    epd_write_bytes(lut_buffer, 153);

    epd_send_cmd(0x3F);
    epd_send_data(lut_buffer[153]);

    epd_send_cmd(0x03);
    epd_send_data(lut_buffer[154]);

    epd_send_cmd(0x04);
    epd_send_data(lut_buffer[155]);
    epd_send_data(lut_buffer[156]);
    epd_send_data(lut_buffer[157]);

    epd_send_cmd(0x2C);
    epd_send_data(lut_buffer[158]);
}

static void epd_apply_core_registers(void)
{
    epd_send_cmd(0x01);
    epd_send_data((BSP_DISPLAY_HEIGHT - 1) & 0xFF);
    epd_send_data(((BSP_DISPLAY_HEIGHT - 1) >> 8) & 0xFF);
    epd_send_data(0x00);

    epd_send_cmd(0x11);
    epd_send_data(0x03); // Increment X, Increment Y

    epd_send_cmd(0x3C);
    epd_send_data(0x05);

    epd_send_cmd(0x18);
    epd_send_data(0x80);

    epd_send_cmd(0x22);
    epd_send_data(0xB1); // Load Temp & OTP Waveform
    epd_send_cmd(0x20);
    bsp_display_wait_busy(EPD_FULL_REFRESH_TIMEOUT_MS);

    epd_set_windows(0, 0, (BSP_DISPLAY_WIDTH / 8) - 1, BSP_DISPLAY_HEIGHT - 1);
    epd_set_cursor(0, 0);
}

esp_err_t bsp_display_init(void)
{
    if (s_frame_buffer == NULL) {
        s_frame_buffer = (uint8_t *)heap_caps_malloc(BSP_DISPLAY_BUFFER_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
        if (s_frame_buffer == NULL) s_frame_buffer = (uint8_t *)malloc(BSP_DISPLAY_BUFFER_SIZE);
        if (s_frame_buffer == NULL) return ESP_ERR_NO_MEM;
        memset(s_frame_buffer, 0xFF, BSP_DISPLAY_BUFFER_SIZE);
    }

    if (s_prev_frame_buffer == NULL) {
        s_prev_frame_buffer = (uint8_t *)heap_caps_malloc(BSP_DISPLAY_BUFFER_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
        if (s_prev_frame_buffer == NULL) s_prev_frame_buffer = (uint8_t *)malloc(BSP_DISPLAY_BUFFER_SIZE);
        if (s_prev_frame_buffer == NULL) return ESP_ERR_NO_MEM;
        memset(s_prev_frame_buffer, 0xFF, BSP_DISPLAY_BUFFER_SIZE);
    }

    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << BSP_GPIO_EPD_RST) | (1ULL << BSP_GPIO_EPD_DC)
                                                      | (1ULL << BSP_GPIO_EPD_CS);
    io_conf.mode         = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en   = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1ULL << BSP_GPIO_EPD_BUSY);
    io_conf.mode         = GPIO_MODE_INPUT;
    io_conf.pull_up_en   = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type    = GPIO_INTR_DISABLE;    
    gpio_config(&io_conf);

    //gpio_set_level((gpio_num_t)BSP_GPIO_EPD_3V3_EN, 0); // Power on panel (Active LOW)

    if (s_spi_handle == NULL) {
        spi_bus_config_t buscfg = {};        
        buscfg.mosi_io_num     = BSP_GPIO_EPD_MOSI;
        buscfg.miso_io_num     = -1;
        buscfg.sclk_io_num     = BSP_GPIO_EPD_SCLK;
        buscfg.quadwp_io_num   = -1;
        buscfg.quadhd_io_num   = -1;
        buscfg.max_transfer_sz = BSP_DISPLAY_WIDTH * BSP_DISPLAY_HEIGHT;

        spi_device_interface_config_t devcfg = {};
        devcfg.mode           = 0;
        devcfg.clock_speed_hz = 20 * 1000 * 1000;
        devcfg.spics_io_num   = -1;
        devcfg.queue_size     = 7;

        esp_err_t ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
        if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) return ret;

        ret = spi_bus_add_device(SPI2_HOST, &devcfg, &s_spi_handle);
        if (ret != ESP_OK) return ret;
    }

    // Hardware Reset
    epd_set_rst(1);
    vTaskDelay(pdMS_TO_TICKS(10));
    epd_set_rst(0);
    vTaskDelay(pdMS_TO_TICKS(10));
    epd_set_rst(1);
    vTaskDelay(pdMS_TO_TICKS(10));

    bsp_display_wait_busy(EPD_FULL_REFRESH_TIMEOUT_MS);
    epd_send_cmd(0x12); // SW Reset
    bsp_display_wait_busy(EPD_FULL_REFRESH_TIMEOUT_MS);

    epd_apply_core_registers();

    ESP_LOGI(TAG, "SSD1681 1.54-inch EPD initialized successfully");
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
    if (s_frame_buffer == NULL || s_prev_frame_buffer == NULL) return;

    epd_set_windows(0, 0, (BSP_DISPLAY_WIDTH / 8) - 1, BSP_DISPLAY_HEIGHT - 1);

    epd_set_cursor(0, 0);
    epd_send_cmd(0x24);
    epd_write_bytes(s_frame_buffer, BSP_DISPLAY_BUFFER_SIZE);

    epd_set_cursor(0, 0);
    epd_send_cmd(0x26);
    epd_write_bytes(s_frame_buffer, BSP_DISPLAY_BUFFER_SIZE);

    epd_send_cmd(0x22);
    epd_send_data(0xC7);
    epd_send_cmd(0x20);
    bsp_display_wait_busy(EPD_FULL_REFRESH_TIMEOUT_MS);

    memcpy(s_prev_frame_buffer, s_frame_buffer, BSP_DISPLAY_BUFFER_SIZE);
    s_partial_refresh_count = 0;
}

void bsp_display_flush_partial(void)
{
    bsp_display_flush_partial_area(0, 0, BSP_DISPLAY_WIDTH - 1, BSP_DISPLAY_HEIGHT - 1);
}

void bsp_display_flush_partial_area(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
    if (s_frame_buffer == NULL || s_prev_frame_buffer == NULL) return;

    x_start = std::min<uint16_t>(x_start, BSP_DISPLAY_WIDTH - 1);
    x_end   = std::min<uint16_t>(x_end,   BSP_DISPLAY_WIDTH - 1);
    y_start = std::min<uint16_t>(y_start, BSP_DISPLAY_HEIGHT - 1);
    y_end   = std::min<uint16_t>(y_end,   BSP_DISPLAY_HEIGHT - 1);

    if (x_start > x_end) std::swap(x_start, x_end);
    if (y_start > y_end) std::swap(y_start, y_end);

    uint16_t x_s_byte = x_start / 8;
    uint16_t x_e_byte = x_end / 8;
    uint16_t bytes_per_line = (x_e_byte - x_s_byte) + 1;

    epd_set_windows(x_s_byte, y_start, x_e_byte, y_end);

    // 1. Write Old RAM (0x26)
    epd_set_cursor(x_s_byte, y_start);
    epd_send_cmd(0x26);
    for (uint16_t y = y_start; y <= y_end; y++) {
        uint32_t offset = (y * (BSP_DISPLAY_WIDTH / 8)) + x_s_byte;
        epd_write_bytes(&s_prev_frame_buffer[offset], bytes_per_line);
    }

    // 2. Write New RAM (0x24)
    epd_set_cursor(x_s_byte, y_start);
    epd_send_cmd(0x24);
    for (uint16_t y = y_start; y <= y_end; y++) {
        uint32_t offset = (y * (BSP_DISPLAY_WIDTH / 8)) + x_s_byte;
        epd_write_bytes(&s_frame_buffer[offset], bytes_per_line);
    }

    // 3. Load SSD1681 Partial LUT
    epd_load_custom_lut(WF_PARTIAL_1IN54);

    // 4. Trigger Display Update (Mode 1 Display Execution)
    epd_send_cmd(0x22);
    epd_send_data(0xC7);
    epd_send_cmd(0x20);
    bsp_display_wait_busy(EPD_PARTIAL_REFRESH_TIMEOUT_MS);

    // 5. Synchronize local frame buffer
    for (uint16_t y = y_start; y <= y_end; y++) {
        uint32_t offset = (y * (BSP_DISPLAY_WIDTH / 8)) + x_s_byte;
        memcpy(&s_prev_frame_buffer[offset], &s_frame_buffer[offset], bytes_per_line);
    }

    s_partial_refresh_count++;
}

void bsp_display_deep_sleep(void)
{
    bsp_display_wait_busy(EPD_FULL_REFRESH_TIMEOUT_MS);
    epd_send_cmd(0x3C);
    epd_send_data(0x01);
    epd_send_cmd(0x10);
    epd_send_data(0x01);
}

void bsp_display_draw_pixel(uint16_t x, uint16_t y, bsp_display_color_t color)
{
    if (x >= BSP_DISPLAY_WIDTH || y >= BSP_DISPLAY_HEIGHT || s_frame_buffer == NULL) return;
    uint32_t index = y * (BSP_DISPLAY_WIDTH / 8) + (x >> 3);
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