# ESP32-S3 ePaper 1.54" V2 — Peripherals & Sleep Verification Test Suite

This example serves as the official hardware verification and comprehensive self-test suite for the **Waveshare ESP32-S3 ePaper 1.54" V2** development board (`ESP32-S3-PICO-1-N8R8`).

---

## 🎯 Test Coverage

### 1. Static Hardware Peripheral Tests (15 Subsystems)
1. **Dynamic Hardware Initialization**: Validates `BSP_INIT_MODE_FULL`, `BSP_INIT_MODE_FAST`, and `BSP_INIT_MODE_MIN`.
2. **System Identification, Silicon Revision & Diagnostics API**: Queries MAC device ID, friendly name, silicon revision (e.g. `v0.2`), CPU core count, and memory heaps.
3. **Power Rail Latch & Battery ADC**: Verifies active-HIGH LDO latch on `GPIO 17` and calibrates battery ADC voltage reading (`GPIO 4`, ADC1 CH3).
4. **SHTC3 Environmental Sensor**: Reads temperature (Kelvin / Celsius / Fahrenheit) and relative humidity (%RH) over shared I2C bus (`GPIO 47/48`).
5. **NXP PCF85063A Hardware RTC & NVRAM**: Tests atomic calendar datetime reads/writes and 8-bit persistent NVRAM storage (Register `0x03`).
6. **ESP32-S3 RTC Slow Memory Engine**: Validates boot counting, sleep cycle tracking, and 32-byte scratchpad state retention across resets.
7. **Tactile Buttons**: Initializes debounced interrupt handlers for `BOOT` (`GPIO 0`) and `POWER` (`GPIO 18`).
8. **ES8311 Audio Codec & NS4168 Class-D Amp**: Powers up the audio domain (`GPIO 42/46`) and synthesizes a test chime.
9. **Formatted Time & Date String Generators**: Exercises 4 time formats (24h/12h with seconds/minutes) and 3 date formats (`MM/DD/YY`, Day of Week, `MM/DD/YY DOW`).
10. **ThingsBoard Generic Telemetry Serialization**: Validates generic `bsp_tb_entry_t` struct serialization into JSON payloads.
11. **Base57 Unambiguous Key Generator**: Tests random key generation for 8-character BLE PoP and 6-character Claiming token without confusing characters (`0`, `O`, `o`, `1`, `l`, `I`).
12. **NVS Storage**: Tests non-volatile key-value write, read, and erase operations.
13. **MicroSD Card**: Attempts mounting FATFS filesystem in 1-bit mode and performs test file I/O.
14. **Wi-Fi Station & Passive Scanner**: Initializes Wi-Fi MAC/PHY and performs active scan for nearby APs.
15. **SSD1681 E-Paper & LVGL v9 Rendering**: Renders test dashboard and 1-bit monochrome QR code on the 200×200 display panel.

---

### 2. Sleep & Wake Functional Verification Suite
The test harness validates power-down, sleep gating, and wake state transitions across multiple cycles:
- **`LS-1` — Light Sleep + Internal Timer (3 sec)**: Verifies CPU clock gating, immediate resumption without reboot, and verifies timer accuracy.
- **`LS-2` — Light Sleep + External PCF85063A RTC INT (3 sec)**: Programs PCF85063A countdown timer, sleeps with `GPIO 5` INT wake trigger, and verifies wakeup and timer flag clearing.
- **`DS-1` — Deep Sleep + Internal Timer (4 sec) $\rightarrow$ `FAST` Mode**: Powers off CPU/peripherals, wakes from `ESP_SLEEP_WAKEUP_TIMER`, and resumes via fast partial e-Paper refresh without full-screen flicker.
- **`DS-2` — Deep Sleep + External PCF85063A RTC INT (4 sec) $\rightarrow$ `MIN` Mode**: Wakes from `ESP_SLEEP_WAKEUP_EXT1` (`GPIO 5`), clears RTC timer flag, and runs minimal telemetry burst mode.

---

## 🕹️ Interactive Controls (Heartbeat Mode)
Once all automated tests pass, the node enters a 5-second periodic monitoring heartbeat:
- **Click `BOOT` Button (`GPIO 0`)**: Triggers an on-demand 5-second **Light Sleep** cycle.
- **Long Press `BOOT` Button (2 seconds)**: Enters indefinite **Deep Sleep** configured to wake exclusively when `BOOT` or `POWER` is pressed.

---

## 🚀 How to Build and Flash

```powershell
# 1. Export ESP-IDF v6.1 (or v5.1+)
. C:\esp\v6.1\esp-idf\export.ps1

# 2. Build the project
idf.py build

# 3. Flash and open serial monitor
idf.py -p COMx flash monitor
```
