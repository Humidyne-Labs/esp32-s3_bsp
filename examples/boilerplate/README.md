# Humid1 OS Production Boilerplate Template (`boilerplate`)

The official production starter template for applications built on **Humid1 OS** and the **ESP32-S3 BSP (`esp32_s3_bsp`)**.

## Features Included
- **Full BSP Lifecycle**: Automated initialization of I2C bus, SHTC3 sensors, PCF85063 RTC, battery ADC, and LVGL 9 UI renderer via `bsp_init()`.
- **Power Button Callbacks**:
  - **Short Click**: Force display refresh.
  - **Long Press**: Safe clean power-down (`bsp_power_off()`).
- **Telemetry UI Screen**: Real-time on-screen dashboard displaying battery voltage/percentage, temperature (°C), relative humidity (%), and RTC clock string.
- **5-Second Update Loop**: Lightweight main loop updating readings every 5 seconds.

## Hardware Setup
- **Target Board**: Waveshare ESP32-S3 ePaper 1.54 V2
- **Display**: Solomon Systech SSD1681 200x200 1-bit Monochrome E-Paper

## Building & Flashing
```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```
