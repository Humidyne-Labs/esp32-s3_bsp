# GPIO Pin Map & Electrical Characteristics — ESP32-S3 Touch ePaper 1.54″ V2

Hardware target: **Waveshare ESP32-S3-Touch-ePaper-1.54 V2** (ESP32-S3-PICO-1-N8R8)

---

## 1. GPIO Pin Multiplexing & Electrical Specifications

| GPIO | Signal Name | Target Peripheral | Direction / Mode | Electrical Characteristic & Polarity | Default State / Initialization | Notes |
|---|---|---|---|---|---|---|
| **IO0** | `BOOT0` | Tactile Button / Bootloader | Digital Input | **Active LOW** (0 = Pressed, 1 = Idle) | `INPUT_PULLUP` enabled | Internal pull-up required (no external pull resistor) |
| **IO1** | `IO1` | Expansion Header | GPIO / Output | Push-Pull | Tristate / User | Available GPIO header |
| **IO2** | `IO2` | Expansion Header | GPIO / Output | Push-Pull | Tristate / User | Available GPIO header |
| **IO3** | `LED_GREEN` | User Status Indicator | Digital Output | **Open-Drain, Active LOW** (0 = ON, 1 = OFF) | `1` (OFF) | Cathode connected to IO3; drive 0 to illuminate |
| **IO4** | `BAT_ADC` | Power Management | Analog Input | ADC1 Channel 3 | Linear 0-3.1V sensing | Measured via 1:2 divider (100kΩ / 100kΩ; VBAT = VADC * 2.0) |
| **IO5** | `RTC_INT` | PCF85063A RTC | Digital Input | **Active LOW**, Open-Drain | `INPUT_PULLUP` enabled | PCF85063A INT is open-drain; PCB has NO pull-up $\rightarrow$ internal pull-up mandatory |
| **IO6** | `EPD3V3_EN` | 1.54″ e-Paper Display | Digital Output | **Active LOW** (0 = ON, 1 = OFF) | `0` (Power ON) | Gate control for EPD 3.3V power rail; drive 1 + `gpio_hold_en` in sleep |
| **IO7** | `EPD_TP_RST` | FT6336 Touch Panel | Digital Output | **Active LOW** (0 = Reset, 1 = Run) | `1` (Not in reset) | Toggled low for 10ms during hardware reset sequence |
| **IO8** | `EPD_BUSY` | 1.54″ e-Paper Display | Digital Input | **Active HIGH** (1 = Busy, 0 = Ready) | Floating Input | Monitored during SSD1681 LUT execution and refreshes |
| **IO9** | `EPD_RST` | 1.54″ e-Paper Display | Digital Output | **Active LOW** (0 = Reset, 1 = Run) | `1` (Not in reset) | SSD1681 display controller hardware reset |
| **IO10** | `EPD_D/C` | 1.54″ e-Paper Display | Digital Output | Push-Pull (0 = Command, 1 = Data) | `1` (Data mode) | Controls byte interpretation during SPI transactions |
| **IO11** | `EPD_CS` | 1.54″ e-Paper Display | Digital Output | **Active LOW** (0 = Selected, 1 = Idle) | `1` (Deselected) | Display SPI Chip Select |
| **IO12** | `EPD_SCLK` | 1.54″ e-Paper Display | SPI2 SCK Output | SPI Master Clock (up to 20MHz) | Driven by SPI driver | Serial clock line for display panel |
| **IO13** | `EPD_SDI` | 1.54″ e-Paper Display | SPI2 MOSI Output | SPI Master Out Slave In | Driven by SPI driver | Serial data stream to SSD1681 display RAM |
| **IO14** | `I2S_MCLK` | Audio Codec (ES8311) | I2S0 MCLK Output | Master Clock (256 * Fs) | Driven by I2S driver | Master clock for codec synchronization |
| **IO15** | `I2S_SCLK` | Audio Codec (ES8311) | I2S0 BCLK Output | Bit / Serial Clock | Driven by I2S driver | Continuous clocking for mono/stereo audio frames |
| **IO16** | `I2S_ASDOUT` | Audio Codec (ES8311) | I2S0 DIN Input | Serial Audio Data In from Codec | Driven by I2S driver | Codec microphone / ADC data line |
| **IO17** | `BAT_Control` | Power Management | Digital Output | **Active HIGH** (1 = Hold ON, 0 = OFF) | `1` (Power Latched) | Must be driven HIGH at boot and frozen with `gpio_deep_sleep_hold_en()` |
| **IO18** | `BAT_KEY` | Power Management | Digital Input | **Active LOW** (0 = Pressed, 1 = Idle) | `INPUT_PULLUP` enabled | Hardware battery power key / wakeup trigger |
| **IO19** | `U_N` | USB-JTAG-Serial / D- | USB Native / Output | Differential D- | USB PHY | Native USB negative line |
| **IO20** | `U_P` | USB-JTAG-Serial / D+ | USB Native / Output | Differential D+ | USB PHY | Native USB positive line |
| **IO21** | `EPD_TP_INT` | FT6336 Touch Panel | Digital Input | **Active LOW** (0 = Touch detected) | `INPUT_PULLUP` enabled | Falling edge interrupt when contact points are registered |
| **IO38** | `I2S_LRCK` | Audio Codec (ES8311) | I2S0 WS Output | Left / Right Word Select (16kHz) | Driven by I2S driver | Synchronizes audio left-slot mono packets |
| **IO39** | `SD_CLK` | MicroSD Card Slot | SDMMC / SPI Clock | Clock line (400kHz init, up to 20MHz) | Driven by SD driver | MicroSD clock |
| **IO40** | `SD_MISO` | MicroSD Card Slot | SDMMC D0 / SPI MISO | Master In Slave Out (Data 0) | `INPUT_PULLUP` enabled | MicroSD read line |
| **IO41** | `SD_MOSI` | MicroSD Card Slot | SDMMC CMD / SPI MOSI | Master Out Slave In (Command) | `INPUT_PULLUP` enabled | MicroSD command line |
| **IO42** | `PA_EN` | Audio Power Amp (NS4168)| Digital Output | **Active LOW** (0 = Power ON, 1 = OFF) | `1` (PA Power Cut) | High-side power switch for Class-D amplifier rail |
| **IO43** | `TXD` | UART0 TX | UART Output | Serial Debug Transmit | 115200 8N1 default | Console log output |
| **IO44** | `RXD` | UART0 RX | UART Input | Serial Debug Receive | 115200 8N1 default | Console log input / flashing |
| **IO45** | `I2S_DSDIN` | Audio Codec (ES8311) | I2S0 DOUT Output | Serial Audio Data Out to Codec | Driven by I2S driver | Mono audio playback stream |
| **IO46** | `PA_CTRL` | Audio Power Amp (NS4168)| Digital Output | **Active HIGH** (1 = Enabled, 0 = Standby)| `0` (Muted) | Controlled by ES8311 codec driver PA pin interface |
| **IO47** | `RTC_SDA` | Shared I2C0 Bus | I2C Open-Drain | **Fast Mode (400 kHz)** | External 4.7kΩ pull-up | Shared by SHTC3 (0x70), PCF85063 (0x51), FT6336 (0x38), ES8311 (0x18) |
| **IO48** | `RTC_SCL` | Shared I2C0 Bus | I2C Open-Drain | **Fast Mode (400 kHz)** | External 4.7kΩ pull-up | Shared clock line; all devices require standard I2C ACK |

---

## 2. Hardware Subsystem Bus Architecture

```mermaid
graph TD
    subgraph ESP32-S3 ["ESP32-S3-PICO-1-N8R8 (Dual LX7 240MHz)"]
        CORE0["Core 0: Networking / OS / BLE / MQTT / Time"]
        CORE1["Core 1: Display / LVGL v9 / Touch / Assets"]
    end

    subgraph I2C_BUS ["Shared I2C0 Bus (IO47 SDA, IO48 SCL @ 400kHz)"]
        SHTC3["SHTC3 Temp/Humidity (0x70)"]
        PCF85063["PCF85063A RTC (0x51, INT=IO5)"]
        FT6336["FT6336 Touch (0x38, RST=IO7, INT=IO21)"]
        ES8311_CTRL["ES8311 Codec Control (0x18)"]
    end

    subgraph DISPLAY_BUS ["SPI2 Master Bus"]
        SSD1681["SSD1681 1.54\" Mono EPD (CS=IO11, DC=IO10, RST=IO9, 3V3_EN=IO6, BUSY=IO8)"]
    end

    subgraph AUDIO_BUS ["I2S0 Audio Interface"]
        I2S_STREAM["ES8311 DAC (MCLK=IO14, SCLK=IO15, LRCK=IO38, DOUT=IO45)"]
        NS4168_AMP["NS4168 Class-D Amp (PA_EN=IO42 Active Low, PA_CTRL=IO46 Active High)"]
    end

    subgraph POWER_MGT ["Power Management"]
        BAT_LATCH["BAT_Control (IO17 Active High)"]
        BAT_ADC["BAT_ADC (IO4 ADC1_CH3 Divider)"]
        STATUS_LED["Green LED (IO3 Open-Drain Active Low)"]
    end

    ESP32-S3 --> I2C_BUS
    ESP32-S3 --> DISPLAY_BUS
    ESP32-S3 --> AUDIO_BUS
    ESP32-S3 --> POWER_MGT
```