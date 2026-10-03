# LVGL 9 UI Widgets Sample (`lvgl_widgets`)

This sample demonstrates using the thread-safe LVGL v9 mutex API (`bsp_lvgl_lock()` / `bsp_lvgl_unlock()`) to render UI labels and layout structures on the 1-bit monochrome SSD1681 e-Paper display.

## Hardware Setup
- **Target Board**: Waveshare ESP32-S3 ePaper 1.54 V2
- **Display Driver**: Solomon Systech SSD1681 (200x200 pixels, 1-bit monochrome)

## Compilation & Flashing
```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```
