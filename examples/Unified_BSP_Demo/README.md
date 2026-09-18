# Unified BSP Demo — ESP32-S3 Touch ePaper

A reference application demonstrating all peripherals of the **ESP32-S3 Touch ePaper** BSP on the `ESP32-S3-PICO-1-N8R8` (8 MB QSPI Flash · 8 MB Octal PSRAM).

---

## Hardware Specifications

| Component | Details |
|---|---|
| **SoC** | ESP32-S3-PICO-1-N8R8 |
| **Flash** | 8 MB Quad SPI (QIO, 80 MHz) |
| **PSRAM** | 8 MB Octal SPI (80 MHz) |
| **Display** | 1.54″ e-Paper monochrome SPI (200×200 px) |
| **Touch** | FT6336 capacitive touch controller (I2C 0x38) |
| **Audio** | ES8311 I2S codec + NS4168 power amplifier |
| **Sensor** | SHTC3 temperature (K & °C) and humidity (I2C 0x70) |
| **Storage** | MicroSD card (SDMMC 1-line VFS, FatFS) |

---

## Prerequisites

**ESP-IDF v5.1+** (or v6.x) must be installed and sourced:

```bash
# Linux / macOS
. $HOME/esp/esp-idf/export.sh

# Windows PowerShell
. C:\esp\esp-idf\export.ps1
```

---

## Build & Flash

### 1 — Set target

```bash
cd examples/Unified_BSP_Demo
idf.py set-target esp32s3
```

This applies `sdkconfig.defaults` automatically, configuring 8 MB Flash, 8 MB Octal PSRAM, and the custom `partitions.csv` partition table.

### 2 — Build

```bash
idf.py build
```

### 3 — Flash and monitor

```bash
idf.py -p PORT flash monitor
```

Replace `PORT` with your serial port (e.g., `COM3` on Windows or `/dev/ttyACM0` on Linux/macOS).

> **Tip:** If flashing fails, hold **BOOT**, press **RST** once, then release **BOOT** to force the board into download mode.

---

## BSP API Quick Reference

```cpp
#include "bsp/bsp.h"

// ── Board Initialization ──────────────────────────────────────────────────────
bsp_board_init();                               // Power latch, LED, NVS, I2C bus

// ── Device Identity ───────────────────────────────────────────────────────────
char dev_id[32], dev_name[32];
bsp_get_device_id(dev_id, sizeof(dev_id));      // e.g. "ESP32S3-70041D3B"
bsp_get_device_name(dev_name, sizeof(dev_name));// e.g. "HumidOS-70041D3B"

// ── NVS Persistent Storage ────────────────────────────────────────────────────
bsp_nvs_set_str("wifi_ssid", "MyNetwork");
bsp_nvs_get_str("wifi_ssid", buf, sizeof(buf));

// ── Environmental Sensor (SHTC3) ──────────────────────────────────────────────
bsp_shtc3_data_t sensor;
bsp_shtc3_read(&sensor);
// sensor.temperature_k    → temperature in Kelvin
// sensor.humidity_percent → relative humidity %

// ── Battery Monitoring ────────────────────────────────────────────────────────
uint32_t voltage_mv = 0;
bsp_battery_get_voltage(&voltage_mv, NULL);     // millivolts
uint8_t pct = bsp_battery_get_percentage();     // 0–100 %
```

---

## VS Code Workflow

See [`docs/VSCODE_ESP_IDF_GUIDE.md`](../../docs/VSCODE_ESP_IDF_GUIDE.md) for a full guide to building, flashing, and monitoring using the Espressif IDF Extension.
