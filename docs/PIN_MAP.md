# GPIO Pin Map — ESP32-S3 Touch ePaper 1.54″

Pin multiplexing reference derived from the hardware pinout diagram[cite: 1, 2].

---

## GPIO Multiplexing Table

| GPIO | Signal Name | Target Peripheral | Description |
|---|---|---|---|
| **IO0** | `BOOT0` | System / Bootloader | Boot mode control[cite: 2] |
| **IO1** | `IO1` | Output | General Purpose Output[cite: 2] |
| **IO2** | `IO2` | Output | General Purpose Output[cite: 2] |
| **IO3** | `IO3` | Output | General Purpose Output[cite: 2] |
| **IO4** | `BAT_ADC` | Power Management | Battery ADC monitoring[cite: 2] |
| **IO5** | `RTC_INT` | RTC | Real-Time Clock hardware interrupt[cite: 2] |
| **IO6** | `EPD3V3_EN` | 1.54″ e-Paper | Display 3.3V power enable[cite: 2] |
| **IO7** | `EPD_TP_RST` | Touch Panel | Touch reset[cite: 2] |
| **IO8** | `EPD_BUSY` | 1.54″ e-Paper | Display busy status[cite: 2] |
| **IO9** | `EPD_RST` | 1.54″ e-Paper | Display reset[cite: 2] |
| **IO10** | `EPD_D/C` | 1.54″ e-Paper | Display Data/Command control[cite: 2] |
| **IO11** | `EPD_CS` | 1.54″ e-Paper | Display Chip Select[cite: 2] |
| **IO12** | `EPD_SCLK` | 1.54″ e-Paper | SPI Clock[cite: 2] |
| **IO13** | `EPD_SDI` | 1.54″ e-Paper | SPI MOSI (Data In)[cite: 2] |
| **IO14** | `I2S_MCLK` | Audio Codec | I2S Master Clock[cite: 2] |
| **IO15** | `I2S_SCLK` | Audio Codec | I2S Serial Clock[cite: 2] |
| **IO16** | `I2S_ASDOUT` | Audio Codec | I2S Audio Serial Data Out[cite: 2] |
| **IO17** | `BAT_Control` | Power Management | Battery circuit control[cite: 2] |
| **IO18** | `BAT_KEY` | Power Management | Battery key / switch[cite: 2] |
| **IO19** | `U_N` / `IO19` | UART & USB / Output | USB / UART D- & Output[cite: 2] |
| **IO20** | `U_P` / `IO20` | UART & USB / Output | USB / UART D+ & Output[cite: 2] |
| **IO21** | `EPD_TP_INT` | Touch Panel | Touch interrupt request[cite: 2] |
| **IO38** | `I2S_LRCK` | Audio Codec | I2S Left/Right Clock (WS)[cite: 2] |
| **IO39** | `SD_CLK` | MicroSD Card | SD SPI Clock[cite: 2] |
| **IO40** | `SD_MISO` | MicroSD Card | SD SPI MISO[cite: 2] |
| **IO41** | `SD_MOSI` | MicroSD Card | SD SPI MOSI[cite: 2] |
| **IO42** | `PA_EN` | Power Amp / Audio | Power amplifier enable[cite: 2] |
| **IO43** | `TXD` / `IO43` | UART & USB / Output | UART Transmit & Output[cite: 2] |
| **IO44** | `RXD` / `IO44` | UART & USB / Output | UART Receive & Output[cite: 2] |
| **IO45** | `I2S_DSDIN` | Audio Codec | I2S Audio Serial Data In[cite: 2] |
| **IO46** | `PA_CTRL` | Power Amp / Audio | Power amplifier control[cite: 2] |
| **IO47** | `RTC_SDA` / `SDA` / `IO47` | I2C / Output | Shared I2C Data line & Output[cite: 2] |
| **IO48** | `RTC_SCL` / `SCL` / `IO48` | I2C / Output | Shared I2C Clock line & Output[cite: 2] |

---

## Hardware Bus Architecture

| Bus | GPIOs | Peripherals |
|---|---|---|
| **Shared I2C** | IO48 (SCL), IO47 (SDA) | RTC & System I2C devices[cite: 2] |
| **Primary SPI (e-Paper)** | IO11 (CS), IO12 (SCLK), IO13 (SDI), IO8 (BUSY), IO9 (RST), IO10 (D/C), IO6 (3V3_EN) | 1.54″ e-Paper Display[cite: 2] |
| **Secondary SPI (SD Card)** | IO39 (CLK), IO40 (MISO), IO41 (MOSI) | MicroSD card interface[cite: 2] |
| **I2S Audio** | IO14 (MCLK), IO15 (SCLK), IO16 (ASDOUT), IO38 (LRCK), IO45 (DSDIN), IO42 (PA_EN), IO46 (PA_CTRL) | Audio Codec & Power Amplifier[cite: 2] |
| **USB / UART** | IO19 (U_N), IO20 (U_P), IO43 (TXD), IO44 (RXD) | Serial communication & USB interface[cite: 2] |
| **Touch Interface** | IO7 (TP_RST), IO21 (TP_INT) | Touch panel control[cite: 2] |