# GPIO Pin Map — ESP32-S3 Touch ePaper 1.54″

Pin multiplexing reference for the [Waveshare ESP32-S3-ePaper-1.54 V2](https://www.waveshare.com/esp32-s3-epaper-1.54.htm?sku=32298) board, derived from the manufacturer's public schematic.

---

## GPIO Multiplexing Table

| GPIO | Signal Name | Target Peripheral | Description |
|---|---|---|---|
| **GPIO0** | `BOOT_KEY` | System / Bootloader | User BOOT switch |
| **GPIO1** | `BAT_ADC` | Power Management | Battery voltage monitoring via ADC1_CH0 |
| **GPIO2** | `PWR_KEY` | Power Management | Power hold / power switch latch |
| **GPIO3** | `RTC_INT` | PCF8563 RTC | Real-Time Clock hardware interrupt |
| **GPIO4** | `TOUCH_RST` | Capacitive Touch (FT6336) | Touch controller hardware reset |
| **GPIO5** | `TOUCH_INT` | Capacitive Touch (FT6336) | Touch panel interrupt request |
| **GPIO6** | `I2C_SCL` | Shared I2C Bus | Clock line — ES8311, SHTC3, PCF8563, FT6336 |
| **GPIO7** | `I2C_SDA` | Shared I2C Bus | Data line — ES8311, SHTC3, PCF8563, FT6336 |
| **GPIO8** | `EPD_DC` | 1.54″ e-Paper | Data / Command control |
| **GPIO9** | `EPD_RST` | 1.54″ e-Paper | Display hardware reset |
| **GPIO10** | `EPD_CS` | 1.54″ e-Paper | SPI Chip Select |
| **GPIO11** | `EPD_SCLK` | 1.54″ e-Paper | SPI Clock |
| **GPIO12** | `EPD_MOSI` | 1.54″ e-Paper | SPI MOSI (data in) |
| **GPIO13** | `EPD_BUSY` | 1.54″ e-Paper | Busy status line |
| **GPIO14** | `I2S_MCLK` | ES8311 Audio Codec | I2S Master Clock |
| **GPIO15** | `I2S_SCLK` | ES8311 Audio Codec | I2S Bit Clock (BCLK) |
| **GPIO16** | `I2S_LRCK` | ES8311 Audio Codec | I2S Frame Clock / Word Select (WS) |
| **GPIO17** | `I2S_ASOUT` | ES8311 Audio Codec | Serial audio data — Codec DOUT → ESP32 DIN |
| **GPIO18** | `I2S_DSIN` | ES8311 Audio Codec | Serial audio data — ESP32 DOUT → Codec DIN |
| **GPIO38** | `USER_LED` | Status Indicator | Onboard status LED |
| **GPIO39** | `SD_CLK` | MicroSD Card | SPI Clock |
| **GPIO40** | `SD_MISO` | MicroSD Card | SPI MISO (data out) |
| **GPIO41** | `SD_MOSI` | MicroSD Card | SPI MOSI (data in) |
| **GPIO42** | `SD_CS` | MicroSD Card | SPI Chip Select |
| **GPIO47** | `PA_CTRL` | NS4168 Power Amp | Amplifier gain / mode control |
| **GPIO48** | `PA_EN` | NS4168 Power Amp | Power amplifier enable / shutdown |

---

## Hardware Bus Architecture

| Bus | GPIOs | Peripherals |
|---|---|---|
| **Shared I2C** | GPIO6 (SCL), GPIO7 (SDA) | ES8311 Audio Codec · SHTC3 Temp/Humidity · PCF8563 RTC · FT6336 Touch |
| **Primary SPI (e-Paper)** | GPIO10–13, + GPIO8, GPIO9 | 1.54″ e-Paper display (CS, SCLK, MOSI, BUSY, DC, RST) |
| **Secondary SPI (SD Card)** | GPIO39–42 | TF / MicroSD card storage |
| **I2S Audio** | GPIO14–18, GPIO47, GPIO48 | ES8311 codec (full-duplex) + NS4168 power amplifier |

---

## I2C Device Address Map

| Device | I2C Address | Function |
|---|---|---|
| FT6336 | `0x38` | Capacitive touch controller |
| SHTC3 | `0x70` | Temperature & humidity sensor |
| ES8311 | `0x18` | Audio codec control interface |
| PCF8563 | `0x51` | Real-Time Clock |

---

*Source: Waveshare ESP32-S3-ePaper-1.54 V2 public schematic. BSP integration by [Humidyne Labs](https://github.com/Humidyne-Labs).*
