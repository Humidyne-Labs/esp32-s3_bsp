# Unified ESP32-S3 Touch ePaper BSP

This repository provides a unified Board Support Package (BSP) for the **ESP32-S3 Touch ePaper** hardware platform (`ESP32-S3-PICO-1-N8R8` with 8 MB QSPI Flash and 8 MB Octal SPI PSRAM).

[![Donate to Humid1](https://custom-icon-badges.demolab.com/badge/Donate-Humid1.com-4A154B?style=plastic&logo=signupgenius&logoColor=white)](https://tools.signupgenius.com/c/support-humid1-project)

## Hardware Architecture & Pin Map

Detailed pin multiplexing and hardware routing can be found in [`REAL_PIN_MAP.md`](PIN_MAP.md).

| Peripherals | Chip / Interface | GPIO Pins | BSP Driver Header |
| --- | --- | --- | --- |
| **System & Power** | Power Hold & LED | GPIO0, GPIO2, GPIO38 | [`bsp_power.h`](components/bsp/include/bsp/bsp_power.h) |
| **Battery Monitoring** | ADC1 Channel 0 | GPIO1 | [`bsp_power.h`](components/bsp/include/bsp/bsp_power.h) |
| **Shared I2C Bus** | I2C Master (400kHz) | SCL: GPIO6, SDA: GPIO7 | [`bsp_i2c.h`](components/bsp/include/bsp/bsp_i2c.h) |
| **e-Paper Display** | 1.54" SPI 200x200 | CS:10, SCLK:11, MOSI:12, DC:8, RST:9, BUSY:13 | [`bsp_display.h`](components/bsp/include/bsp/bsp_display.h) |
| **Capacitive Touch** | FT6336 (I2C 0x38) | RST: GPIO4, INT: GPIO5 | [`bsp_touch.h`](components/bsp/include/bsp/bsp_touch.h) |
| **GUI Framework** | LVGL v9 Integration | Display Flush + Pointer Input | [`bsp_lvgl.h`](components/bsp/include/bsp/bsp_lvgl.h) |
| **Environment Sensor** | SHTC3 (I2C 0x70) | Temp (Kelvin & °C) + Humidity | [`bsp_sensors.h`](components/bsp/include/bsp/bsp_sensors.h) |
| **Audio Codec & Amp** | ES8311 + NS4168 PA | I2S: GPIO14-18, PA: GPIO47, 48 | [`bsp_audio.h`](components/bsp/include/bsp/bsp_audio.h) |
| **MicroSD Card** | SDMMC 1-line VFS | CLK:39, MISO:40, MOSI:41, CS:42 | [`bsp_sdcard.h`](components/bsp/include/bsp/bsp_sdcard.h) |
| **NVS Parameters** | Non-Volatile Flash | WiFi SSID/Passkey, Device Params | [`bsp_nvs.h`](components/bsp/include/bsp/bsp_nvs.h) |

---

## Repository Structure

```
├── components/
│   └── bsp/                       # Unified BSP Component
│       ├── CMakeLists.txt         # IDF component build rules
│       ├── Kconfig                # ESP-IDF menuconfig parameters
│       ├── include/bsp/           # Public driver headers
│       │   ├── bsp.h              # Master umbrella header
│       │   ├── pinout.h           # Hardware pin definitions
│       │   ├── bsp_i2c.h          # Shared I2C master bus
│       │   ├── bsp_power.h        # Power latch & Battery ADC
│       │   ├── bsp_display.h      # e-Paper display driver
│       │   ├── bsp_touch.h        # FT6336 touch driver
│       │   ├── bsp_lvgl.h         # LVGL v9 port
│       │   ├── bsp_sensors.h      # SHTC3 sensor (Kelvin & °C)
│       │   ├── bsp_audio.h        # Audio codec & PA amplifier
│       │   ├── bsp_sdcard.h       # MicroSD FatFS VFS driver
│       │   └── bsp_nvs.h          # Persistent parameter storage
│       └── src/                   # Driver C/C++ implementations
├── examples/
│   └── Unified_BSP_Demo/          # Clean demo project using unified BSP
│       ├── CMakeLists.txt
│       ├── partitions.csv         # 8MB Flash partition table
│       ├── sdkconfig.defaults     # ESP32-S3-PICO-1-N8R8 hardware defaults
│       ├── README.md              # Build & Flash instructions
│       └── main/main.cpp
├── REAL_PIN_MAP.md                # Pin multiplexing & hardware schema
└── README.md
```

---

## How to Build and Compile

1. **Source ESP-IDF environment**:
   ```bash
   . $HOME/esp/esp-idf/export.sh
   ```

2. **Navigate to the demo application**:
   ```bash
   cd examples/Unified_BSP_Demo
   ```

3. **Set target to ESP32-S3**:
   ```bash
   idf.py set-target esp32s3
   ```

4. **Build and Flash**:
   ```bash
   idf.py build
   idf.py -p PORT flash monitor
   ```

---

## BSP Scope vs. Application Layer

| Feature / System | Scope | Responsibility |
| --- | --- | --- |
| **Pinouts & Bus Init** | **BSP** | Centralized in `pinout.h`, `bsp_board_init()`, and `bsp_i2c` |
| **Driver Abstraction** | **BSP** | Standard C/C++ APIs for Display, Touch, Audio, Sensors, Battery |
| **Unique Hardware ID** | **BSP** | `bsp_get_device_id()` grabs eFuse MAC into `"ESP32S3-XXXXXXXX"` |
| **NVS Param Storage** | **BSP** | `bsp_nvs_set_str()` / `bsp_nvs_get_str()` for persistent key-values |
| **Partition Table** | **BSP/Project**| `partitions.csv` tuned for 8MB Flash (NVS, App, Storage) |
| **BLE Provisioning** | **App** | Web Bluetooth / GATT server runs in app layer using `bsp_nvs` for credentials |

---

## 📄 License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.

## 👥 Contributors

[![none](https://wsrv.nl/?url=github.com/Humiditron.png&w=32&h=32&fit=cover&mask=circle&filt=greyscale "@Humiditron")](https://github.com/Humiditron/)
[![none](https://wsrv.nl/?url=github.com/google-gemini.png&w=32&h=32&fit=cover&mask=circle&filt=greyscale "@google-gemini")](https://github.com/google-gemini/)

© 2026 **Humidyne-Labs**

