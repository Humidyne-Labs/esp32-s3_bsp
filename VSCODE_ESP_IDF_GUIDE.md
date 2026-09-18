# VS Code + ESP-IDF Extension Setup & Getting Started Guide

This guide details how to set up, configure, build, flash, and monitor **HUMID1-OS** and the **ESP32-S3 ePaper BSP** using **Visual Studio Code** and the official **Espressif IDF Extension**.

---

## 1. Prerequisites & Installation

### Step 1: Install Visual Studio Code
Download and install [Visual Studio Code](https://code.visualstudio.com/).

### Step 2: Install the Espressif IDF Extension
1. Open VS Code.
2. Open the Extensions Marketplace (`Ctrl+Shift+X` on Windows/Linux or `Cmd+Shift+X` on macOS).
3. Search for **Espressif IDF** (Extension ID: `espressif.esp-idf-extension`).
4. Click **Install**.

### Step 3: Run Extension Setup Wizard
1. Open the Command Palette (`Ctrl+Shift+P` / `Cmd+Shift+P`).
2. Type and select: **`ESP-IDF: Configure ESP-IDF Extension`**.
3. Choose **EXPRESS** installation.
4. Select ESP-IDF version **v5.1** or **v5.3+** (or Release v6.x).
5. Click **Install** to automatically download python environments, CMake, Ninja, and the `xtensa-esp32s3-elf` toolchain.

---

## 2. Opening and Configuring the Project

1. In VS Code, open the repository workspace folder:
   - `File -> Open Folder...` -> Select `esp32-s3_bsp` (or `examples/Unified_BSP_Demo`).

2. **Set the Target Device**:
   - Click on the **ESP-IDF Target** icon in the VS Code bottom status bar (or press `Ctrl+Shift+P` and type `ESP-IDF: Set Espressif Device Target`).
   - Select **`ESP32-S3`**.
   - Select **`ESP32-S3 chip via ESP-PROG or USB-Bridge`**.

3. **Set the Serial Port**:
   - Connect your ESP32-S3 ePaper board via USB-C.
   - Click the **Port** icon in the status bar (e.g., `COM3` on Windows or `/dev/ttyACM0` on Linux/macOS).

---

## 3. Building, Flashing, and Monitoring

The VS Code status bar at the bottom provides quick one-click icons for all development actions:

| Action | Status Bar Icon | Keyboard Shortcut | Command Palette (`Ctrl+Shift+P`) |
| --- | --- | --- | --- |
| **Set Target** | `[esp32s3]` | — | `ESP-IDF: Set Espressif Device Target` |
| **Select Port** | `[COMx]` | — | `ESP-IDF: Select Port to Use` |
| **Build Project** | ⚙️ (Gear) | `Ctrl+E B` | `ESP-IDF: Build your Project` |
| **Flash Device** | ⚡ (Lightning) | `Ctrl+E F` | `ESP-IDF: Flash your Project` |
| **Monitor Log** | 🖥️ (Monitor) | `Ctrl+E M` | `ESP-IDF: Launch IDF Monitor` |
| **Build, Flash & Monitor** | ▶️ (Play) | `Ctrl+E D` | `ESP-IDF: Build, Flash and start Monitor` |

---

## 4. Hardware Configuration (`ESP32-S3-PICO-1-N8R8`)

The project contains a pre-configured [`sdkconfig.defaults`](examples/Unified_BSP_Demo/sdkconfig.defaults) file tailored for the Waveshare V2 hardware:

- **Target**: `ESP32-S3`
- **Flash Size**: 8 MB (Quad SPI / QSPI)
- **PSRAM**: 8 MB (Octal SPI / OPI)
- **Partition Table**: Custom dual-slot OTA [`partitions.csv`](examples/Unified_BSP_Demo/partitions.csv)

To visually inspect or edit board parameters:
1. Press `Ctrl+Shift+P` and select **`ESP-IDF: SDK Configuration Editor`** (or click the Gear/Slider icon in the status bar).
2. Browse settings like Wi-Fi, NVS, and BSP options in the interactive GUI editor.

---

## 5. Troubleshooting & Tips

- **Board in Download Mode**: If flashing fails, press and hold the **BOOT** button, press the **RESET/PWR** button once, and release **BOOT** to force bootloader download mode.
- **Erasing Flash**: Run `ESP-IDF: Erase Flash` from the Command Palette if switching partition structures.
- **Clean Build**: If header files are changed, run `ESP-IDF: Full Clean` (`Ctrl+E C`).
