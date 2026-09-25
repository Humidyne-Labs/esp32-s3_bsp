/**
 * @file pinout.h
 * @brief Complete Hardware Pinout Mapping for Waveshare ESP32-S3-Touch-ePaper-1.54 V2
 * 
 * Defines all hardware GPIO allocations, peripheral bus assignments, and power control
 * pins according to the official Waveshare V2 schematics.
 * 
 * Hardware Subsystems:
 *  - E-Paper SPI Display (SSD1681 200x200 1-bit Mono)
 *  - Shared I2C Bus (SHTC3, PCF85063A, CST816S)
 *  - I2S Audio Amplifier (MAX98357A)
 *  - MicroSD Card Slot (SPI Mode)
 *  - Power Management & Battery ADC Divider
 *  - Tactile User Input Buttons
 * 
 * @attribution
 * - Board Schematic: Waveshare Electronics (https://www.waveshare.com)
 * - BSP Unification: Humidyne Labs / Humiditron (2026)
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
 * @brief Power Hold Latch Output Pin.
 * Must be driven HIGH on boot to maintain LDO power rail from battery/regulator.
 */
#define BSP_PIN_POWER_HOLD          GPIO_NUM_2

/**
 * @brief Battery Voltage ADC Sensing Input Pin (ADC1 Channel 4).
 * Connected to a 1:2 resistor voltage divider network (R1=100k, R2=100k).
 */
#define BSP_PIN_BATTERY_ADC         GPIO_NUM_5

/**
 * @brief Onboard User / Status Indicator LED (Active High).
 */
#define BSP_PIN_LED_STATUS          GPIO_NUM_1

/* =========================================================================
 * 2. Tactile User Input Buttons
 * ========================================================================= */
/**
 * @brief Hardware Boot / User Action Button (Active Low, internal pull-up).
 * Used for audio chime trigger (single click) and factory reset wipe (5s long press).
 */
#define BSP_PIN_BUTTON_BOOT         GPIO_NUM_0

/**
 * @brief Hardware Power / Wakeup Key (Active Low, internal pull-up).
 * Connected to RTC GPIO for waking from deep sleep and performing safe power-off.
 */
#define BSP_PIN_BUTTON_POWER        GPIO_NUM_3

/* =========================================================================
 * 3. Shared I2C Master Bus (Sensors, RTC, Capacitive Touch)
 * ========================================================================= */
#define BSP_PIN_I2C_SDA             GPIO_NUM_15  /*!< Shared I2C Serial Data line */
#define BSP_PIN_I2C_SCL             GPIO_NUM_20  /*!< Shared I2C Serial Clock line (400 kHz) */
#define BSP_PIN_RTC_INT             GPIO_NUM_21  /*!< PCF85063A Real-Time Clock interrupt */
#define BSP_PIN_TOUCH_INT           GPIO_NUM_19  /*!< CST816S Capacitive Touch interrupt */
#define BSP_PIN_TOUCH_RST           GPIO_NUM_14  /*!< CST816S Capacitive Touch reset (Active Low) */

/* 7-Bit I2C Slave Addresses */
#define BSP_I2C_ADDR_SHTC3          0x70         /*!< Sensirion SHTC3 Environmental Sensor */
#define BSP_I2C_ADDR_PCF85063       0x51         /*!< NXP PCF85063A Real-Time Clock */
#define BSP_I2C_ADDR_CST816S        0x15         /*!< Hynitron CST816S Touch Controller */

/* =========================================================================
 * 4. 1.54" SPI e-Paper Display (SSD1681 200x200 Mono)
 * ========================================================================= */
#define BSP_PIN_EPD_MOSI            GPIO_NUM_7   /*!< SPI Master Out Slave In (Data) */
#define BSP_PIN_EPD_SCK             GPIO_NUM_6   /*!< SPI Serial Clock */
#define BSP_PIN_EPD_CS              GPIO_NUM_18  /*!< Display SPI Chip Select (Active Low) */
#define BSP_PIN_EPD_DC              GPIO_NUM_17  /*!< Display Data / Command control line */
#define BSP_PIN_EPD_RST             GPIO_NUM_16  /*!< Display Hardware Reset (Active Low) */
#define BSP_PIN_EPD_BUSY            GPIO_NUM_4   /*!< Display Busy Line (High = Busy) */

/* Display Geometry */
#define BSP_DISPLAY_WIDTH           200          /*!< Physical Width in Pixels */
#define BSP_DISPLAY_HEIGHT          200          /*!< Physical Height in Pixels */
#define BSP_DISPLAY_DPI             188          /*!< Pixel Density (DPI) */

/* =========================================================================
 * 5. I2S Audio Amplifier (MAX98357A Mono Class-D)
 * ========================================================================= */
#define BSP_PIN_I2S_BCLK            GPIO_NUM_10  /*!< I2S Bit Clock (BCLK) */
#define BSP_PIN_I2S_LRCK            GPIO_NUM_11  /*!< I2S Left/Right Clock (Word Select / WS) */
#define BSP_PIN_I2S_DOUT            GPIO_NUM_12  /*!< I2S Serial Audio Data Out */

/* =========================================================================
 * 6. MicroSD Card Storage (SPI Mode)
 * ========================================================================= */
#define BSP_PIN_SD_CS               GPIO_NUM_21  /*!< MicroSD Card SPI Chip Select */
#define BSP_PIN_SD_MOSI             GPIO_NUM_7   /*!< Shared SPI MOSI */
#define BSP_PIN_SD_MISO             GPIO_NUM_8   /*!< SPI Master In Slave Out (SD Data Out) */
#define BSP_PIN_SD_SCK              GPIO_NUM_6   /*!< Shared SPI Clock */

#ifdef __cplusplus
}
#endif

#endif /* BSP_PINOUT_H */
