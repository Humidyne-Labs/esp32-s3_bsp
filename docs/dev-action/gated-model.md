# Temperature and Time ULP (Lockout/Gating) Model

To execute the dual-trigger cooldown gate, use an integer-based fixed-point comparator in RTC_SLOW_MEM. The ULP-RISC-V evaluates:
Because the ULP-RISC-V core does not have a hardware Floating Point Unit (FPU), store temperatures in centi-degrees Celsius (1^\circ\text{C} = 100\text{ units}) to prevent precision loss without software floating-point emulation overhead.
1. Shared Memory Structure (shared_vars.h)
```
#pragma once
#include <stdint.h>
#include <stdbool.h>

#define SENSOR_HISTORY_MAX 16

typedef struct {
    int16_t  temp_centi_c;    // e.g., 2330 = 23.30 °C
    uint16_t rh_centi_pct;    // e.g., 6100 = 61.00 % RH
    uint32_t sample_timestamp;
} env_reading_t;

typedef struct {
    // --- Gating Parameters (Configured by Xtensa before sleep) ---
    int16_t  z_initial_temp_centi_c; // Hot die/package baseline at sleep entry
    int16_t  y_delta_drop_centi_c;   // Required drop (e.g., 200 = 2.00 °C)
    uint32_t x_timeout_ticks;        // Safety timeout (e.g., 180s / 60s = 3 ticks)

    // --- State Variables (Managed by ULP) ---
    uint32_t elapsed_ticks;          // 60-second ticks elapsed since sleep entry
    bool     cooldown_complete;      // Latched true once either condition triggers
    int16_t  last_probed_temp_centi_c;

    // --- Output Buffers ---
    uint32_t sample_count;
    uint32_t head_idx;
    env_reading_t latest_sample;
    env_reading_t history[SENSOR_HISTORY_MAX];
} ulp_shared_data_t;
```
2. ULP-RISC-V Firmware (ulp/main.c)
The ULP wakes every 60 seconds via the internal RTC Slow Clock timer. It checks the gating state, probes the current temperature, evaluates the delta or timeout, and executes the full high-accuracy sample once the criteria are satisfied.
```
#include <stdint.h>
#include <stdbool.h>
#include "ulp_riscv.h"
#include "ulp_riscv_utils.h"
#include "shared_vars.h"

ulp_shared_data_t shared_data;

// External I2C driver functions implemented for the RTC GPIO domain
extern int16_t ulp_read_probe_temp_centi_c(void);
extern bool ulp_read_full_env_sensor(int16_t *temp, uint16_t *rh);

int main(void) {
    // Step 1: Evaluate Thermal Cooldown Gate
    if (!shared_data.cooldown_complete) {
        shared_data.elapsed_ticks++;

        // Probe current temperature (takes < 1 ms)
        int16_t current_temp = ulp_read_probe_temp_centi_c();
        shared_data.last_probed_temp_centi_c = current_temp;

        // Calculate temperature drop from initial hot baseline
        int16_t temp_drop = shared_data.z_initial_temp_centi_c - current_temp;

        bool delta_condition_met = (temp_drop >= shared_data.y_delta_drop_centi_c);
        bool timeout_condition_met = (shared_data.elapsed_ticks >= shared_data.x_timeout_ticks);

        if (delta_condition_met || timeout_condition_met) {
            // Latch gate open for this and future 60-second samples
            shared_data.cooldown_complete = true;
        } else {
            // Board still dissipating heat; halt immediately (total active time < 2 ms)
            return 0;
        }
    }

    // Step 2: Ground plane is cooled (or timed out). Perform pristine T/RH read.
    int16_t temp = 0;
    uint16_t rh = 0;

    if (ulp_read_full_env_sensor(&temp, &rh)) {
        shared_data.latest_sample.temp_centi_c = temp;
        shared_data.latest_sample.rh_centi_pct = rh;
        shared_data.latest_sample.sample_timestamp = shared_data.elapsed_ticks;

        // Store into circular buffer
        uint32_t idx = shared_data.head_idx;
        shared_data.history[idx] = shared_data.latest_sample;
        shared_data.head_idx = (idx + 1) % SENSOR_HISTORY_MAX;
        shared_data.sample_count++;
    }

    return 0;
}
```
3. Xtensa Core Pre-Sleep Initialization (main/app_main.c)
Before entering deep sleep, the Xtensa core samples its internal die temperature sensor (or the sensor package baseline), computes z, writes the threshold parameters into RTC_SLOW_MEM, and resets the gate.
```
#include <stdio.h>
#include "esp_sleep.h"
#include "driver/temperature_sensor.h"
#include "ulp_riscv.h"
#include "ulp_main.h"
#include "shared_vars.h"

#define PIN_EXT_RTC_INT        GPIO_NUM_4
#define PIN_USER_BUTTON        GPIO_NUM_0

#define ULP_TICK_PERIOD_US     (60ULL * 1000 * 1000) // 60 seconds
#define TIMEOUT_FALLBACK_SEC   (300)                 // x = 300 seconds (5 min)
#define DELTA_DROP_TARGET_C    (1.75f)               // y = 1.75 °C drop
#define USER_SLEEP_PERIOD_MIN  (15)

static temperature_sensor_handle_t s_temp_sensor = NULL;

static void init_die_temp_sensor(void) {
    temperature_sensor_config_t temp_cfg = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 60);
    ESP_ERROR_CHECK(temperature_sensor_install(&temp_cfg, &s_temp_sensor));
    ESP_ERROR_CHECK(temperature_sensor_enable(s_temp_sensor));
}

static int16_t read_current_die_temp_centi_c(void) {
    float temp_c = 0.0f;
    ESP_ERROR_CHECK(temperature_sensor_get_celsius(s_temp_sensor, &temp_c));
    return (int16_t)(temp_c * 100.0f);
}

void app_main(void) {
    init_die_temp_sensor();
    esp_sleep_wakeup_cause_t wake_cause = esp_sleep_get_wakeup_cause();

    if (wake_cause == ESP_SLEEP_WAKEUP_UNDEFINED) {
        // Cold boot initialization
        ESP_ERROR_CHECK(ulp_riscv_load_binary(ulp_main_bin_start, 
                                              (ulp_main_bin_end - ulp_main_bin_start)));
        ulp_set_wakeup_period(0, ULP_TICK_PERIOD_US);
        ESP_ERROR_CHECK(ulp_riscv_run());
    } else {
        // Handle Ext1 Wakeup (RTC Alarm or User Button)
        if (ulp_shared_data.sample_count > 0) {
            printf("Pristine Environmental Data: T=%.2f C, RH=%.2f %%\n",
                   ulp_shared_data.latest_sample.temp_centi_c / 100.0f,
                   ulp_shared_data.latest_sample.rh_centi_pct / 100.0f);
        }

        // Sub/Pub Network Routine (Wi-Fi PA generates thermal heat here)
        // ...
    }

    // --- Configure Thermal Cooldown Gate Before Sleeping ---
    int16_t current_die_temp = read_current_die_temp_centi_c();

    // z: Initial hot baseline
    ulp_shared_data.z_initial_temp_centi_c = current_die_temp; 
    // y: Target temperature reduction
    ulp_shared_data.y_delta_drop_centi_c   = (int16_t)(DELTA_DROP_TARGET_C * 100.0f);
    // x: Safety timeout in 60-second tick units
    ulp_shared_data.x_timeout_ticks        = (TIMEOUT_FALLBACK_SEC / 60); 

    // Reset gate states
    ulp_shared_data.elapsed_ticks     = 0;
    ulp_shared_data.cooldown_complete = false;

    // Arm wakeups
    const uint64_t ext1_mask = (1ULL << PIN_EXT_RTC_INT) | (1ULL << PIN_USER_BUTTON);
    ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup(ext1_mask, ESP_EXT1_WAKEUP_ANY_LOW));
    ESP_ERROR_CHECK(esp_sleep_enable_ulp_wakeup());

    // Enter Deep Sleep
    esp_deep_sleep_start();
}
```
4. Implementation Options for ulp_read_probe_temp_centi_c()
| Method | Implementation | Pros / Cons |
|---|---|---|
| SHTC3 Fast Low-Power Command (Recommended) | The ULP sends command 0x6458 (Low Power, Temp First) over RTC I2C. | Direct measurement. Probes the temperature directly at the sensor pad where RH errors occur. Uses < 0.2 µA average; eliminates die-to-board thermal modeling discrepancies. |
| ESP32-S3 Internal TSENS Registers | The ULP reads the APB_SARADC_APB_TSENS_CTRL_REG directly. | Requires keeping the SAR ADC / TOP peripheral domain clocked during ULP execution, which increases sleep leakage current (~1–2 mA). |
Using the SHTC3 low-power read for the probe:
```
int16_t ulp_read_probe_temp_centi_c(void) {
    // Send SHTC3 Low-Power Temp First command (0x6458)
    // Read raw 16-bit word, calculate: T = -45 + 175 * (raw / 65536)
    // Fixed point: ((17500 * raw) >> 16) - 4500
    uint16_t raw_t = 0;
    if (!ulp_i2c_read_raw_temp_reg(&raw_t)) {
        return 0x7FFF; // Error sentinel
    }
    return (int16_t)(((17500LL * raw_t) >> 16) - 4500);
}
```
### Verification Checklist, Requirement, ***Optional***
 * Fixed-Point Conversion Test: Verify that DELTA_DROP_TARGET_C translates to the expected raw centi-degree offset (e.g., 1.75^\circ\text{C} = 175 integer units).
 * Timeout Boundary Test: Force y_delta_drop_centi_c = 1000 (an unattainable 10°C drop) and verify via debugger or serial log that the ULP unlocks sampling exactly at t = x (elapsed_ticks >= x_timeout_ticks).
 * Delta Trigger Validation: Warm the board via a 5-second Wi-Fi transmission, sleep, and record last_probed_temp_centi_c every 60 seconds to determine the board's exact cooling curve time constant (\tau) under still-air enclosure conditions.

> source: Gemini

---

## Sudo Execution Flow

- During wake init, after general IO comes up and all latch/holds have been cleared, before screen init and persistent bw/red ram/frames are pushed, read the EPD temperature sensor (init minimum required operating params, prior to thermistor reading, IE init EPD SPI clock, enable EPD internal temp sensor). This is to ensure the environmental reading is representative of ambient temp. This reading should take place ***before*** any auxiliary screen operation to ensure the ssd1681 remains as close to, if not exactly, ambient temperature.

- ULP will cycle at a period of 60 human seconds, regardless of xtensias deep sleep period. If the temp delta is less than x-degrees ***OR*** xtensia idle/sleep time is greater then y-seconds, read shtc3 ~~and esp32-s3 die temperature~~. This allows for more efficient execution of the low power read function on the ULP in the event runtime operations ***do not*** heat the PCB substrate to its typical thermal offset (example scenario, xtensia woke then became idle or immediately went into deep sleep mode)

- ***IF*** the ULP did not execute an environmental reading prior to the xtensia wake (IE User pressed a button for premature wake) the temperature and relitive humidity reading will fall back to the value prior to the wake event, IE last known value. This ensures a forced reading Isent compromised by an uncontrolled thermal offset via ***heat-soak*** from run time operations.

- ULP gate temperature will ***NOT*** be a fixed temperature as I initially thought, but rather a delta, calculated from two consecutive ~~die~~ temperature readings over z-time. The gate opens ***IF*** the delta between the two readings is 0.05C-0.20C. This indicates that cooling has come to a plateau, the board has reach a thermal steady state or stable thermal equilibrium close to ambient (PCB plane will cool to ambient is given enough time, ***rate*** is unknown at time of writing). Our primary entry point for this "rate of change" measurement will be the shtc3, due to its high resolution. The ESP32-S3 die temperature is no longer the ideal 'probe' for board temperature. However, since the ESP32-S3 die is basically directly coupled to the ground pore, with a high degree of thermal conductiveity, we ***should*** somehow use its coarse temperature measurement for verification.

- The ULP run loop, will be initialized during BSP init (cold boot), and remain running for the life of the application, untill a hardware reset or watchdog timeout occurs. This ensures gating, environmental readings and verification will happen continuously in the background, regardless of the applications state, idle, wake, sleep. Gating will ensure accurate environment readings, reference the previous bullet point for details.

- The application sleep period will be configured at runtime, for a dynamic telemetry period, note, the period will directly affect the lifespan of the device, due to its small battery, 400mah. A longer sleep period will give the device a longer lifespan. minimum sleep time (realized from above bullet points), will be 3-5 minutes. This gives the board enough time for passive heat dissipation, and allow for at least one (assumed based on gate conditions) valid environmental reading.

- Given the minimum wake period is set to 3-5min, the samples sliding window time period will be a product of the samples buffer capacity and set wake period. (1 sample every 3 minutes multiplied by 10 samples, gives one a window of approximately a 1/2 hour)

- We ***should*** track the valid sample count over time, as just placing valid samples in a ring buffer will not give us a valid metric for sample rate calculations. Example, 1 sample every 3 min (provided gateing conditions are met), with a buffer size of 10, should give us a sample rate of, 20 samples / hour. This resolution is more than acceptable, given the application and the  environments approximate rate of change. The applications environment being a close system, humidification source is regulated at 70%RH, temperature change is in phase with standard indoor fluctuations.

- Other potential application environments may consist of a closed loop, moving air system, humidification source fixed and regulated at 70%RH. Approx sample rate, still assumed to be valid under moving air conditions, as cooling is based of the thermal capacity of standard atmospheric ***air***, not helium or any other inert gas, or mixture there of. Main method of temperature regulation would be that of cool air convection, producing temperature less than ambient, by a magnitude of 10-20F, approximate.

> Future research focusing on the thermal conductivity of a closed loop, moving air system, filled with helium. May infact be beneficial, in the assessment of a long term storage solution for sensitive organic assets. ***However, this is well beyond the scope of this project, the availability of resources is also extremely limited, at this time. Please note, there are certain risks associated with working on such a device inside a closed room. A gas leak ***CAN*** void the room of oxygen causing, undesirable outcomes, such as asphyxiation. It is ***NOT*** recommended to work on such a system indoors, without proper ventilation, and or, safety devices such as an oxygen monitor, independent respiratory apperatus, or local gravity detector/altimeter. In the event the room becomes airborne please contact the FAA, the likelihood of "deadly" impact depends upon altitude. Ideally the non enclosed room should be comprised of removable depleted uranium ore wall panels to prevent unforseen aeronautical disasters. The addition of solid furniture in contrast to hollow fixtures will also prevent bodily damage should unanchored tables/chairs become mobile due to a gaseous leak or sudden zero gravity event.***

- I'm pretty sure the above bullet points correctly discribe the operating conditions and execution flow with a fair degree of accuracy. If you have questions regarding the operating spec or flow, please draft a detailed concerns_and_questions.md in markdown for review, outline anything that's unclear, or that requires further clarification. Please ***do*** create an internal markdown to document your progress throughout the BSP modification/revision during this session.

---

### Code Expansion

Almost forgot, so while working on this new architecture, we should try to cleanly implement everything. I think the additional maths involved should be placed in a file file pair instead of adding it to the shtc3 bsp layer.

All processing, and low level approximation should be in a custom bsp_equ_math.c file. Infact maybe abstracting all mathematical operation into a single file would be a good idea, that way we can see the logic as opposed to math/logic. We can migrate existing math per case, the main priority should be the demo and bsp.

### Tool Chain Build Environment 

If your able, please doc the build chain setup as a front facing public doc for the repo. Please ***try*** to use C as the ULP language. I don't know if we can program it in C or if it's strictly ASM. I need to explore the documentation and project examples in more detail to confirm the language and syntax.

> (see below for information scraped by Gemini)

Yes, you can write native C for the ESP32-S3's ULP. Unlike the legacy ESP32 FSM coprocessor, the ESP32-S3 features an actual 32-bit RV32IMC RISC-V core (base integer + hardware multiply/divide + compressed instructions), allowing complete implementation in standard C without assembly macros.
Toolchain & Development Environment
The development environment requires no secondary IDE or external configurations beyond the standard ESP-IDF toolchain.

 * Compiler: riscv32-esp-elf-gcc cross-compiler. ESP-IDF automatically fetches and installs this toolchain alongside the main xtensa-esp32s3-elf-gcc compiler whenever running install.sh or install.ps1.

 * Standard Libraries: Runs a stripped freestanding C environment (no standard dynamic memory allocation/malloc, no POSIX threads). Espressif provides an embedded RTC runtime containing optimized delay, GPIO, I2C, and interrupt primitives in ulp_riscv.h, ulp_riscv_gpio.h, and ulp_riscv_i2c.h.

 * IDE / Build Tools: Standard VS Code (with ESP-IDF extension), CLion, or terminal using idf.py build.
Project Structure & CMake Integration
The build chain functions as a nested sub-project compilation. When building the host firmware, ESP-IDF isolates the ULP source tree, invokes the RISC-V compiler to emit a standalone ELF and raw binary, and links the resulting binary symbols directly into the Xtensa application.

### Directory Layout
```
my_project/
├── CMakeLists.txt
├── sdkconfig
└── main/
    ├── CMakeLists.txt
    ├── main.c              <-- Host Xtensa CPU code (ESP32-S3)
    └── ulp/
        └── main.c          <-- ULP RISC-V C source code
```
main/CMakeLists.txt
In ESP-IDF v5.x, link the ULP application using the ulp_embed_binary CMake function:
```
idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS ".")

# Define ULP sub-application
set(ulp_app_name "ulp_main")
set(ulp_sources "ulp/main.c")
set(ulp_exp_dep_srcs "main.c")

ulp_embed_binary(${ulp_app_name} "${ulp_sources}" "${ulp_exp_dep_srcs}")
```
Required sdkconfig Flags
Enable the RISC-V ULP coprocessor in Kconfig (idf.py menuconfig -> Component config -> ESP-IDF ULP configuration):
```
CONFIG_ULP_COPROC_ENABLED=y
CONFIG_ULP_COPROC_TYPE_RISCV=y
CONFIG_ULP_COPROC_RESERVE_MEM=4096   # Default is 4096 bytes (Max 8192 bytes)
```
### How the Build Chain Executes Under the Hood
When executing idf.py build:
```
[ulp/main.c]
     │
     ▼  riscv32-esp-elf-gcc (-march=rv32imc -mabi=ilp32)
[ulp_main.elf]
     │
     ▼  riscv32-esp-elf-objcopy
[ulp_main.bin] ───► Embedded into Xtensa flash image as raw binary blob
     │
     ▼  esp32ulp_mapgen.py
[esp_ulp_main.h] & [ulp_main.ld]
     │
     ▼
Exposes shared global variables to Xtensa compiler with "ulp_" prefix
```
 * Sub-Project Compilation: CMake triggers a separate build pipeline using riscv32-esp-elf-gcc with flags -march=rv32imc -mabi=ilp32.

 * Binary Extraction: objcopy extracts raw instructions and data from the generated .elf into ulp_main.bin.

 * Symbol Table Generation: The esp32ulp_mapgen.py script parses the ULP ELF symbol table and creates an interface header (esp_ulp_main.h) and linker script (ulp_main.ld). Any global variable declared in the ULP C code is automatically exported with the prefix ulp_.

 * Binary Embedding: The binary blob is linked into the main application. At runtime, the Xtensa CPU copies this blob into RTC SLOW Memory before booting the ULP.
Code Architecture: ULP vs. Main CPU

#### 1. ULP Implementation (main/ulp/main.c)
Global variables declared in file scope reside directly in RTC SLOW memory and are accessible by both cores.
```
#include <stdint.h>
#include "ulp_riscv.h"
#include "ulp_riscv_utils.h"
#include "ulp_riscv_gpio.h"

/* Exported to Xtensa CPU via esp_ulp_main.h as 'ulp_sample_counter' */
volatile uint32_t sample_counter = 0;
volatile uint32_t wake_threshold = 50;

int main(void) 
{
    sample_counter++;

    // Check condition to wake the main CPU
    if (sample_counter >= wake_threshold) {
        sample_counter = 0;
        ulp_riscv_wakeup_main_processor();
    }

    // Must return or abort to enter low-power sleep until next timer trigger
    return 0;
}
```
#### 2. Host CPU Loading & Booting (main/main.c)
```
#include <stdio.h>
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "ulp_riscv.h"
#include "ulp_main.h"  // Auto-generated header providing ULP symbols

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[]   asm("_binary_ulp_main_bin_end");

void app_main(void)
{
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

    if (cause != ESP_SLEEP_WAKEUP_ULP) {
        printf("First boot. Initializing ULP...\n");

        // 1. Load ULP binary into RTC Slow Memory
        ESP_ERROR_CHECK(ulp_riscv_load_binary(
            ulp_main_bin_start, 
            (ulp_main_bin_end - ulp_main_bin_start)
        ));

        // 2. Set ULP wakeup period (e.g., run every 5 seconds)
        ulp_set_wakeup_period(0, 5000000);

        // 3. Start the ULP RISC-V timer/engine
        ESP_ERROR_CHECK(ulp_riscv_run());
    } else {
        // Read directly from the shared RTC memory variable
        printf("Woken by ULP! Counter reached: %lu\n", ulp_sample_counter);
    }

    // Arm ULP wake source and go to deep sleep
    ESP_ERROR_CHECK(esp_sleep_enable_ulp_wakeup());
    esp_deep_sleep_start();
}
```
### Hardware & Memory Constraints

 * Memory Budget: The code, global data, and call stack share a maximum pool of 8 KB (RTC SLOW memory). Running large data structures or deep recursion will collide with the stack.

 * Peripherals: The ULP can directly access RTC GPIOs, RTC I2C (software bit-banged or hardware RTC I2C peripheral), and internal temperature sensors. It cannot access main system peripherals (Standard SPI, standard I2C, UART0/1, Wi-Fi/BT hardware).

 * Clock Speeds: Runs on the internal fast RC oscillator (~17.5 MHz) or XTAL32K / RTC_SLOW_CLK (~136 kHz to ~150 kHz), resulting in much slower instruction execution compared to the 240 MHz Xtensa cores.
Actionable Setup & Verification Checklist

 * Verify Toolchain Detection: Run riscv32-esp-elf-gcc --version in your terminal. If missing, re-run install.sh / install.ps1 from your IDF_PATH.

 * Apply Configuration: Open sdkconfig and verify CONFIG_ULP_COPROC_TYPE_RISCV=y is set.

 * Check Section Sizing: Build the project with idf.py build and inspect the console output:

   * Confirm ulp_main.bin size is well below CONFIG_ULP_COPROC_RESERVE_MEM (typically under 1.5 KB for basic tasks).

 * Validate Header Output: Confirm build/esp-idf/main/ulp_main/esp_ulp_main.h is generated containing the ulp_ prefixed references for your variables.


### end of page



