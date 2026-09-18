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

/* System & Power (Waveshare Schematic) */
#define BSP_GPIO_BOOT_KEY       (0)
#define BSP_GPIO_BAT_ADC        (1)
#define BSP_GPIO_PWR_KEY        (2)
#define BSP_GPIO_RTC_INT        (3)
#define BSP_GPIO_USER_LED       (38)

/* Shared I2C Bus */
#define BSP_GPIO_I2C_SCL        (6)
#define BSP_GPIO_I2C_SDA        (7)

/* 1.54" e-Paper SPI Display */
#define BSP_GPIO_EPD_DC         (8)
#define BSP_GPIO_EPD_RST        (9)
#define BSP_GPIO_EPD_CS         (10)
#define BSP_GPIO_EPD_SCLK       (11)
#define BSP_GPIO_EPD_MOSI       (12)
#define BSP_GPIO_EPD_BUSY       (13)

/* Capacitive Touch (FT6336 - Touch Models Only) */
#define BSP_GPIO_TOUCH_RST      (4)
#define BSP_GPIO_TOUCH_INT      (5)

/* MicroSD Card SPI Bus */
#define BSP_GPIO_SD_CLK         (39)
#define BSP_GPIO_SD_MISO        (40)
#define BSP_GPIO_SD_MOSI        (41)
#define BSP_GPIO_SD_CS          (42)

/* ES8311 Audio Codec & Power Amp */
#define BSP_GPIO_I2S_MCLK       (14)
#define BSP_GPIO_I2S_SCLK       (15)
#define BSP_GPIO_I2S_LRCK       (16)
#define BSP_GPIO_I2S_ASOUT      (17)
#define BSP_GPIO_I2S_DSIN       (18)
#define BSP_GPIO_PA_CTRL        (47)
#define BSP_GPIO_PA_EN          (48)

#ifdef __cplusplus
}
#endif

#endif /* BSP_PINOUT_H */
