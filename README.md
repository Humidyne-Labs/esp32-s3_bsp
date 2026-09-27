# ESP32-S3 ePaper 1.54" Unified Board Support Package (BSP)

[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.1%20|%20v5.3%20|%20v6.1-blue.svg)](https://idf.espressif.com/)
[![Target](https://img.shields.io/badge/Hardware-Waveshare%20ESP32--S3--ePaper--1.54%20V2-green.svg)](https://www.waveshare.com)
[![Graphics](https://img.shields.io/badge/LVGL-v9.6.0-orange.svg)](https://lvgl.io/)
[![Version](https://img.shields.io/badge/SemVer-0.1.0-brightgreen.svg)](components/esp32-s3_bsp/include/bsp/bsp_version.h)
[![License](https://img.shields.io/badge/License-MIT-purple.svg)](LICENSE)

An industrial-grade, production-ready Board Support Package (BSP) and peripheral driver framework for the **Waveshare ESP32-S3 ePaper 1.54" V2** development board (ESP32-S3-PICO-1-N8R8). Engineered for ultra-low power IoT telemetry nodes, battery-powered environmental monitors, and ThingsBoard cloud integrations under **ESP-IDF v5.1+ and v6.1**.

---

## 📑 Table of Contents

1. [Hardware Overview](#-hardware-overview)
2. [Dual-Core System Architecture](#-dual-core-system-architecture)
3. [BSP Feature Matrix & Modular Architecture](#-bsp-feature-matrix--modular-architecture)
4. [Pinout & Peripheral Map](#-pinout--peripheral-map)
5. [Dynamic Initialization Modes](#-dynamic-initialization-modes)
6. [Dual Sleep Subsystem & Power Rails](#-dual-sleep-subsystem--power-rails)
7. [Flicker-Free E-Paper Persistence & Partial Refresh](#-flicker-free-e-paper-persistence--partial-refresh)
8. [Silicon Revision & Diagnostics API](#-silicon-revision--diagnostics-api)
9. [Time Synchronization & POSIX Timezone Support](#-time-synchronization--posix-timezone-support)
10. [BLE GATT Provisioning with QR Code & Base57 Keys](#-ble-gatt-provisioning-with-qr-code--base57-keys)
11. [Dual-Slot Seamless OTA Engine](#-dual-slot-seamless-ota-engine)
12. [ThingsBoard Cloud Framework](#-thingsboard-cloud-framework)
13. [Peripherals & Sleep Verification Test Suite](#-peripherals--sleep-verification-test-suite)
14. [Getting Started & Build Instructions](#-getting-started--build-instructions)
15. [License](#-license)

---

## ⚡ Hardware Overview

The Waveshare ESP32-S3 ePaper 1.54" V2 is a compact, battery-powered IoT development platform featuring:
- **MCU**: Espressif Systems ESP32-S3-PICO-1-N8R8 (Xtensa® dual-core 32-bit LX7 @ up to 240 MHz, 8 MB Flash, 8 MB Octal PSRAM).
- **Display**: 1.54-inch 200×200 pixel monochrome bi-stable e-Paper display (SSD1681 driver, 188 DPI). Retains content indefinitely with **zero power draw**.
- **Environmental Sensor**: Sensirion SHTC3 (I2C) high-precision temperature & relative humidity sensor.
- **Real-Time Clock**: NXP PCF85063A ultra-low power calendar RTC (I2C) with battery backup, hardware countdown timer, alarms, and 8-bit NVRAM.
- **Audio Output**: Everest Semi ES8311 I2S Audio Codec + NS4168 Class-D Mono Power Amplifier driving an onboard speaker.
- **Storage**: MicroSD card slot (SPI / 1-bit SDMMC mode) + Onboard SPI Flash with `esp_mmap_assets` zero-copy asset decoding.
- **Power Management**: Discrete LDO power-hold latch (`GPIO 17`), battery voltage ADC divider (`GPIO 4`, ADC1 CH3, 1:2 divider), and user status LED (`GPIO 3`).

---

## 🧠 Dual-Core System Architecture

To prevent network communications, TLS handshakes, and cryptographic hashing from stalling the UI or display bit-blit cycles, execution is strictly partitioned across the ESP32-S3's two Xtensa cores:

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
│ • Remote Firmware HTTPS OTA Worker     │ • ES8311 Codec & NS4168 Audio Synth    │
│ • Deep/Light Sleep Power Transitions   │ • Zero-Copy Flash Asset Rendering      │
└────────────────────────────────────────┴────────────────────────────────────────┘
```

---

## 🛠️ BSP Feature Matrix & Modular Architecture

| Subsystem | Header | Implementation | Description |
|---|---|---|---|
| **Version & SemVer** | `bsp/bsp_version.h` | `bsp_common.c` | SemVer string `0.1.0`, major/minor/patch macros, version value. |
| **Diagnostics & Errors** | `bsp/bsp_err.h` | `bsp_common.c` | Standardized `bsp_err_t`, error names, diagnostics snapshot, silicon revision access. |
| **Master Bringup** | `bsp/bsp.h` | `bsp_common.c` | Modular `bsp_init_mode()`, recommended init mode from reset/wake cause, Base57 generator. |
| **Flash MMAP Assets** | `bsp/bsp_assets.h` | `bsp_assets.c` | Zero-copy SPI flash asset mmap driver & LVGL v9 image decoder. |
| **Audio Synthesizer** | `bsp/bsp_audio.h` | `bsp_audio.c` | ES8311 I2S codec & NS4168 amp power gating and tone/chime generation. |
| **Tactile Buttons** | `bsp/bsp_button.h` | `bsp_button.c` | Debounced interrupt handlers for BOOT (`GPIO 0`) & POWER (`GPIO 18`). |
| **EPD Display** | `bsp/bsp_display.h` | `bsp_display.cpp` | SSD1681 1.54" SPI e-Paper driver with partial/full refresh modes. |
| **Shared I2C Bus** | `bsp/bsp_i2c.h` | `bsp_i2c.c` | Thread-safe, mutex-guarded I2C master with bus recovery and scanning (`GPIO 47/48`). |
| **LVGL v9 Port** | `bsp/bsp_lvgl.h` | `bsp_lvgl.cpp` | Pinned FreeRTOS rendering task with `bsp_lvgl_lock()` / `unlock()`. |
| **NVS Storage** | `bsp/bsp_nvs.h` | `bsp_nvs.c` | Thread-safe persistent key-value storage wrapper. |
| **RTC State Engine** | `bsp/bsp_rtc_mem.h` | `bsp_rtc_mem.c` | ESP32-S3 RTC Slow Memory persistent state engine with 32-byte scratchpad. |
| **Power & Sleep** | `bsp/bsp_power.h` | `bsp_power.c` | LDO power latch (`GPIO 17`), battery ADC curve, Light & Deep Sleep manager. |
| **Calendar RTC** | `bsp/bsp_rtc.h` | `bsp_rtc.c` | PCF85063A hardware RTC, countdown timer, alarms, 8-bit NVRAM read/write. |
| **MicroSD Storage** | `bsp/bsp_sdcard.h` | `bsp_sdcard.c` | FATFS file system mount/unmount manager over SDMMC / SPI. |
| **Environmental** | `bsp/bsp_sensors.h` | `bsp_sensors.c` | SHTC3 sensor acquisition returning native Kelvin and RH%. |
| **Wi-Fi Manager** | `bsp/bsp_wifi.h` | `bsp_wifi.c` | Station mode manager with RTC fast reconnect caching (<400ms). |
| **Time & SNTP** | `bsp/bsp_time.h` | `bsp_time.c` | SNTP sync, POSIX timezone support, 4 time formats, 3 date formats. |
| **BLE Provisioning** | `bsp/bsp_prov.h` | `bsp_prov.c` | Unified BLE GATT provisioning with on-screen QR code and 8-char Base57 PoP. |
| **Seamless OTA** | `bsp/bsp_ota.h` | `bsp_ota.c` | Dual-partition background HTTPS OTA with auto-rollback. |
| **ThingsBoard IoT** | `bsp/bsp_tb.h` | `bsp_tb.c` | High-level MQTTS client (generic entries, attributes, RPC, claiming, alarms). |
| **Splash & Chimes** | `bsp/bsp_splash.h` | `bsp_splash.c` | Visual and acoustic feedback hooks for boot, sleep, shutdown, alarms. |

---

## 📌 Pinout & Peripheral Map

| Pin Name | ESP32-S3 GPIO | Function / Peripheral | Description |
|---|---|---|---|
| **POWER_HOLD** | `GPIO 17` | Digital Output | Main LDO battery power rail latch (**Must be driven HIGH**) |
| **BATTERY_ADC**| `GPIO 4` | ADC1 Channel 3 | Battery voltage 1:2 divider measurement (R1=100k, R2=100k) |
| **LED_STATUS** | `GPIO 3` | Digital Output | Status LED (Open-Drain, Active LOW: 0 = ON, 1 = OFF) |
| **BTN_BOOT**   | `GPIO 0` | Digital Input | Boot / user button (Active LOW, internal pull-up) |
| **BTN_POWER**  | `GPIO 18` | Digital Input | Power / battery key (Active LOW, internal pull-up) |
| **I2C_SDA**    | `GPIO 47` | I2C Data | Shared I2C serial data (SHTC3, PCF85063A, ES8311) |
| **I2C_SCL**    | `GPIO 48` | I2C Clock | Shared I2C serial clock (400 kHz Fast Mode) |
| **RTC_INT**    | `GPIO 5` | Digital Input | PCF85063A RTC interrupt line (Active LOW, internal pull-up) |
| **EPD_3V3_EN** | `GPIO 6` | Digital Output | E-Paper 3.3V Power Rail Enable (Active LOW: 0 = ON) |
| **EPD_BUSY**   | `GPIO 8` | Digital Input | E-Paper panel busy status (High = Busy) |
| **EPD_RST**    | `GPIO 9` | Digital Output | E-Paper hardware active-low reset |
| **EPD_DC**     | `GPIO 10` | Digital Output | E-Paper Data / Command control line |
| **EPD_CS**     | `GPIO 11` | SPI Chip Select | E-Paper SPI CS (Active Low) |
| **EPD_SCK**    | `GPIO 12` | SPI SCLK | Serial Clock for display SPI bus |
| **EPD_MOSI**   | `GPIO 13` | SPI MOSI | Master Out Slave In for display SPI bus |
| **I2S_MCLK**   | `GPIO 14` | I2S Master Clock | ES8311 Audio Codec MCLK |
| **I2S_SCLK**   | `GPIO 15` | I2S Bit Clock | ES8311 Audio Codec BCLK / SCLK |
| **I2S_ASDOUT** | `GPIO 16` | I2S Data In | ES8311 Audio ADC / MIC Data to ESP32 |
| **I2S_LRCK**   | `GPIO 38` | I2S Word Select | ES8311 Left/Right Word Select (WS) |
| **I2S_DSDIN**  | `GPIO 45` | I2S Data Out | ES8311 Audio DAC Data from ESP32 |
| **PA_EN**      | `GPIO 42` | Digital Output | NS4168 Class-D Power Amp Enable (Active LOW: 0 = ON) |
| **PA_CTRL**    | `GPIO 46` | Digital Output | NS4168 Class-D Power Amp Control line |
| **SD_CLK**     | `GPIO 39` | SDMMC Clock | MicroSD Clock line |
| **SD_MISO**    | `GPIO 40` | SDMMC D0 / MISO | MicroSD Data 0 line |
| **SD_MOSI**    | `GPIO 41` | SDMMC CMD / MOSI| MicroSD Command line |

---

## ⚡ Dynamic Initialization Modes

The BSP provides three granular initialization profiles in `bsp/bsp.h` to minimize cold-boot latency and power consumption:

1. **`BSP_INIT_MODE_FULL` (Cold Boot)**:
   - Full peripheral bringup: I2C, RTC, SHTC3, Display SPI, LVGL task, Audio I2S, Buttons, ADC, SD Card, NVS.
2. **`BSP_INIT_MODE_FAST` (Wake Boot)**:
   - Fast bringup with display partial update enabled (`bsp_lvgl_set_first_flush_mode(false)`) and audio hardware skipped to conserve energy.
3. **`BSP_INIT_MODE_MIN` (Lean Telemetry Boot)**:
   - Minimal power profile: Power latch, I2C, SHTC3 sensor acquisition, Battery ADC, RTC, and non-flickering display. Audio hardware held in deep sleep.

```c
// Automatic detection helper based on reset reason and RTC state:
bsp_init_mode_t mode = bsp_get_recommended_init_mode();
bsp_init_mode(mode);
```

---

## 🌙 Dual Sleep Subsystem & Power Rails

The BSP supports both **Light Sleep** (RAM maintained, immediate execution resumption) and **Deep Sleep** (sub-25 µA current, persistent state stored in RTC Slow Memory).

```c
bsp_sleep_config_t sleep_cfg = {
    .mode           = BSP_SLEEP_MODE_DEEP,
    .duration_sec   = 60,                        // 60-second sleep interval
    .wake_sources   = BSP_WAKE_SRC_EXTERNAL_RTC, // PCF85063A RTC INT (GPIO 5)
    .next_init_mode = BSP_INIT_MODE_FAST,
};

bsp_enter_sleep(&sleep_cfg);
```

### Hot Power Rail Protection
When external I2C peripherals (e.g. PCF85063A) are powered while main rails drop, parasitic current can flow through ESD clamping diodes. `bsp_power.c` ensures `POWER_HOLD` (GPIO 17) and `EPD_3V3_EN` (GPIO 6) pad holds are asserted during sleep via `gpio_deep_sleep_hold_en()` to maintain rock-solid bus stability.

---

## 🖥️ Flicker-Free E-Paper Persistence & Partial Refresh

Bi-stable e-Paper panels retain image content across deep sleep cycles without power. The BSP avoids disruptive full-screen flashing upon wake by:
1. Storing screen state and boot counters in ESP32-S3 RTC Slow Memory (`bsp_rtc_mem`).
2. Setting the initial LVGL flush mode to `PARTIAL` on wake boots (`bsp_lvgl_set_first_flush_mode(false)`).
3. Executing differential LUT writes for immediate, flicker-free updates.

---

## 🔍 Silicon Revision & Diagnostics API

Runtime access to ESP32-S3 wafer revision and hardware diagnostics:

```c
uint32_t major = 0, minor = 0;
bsp_get_chip_revision(&major, &minor);
const char *rev = bsp_get_chip_revision_str(); // e.g. "v0.2"

// Dump formatted system snapshot:
bsp_diagnostics_dump();
```

---

## ⏰ Time Synchronization & POSIX Timezone Support

The `bsp_time` module coordinates SNTP network time acquisition with the hardware PCF85063A RTC and provides formatted time and date strings:

```c
// Configure POSIX Timezone (e.g., US Eastern Time with DST)
bsp_time_set_timezone("EST5EDT,M3.2.0,M11.1.0");

// Initialize SNTP with automatic 24-hour periodic resync
bsp_sntp_config_t sntp_cfg = {
    .server1 = "pool.ntp.org",
    .server2 = "time.google.com",
    .sync_rtc_on_update = true,
    .periodic_sync_hours = 24,
};
bsp_time_sntp_init(&sntp_cfg);

// Formatted String Helpers:
char time_str[32], date_str[32];
bsp_time_get_formatted(BSP_TIME_FMT_12H_MIN, time_str, sizeof(time_str)); // "02:35 PM"
bsp_time_get_date_dow_str(date_str, sizeof(date_str));                    // "09/26/26 Sat"
```

---

## 📱 BLE GATT Provisioning with QR Code & Base57 Keys

When Wi-Fi credentials are not provisioned in NVS, the node launches BLE GATT provisioning, rendering a 1-bit monochrome QR code on the 200×200 EPD panel alongside an unambiguous 8-character Base57 Proof-of-Possession key (excluding `0`, `O`, `o`, `1`, `l`, `I`):

```c
char pop_key[16] = {0};
bsp_generate_unambiguous_key(pop_key, 8, NULL);

bsp_prov_config_t prov_cfg = {
    .service_name_prefix = "PROV_",
    .pop                 = pop_key,
    .show_qr_on_display  = true,
    .timeout_ms          = 120000,
};
bsp_prov_init(&prov_cfg);
```

---

## 🔄 Dual-Slot Seamless OTA Engine

The BSP implements background HTTPS firmware updates using dual `ota_0` / `ota_1` flash partitions with automatic rollback protection:

```c
bsp_ota_config_t ota_cfg = {
    .url         = "https://firmware.humidyne.com/v1/update.bin",
    .cert_pem    = (const char *)server_cert_pem_start,
    .timeout_ms  = 30000,
    .auto_reboot = true,
};
bsp_ota_start(&ota_cfg);
```

---

## ☁️ ThingsBoard Cloud Framework

The `bsp_tb` module delivers an end-to-end ThingsBoard IoT framework over TLS with generic schema serialization:

```c
bsp_tb_config_t tb_cfg = {
    .host         = "thingsboard.cloud",
    .port         = 8883,
    .access_token = "YOUR_DEVICE_TOKEN",
};
bsp_tb_init(&tb_cfg);

// Publish Generic Telemetry Entries:
bsp_tb_entry_t entries[] = {
    { .key = "temp_k",   .type = BSP_TB_VAL_FLOAT,  .val.f_val = 295.15f },
    { .key = "rh_pct",   .type = BSP_TB_VAL_FLOAT,  .val.f_val = 58.0f },
    { .key = "batt_pct", .type = BSP_TB_VAL_INT,    .val.i_val = 85 },
    { .key = "status",   .type = BSP_TB_VAL_STRING, .val.s_val = "ONLINE" },
};
bsp_tb_send_telemetry_entries(entries, 4, false, 0);
```

---

## 🧪 Peripherals & Sleep Verification Test Suite

The primary reference example is located in **`examples/Peripherals_Test_Suite`**. It exercises:
1. **15 Static Hardware Tests**:
   - System Diagnostics, Silicon Revision, & SemVer API verification
   - Battery Voltage & Power Latch Hold
   - SHTC3 Environmental Sampling
   - PCF85063A Calendar RTC & 8-Bit NVRAM persistence
   - ESP32-S3 RTC Slow Memory Engine & 32-byte Scratchpad
   - Tactile Buttons (BOOT & POWER) interrupt dispatcher
   - ES8311 Audio Codec & NS4168 Class-D Amp Chime Synthesis
   - Timezone & 4 Time + 3 Date Formatted String Generators
   - ThingsBoard Generic Telemetry Serialization
   - Base57 Unambiguous Key Generator (8-char PoP, 6-char Claim)
   - NVS Key-Value Lifecycle
   - MicroSD Card FATFS Mount & File I/O
   - Wi-Fi Station & Passive Network Scanner
   - SSD1681 E-Paper Display & LVGL v9 1-bit QR Code Rendering
2. **Sleep & Wake Functional Suite**:
   - `LS-1`: Light Sleep via Internal Timer (3 sec)
   - `LS-2`: Light Sleep via External PCF85063A RTC Countdown (3 sec on GPIO 5)
   - `DS-1`: Deep Sleep via Internal Timer (4 sec) $\rightarrow$ `FAST` Init Mode
   - `DS-2`: Deep Sleep via External PCF85063A RTC INT (4 sec on GPIO 5) $\rightarrow$ `MIN` Init Mode
   - Interactive Triggers: Click `BOOT` for on-demand 5s Light Sleep; hold `BOOT` for Deep Sleep with button wakeup.

---

## 🚀 Getting Started & Build Instructions

### Prerequisites
- [ESP-IDF v5.1+ or v6.1](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/)
- Python 3.10+ with ESP-IDF toolchain configured

### Build, Flash, and Monitor
```powershell
# Export ESP-IDF Environment
. C:\esp\v6.1\esp-idf\export.ps1

# Navigate to Peripherals Test Suite
cd examples/Peripherals_Test_Suite

# Set ESP32-S3 Target (first time only)
idf.py set-target esp32s3

# Build, Flash, and Monitor
idf.py build
idf.py -p COMx flash monitor
```

---

## 📄 License

This Board Support Package is open-source software licensed under the [MIT License](LICENSE).  
Copyright (c) 2026 Humidyne Labs.
