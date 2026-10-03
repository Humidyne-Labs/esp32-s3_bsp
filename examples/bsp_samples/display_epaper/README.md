# E-Paper Display Text & Graphics Sample (`display_epaper`)

This sample demonstrates initializing the Solomon Systech SSD1681 1.54" 200x200 monochrome e-Paper display via 4-wire SPI using the `esp32_s3_bsp` Board Support Package.

## Hardware Setup
- **Target Board**: Waveshare ESP32-S3 ePaper 1.54 V2
- **Display Driver**: Solomon Systech SSD1681 (200x200 pixels)

## Compilation & Flashing
```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```
