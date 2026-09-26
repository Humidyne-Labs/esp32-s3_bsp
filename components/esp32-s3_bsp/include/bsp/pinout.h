/**
 * @file pinout.h
 * @brief Complete Hardware Pinout Mapping for Waveshare ESP32-S3-Touch-ePaper-1.54 V2
 * 
 * Hardware Subsystems:
 *  - E-Paper SPI Display (SSD1681 200x200 1-bit Mono)
 *  - Shared I2C Bus (SHTC3, PCF85063A, FT6336)
 *  - I2S Audio Codec & Amp (ES8311 + NS4168)
 *  - MicroSD Card Slot (SPI / SDMMC Mode)
 *  - Power Management (BAT_CTRL Latch, Battery ADC Divider)
 *  - Tactile User Input Buttons (BOOT0, BAT_KEY)
 * 
 * @attribution
 * - Board Schematic: Waveshare Electronics (https://www.waveshare.com)
 * - BSP Implementation: Humidyne Labs / Humiditron (2026)
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef BSP_PINOUT_H
#define BSP_PINOUT_H

#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * 1. Power Control, Battery ADC, & Status LED
 * ========================================================================= */
/**
 * @brief Power Hold Latch Output Pin (BAT_Control).
 * Must be driven HIGH on boot to maintain LDO power rail from battery/regulator.
 */
#define BSP_PIN_POWER_HOLD          GPIO_NUM_17
#define BSP_GPIO_BAT_CTRL           BSP_PIN_POWER_HOLD /* Backward-compatibility alias */

/**
 * @brief Battery Voltage ADC Sensing Input Pin (ADC1 Channel 3).
 * Connected to a 1:2 resistor voltage divider network (R1=100k, R2=100k).
 */
#define BSP_PIN_BATTERY_ADC         GPIO_NUM_4
#define BSP_GPIO_BAT_ADC            BSP_PIN_BATTERY_ADC

/**
 * @brief Onboard User / Status Indicator LED (Open-Drain, Active Low: 0=ON, 1=OFF).
 */
#define BSP_PIN_LED_STATUS          GPIO_NUM_3
#define BSP_GPIO_USER_LED           BSP_PIN_LED_STATUS

/* =========================================================================
 * 2. Tactile User Input Buttons
 * ========================================================================= */
/**
 * @brief Hardware Boot / User Action Button (Active Low, requires internal pull-up).
 */
#define BSP_PIN_BUTTON_BOOT         GPIO_NUM_0
#define BSP_GPIO_BOOT_KEY           BSP_PIN_BUTTON_BOOT

/**
 * @brief Hardware Power / Battery Key (Active Low, requires internal pull-up).
 */
#define BSP_PIN_BUTTON_POWER        GPIO_NUM_18
#define BSP_GPIO_BAT_KEY            BSP_PIN_BUTTON_POWER

/* =========================================================================
 * 3. Shared I2C Master Bus (Sensors, RTC, Capacitive Touch)
 * ========================================================================= */
#define BSP_PIN_I2C_SDA             GPIO_NUM_47  /*!< Shared I2C Serial Data line (RTC_SDA, external 4.7k pull-up) */
#define BSP_PIN_I2C_SCL             GPIO_NUM_48  /*!< Shared I2C Serial Clock line (RTC_SCL, external 4.7k pull-up) */
#define BSP_PIN_RTC_INT             GPIO_NUM_5   /*!< PCF85063A RTC INT (Open-Drain, No external pull-up, requires internal pull-up) */
#define BSP_GPIO_I2C_SDA            BSP_PIN_I2C_SDA
#define BSP_GPIO_I2C_SCL            BSP_PIN_I2C_SCL
#define BSP_GPIO_RTC_INT            BSP_PIN_RTC_INT

/* Capacitive Touch (FT6336) */
#define BSP_PIN_TOUCH_RST           GPIO_NUM_7   /*!< FT6336 Touch reset (Active Low) */
#define BSP_PIN_TOUCH_INT           GPIO_NUM_21  /*!< FT6336 Touch interrupt (Active Low, internal pull-up) */
#define BSP_GPIO_TOUCH_RST          BSP_PIN_TOUCH_RST
#define BSP_GPIO_TOUCH_INT          BSP_PIN_TOUCH_INT

/* 7-Bit I2C Slave Addresses */
#define BSP_I2C_ADDR_SHTC3          0x70         /*!< Sensirion SHTC3 Environmental Sensor */
#define BSP_I2C_ADDR_PCF85063       0x51         /*!< NXP PCF85063A Real-Time Clock */
#define BSP_I2C_ADDR_FT6336         0x38         /*!< FocalTech FT6336 Touch Controller */
#define BSP_I2C_ADDR_TOUCH          BSP_I2C_ADDR_FT6336
#define BSP_I2C_ADDR_ES8311         0x18         /*!< Everest Semi ES8311 Audio Codec */

/* =========================================================================
 * 4. 1.54" SPI e-Paper Display (SSD1681 200x200 Mono)
 * ========================================================================= */
#define BSP_PIN_EPD_3V3_EN          GPIO_NUM_6   /*!< EPD 3.3V Power Enable (Active Low: 0=ON, 1=OFF) */
#define BSP_PIN_EPD_BUSY            GPIO_NUM_8   /*!< Display Busy Line (High = Busy) */
#define BSP_PIN_EPD_RST             GPIO_NUM_9   /*!< Display Hardware Reset (Active Low) */
#define BSP_PIN_EPD_DC              GPIO_NUM_10  /*!< Display Data / Command control line */
#define BSP_PIN_EPD_CS              GPIO_NUM_11  /*!< Display SPI Chip Select (Active Low) */
#define BSP_PIN_EPD_SCK             GPIO_NUM_12  /*!< Display SPI Serial Clock */
#define BSP_PIN_EPD_MOSI            GPIO_NUM_13  /*!< Display SPI Master Out Slave In (Data In) */

#define BSP_GPIO_EPD_3V3_EN         BSP_PIN_EPD_3V3_EN
#define BSP_GPIO_EPD_BUSY           BSP_PIN_EPD_BUSY
#define BSP_GPIO_EPD_RST            BSP_PIN_EPD_RST
#define BSP_GPIO_EPD_DC             BSP_PIN_EPD_DC
#define BSP_GPIO_EPD_CS             BSP_PIN_EPD_CS
#define BSP_GPIO_EPD_SCLK           BSP_PIN_EPD_SCK
#define BSP_GPIO_EPD_MOSI           BSP_PIN_EPD_MOSI

/* Display Geometry */
#define BSP_DISPLAY_WIDTH           200          /*!< Physical Width in Pixels */
#define BSP_DISPLAY_HEIGHT          200          /*!< Physical Height in Pixels */
#define BSP_DISPLAY_DPI             188          /*!< Pixel Density (DPI) */

/* =========================================================================
 * 5. I2S Audio Codec (ES8311) & Class-D Amp (NS4168)
 * ========================================================================= */
#define BSP_PIN_I2S_MCLK            GPIO_NUM_14  /*!< ES8311 Master Clock (MCLK) */
#define BSP_PIN_I2S_SCLK            GPIO_NUM_15  /*!< ES8311 Bit / Serial Clock (BCLK/SCLK) */
#define BSP_PIN_I2S_ASDOUT          GPIO_NUM_16  /*!< ES8311 Serial Audio Data Out to ESP32 (DIN) */
#define BSP_PIN_I2S_LRCK            GPIO_NUM_38  /*!< ES8311 Left/Right Clock (WS) */
#define BSP_PIN_I2S_DSDIN           GPIO_NUM_45  /*!< ES8311 Serial Audio Data In from ESP32 (DOUT) */
#define BSP_PIN_PA_EN               GPIO_NUM_42  /*!< NS4168 Power Amp Enable (Active Low: 0=ON) */
#define BSP_PIN_PA_CTRL             GPIO_NUM_46  /*!< NS4168 Power Amp Control */

#define BSP_GPIO_I2S_MCLK           BSP_PIN_I2S_MCLK
#define BSP_GPIO_I2S_SCLK           BSP_PIN_I2S_SCLK
#define BSP_GPIO_I2S_ASOUT          BSP_PIN_I2S_ASDOUT
#define BSP_GPIO_I2S_LRCK           BSP_PIN_I2S_LRCK
#define BSP_GPIO_I2S_DSIN           BSP_PIN_I2S_DSDIN
#define BSP_GPIO_PA_EN              BSP_PIN_PA_EN
#define BSP_GPIO_PA_CTRL            BSP_PIN_PA_CTRL

/* =========================================================================
 * 6. MicroSD Card Storage (SPI / SDMMC Mode)
 * ========================================================================= */
#define BSP_PIN_SD_CLK              GPIO_NUM_39  /*!< MicroSD Clock */
#define BSP_PIN_SD_MISO             GPIO_NUM_40  /*!< MicroSD MISO / D0 */
#define BSP_PIN_SD_MOSI             GPIO_NUM_41  /*!< MicroSD MOSI / CMD */

#define BSP_GPIO_SD_CLK             BSP_PIN_SD_CLK
#define BSP_GPIO_SD_MISO            BSP_PIN_SD_MISO
#define BSP_GPIO_SD_MOSI            BSP_PIN_SD_MOSI

#ifdef __cplusplus
}
#endif

#endif /* BSP_PINOUT_H */
