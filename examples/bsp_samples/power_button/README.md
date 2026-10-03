# Power Management & Button Callbacks Sample (`power_button`)

This sample demonstrates registering button interrupt callbacks for short clicks and long presses, reading battery ADC voltage, and executing power hold latch management.

## Hardware Setup
- **Target Board**: Waveshare ESP32-S3 ePaper 1.54 V2
- **Power Button**: GPIO 18 (Active Low)
- **Power Latch**: GPIO 17 (BAT_CTRL)

## Compilation & Flashing
```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```
