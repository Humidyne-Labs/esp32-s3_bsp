# 🌊 HUMIDITRON MASTER ACTION PLAN: Tri-Node Sensing, Psychrometric Engine, & ULP Coprocessor

**System Codename:** *Humiditron Vapour-Core (Thermal-Lockout MUX & Sliding-Window Edition)*  
**Repository:** `Humidyne-Labs/esp32-s3_bsp`  
**Target Hardware:** Waveshare ESP32-S3-Touch-ePaper-1.54 V2  
**Framework:** ESP-IDF v5.1+ / v6.1 (C / C++20 / ULP RISC-V C)  
**Document Status:** 🍷 **Approved Master Architecture & Implementation Blueprint**  
**Core Goal:** Eliminate PCB thermal drift, preserve true ambient accuracy, and achieve zero-heat sensing.

---

## 1. Physical Foundation & Philosophy of Operation

### 1.1. The True Physical Mechanism of Thermal Drift
The PCB does **not** warm the ambient air in the room. Rather:
1. Active MCU and power circuitry generate heat that conducts through the **FR4 copper traces and solder pads** directly into the **SHTC3 sensor silicon die / package** ($T_{\text{SHTC3\_die}} > T_{\text{air}}$).
2. As the sensor die warms, the saturation vapor pressure $e_s(T_{\text{die}})$ inside the package cavity increases.
3. The capacitive polymer hygrometer measures relative humidity relative to the **heated die temperature**:
   $$RH_{\text{measured}} = \frac{e}{e_s(T_{\text{die}})} \times 100\% < RH_{\text{ambient}}$$
4. **The Thermodynamic Invariant:** Because no moisture mass is added or removed, the **actual water vapor pressure ($e$)** and the **Dew Point ($T_d$)** of the ambient air entering the sensor package remain **strictly constant and true**.

### 1.2. Legacy Calibration Purge & Reversion to Pristine SHTC3 Driver
- The old synthetic thermal divider calibration code (`bsp_sensor_cal.c` and `bsp_sensor_cal.h`) has been **completely removed** from the repository and build pipeline.
- The underlying driver [`bsp_sensors.h`](file:///c:/Users/Matt/Documents/GitHub/esp32-s3_bsp/components/esp32-s3_bsp/include/bsp/bsp_sensors.h) / [`bsp_sensors.c`](file:///c:/Users/Matt/Documents/GitHub/esp32-s3_bsp/components/esp32-s3_bsp/src/bsp_sensors.c) is now **100% pristine**, directly exposing clean, unadulterated NIST-traceable SHTC3 reads (`bsp_shtc3_read()`, `bsp_shtc3_read_lp()`).

---

## 2. Hardware Peripheral Access Lockout (Thermal MUX Gatekeeper)

To guarantee that sensitive environmental sensors are **never queried while the board is thermally unsettled**:

```
                                THERMAL LOCKOUT GATEKEEPER (MUX)
                                ═════════════════════════════════

                 Application Requests Environmental Telemetry
                                      │
                                      ▼
                      ┌───────────────────────────────┐
                      │ Is System in Thermal Cooldown │
                      │   (Time since wake < 0.5*T_sl)│
                      └───────────────┬───────────────┘
                                      │
                      ┌───────────────┴───────────────┐
                      │                               │
                 [YES: LOCKED OUT]              [NO: EQUILIBRIUM]
                      │                               │
                      ▼                               ▼
       ┌─────────────────────────────┐ ┌─────────────────────────────┐
       │ Serve Cached Cold Reading   │ │ Execute Direct Hardware     │
       │ from RTC Slow Memory Buffer │ │ Read (Sensor Die Acclimated)│
       │ (Unperturbed Ambient Floor) │ └─────────────────────────────┘
       └─────────────────────────────┘
```

### Access Control Rules:
1. **Rule A (Read Before Wake):** Primary critical environmental readings (SHTC3 Temp, RH, TSENS Die) are acquired by the **ULP Coprocessor during deep sleep** when the board is in true quiescent ambient equilibrium.
2. **Rule B (Thermal Lockout Timer):** If the Main CPU executes an active workload (e-Paper waveform refresh, Wi-Fi radio burst, audio playback), direct live queries to SHTC3 are **locked out** for a minimum cooldown duration:
   $$\tau_{\text{lockout}} = 0.5 \times T_{\text{sleep\_interval}} \quad (\ge 30\dots 60\text{ s})$$
3. **Lockout Fallback:** Any telemetry query during the lockout window transparently receives the **cached, unperturbed cold reading** from the RTC ring buffer rather than measuring a hot silicon die.

---

## 3. Main App Peripheral Initialization & ULP Hand-Off Pattern

To keep the ULP RISC-V firmware minimal, robust, and lightning fast (<1.5 ms execution), the **Main Application initializes all hardware peripherals once during initial boot** before handing off periodic acquisition to the ULP:

```
[COLD BOOT: MAIN APPLICATION (Xtensa LX7 Core)]
  ├── 1. Initializes I2C Master bus & pad routing (GPIO 47 SDA, GPIO 48 SCL).
  ├── 2. Probes & initializes SHTC3 (verifies Product ID 0x0807, asserts Software Reset, puts SHTC3 into low-power sleep 0xB098).
  ├── 3. Installs & enables internal TSENS analog temperature sensor driver (`temperature_sensor_install`).
  ├── 4. Configures RTC power domain retention (`ESP_PD_DOMAIN_RTC_PERIPH` kept ON).
  ├── 5. Loads ULP binary into RTC Slow Memory (`ulp_riscv_load_binary`).
  ├── 6. Sets ULP periodic timer interval (e.g. 60s or 120s via `ulp_set_wakeup_period`).
  ├── 7. Starts ULP Coprocessor (`ulp_riscv_run`).
  └── 8. Enters Deep Sleep (`esp_deep_sleep_start`).
                     │
                     ▼
[DEEP SLEEP: ULP RISC-V COPROCESSOR PERIODIC ACQUISITION]
  ├── Wakes every 60s/120s directly into lightweight register reads (no peripheral init needed).
  ├── Captures raw integer words into RTC Slow Memory Ring Buffer.
  └── Returns to Deep Sleep (Main CPU wakes strictly on scheduled display timer or button press).
```

---

## 4. Execution Lifecycle & Division of Labor

```
                                    TIMELINE & EXECUTION LIFECYCLE
                                    ═══════════════════════════════

   [DEEP SLEEP STATE] (Main CPU / PSRAM Dormant: 10 µA Current Draw)
   ────────────────────────────────────────────────────────────────────────────────────────
   • Every 60s / 120s RTC Timer: ULP RISC-V Coprocessor wakes in RTC domain (~30 µA, < 0.1 mW)
     1. ULP wakes SHTC3 (I2C Cmd 0x3517, waits 250 µs).
     2. ULP triggers Low-Power measurement (Cmd 0x609C, waits 800 µs).
     3. ULP reads 6-byte raw payload (raw_temp_u16, CRC, raw_rh_u16, CRC) & verifies CRC8.
     4. ULP puts SHTC3 back to sleep (Cmd 0xB098).
     5. ULP reads raw TSENS internal die temperature register.
     6. ULP writes 12-byte raw entry into RTC Slow Memory Ring Buffer.
     7. ULP checks Scheduled Wake Timer:
        - If display refresh interval elapsed (e.g. 5 min / 10 min) -> WAKES Main CPU.
        - Otherwise -> Returns to 10 µA Deep Sleep.

   [ACTIVE WAKE STATE] (Main Xtensa LX7 CPU Active: 240 MHz Dual-Core with Hardware FPU)
   ────────────────────────────────────────────────────────────────────────────────────────
   • Upon Wake (Scheduled Display Timer or User Tactile Button):
     1. Main App powers up Display Rail (GPIO 6 EPD3V3_EN = 0).
     2. Main App reads SSD1681 Glass Surface Temp via SPI Command 0x1B.
     3. Main App reads the raw time-series ring buffer from RTC Slow Memory.
     4. Main App processes the dynamic **Sliding Window Pipeline**:
        - Updates dynamic sliding baseline floor $T_{\text{amb\_floor}}$ for the current cycle.
        - Applies EWMA smoothing and computes sliding Min / Max / Trend rates.
        - Reconstructs Ambient Temp, Ambient RH, Dew Point, and Absolute Humidity.
        - Computes dynamic Thermal Confidence Score.
     5. Main App renders updated metrics onto 1.54" E-Paper Display (partial refresh).
     6. Main App publishes telemetry payload to ThingsBoard IoT (if Wi-Fi cycle scheduled).
     7. Main App powers down Display Rail (GPIO 6 EPD3V3_EN = 1).
     8. Main App re-enters Deep Sleep.
```

---

## 5. Dynamic Sliding-Window Processing & Filtering Pipeline

Because the ambient environment is **dynamic** (diurnal temperature swings, HVAC cycling, open windows), the thermal floor and statistical metrics are recalculated **every single wake cycle across a sliding window** of the latest $M$ samples:

```
[RTC Slow Memory Raw Ring Buffer (64 Samples)]
                      │
                      ▼
┌─────────────────────────────────────────────────────────────┐
│ 1. Dynamic Sliding Ambient Floor ($T_{\text{amb\_floor}}$)  │
│    Calculated over the sliding window of the last M samples:│
│    T_amb_floor = min(TSENS_die[latest - M .. latest])       │
│    Adapts continuously to real environmental temperature    │
│    shifts while filtering out short-term wake heating!      │
└─────────────────────┬───────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────┐
│ 2. Exponentially Weighted Moving Average (EWMA)             │
│    y[n] = alpha * x[n] + (1 - alpha) * y[n-1]  (alpha=0.25) │
│    Smooths sensor quantization noise without phase lag.     │
└─────────────────────┬───────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────┐
│ 3. Sliding Window Extremes & Rate-of-Change                 │
│    - Calculates Min and Max RH over the sliding window.     │
│    - Computes Trend Rate: d(RH)/dt (%/hr) for UI arrows.    │
└─────────────────────┬───────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────┐
│ 4. Psychrometric Invariant Reconstruction                   │
│    Reconstructs true Ambient RH, Dew Point, and Abs Humidity│
└─────────────────────────────────────────────────────────────┘
```

### 5.1. Dynamic Sliding Ambient Floor ($T_{\text{amb\_floor}}$)
Over a sliding window of the last $M$ samples (e.g. $M = 10\dots 30$, representing the last 10–30 minutes):

$$T_{\text{amb\_floor}} = \min_{i=0}^{M-1}\left( T_{\text{die}}[\text{head} - i] \right)$$

Combined with the on-glass sensor $T_{\text{epd}}$ acquired during wake:

$$T_{\text{amb}} = \text{median}\left( T_{\text{amb\_floor}}, T_{\text{epd}}, T_{\text{die\_latest}} \right)$$

### 5.2. Invariant Psychrometric Reconstruction (On Main Xtensa FPU)

1. **Actual Invariant Vapor Pressure ($e$ in $\text{hPa}$):**
   $$e = \left( \frac{RH_{\text{sens}}}{100} \right) \cdot 6.112 \cdot \exp\left( \frac{17.625 \cdot T_{\text{sens}}}{243.04 + T_{\text{sens}}} \right)$$

2. **Invariant Dew Point ($T_d$ in $^\circ\text{C}$):**
   $$\gamma = \ln\left( \frac{RH_{\text{sens}}}{100} \right) + \frac{17.625 \cdot T_{\text{sens}}}{243.04 + T_{\text{sens}}}$$
   $$T_d = \frac{243.04 \cdot \gamma}{17.625 - \gamma}$$

3. **Reconstructed Ambient Relative Humidity ($RH_{\text{amb}}$):**
   $$RH_{\text{amb}} = RH_{\text{sens}} \cdot \exp\left( \frac{17.625 \cdot T_{\text{sens}}}{243.04 + T_{\text{sens}}} - \frac{17.625 \cdot T_{\text{amb}}}{243.04 + T_{\text{amb}}} \right)$$

4. **Invariant Absolute Humidity ($d_v$ in $\text{g/m}^3$):**
   $$d_v = 216.7 \cdot \frac{e}{273.15 + T_{\text{amb}}}$$

5. **Dynamic Thermal Confidence Score:**
   $$\Delta T = |T_{\text{sens}} - T_{\text{amb}}|$$
   $$\text{Confidence (\%)} = \text{clamp}\left( 100.0 - (\Delta T \times 10.0), 10.0, 100.0 \right)$$

---

## 6. Polling Intervals & Thermal Duty Cycle Budget

| Parameter | Recommended Setting | Rationale & Thermal Behavior |
|---|:---:|---|
| **ULP Sensor Poll Interval** | **$60\text{ s}$ to $120\text{ s}$** | ULP takes $1.5\text{ ms}$ at $30\,\mu\text{A}$ ($< 0.15\,\mu\text{W}$). Duty cycle is $0.0025\%$, resulting in **$0.000^\circ\text{C}$ thermal rise**. |
| **Main App Display Wake Interval** | **$300\text{ s}$ to $600\text{ s}$ (5–10 min)** | Allows full PCB thermal decay between partial e-Paper refreshes. |
| **Thermal Lockout Window ($\tau_{\text{lockout}}$)** | **$\ge 0.5 \times T_{\text{sleep}}$ ($30\dots 60\text{ s}$)** | Prevents reading hot silicon immediately after active rendering / radio burst. |
| **Sliding Window Size ($M$)** | **$10\dots 30$ samples** | Covers the last 10 to 30 minutes for rolling floor tracking and $\min/\max$ calculations. |
| **Ring Buffer Capacity** | **64 samples** | Stores over 1 hour of raw time-series telemetry in $< 1\text{ KB}$ of RTC Slow RAM. |

---

## 7. Hardware Pinning & Power Control

```
┌────────────────────────────────────────────────────────────────────────────────────────┐
│                        Hardware Interconnect & Pin Mapping                             │
├──────────────────────┬─────────────┬───────────────────────────────────────────────────┤
│ Signal Description   │ Pin Number  │ Operating Role & Hardware Constraints             │
├──────────────────────┼─────────────┼───────────────────────────────────────────────────┤
│ Shared I2C SDA       │ GPIO 47     │ SHTC3 (0x70), PCF85063 (0x51), ES8311 (0x18)      │
│ Shared I2C SCL       │ GPIO 48     │ SHTC3, PCF85063, ES8311 Clock                     │
│ Audio Rail Gate      │ GPIO 42     │ PA_EN: MUST BE HELD LOW (0V) TO PREVENT CLAMPING  │
│ EPD Power Gate       │ GPIO 6      │ EPD3V3_EN: 0=ON (Active Wake), 1=OFF (Deep Sleep) │
│ EPD SPI CS           │ GPIO 11     │ Display Chip Select (Active LOW)                  │
│ EPD SPI DC           │ GPIO 10     │ Data / Command Control Line                       │
│ EPD SPI SCK          │ GPIO 12     │ Display SPI Clock                                 │
│ EPD SPI MOSI / SDIN  │ GPIO 13     │ Bidirectional Master Data (Cmd 0x1B Read)         │
│ EPD BUSY             │ GPIO 8      │ Display Refresh Busy Line (HIGH = Busy)           │
│ Power Latch (BAT)    │ GPIO 17     │ High-side regulator latch (Assert 1 on boot)      │
│ RTC Interrupt        │ GPIO 5      │ PCF85063A Alarm Wakeup Line (Active LOW)          │
│ BOOT0 Button         │ GPIO 0      │ User Button / Ext1 Deep Sleep Wake                │
└──────────────────────┴─────────────┴───────────────────────────────────────────────────┘
```

---

## 8. RTC Memory Partitioning & Dual-Domain Layout Strategy

The ESP32-S3 contains **16 KB of total RTC SRAM**, split into two distinct 8 KB hardware power domains retained during Deep Sleep:
1. **RTC Slow Memory (8 KB @ `0x5000_0000` to `0x5000_1FFF`):** Accessible by both the Main Xtensa CPU and the ULP RISC-V Coprocessor (executable code space for ULP).
2. **RTC Fast Memory (8 KB @ `0x600F_E000` to `0x600F_FFFF`):** Accessible by the Main Xtensa CPU during wake and fast boot stubs (`RTC_FAST_ATTR`).

```
                              RTC MEMORY PARTITIONING BLUEPRINT
                              ═════════════════════════════════

   RTC SLOW SRAM (8 KB @ 0x5000_0000)               RTC FAST SRAM (8 KB @ 0x600F_E000)
   ┌────────────────────────────────────────┐       ┌────────────────────────────────────────┐
   │ ULP RISC-V Executable Binary & Vector  │       │ 200x200 1-Bit E-Paper Frame Buffer     │
   │ Table (~1.5 KB - 2.0 KB)               │       │ (`RTC_FAST_ATTR s_rtc_frame_buffer`)   │
   │ (CONFIG_ULP_COPROC_RESERVE_MEM = 2048) │       │ (5,000 bytes)                          │
   ├────────────────────────────────────────┤       │ [Moved out of Slow RAM to free 5 KB!]  │
   │ ULP Stack Space (~512 bytes)           │       ├────────────────────────────────────────┤
   ├────────────────────────────────────────┤       │ Fast Boot Wake Stubs / Scratchpad      │
   │ Raw Telemetry Ring Buffer (768 bytes)  │       │ (~512 bytes)                           │
   │ (64 entries * 12 bytes/entry)          │       ├────────────────────────────────────────┤
   ├────────────────────────────────────────┤       │ Available Free Fast RAM (~2.5 KB)      │
   │ Main CPU Static `RTC_DATA_ATTR` State: │       └────────────────────────────────────────┘
   │ - `s_rtc_state` (Lifecycle/Boot: ~64B) │
   │ - Wi-Fi Fast Reconnect Cache (~32B)    │
   ├────────────────────────────────────────┤
   │ Available Free Slow RAM (~4.5 KB)      │
   └────────────────────────────────────────┘
```

### Key Memory Partitioning Decisions:
1. **Relocating E-Paper Frame Buffer to RTC Fast Memory:**
   - In [`bsp_rtc_mem.c`](file:///c:/Users/Matt/Documents/GitHub/esp32-s3_bsp/components/esp32-s3_bsp/src/bsp_rtc_mem.c), the 5,000-byte display frame buffer (`s_rtc_frame_buffer[5000]`) is declared with `RTC_FAST_ATTR` instead of `RTC_DATA_ATTR`.
   - **Benefit:** Instantly frees **5,000 bytes (61% of total capacity)** in RTC Slow Memory!
2. **Reserving ULP Code Space (`CONFIG_ULP_COPROC_RESERVE_MEM = 2048`):**
   - The ESP-IDF linker script reserves the bottom 2,048 bytes of RTC Slow Memory (`0x5000_0000` to `0x5000_07FF`) exclusively for the ULP RISC-V binary and stack.
   - Our lightweight raw-acquisition ULP firmware compiles to only $\sim 800\dots 1200\text{ bytes}$, leaving ample headroom.
3. **Placing Shared Raw Ring Buffer & Static Variables:**
   - The Main CPU's `RTC_DATA_ATTR` variables (`s_rtc_state`, Wi-Fi cache) and the 768-byte ring buffer (`bsp_ulp_shared_mem_t`) sit cleanly in the remaining 6 KB of RTC Slow Memory with over 4.5 KB to spare!

---

## 9. Implementation Code Structures

### 9.1. ULP Raw Telemetry Shared Memory Contract (`bsp_ulp_shared.h`)

```c
#define BSP_ULP_RING_BUFFER_CAPACITY 64
#define BSP_ULP_MAGIC_TOKEN          0x56415052 /* "VAPR" */

/**
 * @brief Raw sample stored by ULP in RTC Slow Memory (Only 12 bytes per sample!)
 */
typedef struct __attribute__((packed)) {
    uint32_t timestamp_sec;      /* Relative uptime or RTC seconds */
    uint16_t raw_shtc3_temp_u16; /* SHTC3 Raw 16-bit Temperature word */
    uint16_t raw_shtc3_rh_u16;   /* SHTC3 Raw 16-bit Relative Humidity word */
    int16_t  raw_tsens_val;      /* TSENS raw ADC register reading */
    uint8_t  flags;              /* Bit 0: Valid CRC */
    uint8_t  reserved;
} bsp_ulp_raw_sample_t;

/**
 * @brief Shared Memory Structure in RTC Slow Memory
 */
typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint32_t head_index;
    uint32_t total_samples;
    uint32_t poll_interval_sec;
    bsp_ulp_raw_sample_t samples[BSP_ULP_RING_BUFFER_CAPACITY];
} bsp_ulp_shared_mem_t;
```

### 8.2. Main Application Processed Telemetry (`bsp_sensors.h`)

```c
typedef struct {
    // 1. Reconstructed Ambient Temperature
    float temperature_c;        ///< Ambient Temperature (°C)
    float temperature_f;        ///< Ambient Temperature (°F)
    float temperature_k;        ///< Ambient Temperature in Kelvin (K)

    // 2. Reconstructed Ambient Relative Humidity
    float humidity_percent;     ///< Ambient Relative Humidity (% RH)

    // 3. Invariant Dew Point
    float dew_point_c;          ///< Dew Point (°C)
    float dew_point_f;          ///< Dew Point (°F)
    float dew_point_k;          ///< Dew Point in Kelvin (K)

    // 4. Invariant Absolute Humidity
    float absolute_humidity_g;  ///< Absolute Humidity (g/m³)

    // 5. Sliding Window Statistical Metrics
    float sliding_rh_min_pct;   ///< Minimum RH over sliding window
    float sliding_rh_max_pct;   ///< Maximum RH over sliding window
    float sliding_trend_rate;   ///< RH rate of change (% RH / hour)

    // 6. Diagnostic & Quality Indicators
    float raw_shtc3_temp_c;     ///< Raw uncompensated PCB SHTC3 Temp (°C)
    float raw_shtc3_rh_pct;     ///< Raw uncompensated PCB SHTC3 RH (%)
    float mcu_die_temp_c;       ///< Cold TSENS Die Temp (°C)
    float epd_glass_temp_c;     ///< SSD1681 On-Glass Temp (°C)
    float thermal_delta_c;      ///< |T_sens - T_amb| (°C)
    uint8_t confidence_pct;     ///< Confidence score (0 - 100 %)
    bool  valid;                ///< True if checksum & hardware read passed
} bsp_shtc3_data_t;
```

---

## 9. Phased Implementation Roadmap for Upcoming Session

| Phase | Core Focus | Files Modified / Created | Acceptance Gate |
|:---:|---|---|---|
| **Phase 1** | **SSD1681 SPI Glass Temp** | `bsp_display.cpp`, `bsp_display.h` | Read Command `0x1B` reliably returns on-glass temperature within $\pm 2.0^\circ\text{C}$ of ambient. |
| **Phase 2** | **Main App Math & Sliding Window Core** | `bsp_sensor_cal.c`, `bsp_sensors.h` | Implements Dynamic Sliding Floor + EWMA + Psychrometric Invariant pipeline. |
| **Phase 3** | **ULP RISC-V Raw Acquisition Core** | `components/esp32-s3_bsp/ulp/` | Main app initializes hardware; ULP executes lightweight raw reads in deep sleep and stores to RTC ring buffer. |
| **Phase 4** | **Thermal Lockout Gatekeeper & UI** | `bsp_sensors.c`, `examples/Unified_BSP_Demo/` | Lockout MUX prevents reading hot silicon after wake; 1.54" E-Paper renders ambient telemetry cleanly. |

---

## 10. Next Session Kickoff Action

When entering the coding session:
1. Open [`components/esp32-s3_bsp/src/bsp_display.cpp`](file:///c:/Users/Matt/Documents/GitHub/esp32-s3_bsp/components/esp32-s3_bsp/src/bsp_display.cpp) to add the SSD1681 `0x1B` half-duplex SPI read function.
2. Implement the Sliding-Window Psychrometric Invariant pipeline and Thermal Lockout MUX.
3. Initialize the `components/esp32-s3_bsp/ulp/` coprocessor raw acquisition firmware.
4. Build and test on hardware!
