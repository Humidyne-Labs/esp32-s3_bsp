/**
 * @file main.c
 * @brief Standalone Example for ESP32-S3 BSP Lifecycle and State Engine
 *
 * Demonstrates lifecycle startup, context retrieval, persistent RTC scratchpad
 * state storage, stage tracking, and shutdown hooks without blocking execution.
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "bsp/bsp_lifecycle.h"

static const char *TAG = "lifecycle_example";

typedef struct {
    uint32_t magic;
    uint32_t boot_index;
} app_state_payload_t;

static void on_cold_boot(void *user_data)
{
    ESP_LOGI(TAG, "Lifecycle hook: Cold boot detected");

    bsp_wake_context_t ctx;
    ESP_ERROR_CHECK(bsp_lifecycle_get_context(&ctx));
    ESP_LOGI(TAG, "Boot count: %lu, Stage: %u",
             (unsigned long)ctx.boot_count, (unsigned int)ctx.app_stage);

    app_state_payload_t state = {
        .magic      = 0xCAFEBABE,
        .boot_index = ctx.boot_count,
    };

    ESP_ERROR_CHECK(bsp_lifecycle_save_state(&state, sizeof(state)));
    ESP_ERROR_CHECK(bsp_lifecycle_set_stage(1));
}

static void on_wake(const bsp_wake_context_t *ctx, void *user_data)
{
    ESP_LOGI(TAG, "Lifecycle hook: Resumed from sleep");
    ESP_LOGI(TAG, "Reset reason: %d, Wake cause: %d, Boot count: %lu",
             (int)ctx->reset_reason, (int)ctx->wake_cause,
             (unsigned long)ctx->boot_count);

    app_state_payload_t loaded_state = {0};
    ESP_ERROR_CHECK(bsp_lifecycle_load_state(&loaded_state, sizeof(loaded_state)));
    ESP_LOGI(TAG, "Restored state -> magic: 0x%08lX, boot_index: %lu",
             (unsigned long)loaded_state.magic,
             (unsigned long)loaded_state.boot_index);

    uint8_t current_stage = bsp_lifecycle_get_stage();
    ESP_ERROR_CHECK(bsp_lifecycle_set_stage(current_stage + 1));
}

static void on_before_sleep(bsp_sleep_mode_t mode, uint32_t duration_sec, void *user_data)
{
    ESP_LOGI(TAG, "Lifecycle hook: Before sleep (mode=%d, duration=%lu s)",
             (int)mode, (unsigned long)duration_sec);
}

static void on_shutdown(void *user_data)
{
    ESP_LOGI(TAG, "Lifecycle hook: Shutdown initiated");
}

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing BSP Application Lifecycle Engine...");

    const bsp_app_lifecycle_t lifecycle_cfg = {
        .on_cold_boot    = on_cold_boot,
        .on_wake         = on_wake,
        .on_before_sleep = on_before_sleep,
        .on_shutdown     = on_shutdown,
        .user_data       = NULL,
    };

    ESP_ERROR_CHECK(bsp_app_start(&lifecycle_cfg));

    bsp_wake_context_t ctx;
    ESP_ERROR_CHECK(bsp_lifecycle_get_context(&ctx));

    uint8_t current_stage = bsp_lifecycle_get_stage();
    ESP_LOGI(TAG, "Current stage: %u, Total boots: %lu",
             (unsigned int)current_stage, (unsigned long)ctx.boot_count);

    app_state_payload_t verified_state = {0};
    ESP_ERROR_CHECK(bsp_lifecycle_load_state(&verified_state, sizeof(verified_state)));
    ESP_LOGI(TAG, "Verified payload magic: 0x%08lX",
             (unsigned long)verified_state.magic);

    ESP_LOGI(TAG, "Invoking registered shutdown hook...");
    bsp_lifecycle_invoke_shutdown();

    ESP_LOGI(TAG, "Lifecycle example completed.");
}