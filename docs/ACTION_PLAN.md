# ESP32-S3 Touch ePaper BSP: Modernization & Architecture Action Plan

**Repository:** `Humidyne-Labs/esp32-s3_bsp`  
**Target Hardware:** Waveshare ESP32-S3-Touch-ePaper-1.54 V2  
**Framework:** ESP-IDF v5.1+ (C / C++20)  
**Status:** In Progress / Solidified Proposal  

---

## 1. Architectural Boundary & Separation of Concerns

To keep the Board Support Package (BSP) clean, reusable, and standard-compliant, we establish a strict separation between **BSP (Hardware Abstraction Layer)**, **App Services (Middleware)**, and **Application (Business Logic)**.

```
┌─────────────────────────────────────────────────────────────────────────┐
│                    APPLICATION LAYER (Unified_BSP_Demo)                 │
│  - ThingsBoard Claiming Token Flow & UI                                 │
│  - BLE Provisioning State Machine                                       │
│  - Periodic Telemetry Orchestrator (MQTT Publisher)                     │
└────────────────────────────────────┬────────────────────────────────────┘
                                     │
┌────────────────────────────────────▼────────────────────────────────────┐
│                  APP SERVICES / MIDDLEWARE (App Core)                   │
│  - MQTT Client Wrapper (`app_mqtt.c`) -> Talks to ThingsBoard Topics   │
│  - BLE Provisioning Manager (`app_ble_prov.c`)                          │
│  - Cloud Time Sync Service (`app_time_sync.c` -> SNTP + RTC commit)     │
└────────────────────────────────────┬────────────────────────────────────┘
                                     │
┌────────────────────────────────────▼────────────────────────────────────┐
│                    BOARD SUPPORT PACKAGE (BSP CORE)                     │
│  - Power & Latch (`bsp_power.h/.c`): Latch, Battery ADC, Deep Sleep    │
│  - Display & LVGL (`bsp_display.h`, `bsp_lvgl.h`): EPD SPI & Refresh    │
│  - Capacitive Touch (`bsp_touch.h/.cpp`): FT6336 I2C Driver             │
│  - Sensors (`bsp_sensors.h/.c`): SHTC3 Kelvin / CRC-8 Telemetry         │
│  - Real-Time Clock (`bsp_rtc.h/.c`): PCF85063A Alarm, Sleep Ext0 Wake   │
│  - Storage (`bsp_sdcard.h/.c`, `bsp_nvs.h/.c`): FAT32 VFS & NVS Store   │
│  - Audio Codec (`bsp_audio.h/.c`): ES8311 DAC & NS4168 Amp             │
│  - Network Abstraction (`bsp_wifi.h/.c`): Station Connect & NVS Clear   │
│  - Buttons & Inputs (`bsp_button.h/.c`): Debounce, Long-Press, Wake     │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## 2. Core Functional Requirements

### 2.1. BSP Core Initialization & Orchestration Refactor
- **Current State:** `main.cpp` manually manages 10+ initialization sequences, mutex locks, and timers.
- **Target State:**
  - Introduce `bsp_board_init()` / `bsp_board_init_with_config(const bsp_config_t *cfg)` which initializes all hardware peripherals systematically.
  - Introduce `bsp_lvgl_start()` to encapsulate the FreeRTOS display task, tick interface, and thread-safe lock mechanisms (`bsp_lvgl_lock()` / `bsp_lvgl_unlock()`).
  - Introduce `bsp_system_shutdown(const char *asset_path)` to handle safe display clearing / shutdown image rendering and power latch de-assertion.

### 2.2. NVS Wi-Fi Credential Management & Reset Mechanism
- **In BSP (`bsp_nvs.h/.c` & `bsp_wifi.h/.c`):**
  - Standardized getters/setters: `bsp_nvs_set_wifi_credentials(const char *ssid, const char *password)` and `bsp_nvs_get_wifi_credentials(...)`.
  - Dedicated wipe API: `bsp_nvs_clear_wifi_credentials()`.
  - **Hardware Factory Reset Trigger:** Long-pressing the BOOT button (e.g., 5 seconds during boot) triggers a complete NVS credential wipe, visual feedback on e-Paper, and initiates BLE provisioning.

### 2.3. Automated Cloud Time Sync (SNTP over Web -> PCF85063A RTC)
- **Workflow on Cold Boot / Network Connect:**
  1. Once Wi-Fi is connected, background SNTP client initializes with pools (`pool.ntp.org`, `time.google.com`).
  2. Upon receiving time sync notification callback (`sntp_set_time_sync_notification_cb`):
     - Internal POSIX system clock is updated.
     - New timestamp is automatically converted and committed to the physical **PCF85063A RTC registers** via `bsp_rtc_set_datetime()`.
  3. Subsequent deep-sleep wakeups can instantly read accurate time from the PCF85063A without needing Wi-Fi reconnect.

### 2.4. Ultra-Low Power Deep-Sleep & Fool-Proof Wake Mechanism
- **Power Optimization Objectives:**
  - Turn OFF the display 3.3V power gate (`EPD3V3_EN`, GPIO6 = LOW) to prevent e-Paper driver idle current.
  - Mute & disable NS4168 audio amplifier (`PA_EN`, GPIO42 = LOW).
  - Put SHTC3 sensor into sleep mode (`0xB098`).
  - Hold RTC power rail active while keeping I2C bus lines cleanly isolated (`gpio_hold_en()`).
- **Wakeup Sources:**
  - **RTC Countdown / Alarm (`GPIO5`):** Configured as `esp_sleep_enable_ext0_wakeup(GPIO_NUM_5, 0)` for scheduled interval updates (e.g., wake every 15 mins).
  - **Tactile Button (`GPIO0` / `GPIO18`):** Configured as ext1 wakeup for manual user wake.
  - **Brownout / Safe Latch:** Safe handling of `BAT_CTRL` (GPIO17) to prevent power cutoff while flash operations or e-Paper waveform refreshes are in progress.

### 2.5. Fast Wi-Fi Reconnection via RTC Slow Memory State Caching
- **The Problem:** A standard Wi-Fi station connection takes 2,000–4,500 ms (scanning all 13 channels + DHCP DISCOVER/OFFER/REQUEST/ACK handshake), burning precious battery during every wake cycle.
- **The Fast Connect Solution (< 400 ms Reconnection):**
  - Allocate a cache struct in **RTC Slow Memory** (`RTC_DATA_ATTR`) that survives deep sleep:
    ```c
    typedef struct {
        uint32_t magic;           // Validation token (e.g. 0x53335746 "S3WF")
        uint8_t  bssid[6];        // AP MAC Address
        uint8_t  channel;         // AP Wi-Fi Channel (1-13)
        esp_netif_ip_info_t ip;   // Cached Static IP, Gateway, Netmask
        uint32_t lease_timestamp; // Lease validity reference
    } bsp_wifi_fast_cache_t;
    ```
  - **On Wake from Deep Sleep:**
    1. If `magic == VALID`, set static IP directly on `esp_netif` (bypassing the DHCP negotiation).
    2. Configure `wifi_config.sta.bssid_set = 1`, fill `bssid`, and set `channel`.
    3. Issue `esp_wifi_connect()` directly to the known channel and BSSID.
  - **Fallback Handling:**
    - If fast connect times out or fails (e.g., router changed channel or IP conflict), automatically invalidate cache, re-enable DHCP, perform full channel scan, and update the RTC cache upon successful `IP_EVENT_STA_GOT_IP`.

### 2.6. ThingsBoard Claiming Token & BLE Provisioning Flow
- **6-8 Character Cryptographically Secure Claiming Key:**
  - Generated using ESP32 hardware RNG (`esp_fill_random()`):
    - Length: 6–8 characters
    - Character set: Alphanumeric uppercase `[A-Z0-9]` (no special characters or extended ASCII).
  - Published to ThingsBoard Secure MQTTS (`port 8333` over TLS) topic `v1/devices/me/claim` with configurable expiration duration (e.g., 300s).
  - Rendered conspicuously on the 1.54″ e-Paper screen for user pairing in the Chrome web dashboard.
- **Telemetry Payload Format (Native Kelvin):**
  - Published to `v1/devices/me/telemetry` as `{"temperature": <Kelvin>, "humidity": <RH%>, "battery_mv": <mV>, "battery_pct": <%>}`.
  - Converted to user-friendly °C and °F on the local 1.54" e-Paper display.
- **BLE GATT Provisioning Service (`wifi_prov_scheme_ble`):**
  - Advertises as `PROV_ESP32S3-XXXXXXXX` using ESP-IDF `wifi_provisioning` component.
  - Implements GATT service interface compatible with Chrome Web Bluetooth API / Web Dashboard.
  - Provisions SSID and Password directly into `bsp_nvs`.
- **Fallback Credentials (`app_secrets.h`):**
  - Dedicated configuration header containing fallback Wi-Fi and Secure ThingsBoard MQTTS access parameters if NVS is unconfigured.

### 2.7. ThingsBoard Remote Firmware OTA Updates
- **ThingsBoard Firmware Update Architecture:**
  - Device subscribes to `v1/devices/me/attributes` on MQTT connect.
  - ThingsBoard pushes shared firmware attributes: `fw_title`, `fw_version`, `fw_url`, `fw_checksum`.
  - Device state machine reports OTA progress back to ThingsBoard:
    - `"fw_state": "DOWNLOADING"` -> Streams binary into inactive `ota_0` or `ota_1` partition via `esp_https_ota`.
    - `"fw_state": "DOWNLOADED"` -> Verifies image checksum & cryptographic signature.
    - `"fw_state": "VERIFIED"` / `"UPDATING"` -> Marks partition as bootable (`esp_ota_set_boot_partition`) and schedules clean system restart.
    - `"fw_state": "UPDATED"` -> Confirms valid boot on next startup via `esp_ota_mark_app_valid_cancel_rollback()`.

### 2.8. Complete Device Lifecycle & Power Architecture
- **Phase 1: Boot & BLE Provisioning (If unconfigured or connection fails)**
  - Dedicated **BLE Provisioning Screen** (`PROV_ESP32S3-XXXXXXXX`).
  - Device stays awake on network failure so the user can easily diagnose and configure.
- **Phase 2: Claiming Screen (When ready to claim)**
  - Dedicated full-screen **Claiming Display**: Shows only the 6–8 char claiming key `[ XXXXXXXX ]` and expiry countdown.
  - Automatically transitions to the Active Dashboard once claimed in ThingsBoard (or advances via BOOT button).
- **Phase 3: Active Telemetry Dashboard (Once claimed & running)**
  - Shows SHTC3 temperature, relative humidity, and live battery gauge.
  - **Passive Card Inversion**: Solid black background with white text when sensor values breach threshold bounds.
  - **Low Battery Indicator**: Visual `! LOW BATT <20%` banner.
  - **Network Failure Guard**: Displays `! NO NETWORK / RETRY` and prevents the device from going to sleep.
- **Phase 4: Ultra-Low Power Deep Sleep Cycle**
  - The device spends the majority of its lifetime in deep sleep.
  - Screen image is preserved with zero power draw on the bi-stable 1.54″ e-Paper display.
  - On wake: reads sensor, reconnects via RTC fast cache (<400ms), publishes telemetry, updates e-Paper, and resumes deep sleep.
- **Phase 5: Power Off Only**
  - Renders `space_cat.bin` image onto the e-Paper panel only when the user executes a clean shutdown via the POWER button.

---

## 3. Phased Implementation Roadmap

| Phase | Milestone | Deliverables |
|---|---|---|
| **Phase 1** | **BSP Core Refactoring & Abstraction** | - `bsp_config_t` & comprehensive `bsp_board_init()`<br>- Automated LVGL task encapsulation & mutex API<br>- Fool-proof deep sleep and power gating subsystem (`bsp_power_enter_deep_sleep`) |
| **Phase 2** | **NVS Credentials, Fast Wi-Fi & Time Sync** | - `bsp_wifi.c/.h` with NVS credential store & wipe APIs<br>- Fast Wi-Fi reconnect via RTC slow memory state cache (BSSID, channel, static IP)<br>- BOOT button 5-sec factory wipe handler<br>- SNTP-to-PCF85063A time synchronization service |
| **Phase 3** | **BLE GATT Provisioning & ThingsBoard MQTT/OTA** | - 6-8 char alphanumeric Claiming Key Generator (`app_claiming.h/.c`)<br>- BLE GATT Provisioning manager (`app_ble_prov.h/.c`)<br>- Fallback secrets configuration (`app_secrets.h`)<br>- ThingsBoard MQTT client with telemetry, claiming, and remote OTA state machine |
| **Phase 4** | **Telemetry Application & UI Refactor** | - Refactor `Unified_BSP_Demo/main/main.cpp` into a complete environmental telemetry node<br>- Render claiming code, sensor metrics, battery gauge, and connectivity on 1.54" e-Paper<br>- Handle user interactions, fast deep sleep, and factory reset |
| **Phase 5** | **Documentation & Release Readiness** | - Comprehensive `README.md` with wiring diagrams, API references, and power budgets<br>- Sync `docs/PIN_MAP.md` & `Kconfig`<br>- Validate against ESP-IDF component registry specifications |

---

## 4. Immediate Next Steps for Collaborative Review

1. Approve the separation boundary (BSP Hardware Layer vs App Middleware).
2. Review the proposed `bsp_config_t` structure and deep-sleep power-down sequence.
3. Begin Phase 1 implementation.
