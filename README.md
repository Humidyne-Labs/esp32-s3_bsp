# ESP32-S3 Touch ePaper 1.54" Unified Board Support Package (BSP)

[![Platform](https://img.shields.io/badge/ESP--IDF-v5.3%2B-blue.svg)](https://idf.espressif.com/)
[![Target](https://img.shields.io/badge/Hardware-Waveshare%20ESP32--S3--Touch--ePaper--1.54%20V2-green.svg)](https://www.waveshare.com)
[![Graphics](https://img.shields.io/badge/LVGL-v9.2-orange.svg)](https://lvgl.io/)
[![License](https://img.shields.io/badge/License-MIT-purple.svg)](LICENSE)

An industrial-grade, production-ready Board Support Package (BSP) and application framework for the **Waveshare ESP32-S3-Touch-ePaper-1.54 V2** development board. Engineered for ultra-low power IoT telemetry nodes, battery-powered environmental monitors, and ThingsBoard cloud integrations.

---

## 📑 Table of Contents

1. [Hardware Overview](#-hardware-overview)
2. [Dual-Core System Architecture](#-dual-core-system-architecture)
3. [BSP Feature Matrix](#-bsp-feature-matrix)
4. [Pinout & Peripheral Map](#-pinout--peripheral-map)
5. [Device Operational Flow](#-device-operational-flow)
6. [ThingsBoard Integration & Data Models](#-thingsboard-integration--data-models)
7. [Getting Started & Build Instructions](#-getting-started--build-instructions)
8. [Module Reference Guide](#-module-reference-guide)
9. [Power Management & Battery Life](#-power-management--battery-life)

---

## ⚡ Hardware Overview

The Waveshare ESP32-S3-Touch-ePaper-1.54 V2 is a compact, battery-capable development board integrating:
- **Microcontroller**: Espressif ESP32-S3 (Xtensa® 32-bit dual-core LX7 running up to 240 MHz).
- **Display**: 1.54-inch 200×200 pixel monochrome bi-stable e-Paper display (SSD1681 driver). Retains image with **zero power draw**.
- **Touch Controller**: CST816S capacitive single-point touch with gesture recognition.
- **Environmental Sensor**: Sensirion SHTC3 (I2C) high-precision temperature & relative humidity sensor.
- **Real-Time Clock**: NXP/PCF PCF85063A ultra-low power calendar RTC (I2C) with battery backup pin.
- **Audio Output**: MAX98357A I2S Class-D mono audio amplifier driving a micro-speaker.
- **Storage**: MicroSD card slot (SPI mode) + Onboard SPI Flash with `esp_mmap_assets` support.
- **Power Management**: SY6970 / discrete buck-boost power circuit, discrete LDO power-hold latch (GPIO 2), and battery voltage ADC divider (GPIO 5).

---

## 🧠 Dual-Core System Architecture

To prevent network communications, TLS handshakes, and cryptographic hashing from causing UI frame drops or display rendering stutter, the firmware strictly partitions tasks across the ESP32-S3's two Xtensa cores:

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                                ESP32-S3 DUAL-CORE LX7                           │
├────────────────────────────────────────┬────────────────────────────────────────┤
│          CORE 0: NETWORKING & CLOUD    │           CORE 1: UI & SENSORS         │
├────────────────────────────────────────┼────────────────────────────────────────┤
│ • Wi-Fi Station (Fast RTC Cache <400ms)│ • LVGL v9 Display Port Task (Pri 5)    │
│ • BLE GATT Provisioning (wifi_prov)    │ • SSD1681 1-bit Mono EPD Bit-Blit SPI │
│ • SNTP Network Time Synchronization    │ • Tactile Button State Handlers        │
│ • ThingsBoard Secure MQTTS (Port 8883) │ • SHTC3 Sensor I2C Acquisition         │
│ • Remote Firmware HTTPS OTA Worker     │ • Passive UI Card Inversion Engine     │
│ • Deep Sleep Power Transition Manager  │ • MAX98357A I2S Audio Synth Engine     │
└────────────────────────────────────────┴────────────────────────────────────────┘
```

---

## 🛠️ BSP Feature Matrix

| Subsystem | Header | Implementation | Description |
|---|---|---|---|
| **Master Bringup** | `bsp/bsp.h` | `bsp_common.c` | Unified `bsp_board_init()` with modular initialization flags. |
| **Flash MMAP Assets** | `bsp/bsp_assets.h` | `bsp_assets.c` | Zero-copy SPI flash asset mmap driver & LVGL v9 image decoder. |
| **Audio Synthesizer** | `bsp/bsp_audio.h` | `bsp_audio.c` | Non-blocking I2S audio amplifier driver with tone synthesis. |
| **Tactile Buttons** | `bsp/bsp_button.h` | `bsp_button.c` | Debounced interrupt handlers for BOOT (GPIO 0) & POWER (GPIO 3). |
| **EPD Display** | `bsp/bsp_display.h` | `bsp_display.cpp` | SSD1681 1.54" SPI e-Paper driver with partial/full refresh modes. |
| **Shared I2C Bus** | `bsp/bsp_i2c.h` | `bsp_i2c.c` | Thread-safe, mutex-guarded I2C master for SHTC3, PCF85063, & CST816. |
| **LVGL v9 Port** | `bsp/bsp_lvgl.h` | `bsp_lvgl.cpp` | Pinned FreeRTOS rendering task with `bsp_lvgl_lock()` / `unlock()`. |
| **NVS Storage** | `bsp/bsp_nvs.h` | `bsp_nvs.c` | Persistent storage for Wi-Fi credentials, tokens, and boot counts. |
| **Power & Battery** | `bsp/bsp_power.h` | `bsp_power.c` | LDO power hold latch, battery ADC voltage curve, and deep sleep. |
| **Calendar RTC** | `bsp/bsp_rtc.h` | `bsp_rtc.c` | PCF85063A hardware RTC reader/writer with alarm support. |
| **MicroSD Storage** | `bsp/bsp_sdcard.h` | `bsp_sdcard.c` | SPI-mode FATFS file system mount/unmount manager. |
| **Environmental** | `bsp/bsp_sensors.h` | `bsp_sensors.c` | SHTC3 sensor acquisition returning native Kelvin & RH%. |
| **Capacitive Touch** | `bsp/bsp_touch.h` | `bsp_touch.cpp` | CST816S I2C touch controller interface and LVGL indev driver. |
| **Wi-Fi Manager** | `bsp/bsp_wifi.h` | `bsp_wifi.c` | Station mode manager with RTC fast reconnect caching (<400ms). |

---

## 📌 Pinout & Peripheral Map

| Pin Name | ESP32-S3 GPIO | Function / Peripheral | Description |
|---|---|---|---|
| **EPD_BUSY** | `GPIO 4` | Digital Input | E-Paper panel busy status (High = Busy) |
| **EPD_RST** | `GPIO 16` | Digital Output | E-Paper hardware active-low reset |
| **EPD_DC** | `GPIO 17` | Digital Output | E-Paper Data / Command control line |
| **EPD_CS** | `GPIO 18` | SPI Chip Select | E-Paper SPI CS (Active Low) |
| **EPD_MOSI** | `GPIO 7` | SPI MOSI | Master Out Slave In for display SPI bus |
| **EPD_SCK** | `GPIO 6` | SPI SCLK | Serial Clock for display SPI bus |
| **I2C_SDA** | `GPIO 15` | I2C Data | Shared I2C data bus (SHTC3, PCF85063, CST816) |
| **I2C_SCL** | `GPIO 20` | I2C Clock | Shared I2C clock bus (400 kHz Fast Mode) |
| **RTC_INT** | `GPIO 21` | Digital Input | PCF85063A RTC interrupt line |
| **I2S_BCLK** | `GPIO 10` | I2S Bit Clock | MAX98357A I2S bit clock |
| **I2S_LRCK** | `GPIO 11` | I2S Word Select | MAX98357A I2S Left/Right clock |
| **I2S_DOUT** | `GPIO 12` | I2S Data Out | MAX98357A I2S serial audio data |
| **SD_CS** | `GPIO 21` | SPI Chip Select | MicroSD Card SPI Chip Select |
| **SD_MOSI** | `GPIO 7` | SPI MOSI | Shared SPI MOSI |
| **SD_MISO** | `GPIO 8` | SPI MISO | SPI Master In Slave Out for SD Card |
| **SD_SCK** | `GPIO 6` | SPI SCLK | Shared SPI Clock |
| **VBAT_ADC** | `GPIO 5` | ADC1 Channel 4 | Battery voltage 1:2 divider measurement |
| **BTN_BOOT** | `GPIO 0` | Digital Input | User boot / action button (Active Low) |
| **BTN_POWER**| `GPIO 3` | Digital Input | Power key / deep sleep wakeup (Active Low) |
| **PWR_HOLD** | `GPIO 2` | Digital Output | Main LDO power rail latch (Must be driven HIGH) |
| **LED_STATUS**| `GPIO 1` | Digital Output | Board status LED indicator |

---

## 🔄 Device Operational Flow

```
                                  [ Device Boot ]
                                         │
                        ┌────────────────┴────────────────┐
                        ▼                                 ▼
              [ Wi-Fi Not Provisioned ]          [ Wi-Fi Provisioned ]
              [ or Connection Failure ]                   │
                        │                        (Fast RTC Reconnect <400ms)
                        ▼                                 │
           ┌────────────────────────┐                     ▼
           │  BLE PROVISION SCREEN  │           ┌───────────────────┐
           │   PROV_ESP32S3-XXXX    │           │ Unclaimed Device? │
           │  (Stays Awake / Wait)  │           └─────────┬─────────┘
           └────────────────────────┘                     │
                        │                     YES ┌───────┴───────┐ NO
                        │                         ▼               ▼
                        │               ┌──────────────────┐  ┌───────────────────────┐
                        │               │ CLAIMING SCREEN  │  │   ACTIVE DASHBOARD    │
                        │               │  [ XXXXXXXX ]    │  │ (Telemetry & Sensors) │
                        │               │(Only shows token)│  └───────────┬───────────┘
                        │               └────────┬─────────┘              │
                        │                        │ (Claimed/Confirmed)    │
                        └────────────────────────┴────────────────────────┘
                                                 │
                                                 ▼
                                     ┌───────────────────────┐
                                     │  PUBLISH & REFRESH    │
                                     │  • Sample SHTC3/Batt  │
                                     │  • Publish to MQTTS   │
                                     │  • Refresh E-Paper    │
                                     └───────────┬───────────┘
                                                 │
                                                 ▼
                                     ┌───────────────────────┐
                                     │ ULTRA-LOW DEEP SLEEP  │
                                     │ • Screen Image Held   │
                                     │ • RTC Timer Wakeup    │
                                     │ • BOOT Button Wakeup  │
                                     └───────────────────────┘
```

1. **BLE Provisioning Phase**: If Wi-Fi is unconfigured or failed, the node displays `PROV_ESP32S3-XXXXXXXX` and stays awake for Chrome Web Bluetooth pairing.
2. **Claiming Phase**: Once connected, if unclaimed, the node publishes `v1/devices/me/claim` and displays **only the Claiming Token Card** until claimed or expired.
3. **Active Telemetry Dashboard**: Displays live Kelvin-converted readings. Features **passive card inversion** (white text on black) when alarm thresholds are breached.
4. **Ultra-Low Power Deep Sleep**: Telemetry image remains visible on the e-Paper panel with **zero power draw**. The node sleeps between sample intervals, waking only to sample, publish, refresh, and sleep.
5. **Connection Failure Protection**: If network connectivity fails, the device displays `! NO NETWORK / RETRY` and **stays awake** (refuses to sleep) to keep the user informed.
6. **Power Off**: Manual shutdown via the POWER button renders `space_cat.bin` on the display before cutting the power latch.

---

## ☁️ ThingsBoard Integration & Data Models

### 1. Telemetry Payload (`v1/devices/me/telemetry`)
Published periodically during wake cycles. Temperature is reported in **native Kelvin (K)** for unit-agnostic evaluation in the ThingsBoard Rule Engine:
```json
{
  "temp": 293.15,
  "rh": 68.00,
  "battery": 80,
  "rssi": -55
}
```

### 2. Claiming Payload (`v1/devices/me/claim`)
Published upon initial registration to enable user dashboard binding:
```json
{
  "secretKey": "70041D3B",
  "durationMs": 180000
}
```

### 3. Client Attributes (`v1/devices/me/attributes`)
Reports diagnostic hardware status on startup:
```json
{
  "fw_version": "v1.0.4",
  "device_name": "HumidOS-70041D3B",
  "mac_address": "ESP32S3-70041D3B",
  "ssid": "Home-Network-5G",
  "ip_address": "192.168.1.150",
  "has_sd_card": false,
  "audio_synced": true
}
```

### 4. Shared Attributes Synchronization
The device subscribes to `v1/devices/me/attributes` and dynamically applies:
- `sleep_interval_sec`: Dynamic sleep duration in seconds.
- `temp_unit`: `"F"`, `"C"`, or `"K"` for on-screen user conversion.
- `sound_enabled`: Toggles audio chime alerts.
- `alarm_thresholds`: Dynamic Kelvin thresholds for temperature and RH hysteresis.

---

## 🚀 Getting Started & Build Instructions

### Prerequisites
- [ESP-IDF v5.1, v5.2, or v5.3](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/)
- CMake 3.16+ and Ninja build system.

### Build and Flash
```bash
# Clone the repository
git clone https://github.com/humiditron/esp32-s3-touch-epaper-bsp.git
cd esp32-s3-touch-epaper-bsp/examples/Unified_BSP_Demo

# Set ESP32-S3 Target
idf.py set-target esp32s3

# Configure Secrets & Broker Settings (Optional)
# cp main/app_secrets.h.example main/app_secrets.h

# Build, Flash, and Monitor
idf.py build
idf.py -p /dev/ttyACM0 flash monitor
```

---

## 🔋 Power Management & Battery Life

- **Active Wake Window**: ~1.2 seconds (Sensor read, Fast Wi-Fi reconnect <400ms, TLS telemetry publish, E-Paper refresh).
- **Deep Sleep Current**: < 25 µA (ESP32-S3 in deep sleep, sensors in ultra-low power sleep, EPD retaining image with 0 µA).
- **Battery Life Estimate**: 
  - 1-minute interval: ~3.5 months on a 1200 mAh LiPo cell.
  - 15-minute interval: > 2.5 years on a 1200 mAh LiPo cell.

---

## 📄 License

This Board Support Package is open-source software licensed under the [MIT License](LICENSE).
Copyright (c) 2026 Humidyne Labs / Humiditron.
