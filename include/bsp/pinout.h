/**
 * @file pinout.h
 * @brief Hardware GPIO Pinout Definitions for ESP32-S3 Touch ePaper 1.54 Board
 * 
 * Hardware pin definitions derived from Waveshare ESP32-S3-ePaper-1.54 V2 schematic
 * and hardware multiplexing documentation.
 * 
 * @attribution
 * - Hardware Schematic & Pin Assignments: Waveshare Electronics (https://www.waveshare.com)
 * - Microcontroller: Espressif Systems ESP32-S3 (https://www.espressif.com)
 * - BSP Unification: Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_PINOUT_H
#define BSP_PINOUT_H

#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/* System & Power */
#define BSP_GPIO_BOOT_KEY       CONFIG_BSP_GPIO_BOOT_KEY
#define BSP_GPIO_USER_LED       CONFIG_BSP_GPIO_USER_LED
#define BSP_GPIO_BAT_ADC        CONFIG_BSP_GPIO_BAT_ADC
#define BSP_GPIO_BAT_CTRL       CONFIG_BSP_GPIO_BAT_CTRL
#define BSP_GPIO_BAT_KEY        CONFIG_BSP_GPIO_BAT_KEY
#define BSP_GPIO_RTC_INT        CONFIG_BSP_GPIO_RTC_INT

/* Shared I2C Bus */
#define BSP_GPIO_I2C_SCL        CONFIG_BSP_GPIO_I2C_SCL
#define BSP_GPIO_I2C_SDA        CONFIG_BSP_GPIO_I2C_SDA

/* 1.54" e-Paper SPI Display */
#define BSP_GPIO_EPD_3V3_EN     CONFIG_BSP_GPIO_EPD_3V3_EN
#define BSP_GPIO_EPD_DC         CONFIG_BSP_GPIO_EPD_DC
#define BSP_GPIO_EPD_RST        CONFIG_BSP_GPIO_EPD_RST
#define BSP_GPIO_EPD_CS         CONFIG_BSP_GPIO_EPD_CS
#define BSP_GPIO_EPD_SCLK       CONFIG_BSP_GPIO_EPD_SCLK
#define BSP_GPIO_EPD_MOSI       CONFIG_BSP_GPIO_EPD_MOSI
#define BSP_GPIO_EPD_BUSY       CONFIG_BSP_GPIO_EPD_BUSY

/* Capacitive Touch (FT6336 - Touch Models Only) */
#define BSP_GPIO_TOUCH_RST      CONFIG_BSP_GPIO_TOUCH_RST
#define BSP_GPIO_TOUCH_INT      CONFIG_BSP_GPIO_TOUCH_INT

/* MicroSD Card SPI Bus */
#define BSP_GPIO_SD_CLK         CONFIG_BSP_GPIO_SD_CLK
#define BSP_GPIO_SD_MISO        CONFIG_BSP_GPIO_SD_MISO
#define BSP_GPIO_SD_MOSI        CONFIG_BSP_GPIO_SD_MOSI

/* ES8311 Audio Codec & NS4150B Power Amp */
#define BSP_GPIO_I2S_MCLK       CONFIG_BSP_GPIO_I2S_MCLK
#define BSP_GPIO_I2S_SCLK       CONFIG_BSP_GPIO_I2S_SCLK
#define BSP_GPIO_I2S_LRCK       CONFIG_BSP_GPIO_I2S_LRCK
#define BSP_GPIO_I2S_ASOUT      CONFIG_BSP_GPIO_I2S_ASOUT
#define BSP_GPIO_I2S_DSIN       CONFIG_BSP_GPIO_I2S_DSIN
#define BSP_GPIO_PA_CTRL        CONFIG_BSP_GPIO_PA_CTRL
#define BSP_GPIO_PA_EN          CONFIG_BSP_GPIO_PA_EN

#ifdef __cplusplus
}
#endif

#endif /* BSP_PINOUT_H */
