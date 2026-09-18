# ESP32-S3 Touch ePaper Unified BSP Demo

This application demonstrates the complete Board Support Package (BSP) for the **ESP32-S3 Touch ePaper** hardware target (`ESP32-S3-PICO-1-N8R8` with 8MB Flash QSPI and 8MB PSRAM Octal SPI).

---

## Hardware Target & Specifications

- **SoC**: ESP32-S3-PICO-1-N8R8
- **Flash**: 8 MB Quad SPI (QSPI)
- **PSRAM**: 8 MB Octal SPI (OPI)
- **Display**: 1.54" e-Paper Monochrome SPI Display (200x200)
- **Touch**: FT6336 Capacitive Touch Controller over I2C
- **Audio**: ES8311 I2S Audio Codec + NS4168 Power Amp
- **Sensors**: SHTC3 Temperature (Kelvin & °C) & Humidity
- **Storage**: MicroSD Card Slot (SDMMC 1-line mode)

---

## Prerequisites

Ensure you have **ESP-IDF v5.1+ or v6.0+** installed and sourced in your terminal environment.

```bash
# Sourcing ESP-IDF environment (Linux / macOS)
. $HOME/esp/esp-idf/export.sh

# Or in Windows PowerShell:
. C:\esp\esp-idf\export.ps1
```

---

## Building and Flashing

### Step 1: Set Target to ESP32-S3

```bash
cd examples/Unified_BSP_Demo
idf.py set-target esp32s3
```

This automatically applies `sdkconfig.defaults` (configuring 8MB Flash, 8MB Octal PSRAM, and the custom 8MB `partitions.csv` table).

### Step 2: Build the Project

```bash
idf.py build
```

### Step 3: Flash and Monitor Serial Logs

Connect your ESP32-S3 Touch ePaper board via USB-C, then run:

```bash
idf.py -p PORT flash monitor
```

*(Replace `PORT` with your serial port, e.g., `COM3` on Windows or `/dev/ttyACM0` on Linux).*

---

## BSP API Summary

```cpp
#include "bsp/bsp.h"

// 1. Board Master Initialization
bsp_board_init();

// 2. Hardware Unique Serial / Device ID
char dev_id[32];
bsp_get_device_id(dev_id, sizeof(dev_id)); // Output: "ESP32S3-70041D3B"

// 3. NVS Parameter Storage (Wi-Fi SSID & Passkey)
bsp_nvs_set_str("wifi_ssid", "MyHomeNetwork");
bsp_nvs_get_str("wifi_ssid", buf, sizeof(buf));

// 4. Environmental Sensor (Native Kelvin)
bsp_shtc3_data_t sensor;
bsp_shtc3_read(&sensor);
// sensor.temperature_k -> Kelvin
// sensor.humidity_percent -> RH%

// 5. Battery Monitoring
uint32_t v_mv = 0;
bsp_battery_get_voltage(&v_mv, NULL);
uint8_t pct = bsp_battery_get_percentage();
```
