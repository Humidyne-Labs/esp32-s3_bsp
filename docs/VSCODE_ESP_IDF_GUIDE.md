# VS Code + ESP-IDF Extension — Setup Guide

This guide walks through setting up, building, flashing, and monitoring the **ESP32-S3 Touch ePaper BSP** using **Visual Studio Code** and the official **Espressif IDF Extension**.

---

## 1. Prerequisites & Installation

### Step 1 — Install Visual Studio Code

Download and install [Visual Studio Code](https://code.visualstudio.com/).

### Step 2 — Install the Espressif IDF Extension

1. Open VS Code.
2. Open the Extensions Marketplace (`Ctrl+Shift+X` on Windows/Linux, `Cmd+Shift+X` on macOS).
3. Search for **Espressif IDF** (Extension ID: `espressif.esp-idf-extension`).
4. Click **Install**.

### Step 3 — Run the Setup Wizard

1. Open the Command Palette (`Ctrl+Shift+P` / `Cmd+Shift+P`).
2. Select: **`ESP-IDF: Configure ESP-IDF Extension`**.
3. Choose **EXPRESS** installation.
4. Select ESP-IDF **v5.1** or **v5.3+** (v6.x is also supported).
5. Click **Install** — this automatically downloads Python environments, CMake, Ninja, and the `xtensa-esp32s3-elf` toolchain.

---

## 2. Opening and Configuring the Project

1. Open the repository in VS Code:
   `File → Open Folder…` → select `esp32-s3_bsp/` (or `examples/Unified_BSP_Demo/` to work directly in the demo app).

2. **Set the Target Device**:
   - Click the **ESP-IDF Target** chip icon in the VS Code status bar (or press `Ctrl+Shift+P` → `ESP-IDF: Set Espressif Device Target`).
   - Select **`ESP32-S3`**.
   - Select **`ESP32-S3 chip via ESP-PROG or USB-Bridge`**.

3. **Set the Serial Port**:
   - Connect your board via USB-C.
   - Click the **Port** icon in the status bar and select the appropriate port (`COM3` on Windows, `/dev/ttyACM0` on Linux/macOS).

---

## 3. Build, Flash & Monitor

The VS Code status bar provides one-click actions for all development tasks:

| Action | Status Bar | Keyboard Shortcut | Command Palette |
|---|---|---|---|
| **Set Target** | `[esp32s3]` | — | `ESP-IDF: Set Espressif Device Target` |
| **Select Port** | `[COMx]` | — | `ESP-IDF: Select Port to Use` |
| **Build** | ⚙️ | `Ctrl+E B` | `ESP-IDF: Build your Project` |
| **Flash** | ⚡ | `Ctrl+E F` | `ESP-IDF: Flash your Project` |
| **Monitor** | 🖥️ | `Ctrl+E M` | `ESP-IDF: Launch IDF Monitor` |
| **Build, Flash & Monitor** | ▶️ | `Ctrl+E D` | `ESP-IDF: Build, Flash and start Monitor` |

---

## 4. Hardware Configuration

The project includes a pre-configured [`sdkconfig.defaults`](../examples/Unified_BSP_Demo/sdkconfig.defaults) tuned for the `ESP32-S3-PICO-1-N8R8`:

| Setting | Value |
|---|---|
| **Target** | ESP32-S3 |
| **Flash** | 8 MB Quad SPI (QIO, 80 MHz) |
| **PSRAM** | 8 MB Octal SPI (80 MHz) |
| **Partition Table** | Custom dual-slot OTA ([`partitions.csv`](../examples/Unified_BSP_Demo/partitions.csv)) |
| **FreeRTOS Tick Rate** | 1000 Hz |

To inspect or modify board parameters interactively:
- Press `Ctrl+Shift+P` → **`ESP-IDF: SDK Configuration Editor`** (menuconfig GUI).

---

## 5. Partition Layout

| Name | Type | Offset | Size | Notes |
|---|---|---|---|---|
| `nvs` | data/nvs | 0x9000 | 24 KB | Persistent key-value storage |
| `otadata` | data/ota | 0xF000 | 8 KB | OTA slot selection metadata |
| `phy_init` | data/phy | 0x11000 | 4 KB | RF PHY calibration data |
| `ota_0` | app/ota_0 | 0x20000 | 3 MB | Primary application slot |
| `ota_1` | app/ota_1 | 0x320000 | 3 MB | Secondary OTA update slot |
| `storage` | data/fat | 0x620000 | 1.875 MB | FatFS internal file storage |

---

## 6. Troubleshooting

| Symptom | Solution |
|---|---|
| **Flash fails / not detected** | Hold **BOOT**, press **RST** once, release **BOOT** to enter download mode manually |
| **Partition errors after update** | Run `ESP-IDF: Erase Flash` from the Command Palette before re-flashing |
| **Stale build after header changes** | Run `ESP-IDF: Full Clean` (`Ctrl+E C`) then rebuild |
| **Monitor shows garbled output** | Confirm baud rate is **115200** in the monitor settings |
