/**
 * @file bsp_rtc_mem.c
 * @brief Internal ESP32-S3 RTC Slow Memory Persistent Storage Implementation
 * 
 * @attribution
 * - BSP Architecture: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "bsp/bsp_rtc_mem.h"

static const char *TAG = "bsp_rtc_mem";

RTC_DATA_ATTR static bsp_rtc_state_t s_rtc_state;

esp_err_t bsp_rtc_mem_init(void)
{
    if (s_rtc_state.magic != BSP_RTC_MEM_MAGIC) {
        ESP_LOGI(TAG, "RTC Slow Memory uninitialized or invalid (magic 0x%08lX); re-initializing", 
                 (unsigned long)s_rtc_state.magic);
        bsp_rtc_mem_reset();
    } else {
        s_rtc_state.boot_count++;
        ESP_LOGI(TAG, "RTC Slow Memory valid (Boot count: %lu, Deep sleep count: %lu)",
                 (unsigned long)s_rtc_state.boot_count, (unsigned long)s_rtc_state.deep_sleep_count);
    }
    return ESP_OK;
}

bsp_rtc_state_t *bsp_rtc_mem_get_state(void)
{
    return &s_rtc_state;
}

uint32_t bsp_rtc_mem_get_boot_count(void)
{
    return s_rtc_state.boot_count;
}

void bsp_rtc_mem_set_next_init_mode(bsp_init_mode_t mode)
{
    s_rtc_state.next_init_mode = (uint8_t)mode;
}

bsp_init_mode_t bsp_rtc_mem_get_next_init_mode(void)
{
    return (bsp_init_mode_t)s_rtc_state.next_init_mode;
}

esp_err_t bsp_rtc_mem_read_scratchpad(uint8_t *dest, size_t len)
{
    if (dest == NULL || len == 0 || len > sizeof(s_rtc_state.scratchpad)) {
        return ESP_ERR_INVALID_ARG;
    }
    memcpy(dest, s_rtc_state.scratchpad, len);
    return ESP_OK;
}

esp_err_t bsp_rtc_mem_write_scratchpad(const uint8_t *src, size_t len)
{
    if (src == NULL || len == 0 || len > sizeof(s_rtc_state.scratchpad)) {
        return ESP_ERR_INVALID_ARG;
    }
    memcpy(s_rtc_state.scratchpad, src, len);
    return ESP_OK;
}

void bsp_rtc_mem_reset(void)
{
    memset(&s_rtc_state, 0, sizeof(bsp_rtc_state_t));
    s_rtc_state.magic          = BSP_RTC_MEM_MAGIC;
    s_rtc_state.boot_count     = 1;
    s_rtc_state.last_init_mode = (uint8_t)BSP_INIT_MODE_FULL;
    s_rtc_state.next_init_mode = (uint8_t)BSP_INIT_MODE_FULL;
}
