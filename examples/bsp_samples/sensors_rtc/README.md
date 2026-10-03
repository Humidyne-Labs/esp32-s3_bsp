# SHTC3 Sensors & PCF85063 RTC Sample (`sensors_rtc`)

This sample demonstrates reading temperature and humidity from the Sensirion SHTC3 I2C environmental sensor alongside timekeeping from the NXP PCF85063 Real-Time Clock on the shared I2C bus.

## Hardware Requirements
- **Target Board**: Waveshare ESP32-S3 ePaper 1.54 V2
- **Sensors**: SHTC3 (I2C 0x70) & PCF85063 (I2C 0x51)

## Compilation & Flashing
```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```
