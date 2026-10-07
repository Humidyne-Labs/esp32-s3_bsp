# ESP32-S3 Touch ePaper BSP: Modernization & Architecture Action Plan

**Repository:** `Humidyne-Labs/esp32-s3_bsp`  
**Target Hardware:** Waveshare ESP32-S3-Touch-ePaper-1.54 V2  
**Framework:** ESP-IDF v5.1+ / v6.1 (C / C++20)  
**Architecture Specification:** [System Architecture & Multi-Threaded Runtime](file:///c:/Users/Matt/Documents/GitHub/esp32-s3_bsp/docs/ARCHITECTURE.md)  
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
│  - MQTT Client Wrapper (`app_mqtt.c`) -> Talks to ThingsBoard Topics    │
│  - BLE Provisioning Manager (`app_ble_prov.c`)                          │
│  - Cloud Time Sync Service (`app_time_sync.c` -> SNTP + RTC commit)     │
└────────────────────────────────────┬────────────────────────────────────┘
                                     │
┌────────────────────────────────────▼────────────────────────────────────┐
│                    BOARD SUPPORT PACKAGE (BSP CORE)                     │
│  - Power & Latch (`bsp_power.h/.c`): Latch, Battery ADC, Deep Sleep     │
│  - Display & LVGL (`bsp_display.h`, `bsp_lvgl.h`): EPD SPI & Refresh    │
│  - Capacitive Touch (`bsp_touch.h/.cpp`): FT6336 I2C Driver             │
│  - Sensors (`bsp_sensors.h/.c`): SHTC3 Kelvin / CRC-8 Telemetry         │
│  - Real-Time Clock (`bsp_rtc.h/.c`): PCF85063A Alarm, Sleep Ext0 Wake   │
│  - Storage (`bsp_sdcard.h/.c`, `bsp_nvs.h/.c`): FAT32 VFS & NVS Store   │
│  - Audio Codec (`bsp_audio.h/.c`): ES8311 DAC & NS4168 Amp              │
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

### 2.4. Audio Subsystem Architecture (ES8311 Codec & NS4150B/NS4168 Amp)
- **Hardware Circuit Topology:**
  - **Codec:** Everest Semi ES8311 Mono Low-Power Audio Codec (7-bit I2C: `0x18`).
  - **Amplifier:** NS4150B / NS4168 Class-D Mono Power Amplifier (Pin 1 `CTRL` enabled via `BSP_PIN_PA_CTRL` GPIO 46, active-HIGH).
  - **Power Gate Rail (`PA_EN` / GPIO 42):** P-MOSFET high-side switch (Active-LOW: 0 = Power ON). **Crucial Hardware Note:** The 3.3V rail to the codec must remain active (0V on GPIO 42) across normal operation and sleep states to prevent internal ESD protection diodes from pulling down the shared I2C bus (SDA: 47, SCL: 48).
  - **I2S Master Pinout:**
    - `MCLK`: GPIO 14 ($16\text{ kHz} \times 256 = 4.096\text{ MHz}$)
    - `BCLK` / `SCLK`: GPIO 15 ($512\text{ kHz}$)
    - `LRCK` / `WS`: GPIO 38 ($16\text{ kHz}$)
    - `DSDIN` (ESP32 DOUT $\rightarrow$ ES8311 DSDIN): GPIO 45
    - `ASDOUT` (ES8311 ASDOUT $\rightarrow$ ESP32 DIN): GPIO 16

- **Clock Configuration & Register Blueprint (16 kHz Fs, 4.096 MHz MCLK):**
  - Native 16-bit Philips Mono I2S frame (`I2S_SLOT_MODE_MONO`, `I2S_STD_SLOT_LEFT` mapped directly to ES8311 Mono DAC, 1 sample per frame).
  - `0x00 = 0x80`: Clock State Machine on, Slave serial port mode.
  - `0x01 = 0x3F`: Derived from MCLK pin (`MCLK_SEL = 0`), internal digital/analog clocks active.
  - `0x02 = 0x00`: Pre-divider = 1, Pre-multiplier = 1 ($4.096\text{ MHz}$ internal master clock).
  - `0x03 = 0x10`: Single speed mode, ADC OSR.
  - `0x04 = 0x20`: DAC Over-Sampling Ratio for $16\text{ kHz}$ sample rate.
  - `0x05 = 0x00`: ADC/DAC digital clock divider = 1.
  - `0x06 = 0x03`: BCLK master divider ratio = 4.
  - `0x07 = 0x00`, `0x08 = 0xFF`: Master LRCK divider high/low bytes.
  - `0x09 = 0x0C`, `0x0A = 0x0C`: 16-bit Philips I2S format, SDP in/out unmuted.
  - `0x0D = 0x01`: Analog reference circuits enabled, normal VMID operation.
  - `0x0E = 0x02`: Analog blocks powered, VROI enabled.
  - `0x12 = 0x00`: DAC analog power on (`PDN_DAC = 0`).
  - `0x31 = 0x00`: DAC DSM/DEM soft unmute.
  - `0x32 = 0xBF`: 0 dB nominal full-scale digital volume.
  - `0x44 = 0x58`: DAC internal reference routing (`ADCL + DACR`) enabled.

- **Zero-Heat Deep Standby & Thermal Protection for SHTC3:**
  - Leaving ADC modulators, PGA, analog bias, and VMID charging causes continuous $15\dots30\text{ mA}$ dissipation ($\sim 50\dots100\text{ mW}$), heating the PCB and artificially inflating SHTC3 temperature readings by $+3^\circ\text{C}\dots+5^\circ\text{C}$.
  - **Zero-Heat Suspension Sequence:**
    1. Mute Class-D amplifier: `BSP_PIN_PA_CTRL = 0`.
    2. Soft-mute DAC DSM/DEM: Reg `0x31 = 0x60`.
    3. Power down PGA & ADC: Reg `0x0E = 0xFF`, `0x14 = 0x00`.
    4. Power down DAC stage: Reg `0x12 = 0x02`, `0x32 = 0x00`.
    5. Power down analog reference & VMID: Reg `0x0D = 0xFC`.
    6. Assert digital reset and gate clocks: Reg `0x00 = 0x1F`, `0x01 = 0x00`.
    7. Halt I2S channel clocks via `i2s_channel_disable()`.
    - Result: Quiescent current drops to $\approx 0\,\mu\text{A}$ without dropping the 3.3V rail, eliminating thermal drift on SHTC3 while preserving I2C bus stability.
  - **Thermal Analysis & 60-Second Wake Cycle Characterization:**
    - Thermal camera imaging confirmed **zero hotspot around the ES8311 codec** in suspended standby.
    - The observed periodic $\Delta T \approx 10^\circ\text{C}$ transient during wake is primarily generated by active CPU execution (240 MHz dual-core MCU + 80 MHz Octal PSRAM) and the MP1605 DC-DC switching regulator during display rendering and post-render blocking delays.
  - **Instantaneous Cold-Sampling Strategy:**
    - Perform SHTC3 environmental sampling within the first $15\dots25\text{ ms}$ of wake initialization (before MCU/PMIC heat can conduct across the PCB ground plane to the SHTC3 package).
    - Eliminate unnecessary busy-waits (`vTaskDelay`) in fast wake paths so the MCU transitions back to deep sleep immediately after display DMA kickoff.

- **Tone Synthesis Engine:**
  - 256-point single-period 16-bit sine lookup table stored in flash (Peak amplitude: 30,000, ~0 dBFS).
  - 5ms linear attack/decay envelopes on tone transitions to eliminate acoustic DC-offset pops.

### 2.5. Ultra-Low Power Deep-Sleep & Fool-Proof Wake Mechanism
- **Power Optimization Objectives:**
  - Turn OFF the display 3.3V power gate (`EPD3V3_EN`, GPIO6 = LOW) to prevent e-Paper driver idle current.
  - Mute & disable NS4150B audio amplifier (`PA_CTRL`, GPIO46 = LOW; maintain `PA_EN` GPIO42 = LOW).
  - Put SHTC3 sensor into sleep mode (`0xB098`).
  - Hold RTC power rail active while keeping I2C bus lines cleanly isolated (`gpio_hold_en()`).
- **Wakeup Sources:**
  - **RTC Countdown / Alarm (`GPIO5`):** Configured as `esp_sleep_enable_ext0_wakeup(GPIO_NUM_5, 0)` for scheduled interval updates (e.g., wake every 15 mins).
  - **Tactile Button (`GPIO0` / `GPIO18`):** Configured as ext1 wakeup for manual user wake.
  - **Brownout / Safe Latch:** Safe handling of `BAT_CTRL` (GPIO17) to prevent power cutoff while flash operations or e-Paper waveform refreshes are in progress.

### 2.6. Fast Wi-Fi Reconnection via RTC Slow Memory State Caching
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

### 2.7. ThingsBoard Claiming Token & BLE Provisioning Flow
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

### 2.8. ThingsBoard Remote Firmware OTA Updates
- **ThingsBoard Firmware Update Architecture:**
  - Device subscribes to `v1/devices/me/attributes` on MQTT connect.
  - ThingsBoard pushes shared firmware attributes: `fw_title`, `fw_version`, `fw_url`, `fw_checksum`.
  - Device state machine reports OTA progress back to ThingsBoard:
    - `"fw_state": "DOWNLOADING"` -> Streams binary into inactive `ota_0` or `ota_1` partition via `esp_https_ota`.
    - `"fw_state": "DOWNLOADED"` -> Verifies image checksum & cryptographic signature.
    - `"fw_state": "VERIFIED"` / `"UPDATING"` -> Marks partition as bootable (`esp_ota_set_boot_partition`) and schedules clean system restart.
    - `"fw_state": "UPDATED"` -> Confirms valid boot on next startup via `esp_ota_mark_app_valid_cancel_rollback()`.

### 2.9. Complete Device Lifecycle & Power Architecture
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

### 2.10. Tri-Node Thermal Fusion, Psychrometric Invariant Engine, & ULP RISC-V Architecture

To overcome the physical reality of PCB thermal inertia without resorting to invalid sign-flipped thermal divider heuristics, the Humiditron environmental sensing architecture adopts a **Tri-Node Physical Fusion & Psychrometric Invariant Model**, coupled with an **Ultra-Low-Power (ULP) RISC-V Coprocessor Sampling Engine**.

```
                      ┌─────────────────────────────────────────────────────────┐
                      │              TRI-NODE THERMAL SENSING MATRIX            │
                      └────────────────────────────┬────────────────────────────┘
                                                   │
        ┌──────────────────────────────────────────┼──────────────────────────────────────────┐
        │                                          │                                          │
        ▼                                          ▼                                          ▼
┌────────────────────────────┐         ┌────────────────────────────┐         ┌────────────────────────────┐
│      Node 1: SHTC3         │         │      Node 2: SSD1681       │         │    Node 3: MCU TSENS       │
│   (PCB Surface Package)    │         │  (E-Paper Glass Substrate) │         │  (ESP32-S3 Silicon Die)    │
│ • I2C Addr: 0x70           │         │ • SPI Read Cmd: 0x1B       │         │ • On-Chip Analog Sensor    │
│ • Fast Thermal Coupling    │         │ • Thermally Decoupled Glass│         │ • <20ms Cold Wake Sample   │
│ • Yields True Invariants:  │         │ • 12-Bit Fixed Point Temp  │         │ • Verified +/-0.24°C from  │
│   Dew Point (Td) &         │         │ • Direct Ambient Baseline  │         │   Room Ambient at Rest     │
│   Actual Vapor Pressure (e)│         │                            │         │                            │
└─────────────┬──────────────┘         └─────────────┬──────────────┘         └─────────────┬──────────────┘
              │                                      │                                      │
              │                                      └──────────────────┬───────────────────┘
              │                                                         │
              │                                                         ▼
              │                                           ┌───────────────────────────┐
              │                                           │ Triangulated Ambient Ref  │
              │                                           │   T_amb = f(TSENS, EPD)   │
              │                                           └─────────────┬─────────────┘
              │                                                         │
              ▼                                                         ▼
┌─────────────────────────────────────────────────────────────────────────────────────────────────────┐
│                          THERMODYNAMIC PSYCHROMETRIC RECONSTRUCTION ENGINE                          │
│                                                                                                     │
│   1. Invariant Actual Vapor Pressure:   e = (RH_sens / 100) * e_s(T_sens)                           │
│   2. Invariant Dew Point (Magnus):      T_d = (243.04 * gamma) / (17.625 - gamma)                   │
│   3. True Ambient Relative Humidity:    RH_amb = RH_sens * exp(factor(T_sens) - factor(T_amb))      │
│   4. Dynamic Confidence Rating:         Confidence = clamp(100% - (|T_sens - T_amb| * 10%), 10, 100)│
└─────────────────────────────────────────────────────────────────────────────────────────────────────┘
```

#### A. Physical & Psychrometric Foundation (Vapor Pressure Invariance)
- **The Core Thermodynamic Invariant:** When a local heat source (buck converter, e-paper drive circuit, or active MCU) warms the SHTC3 package, it raises the temperature of the localized boundary layer air volume **without adding or removing water molecules**.
- **Conservation of Vapor Pressure:** The actual water vapor pressure ($e$) and the dew point ($T_d$) of the air parcel remain identical between the heated PCB microclimate and the surrounding room air.
- **The Saturation Vapor Pressure Ratio Formulation:**
  $$\gamma(T, RH) = \ln\left(\frac{RH}{100}\right) + \frac{17.625 \cdot T}{243.04 + T}$$
  $$T_d = \frac{243.04 \cdot \gamma}{17.625 - \gamma}$$
  $$\text{factor}(T) = \frac{17.625 \cdot T}{243.04 + T}$$
  $$RH_{\text{amb}} = RH_{\text{sens}} \times \exp\left( \text{factor}(T_{\text{sens}}) - \text{factor}(T_{\text{amb}}) \right)$$
- **Empirical Validation (Humiditron 10-Minute Idle Run):**
  - Raw SHTC3: $T_{\text{sens}} = 24.78^\circ\text{C}$ ($76.6^\circ\text{F}$), $RH_{\text{sens}} = 46.1\%$
  - Internal Die Baseline: $T_{\text{die}} = 22.80^\circ\text{C}$ ($73.04^\circ\text{F}$)
  - Ground-Truth Room Ambient: $22.56^\circ\text{C}$ ($72.6^\circ\text{F}$), $54.0\%$ RH
  - **Reconstructed Ambient RH:** $RH_{\text{amb}} = 46.1\% \times \exp(1.6308 - 1.5116) = \mathbf{51.94\%}$ ($\Delta = -2.06\%$ error vs $-7.9\%$ uncompensated raw error).

#### B. Node 2: SSD1681 E-Paper Internal Thermal Sensor (SPI Command `0x1B`)
- **Hardware Integration:** The SSD1681 display controller features an integrated on-glass temperature sensor with $\pm 2^\circ\text{C}$ accuracy ($-25^\circ\text{C}\dots+50^\circ\text{C}$). Because the glass face sits physically exposed to ambient air and isolated from the main FR4 ground plane, it serves as an independent ambient ground-truth node.
- **Protocol & Data Format:**
  - Command: `0x1B` (Temperature Sensor Control - Read from Register).
  - Returns 2 bytes: `[A11..A4]` (MSB) + `[A3..A0, 0000]` (LSB).
  - Conversion:
    - If Bit $D_{11} == 0$: Positive Celsius $\implies T_{\text{EPD}} = +\frac{\text{Raw}_{12}}{16.0}$
    - If Bit $D_{11} == 1$: Negative Celsius $\implies T_{\text{EPD}} = -\frac{\text{Two's Complement}(\text{Raw}_{12})}{16.0}$
- **HAL Expose (`bsp_display.h`):**
  ```c
  esp_err_t bsp_display_read_temperature(float *out_temp_c);
  ```

#### C. Node 3 & Coprocessor Engine: ULP RISC-V Zero-Heat Cold Polling
- **The Zero-Heat Objective:** Active execution of the dual-core 240 MHz Xtensa LX7 MCU and Octal PSRAM dissipates $\sim 150\dots240\text{ mW}$, causing thermal conduction into the PCB ground plane. Running periodic telemetry via the **ULP RISC-V coprocessor** consumes only $\sim 20\dots50\,\mu\text{A}$ ($< 0.15\text{ mW}$), preserving true room thermal equilibrium.
- **Hardware Bus & Power Domain Mapping:**
  - **I2C Bus:** SDA (GPIO 47), SCL (GPIO 48).
  - **Coprocessor Execution Scheme:**
    - **Architecture Path 1 (Tickless Light-Sleep / ULP Bitbang):** ULP RISC-V runs in the RTC domain, bit-banging SHTC3 wake/read commands on GPIO 47/48 while the main CPU cores remain halted in low-power sleep.
    - **Architecture Path 2 (Sub-2ms High-Speed Cold Wakeburst):** High-speed RTC timer wakes the chip directly into an ultra-fast ROM-level cold poll hook ($80\text{ MHz}$, display/WiFi uninitialized), samples TSENS die and SHTC3 low-power mode ($< 800\,\mu\text{s}$ conversion), stores data into `RTC_DATA_ATTR` slow memory circular buffer, and instantly re-enters deep sleep before heat can conduct.
- **RTC Memory Data Pipeline:**
  ```c
  typedef struct {
      uint32_t timestamp_epoch;
      uint16_t raw_shtc3_temp;
      uint16_t raw_shtc3_rh;
      float    die_temp_c;
      float    epd_temp_c;
      float    reconstructed_amb_temp_c;
      float    reconstructed_amb_rh;
      uint8_t  confidence_pct;
  } bsp_ulp_telemetry_entry_t;
  ```
- **Event-Driven Main CPU Wake Triggers:**
  - ULP asserts `ulp_riscv_wakeup_main_processor()` only upon:
    1. Environmental threshold breach (e.g. rapid humidity jump $> 5\%$ indicating localized event).
    2. Scheduled display refresh interval (e.g. every 5 minutes).
    3. User tactile button press (`BOOT0` / `PWR`).

---

## 3. Phased Implementation Roadmap

| Phase | Milestone | Deliverables |
|---|---|---|
| **Phase 1** | **BSP Core Refactoring & Audio Perfection** | - Native 16-bit Mono I2S driver (`bsp_audio.c/.h`) with zero-heat suspension ($\le 1\,\mu\text{A}$)<br>- Single-period 256-point sine lookup table with exact acoustic pitch<br>- Comprehensive `bsp_board_init()` orchestration |
| **Phase 2** | **SSD1681 Temperature & Psychrometric Engine** | - SSD1681 SPI Cmd `0x1B` 12-bit on-glass temperature driver (`bsp_display.cpp`)<br>- Thermodynamic Psychrometric Invariant Engine (`bsp_sensor_cal.c` overhaul)<br>- Multi-node ambient temperature fusion and dynamic confidence score scoring |
| **Phase 3** | **ULP RISC-V Coprocessor Cold-Sampling Subsystem** | - ULP RISC-V assembly/C firmware (`components/esp32-s3_bsp/ulp/`)<br>- Low-power SHTC3 & TSENS sampling pipeline in RTC Slow Memory<br>- Deep-sleep cycle manager with event-driven main CPU wake triggers |
| **Phase 4** | **NVS Credentials, Fast Wi-Fi & Time Sync** | - `bsp_wifi.c/.h` with NVS credential store & wipe APIs<br>- Fast Wi-Fi reconnect via RTC slow memory state cache (BSSID, channel, static IP <400ms)<br>- BOOT button 5-sec factory wipe handler<br>- SNTP-to-PCF85063A time synchronization service |
| **Phase 5** | **BLE GATT Provisioning & ThingsBoard MQTT/OTA** | - 6-8 char alphanumeric Claiming Key Generator (`app_claiming.h/.c`)<br>- BLE GATT Provisioning manager (`app_ble_prov.h/.c`)<br>- Fallback secrets configuration (`app_secrets.h`)<br>- ThingsBoard MQTT client with telemetry, claiming, and remote OTA state machine |
| **Phase 6** | **Telemetry Application & UI Refactor** | - Refactor `Unified_BSP_Demo/main/main.cpp` into a complete environmental telemetry node<br>- Render claiming code, sensor metrics, battery gauge, and connectivity on 1.54" e-Paper<br>- Handle user interactions, fast deep sleep, and factory reset |
| **Phase 7** | **Documentation & Release Readiness** | - Comprehensive `README.md` with wiring diagrams, API references, and power budgets<br>- Sync `docs/PIN_MAP.md` & `Kconfig`<br>- Validate against ESP-IDF component registry specifications |

---

## 4. Immediate Next Steps for Collaborative Review

1. Review and savor the **Tri-Node Thermal Fusion Architecture & Psychrometric Invariant Model** in Section 2.10.
2. Confirm the SSD1681 SPI Cmd `0x1B` protocol integration for Node 2.
3. Review ULP RISC-V power domain partitioning and RTC slow memory circular buffer layout.
4. Prepare for the execution session!

