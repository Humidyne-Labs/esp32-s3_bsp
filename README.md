# ESP32-S3 Touch ePaper — Board Support Package

A unified **Board Support Package (BSP)** for the [Waveshare ESP32-S3-ePaper-1.54 V2](https://www.waveshare.com/esp32-s3-epaper-1.54.htm?sku=32298) hardware platform (`ESP32-S3-PICO-1-N8R8` · 8 MB QSPI Flash · 8 MB Octal PSRAM).

[![Donate to Humid1](https://custom-icon-badges.demolab.com/badge/Donate-Humid1.com-4A154B?style=plastic&logo=signupgenius&logoColor=white)](https://tools.signupgenius.com/c/support-humid1-project)

---

## Hardware Overview & Pin Map

Full GPIO multiplexing details are in [`docs/PIN_MAP.md`](docs/PIN_MAP.md).

| Peripheral | Chip / Interface | GPIO Pins | BSP Header |
|---|---|---|---|
| **System & Power** | Power Hold & Status LED | GPIO0, GPIO2, GPIO38 | [`bsp_power.h`](include/bsp/bsp_power.h) |
| **Battery Monitor** | ADC1 Channel 0 | GPIO1 | [`bsp_power.h`](include/bsp/bsp_power.h) |
| **Shared I2C Bus** | I2C Master (400 kHz) | SCL: GPIO6, SDA: GPIO7 | [`bsp_i2c.h`](include/bsp/bsp_i2c.h) |
| **e-Paper Display** | 1.54″ SPI 200×200 px | CS:10, SCLK:11, MOSI:12, DC:8, RST:9, BUSY:13 | [`bsp_display.h`](include/bsp/bsp_display.h) |
| **Capacitive Touch** | FT6336 (I2C 0x38) | RST: GPIO4, INT: GPIO5 | [`bsp_touch.h`](include/bsp/bsp_touch.h) |
| **GUI Framework** | LVGL v9 Port | Display Flush + Pointer Input | [`bsp_lvgl.h`](include/bsp/bsp_lvgl.h) |
| **Environment Sensor** | SHTC3 (I2C 0x70) | Temp (K & °C) + Humidity | [`bsp_sensors.h`](include/bsp/bsp_sensors.h) |
| **Audio Codec & Amp** | ES8311 + NS4168 PA | I2S: GPIO14–18, PA: GPIO47, 48 | [`bsp_audio.h`](include/bsp/bsp_audio.h) |
| **MicroSD Card** | SDMMC 1-line VFS | CLK:39, MISO:40, MOSI:41, CS:42 | [`bsp_sdcard.h`](include/bsp/bsp_sdcard.h) |
| **NVS Storage** | Non-Volatile Flash | Wi-Fi credentials, device params | [`bsp_nvs.h`](include/bsp/bsp_nvs.h) |

---

## Repository Structure

```
esp32-s3_bsp/
├── include/bsp/               # Public driver headers
│   ├── bsp.h                  # Master umbrella header
│   ├── pinout.h               # Hardware GPIO pin definitions
│   ├── bsp_i2c.h              # Shared I2C master bus
│   ├── bsp_power.h            # Power latch & battery ADC
│   ├── bsp_display.h          # e-Paper display driver
│   ├── bsp_touch.h            # FT6336 capacitive touch driver
│   ├── bsp_lvgl.h             # LVGL v9 port
│   ├── bsp_sensors.h          # SHTC3 temperature & humidity
│   ├── bsp_audio.h            # ES8311 codec & NS4168 amplifier
│   ├── bsp_sdcard.h           # MicroSD FatFS VFS driver
│   └── bsp_nvs.h              # Persistent key-value storage
├── src/                       # Driver C/C++ implementations
│   ├── bsp_common.c
│   ├── bsp_i2c.c
│   ├── bsp_power.c
│   ├── bsp_display.cpp
│   ├── bsp_touch.cpp
│   ├── bsp_sensors.c
│   ├── bsp_audio.c
│   ├── bsp_sdcard.c
│   ├── bsp_lvgl.cpp
│   └── bsp_nvs.c
├── examples/
│   └── Unified_BSP_Demo/      # Reference project for this BSP
│       ├── CMakeLists.txt
│       ├── partitions.csv     # 8 MB dual-slot OTA partition table
│       ├── sdkconfig.defaults # ESP32-S3-PICO-1-N8R8 hardware defaults
│       ├── README.md          # Build & flash instructions
│       └── main/main.cpp
├── docs/
│   ├── PIN_MAP.md             # GPIO multiplexing & hardware schema
│   └── VSCODE_ESP_IDF_GUIDE.md
├── CMakeLists.txt             # IDF component build rules
├── Kconfig                    # ESP-IDF menuconfig parameters
├── idf-component.yml          # ESP Component Registry manifest
└── LICENSE
```

---

## Quick Start

### Prerequisites

- **ESP-IDF v5.1+** (or v6.x) installed and sourced
- **ESP32-S3-PICO-1-N8R8** hardware (8 MB Flash + 8 MB PSRAM)

### Build & Flash

```bash
# Source ESP-IDF (Linux / macOS)
. $HOME/esp/esp-idf/export.sh

# Source ESP-IDF (Windows PowerShell)
. C:\esp\esp-idf\export.ps1

# Navigate to the demo application
cd examples/Unified_BSP_Demo

# Set target and apply sdkconfig.defaults
idf.py set-target esp32s3

# Build
idf.py build

# Flash and monitor (replace PORT with e.g. COM3 or /dev/ttyACM0)
idf.py -p PORT flash monitor
```

For a full VS Code + ESP-IDF Extension workflow see [`docs/VSCODE_ESP_IDF_GUIDE.md`](docs/VSCODE_ESP_IDF_GUIDE.md).

---

## BSP API at a Glance

```cpp
#include "bsp/bsp.h"

// Initialize all board hardware
bsp_board_init();

// Unique device ID from eFuse MAC (e.g. "ESP32S3-70041D3B")
char dev_id[32];
bsp_get_device_id(dev_id, sizeof(dev_id));

// Bluetooth advertisement name (e.g. "HumidOS-70041D3B")
char dev_name[32];
bsp_get_device_name(dev_name, sizeof(dev_name));

// NVS persistent storage
bsp_nvs_set_str("wifi_ssid", "MyNetwork");
bsp_nvs_get_str("wifi_ssid", buf, sizeof(buf));

// Environmental sensor
bsp_shtc3_data_t sensor;
bsp_shtc3_read(&sensor);
// sensor.temperature_k   → Kelvin
// sensor.humidity_percent → RH%

// Battery monitoring
uint32_t voltage_mv = 0;
bsp_battery_get_voltage(&voltage_mv, NULL);
uint8_t pct = bsp_battery_get_percentage();
```

---

## BSP Scope vs. Application Layer

| Feature | Scope | Notes |
|---|---|---|
| Pinouts & Bus Init | **BSP** | Centralized in `pinout.h`, `bsp_board_init()`, and `bsp_i2c` |
| Driver Abstraction | **BSP** | Standard C/C++ APIs for Display, Touch, Audio, Sensors, and Battery |
| Unique Device ID | **BSP** | `bsp_get_device_id()` formats eFuse MAC as `"ESP32S3-XXXXXXXX"` |
| NVS Parameter Storage | **BSP** | `bsp_nvs_set_str()` / `bsp_nvs_get_str()` for persistent key-values |
| Partition Table | **BSP / Project** | `partitions.csv` tuned for 8 MB Flash (NVS, dual OTA slots, Storage) |
| BLE Provisioning | **App** | GATT server runs in the application layer; uses `bsp_nvs` for credentials |

---

## License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

## Contributors

[![Humiditron](https://wsrv.nl/?url=github.com/Humiditron.png&w=32&h=32&fit=cover&mask=circle&filt=greyscale "@Humiditron")](https://github.com/Humiditron/)
[![google-gemini](https://wsrv.nl/?url=github.com/google-gemini.png&w=32&h=32&fit=cover&mask=circle&filt=greyscale "@google-gemini")](https://github.com/google-gemini/)

© 2026 **Humidyne Labs**
