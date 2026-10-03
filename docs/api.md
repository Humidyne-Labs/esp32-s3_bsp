# API Reference

## Classes

| Name | Description |
|------|-------------|
| [`bsp_config_t`](#bsp_config_t) | Modular Hardware Initialization Configuration. |
| [`button_dev_t`](#button_dev_t) |  |
| [`chime_entry_t`](#chime_entry_t) |  |
| [`splash_entry_t`](#splash_entry_t) |  |
| [`bsp_tb_entry_t`](#bsp_tb_entry_t) | ThingsBoard Generic Telemetry / Attribute Key-Value Entry. |
| [`bsp_diag_info_t`](#bsp_diag_info_t) | System Diagnostic Information Snapshot. |
| [`bsp_rtc_alarm_t`](#bsp_rtc_alarm_t) |  |
| [`bsp_rtc_state_t`](#bsp_rtc_state_t) | Persistent RTC State Struct (Stored in RTC Slow Memory). |
| [`bsp_tb_config_t`](#bsp_tb_config_t) | ThingsBoard Client Configuration Struct. |
| [`bsp_shtc3_data_t`](#bsp_shtc3_data_t) | SHTC3 Environmental Sensor Telemetry Data. |
| [`cached_i2c_dev_t`](#cached_i2c_dev_t) |  |
| [`rtc_wifi_cache_t`](#rtc_wifi_cache_t) | RTC Fast Reconnect Session Cache Structure Preserved across Deep Sleep cycles in RTC Slow/Fast memory. |
| [`bsp_rtc_datetime_t`](#bsp_rtc_datetime_t) |  |
| [`bsp_sleep_config_t`](#bsp_sleep_config_t) | Unified Sleep Configuration. |
| [`bsp_wake_context_t`](#bsp_wake_context_t) | Structured Wake Context passed to application on_wake callback. |
| [`battery_lut_point_t`](#battery_lut_point_t) |  |
| [`bsp_app_lifecycle_t`](#bsp_app_lifecycle_t) | Comprehensive Application Lifecycle Configuration. |
| [`bsp_button_config_t`](#bsp_button_config_t) | Button Timing & Feature Configuration. |
| [`bsp_sensor_cal_data_t`](#bsp_sensor_cal_data_t) | Calibrated Environmental & Diagnostic Sensor Telemetry. |
| [`button_callback_entry_t`](#button_callback_entry_t) |  |

## Macros

---

{#bsp_config_default}

### BSP_CONFIG_DEFAULT

```cpp
#define BSP_CONFIG_DEFAULT() { \
    .init_power   = true,      \
    .init_i2c     = true,      \
    .init_sensors = true,      \
    .init_rtc     = true,      \
    .init_buttons = true,      \
    .init_audio   = true,      \
    .audio_volume = 80.0f,     \
    .init_sdcard  = false,     \
    .init_display = true,      \
    .init_nvs     = true,      \
    .start_lvgl   = true       \
}
```

Defined in bsp/bsp.h:73

Default Hardware Initialization Configuration Macro.

---

{#bsp_charset_unambiguous}

### BSP_CHARSET_UNAMBIGUOUS

```cpp
#define BSP_CHARSET_UNAMBIGUOUS "abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ23456789"
```

Defined in bsp/bsp.h:150

Base57 character set excluding visually ambiguous characters (0, O, o, 1, l, I).

---

{#bsp_pin_power_hold}

### BSP_PIN_POWER_HOLD

```cpp
#define BSP_PIN_POWER_HOLD GPIO_NUM_17
```

Defined in bsp/pinout.h:38

Power Hold Latch Output Pin (BAT_Control).

Must be driven HIGH on boot to maintain LDO power rail from battery/regulator.

---

{#bsp_gpio_bat_ctrl}

### BSP_GPIO_BAT_CTRL

```cpp
#define BSP_GPIO_BAT_CTRL BSP_PIN_POWER_HOLD
```

Defined in bsp/pinout.h:39

---

{#bsp_pin_battery_adc}

### BSP_PIN_BATTERY_ADC

```cpp
#define BSP_PIN_BATTERY_ADC GPIO_NUM_4
```

Defined in bsp/pinout.h:45

Battery Voltage ADC Sensing Input Pin (ADC1 Channel 3).

Connected to a 1:2 resistor voltage divider network (R1=100k, R2=100k).

---

{#bsp_gpio_bat_adc}

### BSP_GPIO_BAT_ADC

```cpp
#define BSP_GPIO_BAT_ADC BSP_PIN_BATTERY_ADC
```

Defined in bsp/pinout.h:46

---

{#bsp_pin_led_status}

### BSP_PIN_LED_STATUS

```cpp
#define BSP_PIN_LED_STATUS GPIO_NUM_3
```

Defined in bsp/pinout.h:51

Onboard User / Status Indicator LED (Open-Drain, Active Low: 0=ON, 1=OFF).

---

{#bsp_gpio_user_led}

### BSP_GPIO_USER_LED

```cpp
#define BSP_GPIO_USER_LED BSP_PIN_LED_STATUS
```

Defined in bsp/pinout.h:52

---

{#bsp_pin_button_boot}

### BSP_PIN_BUTTON_BOOT

```cpp
#define BSP_PIN_BUTTON_BOOT GPIO_NUM_0
```

Defined in bsp/pinout.h:60

Hardware Boot / User Action Button (Active Low, requires internal pull-up).

---

{#bsp_gpio_boot_key}

### BSP_GPIO_BOOT_KEY

```cpp
#define BSP_GPIO_BOOT_KEY BSP_PIN_BUTTON_BOOT
```

Defined in bsp/pinout.h:61

---

{#bsp_pin_button_power}

### BSP_PIN_BUTTON_POWER

```cpp
#define BSP_PIN_BUTTON_POWER GPIO_NUM_18
```

Defined in bsp/pinout.h:66

Hardware Power / Battery Key (Active Low, requires internal pull-up).

---

{#bsp_gpio_bat_key}

### BSP_GPIO_BAT_KEY

```cpp
#define BSP_GPIO_BAT_KEY BSP_PIN_BUTTON_POWER
```

Defined in bsp/pinout.h:67

---

{#bsp_pin_i2c_sda}

### BSP_PIN_I2C_SDA

```cpp
#define BSP_PIN_I2C_SDA GPIO_NUM_47
```

Defined in bsp/pinout.h:72

Shared I2C Serial Data line (RTC_SDA, external 4.7k pull-up).

---

{#bsp_pin_i2c_scl}

### BSP_PIN_I2C_SCL

```cpp
#define BSP_PIN_I2C_SCL GPIO_NUM_48
```

Defined in bsp/pinout.h:73

Shared I2C Serial Clock line (RTC_SCL, external 4.7k pull-up).

---

{#bsp_pin_rtc_int}

### BSP_PIN_RTC_INT

```cpp
#define BSP_PIN_RTC_INT GPIO_NUM_5
```

Defined in bsp/pinout.h:74

PCF85063A RTC INT (Open-Drain, No external pull-up, requires internal pull-up).

---

{#bsp_gpio_i2c_sda}

### BSP_GPIO_I2C_SDA

```cpp
#define BSP_GPIO_I2C_SDA BSP_PIN_I2C_SDA
```

Defined in bsp/pinout.h:75

---

{#bsp_gpio_i2c_scl}

### BSP_GPIO_I2C_SCL

```cpp
#define BSP_GPIO_I2C_SCL BSP_PIN_I2C_SCL
```

Defined in bsp/pinout.h:76

---

{#bsp_gpio_rtc_int}

### BSP_GPIO_RTC_INT

```cpp
#define BSP_GPIO_RTC_INT BSP_PIN_RTC_INT
```

Defined in bsp/pinout.h:77

---

{#bsp_i2c_addr_shtc3}

### BSP_I2C_ADDR_SHTC3

```cpp
#define BSP_I2C_ADDR_SHTC3 0x70
```

Defined in bsp/pinout.h:80

Sensirion SHTC3 Environmental Sensor.

---

{#bsp_i2c_addr_pcf85063}

### BSP_I2C_ADDR_PCF85063

```cpp
#define BSP_I2C_ADDR_PCF85063 0x51
```

Defined in bsp/pinout.h:81

NXP PCF85063A Real-Time Clock.

---

{#bsp_i2c_addr_es8311}

### BSP_I2C_ADDR_ES8311

```cpp
#define BSP_I2C_ADDR_ES8311 0x18
```

Defined in bsp/pinout.h:82

Everest Semi ES8311 Audio Codec.

---

{#bsp_pin_epd_3v3_en}

### BSP_PIN_EPD_3V3_EN

```cpp
#define BSP_PIN_EPD_3V3_EN GPIO_NUM_6
```

Defined in bsp/pinout.h:87

EPD 3.3V Power Enable (Active Low: 0=ON, 1=OFF).

---

{#bsp_pin_epd_busy}

### BSP_PIN_EPD_BUSY

```cpp
#define BSP_PIN_EPD_BUSY GPIO_NUM_8
```

Defined in bsp/pinout.h:88

Display Busy Line (High = Busy).

---

{#bsp_pin_epd_rst}

### BSP_PIN_EPD_RST

```cpp
#define BSP_PIN_EPD_RST GPIO_NUM_9
```

Defined in bsp/pinout.h:89

Display Hardware Reset (Active Low).

---

{#bsp_pin_epd_dc}

### BSP_PIN_EPD_DC

```cpp
#define BSP_PIN_EPD_DC GPIO_NUM_10
```

Defined in bsp/pinout.h:90

Display Data / Command control line.

---

{#bsp_pin_epd_cs}

### BSP_PIN_EPD_CS

```cpp
#define BSP_PIN_EPD_CS GPIO_NUM_11
```

Defined in bsp/pinout.h:91

Display SPI Chip Select (Active Low).

---

{#bsp_pin_epd_sck}

### BSP_PIN_EPD_SCK

```cpp
#define BSP_PIN_EPD_SCK GPIO_NUM_12
```

Defined in bsp/pinout.h:92

Display SPI Serial Clock.

---

{#bsp_pin_epd_mosi}

### BSP_PIN_EPD_MOSI

```cpp
#define BSP_PIN_EPD_MOSI GPIO_NUM_13
```

Defined in bsp/pinout.h:93

Display SPI Master Out Slave In (Data In).

---

{#bsp_gpio_epd_3v3_en}

### BSP_GPIO_EPD_3V3_EN

```cpp
#define BSP_GPIO_EPD_3V3_EN BSP_PIN_EPD_3V3_EN
```

Defined in bsp/pinout.h:95

---

{#bsp_gpio_epd_busy}

### BSP_GPIO_EPD_BUSY

```cpp
#define BSP_GPIO_EPD_BUSY BSP_PIN_EPD_BUSY
```

Defined in bsp/pinout.h:96

---

{#bsp_gpio_epd_rst}

### BSP_GPIO_EPD_RST

```cpp
#define BSP_GPIO_EPD_RST BSP_PIN_EPD_RST
```

Defined in bsp/pinout.h:97

---

{#bsp_gpio_epd_dc}

### BSP_GPIO_EPD_DC

```cpp
#define BSP_GPIO_EPD_DC BSP_PIN_EPD_DC
```

Defined in bsp/pinout.h:98

---

{#bsp_gpio_epd_cs}

### BSP_GPIO_EPD_CS

```cpp
#define BSP_GPIO_EPD_CS BSP_PIN_EPD_CS
```

Defined in bsp/pinout.h:99

---

{#bsp_gpio_epd_sclk}

### BSP_GPIO_EPD_SCLK

```cpp
#define BSP_GPIO_EPD_SCLK BSP_PIN_EPD_SCK
```

Defined in bsp/pinout.h:100

---

{#bsp_gpio_epd_mosi}

### BSP_GPIO_EPD_MOSI

```cpp
#define BSP_GPIO_EPD_MOSI BSP_PIN_EPD_MOSI
```

Defined in bsp/pinout.h:101

---

{#bsp_display_width}

### BSP_DISPLAY_WIDTH

```cpp
#define BSP_DISPLAY_WIDTH 200
```

Defined in bsp/pinout.h:104

Physical Width in Pixels.

---

{#bsp_display_height}

### BSP_DISPLAY_HEIGHT

```cpp
#define BSP_DISPLAY_HEIGHT 200
```

Defined in bsp/pinout.h:105

Physical Height in Pixels.

---

{#bsp_display_dpi}

### BSP_DISPLAY_DPI

```cpp
#define BSP_DISPLAY_DPI 188
```

Defined in bsp/pinout.h:106

Pixel Density (DPI).

---

{#bsp_pin_i2s_mclk}

### BSP_PIN_I2S_MCLK

```cpp
#define BSP_PIN_I2S_MCLK GPIO_NUM_14
```

Defined in bsp/pinout.h:111

ES8311 Master Clock (MCLK).

---

{#bsp_pin_i2s_sclk}

### BSP_PIN_I2S_SCLK

```cpp
#define BSP_PIN_I2S_SCLK GPIO_NUM_15
```

Defined in bsp/pinout.h:112

ES8311 Bit / Serial Clock (BCLK/SCLK).

---

{#bsp_pin_i2s_asdout}

### BSP_PIN_I2S_ASDOUT

```cpp
#define BSP_PIN_I2S_ASDOUT GPIO_NUM_16
```

Defined in bsp/pinout.h:113

ES8311 Serial Audio Data Out to ESP32 (DIN).

---

{#bsp_pin_i2s_lrck}

### BSP_PIN_I2S_LRCK

```cpp
#define BSP_PIN_I2S_LRCK GPIO_NUM_38
```

Defined in bsp/pinout.h:114

ES8311 Left/Right Clock (WS).

---

{#bsp_pin_i2s_dsdin}

### BSP_PIN_I2S_DSDIN

```cpp
#define BSP_PIN_I2S_DSDIN GPIO_NUM_45
```

Defined in bsp/pinout.h:115

ES8311 Serial Audio Data In from ESP32 (DOUT).

---

{#bsp_pin_pa_en}

### BSP_PIN_PA_EN

```cpp
#define BSP_PIN_PA_EN GPIO_NUM_42
```

Defined in bsp/pinout.h:116

NS4168 Power Amp Enable (Active Low: 0=ON).

---

{#bsp_pin_pa_ctrl}

### BSP_PIN_PA_CTRL

```cpp
#define BSP_PIN_PA_CTRL GPIO_NUM_46
```

Defined in bsp/pinout.h:117

NS4168 Power Amp Control.

---

{#bsp_gpio_i2s_mclk}

### BSP_GPIO_I2S_MCLK

```cpp
#define BSP_GPIO_I2S_MCLK BSP_PIN_I2S_MCLK
```

Defined in bsp/pinout.h:119

---

{#bsp_gpio_i2s_sclk}

### BSP_GPIO_I2S_SCLK

```cpp
#define BSP_GPIO_I2S_SCLK BSP_PIN_I2S_SCLK
```

Defined in bsp/pinout.h:120

---

{#bsp_gpio_i2s_asout}

### BSP_GPIO_I2S_ASOUT

```cpp
#define BSP_GPIO_I2S_ASOUT BSP_PIN_I2S_ASDOUT
```

Defined in bsp/pinout.h:121

---

{#bsp_gpio_i2s_lrck}

### BSP_GPIO_I2S_LRCK

```cpp
#define BSP_GPIO_I2S_LRCK BSP_PIN_I2S_LRCK
```

Defined in bsp/pinout.h:122

---

{#bsp_gpio_i2s_dsin}

### BSP_GPIO_I2S_DSIN

```cpp
#define BSP_GPIO_I2S_DSIN BSP_PIN_I2S_DSDIN
```

Defined in bsp/pinout.h:123

---

{#bsp_gpio_pa_en}

### BSP_GPIO_PA_EN

```cpp
#define BSP_GPIO_PA_EN BSP_PIN_PA_EN
```

Defined in bsp/pinout.h:124

---

{#bsp_gpio_pa_ctrl}

### BSP_GPIO_PA_CTRL

```cpp
#define BSP_GPIO_PA_CTRL BSP_PIN_PA_CTRL
```

Defined in bsp/pinout.h:125

---

{#bsp_pin_sd_clk}

### BSP_PIN_SD_CLK

```cpp
#define BSP_PIN_SD_CLK GPIO_NUM_39
```

Defined in bsp/pinout.h:130

MicroSD Clock.

---

{#bsp_pin_sd_miso}

### BSP_PIN_SD_MISO

```cpp
#define BSP_PIN_SD_MISO GPIO_NUM_40
```

Defined in bsp/pinout.h:131

MicroSD MISO / D0.

---

{#bsp_pin_sd_mosi}

### BSP_PIN_SD_MOSI

```cpp
#define BSP_PIN_SD_MOSI GPIO_NUM_41
```

Defined in bsp/pinout.h:132

MicroSD MOSI / CMD.

---

{#bsp_gpio_sd_clk}

### BSP_GPIO_SD_CLK

```cpp
#define BSP_GPIO_SD_CLK BSP_PIN_SD_CLK
```

Defined in bsp/pinout.h:134

---

{#bsp_gpio_sd_miso}

### BSP_GPIO_SD_MISO

```cpp
#define BSP_GPIO_SD_MISO BSP_PIN_SD_MISO
```

Defined in bsp/pinout.h:135

---

{#bsp_gpio_sd_mosi}

### BSP_GPIO_SD_MOSI

```cpp
#define BSP_GPIO_SD_MOSI BSP_PIN_SD_MOSI
```

Defined in bsp/pinout.h:136

---

{#mqtt_connected_bit}

### MQTT_CONNECTED_BIT

```cpp
#define MQTT_CONNECTED_BIT BIT0
```

Defined in bsp_tb.c:35

---

{#mqtt_pub_ack_bit}

### MQTT_PUB_ACK_BIT

```cpp
#define MQTT_PUB_ACK_BIT BIT1
```

Defined in bsp_tb.c:36

---

{#bsp_err_base}

### BSP_ERR_BASE

```cpp
#define BSP_ERR_BASE 0x8000
```

Defined in bsp/bsp_err.h:30

---

{#bsp_err_not_initialized}

### BSP_ERR_NOT_INITIALIZED

```cpp
#define BSP_ERR_NOT_INITIALIZED (BSP_ERR_BASE + 1)
```

Defined in bsp/bsp_err.h:31

Requested subsystem is not initialized.

---

{#bsp_err_i2c_bus_locked}

### BSP_ERR_I2C_BUS_LOCKED

```cpp
#define BSP_ERR_I2C_BUS_LOCKED (BSP_ERR_BASE + 2)
```

Defined in bsp/bsp_err.h:32

Shared I2C bus mutex acquisition timed out.

---

{#bsp_err_sensor_crc_fail}

### BSP_ERR_SENSOR_CRC_FAIL

```cpp
#define BSP_ERR_SENSOR_CRC_FAIL (BSP_ERR_BASE + 3)
```

Defined in bsp/bsp_err.h:33

SHTC3 sensor CRC checksum verification failed.

---

{#bsp_err_display_busy_timeout}

### BSP_ERR_DISPLAY_BUSY_TIMEOUT

```cpp
#define BSP_ERR_DISPLAY_BUSY_TIMEOUT (BSP_ERR_BASE + 4)
```

Defined in bsp/bsp_err.h:34

E-Paper display busy signal timed out.

---

{#bsp_err_audio_not_ready}

### BSP_ERR_AUDIO_NOT_READY

```cpp
#define BSP_ERR_AUDIO_NOT_READY (BSP_ERR_BASE + 5)
```

Defined in bsp/bsp_err.h:35

ES8311 codec or I2S channel not configured.

---

{#bsp_err_sd_card_mount}

### BSP_ERR_SD_CARD_MOUNT

```cpp
#define BSP_ERR_SD_CARD_MOUNT (BSP_ERR_BASE + 6)
```

Defined in bsp/bsp_err.h:36

MicroSD card failed to mount filesystem.

---

{#bsp_err_wifi_disconnected}

### BSP_ERR_WIFI_DISCONNECTED

```cpp
#define BSP_ERR_WIFI_DISCONNECTED (BSP_ERR_BASE + 7)
```

Defined in bsp/bsp_err.h:37

Wi-Fi interface is disconnected or down.

---

{#bsp_err_ota_validation}

### BSP_ERR_OTA_VALIDATION

```cpp
#define BSP_ERR_OTA_VALIDATION (BSP_ERR_BASE + 8)
```

Defined in bsp/bsp_err.h:38

OTA firmware binary verification failed.

---

{#max_cached_devices}

### MAX_CACHED_DEVICES

```cpp
#define MAX_CACHED_DEVICES 8
```

Defined in bsp_i2c.c:33

---

{#bsp_nvs_namespace}

### BSP_NVS_NAMESPACE

```cpp
#define BSP_NVS_NAMESPACE "humid_bsp"
```

Defined in bsp_nvs.c:23

---

{#bsp_rtc_reg_control_1}

### BSP_RTC_REG_CONTROL_1

```cpp
#define BSP_RTC_REG_CONTROL_1 0x00
```

Defined in bsp_rtc.c:53

---

{#bsp_rtc_reg_control_2}

### BSP_RTC_REG_CONTROL_2

```cpp
#define BSP_RTC_REG_CONTROL_2 0x01
```

Defined in bsp_rtc.c:54

---

{#bsp_rtc_reg_offset}

### BSP_RTC_REG_OFFSET

```cpp
#define BSP_RTC_REG_OFFSET 0x02
```

Defined in bsp_rtc.c:55

---

{#bsp_rtc_reg_ram_byte}

### BSP_RTC_REG_RAM_BYTE

```cpp
#define BSP_RTC_REG_RAM_BYTE 0x03
```

Defined in bsp_rtc.c:56

---

{#bsp_rtc_reg_time}

### BSP_RTC_REG_TIME

```cpp
#define BSP_RTC_REG_TIME 0x04
```

Defined in bsp_rtc.c:57

---

{#bsp_rtc_reg_alarm_sec}

### BSP_RTC_REG_ALARM_SEC

```cpp
#define BSP_RTC_REG_ALARM_SEC 0x0B
```

Defined in bsp_rtc.c:58

---

{#bsp_rtc_reg_timer_val}

### BSP_RTC_REG_TIMER_VAL

```cpp
#define BSP_RTC_REG_TIMER_VAL 0x10
```

Defined in bsp_rtc.c:59

---

{#bsp_rtc_reg_timer_mod}

### BSP_RTC_REG_TIMER_MOD

```cpp
#define BSP_RTC_REG_TIMER_MOD 0x11
```

Defined in bsp_rtc.c:60

---

{#ctrl1_ext_test}

### CTRL1_EXT_TEST

```cpp
#define CTRL1_EXT_TEST (1 << 7)
```

Defined in bsp_rtc.c:63

---

{#ctrl1_stop}

### CTRL1_STOP

```cpp
#define CTRL1_STOP (1 << 5)
```

Defined in bsp_rtc.c:64

---

{#ctrl1_sr}

### CTRL1_SR

```cpp
#define CTRL1_SR (1 << 4)
```

Defined in bsp_rtc.c:65

---

{#ctrl1_cie}

### CTRL1_CIE

```cpp
#define CTRL1_CIE (1 << 2)
```

Defined in bsp_rtc.c:66

---

{#ctrl1_12_24}

### CTRL1_12_24

```cpp
#define CTRL1_12_24 (1 << 1)
```

Defined in bsp_rtc.c:67

---

{#ctrl1_cap_sel_12_5pf}

### CTRL1_CAP_SEL_12_5PF

```cpp
#define CTRL1_CAP_SEL_12_5PF (1 << 0)
```

Defined in bsp_rtc.c:68

---

{#ctrl1_sw_reset_cmd}

### CTRL1_SW_RESET_CMD

```cpp
#define CTRL1_SW_RESET_CMD 0x58
```

Defined in bsp_rtc.c:69

---

{#ctrl2_aie}

### CTRL2_AIE

```cpp
#define CTRL2_AIE (1 << 7)
```

Defined in bsp_rtc.c:72

---

{#ctrl2_af}

### CTRL2_AF

```cpp
#define CTRL2_AF (1 << 6)
```

Defined in bsp_rtc.c:73

---

{#ctrl2_mi}

### CTRL2_MI

```cpp
#define CTRL2_MI (1 << 5)
```

Defined in bsp_rtc.c:74

---

{#ctrl2_hmi}

### CTRL2_HMI

```cpp
#define CTRL2_HMI (1 << 4)
```

Defined in bsp_rtc.c:75

---

{#ctrl2_tf}

### CTRL2_TF

```cpp
#define CTRL2_TF (1 << 3)
```

Defined in bsp_rtc.c:76

---

{#ctrl2_cof_off}

### CTRL2_COF_OFF

```cpp
#define CTRL2_COF_OFF 0x07
```

Defined in bsp_rtc.c:77

---

{#timer_mode_tcf_1hz}

### TIMER_MODE_TCF_1HZ

```cpp
#define TIMER_MODE_TCF_1HZ (2 << 3)
```

Defined in bsp_rtc.c:80

---

{#timer_mode_te}

### TIMER_MODE_TE

```cpp
#define TIMER_MODE_TE (1 << 2)
```

Defined in bsp_rtc.c:81

---

{#timer_mode_tie}

### TIMER_MODE_TIE

```cpp
#define TIMER_MODE_TIE (1 << 1)
```

Defined in bsp_rtc.c:82

---

{#timer_mode_ti_tp}

### TIMER_MODE_TI_TP

```cpp
#define TIMER_MODE_TI_TP (1 << 0)
```

Defined in bsp_rtc.c:83

---

{#wifi_connected_bit}

### WIFI_CONNECTED_BIT

```cpp
#define WIFI_CONNECTED_BIT BIT0
```

Defined in bsp_wifi.c:37

---

{#wifi_fail_bit}

### WIFI_FAIL_BIT

```cpp
#define WIFI_FAIL_BIT BIT1
```

Defined in bsp_wifi.c:38

---

{#rtc_wifi_cache_magic}

### RTC_WIFI_CACHE_MAGIC

```cpp
#define RTC_WIFI_CACHE_MAGIC 0x57494649
```

Defined in bsp_wifi.c:56

---

{#bsp_sleep_config_default}

### BSP_SLEEP_CONFIG_DEFAULT

```cpp
#define BSP_SLEEP_CONFIG_DEFAULT() { \
    .mode           = BSP_SLEEP_MODE_DEEP, \
    .duration_sec   = 0, \
    .wake_sources   = BSP_WAKE_SRC_ALL, \
    .next_init_mode = BSP_INIT_MODE_FAST \
}
```

Defined in bsp/bsp_power.h:71

Default Deep Sleep Configuration Macro.

---

{#bsp_adc_battery_channel}

### BSP_ADC_BATTERY_CHANNEL

```cpp
#define BSP_ADC_BATTERY_CHANNEL ADC_CHANNEL_3
```

Defined in bsp_power.c:52

---

{#config_bsp_battery_correction_factor_mv}

### CONFIG_BSP_BATTERY_CORRECTION_FACTOR_MV

```cpp
#define CONFIG_BSP_BATTERY_CORRECTION_FACTOR_MV 50
```

Defined in bsp_power.c:54

---

{#battery_lut_size}

### BATTERY_LUT_SIZE

```cpp
#define BATTERY_LUT_SIZE (sizeof(s_battery_ocv_lut) / sizeof(s_battery_ocv_lut[0]))
```

Defined in bsp_power.c:56

---

{#bsp_sdcard_mount_point}

### BSP_SDCARD_MOUNT_POINT

```cpp
#define BSP_SDCARD_MOUNT_POINT "/sdcard"
```

Defined in bsp/bsp_sdcard.h:30

---

{#timer_interval_ms}

### TIMER_INTERVAL_MS

```cpp
#define TIMER_INTERVAL_MS 10
```

Defined in bsp_button.c:28

---

{#partial_refresh_limit}

### PARTIAL_REFRESH_LIMIT

```cpp
#define PARTIAL_REFRESH_LIMIT 20
```

Defined in bsp_lvgl.cpp:37

---

{#lvgl_i1_palette_size}

### LVGL_I1_PALETTE_SIZE

```cpp
#define LVGL_I1_PALETTE_SIZE 8
```

Defined in bsp_lvgl.cpp:38

---

{#bsp_display_buffer_size}

### BSP_DISPLAY_BUFFER_SIZE

```cpp
#define BSP_DISPLAY_BUFFER_SIZE ((BSP_DISPLAY_WIDTH * BSP_DISPLAY_HEIGHT) / 8)
```

Defined in bsp/bsp_display.h:44

1-Bit Packed Framebuffer Size (200 * 200 / 8 = 5000 bytes)

---

{#bsp_version_major}

### BSP_VERSION_MAJOR

```cpp
#define BSP_VERSION_MAJOR 1
```

Defined in bsp/bsp_version.h:27

---

{#bsp_version_minor}

### BSP_VERSION_MINOR

```cpp
#define BSP_VERSION_MINOR 0
```

Defined in bsp/bsp_version.h:28

---

{#bsp_version_patch}

### BSP_VERSION_PATCH

```cpp
#define BSP_VERSION_PATCH 0
```

Defined in bsp/bsp_version.h:29

---

{#bsp_version_string}

### BSP_VERSION_STRING

```cpp
#define BSP_VERSION_STRING "1.0.0"
```

Defined in bsp/bsp_version.h:30

---

{#bsp_version_val}

### BSP_VERSION_VAL

```cpp
#define BSP_VERSION_VAL(major, minor, patch) (((major) << 16) | ((minor) << 8) | (patch))
```

Defined in bsp/bsp_version.h:32

---

{#bsp_current_version}

### BSP_CURRENT_VERSION

```cpp
#define BSP_CURRENT_VERSION BSP_VERSION_VAL(BSP_VERSION_MAJOR, BSP_VERSION_MINOR, BSP_VERSION_PATCH)
```

Defined in bsp/bsp_version.h:33

---

{#c_to_f}

### C_TO_F

```cpp
#define C_TO_F(c) (((c) * 1.8f) + 32.0f)
```

Defined in bsp_sensors.c:36

Conversion MACRO Celsius to Fahrenheit.

---

{#c_to_k}

### C_TO_K

```cpp
#define C_TO_K(c) ((c) + 273.15f)
```

Defined in bsp_sensors.c:37

Conversion MACRO Celsius to Kelvin.

---

{#shtc3_cmd_wakeup}

### SHTC3_CMD_WAKEUP

```cpp
#define SHTC3_CMD_WAKEUP 0x3517
```

Defined in bsp_sensors.c:40

Wakeup command.

---

{#shtc3_cmd_sleep}

### SHTC3_CMD_SLEEP

```cpp
#define SHTC3_CMD_SLEEP 0xB098
```

Defined in bsp_sensors.c:41

Sleep command.

---

{#shtc3_cmd_swrst}

### SHTC3_CMD_SWRST

```cpp
#define SHTC3_CMD_SWRST 0x805D
```

Defined in bsp_sensors.c:42

Software Reset Command.

---

{#shtc3_cmd_read_id}

### SHTC3_CMD_READ_ID

```cpp
#define SHTC3_CMD_READ_ID 0xEFC8
```

Defined in bsp_sensors.c:43

Read ID register.

---

{#shtc3_cmd_meas_normal_t_first}

### SHTC3_CMD_MEAS_NORMAL_T_FIRST

```cpp
#define SHTC3_CMD_MEAS_NORMAL_T_FIRST 0x7866
```

Defined in bsp_sensors.c:44

Measure Normal Power: Temp First, Clock Stretching Disabled.

---

{#shtc3_cmd_meas_normal_h_first}

### SHTC3_CMD_MEAS_NORMAL_H_FIRST

```cpp
#define SHTC3_CMD_MEAS_NORMAL_H_FIRST 0x58E0
```

Defined in bsp_sensors.c:45

Measure Normal Power: Humidity First, Clock Stretching Disabled.

---

{#shtc3_cmd_meas_lowpwr_t_first}

### SHTC3_CMD_MEAS_LOWPWR_T_FIRST

```cpp
#define SHTC3_CMD_MEAS_LOWPWR_T_FIRST 0x609C
```

Defined in bsp_sensors.c:48

Measure Low Power: Temp First, Clock Stretching Disabled.

---

{#shtc3_cmd_meas_lowpwr_h_first}

### SHTC3_CMD_MEAS_LOWPWR_H_FIRST

```cpp
#define SHTC3_CMD_MEAS_LOWPWR_H_FIRST 0x401A
```

Defined in bsp_sensors.c:49

Measure Low Power: Humidity First, Clock Stretching Disabled.

---

{#shtc3_cmd_meas_normal_cs_t_first}

### SHTC3_CMD_MEAS_NORMAL_CS_T_FIRST

```cpp
#define SHTC3_CMD_MEAS_NORMAL_CS_T_FIRST 0x7CA2
```

Defined in bsp_sensors.c:52

Measure Normal Power: Temp First, Clock Stretching Enabled.

---

{#shtc3_cmd_meas_normal_cs_h_first}

### SHTC3_CMD_MEAS_NORMAL_CS_H_FIRST

```cpp
#define SHTC3_CMD_MEAS_NORMAL_CS_H_FIRST 0x5C24
```

Defined in bsp_sensors.c:53

Measure Normal Power: Humidity First, Clock Stretching Enabled.

---

{#shtc3_cmd_meas_lowpwr_cs_t_first}

### SHTC3_CMD_MEAS_LOWPWR_CS_T_FIRST

```cpp
#define SHTC3_CMD_MEAS_LOWPWR_CS_T_FIRST 0x6458
```

Defined in bsp_sensors.c:54

Measure Low Power: Temp First, Clock Stretching Enabled.

---

{#shtc3_cmd_meas_lowpwr_cs_h_first}

### SHTC3_CMD_MEAS_LOWPWR_CS_H_FIRST

```cpp
#define SHTC3_CMD_MEAS_LOWPWR_CS_H_FIRST 0x44DE
```

Defined in bsp_sensors.c:55

Measure Low Power: Humidity First, Clock Stretching Enabled.

---

{#bsp_rtc_mem_magic}

### BSP_RTC_MEM_MAGIC

```cpp
#define BSP_RTC_MEM_MAGIC 0x53335254
```

Defined in bsp/bsp_rtc_mem.h:33

"S3RT" magic token

---

{#epd_full_refresh_timeout_ms}

### EPD_FULL_REFRESH_TIMEOUT_MS

```cpp
#define EPD_FULL_REFRESH_TIMEOUT_MS 10000
```

Defined in bsp_display.cpp:30

---

{#epd_partial_refresh_timeout_ms}

### EPD_PARTIAL_REFRESH_TIMEOUT_MS

```cpp
#define EPD_PARTIAL_REFRESH_TIMEOUT_MS 3000
```

Defined in bsp_display.cpp:31

---

{#c_to_f-1}

### C_TO_F

```cpp
#define C_TO_F(c) (((c) * 1.8f) + 32.0f)
```

Defined in bsp_sensor_cal.c:23

---

{#c_to_k-1}

### C_TO_K

```cpp
#define C_TO_K(c) ((c) + 273.15f)
```

Defined in bsp_sensor_cal.c:24

## Enumerations

---

{#bsp_tb_val_type_t}

### bsp_tb_val_type_t

```cpp
enum bsp_tb_val_type_t
```

Defined in bsp/bsp_tb.h:70

ThingsBoard Telemetry / Attribute Value Types.

| Value | Description |
|-------|-------------|
| `BSP_TB_VAL_INT` | 64-bit Signed Integer (int64_t) |
| `BSP_TB_VAL_FLOAT` | Single-Precision Float. |
| `BSP_TB_VAL_DOUBLE` | Double-Precision Float. |
| `BSP_TB_VAL_BOOL` | Boolean (true/false). |
| `BSP_TB_VAL_STRING` | Null-terminated String. |

---

{#bsp_ota_status_t}

### bsp_ota_status_t

```cpp
enum bsp_ota_status_t
```

Defined in bsp/bsp_ota.h:41

OTA Operation Status.

| Value | Description |
|-------|-------------|
| `BSP_OTA_STATUS_IDLE` | BSP_OTA_STATUS_IDLE value. |
| `BSP_OTA_STATUS_STARTING` | BSP_OTA_STATUS_STARTING value. |
| `BSP_OTA_STATUS_DOWNLOADING` | BSP_OTA_STATUS_DOWNLOADING value. |
| `BSP_OTA_STATUS_VERIFYING` | BSP_OTA_STATUS_VERIFYING value. |
| `BSP_OTA_STATUS_SUCCESS` | BSP_OTA_STATUS_SUCCESS value. |
| `BSP_OTA_STATUS_FAILED` | BSP_OTA_STATUS_FAILED value. |

---

{#bsp_rtc_offset_mode_t}

### bsp_rtc_offset_mode_t

```cpp
enum bsp_rtc_offset_mode_t
```

Defined in bsp/bsp_rtc.h:52

| Value | Description |
|-------|-------------|
| `BSP_RTC_OFFSET_MODE_2_HOURS` | 4.340 ppm/step (Low power) |
| `BSP_RTC_OFFSET_MODE_4_MIN` | 4.069 ppm/step (Fast correction) |

---

{#bsp_prov_event_t}

### bsp_prov_event_t

```cpp
enum bsp_prov_event_t
```

Defined in bsp/bsp_prov.h:39

Provisioning Event Types.

| Value | Description |
|-------|-------------|
| `BSP_PROV_EVENT_STARTED` | BLE advertising started. |
| `BSP_PROV_EVENT_CRED_RECEIVED` | Wi-Fi SSID / Password received from client. |
| `BSP_PROV_EVENT_CRED_SUCCESS` | Station successfully associated. |
| `BSP_PROV_EVENT_CRED_FAILED` | Station association failed. |
| `BSP_PROV_EVENT_FINISHED` | Provisioning ended, BLE de-initialized. |

---

{#bsp_time_format_t}

### bsp_time_format_t

```cpp
enum bsp_time_format_t
```

Defined in bsp/bsp_time.h:44

Formatted Time Representation Modes.

| Value | Description |
|-------|-------------|
| `BSP_TIME_FMT_24H_SEC` | "14:35:08" |
| `BSP_TIME_FMT_24H_MIN` | "14:35" |
| `BSP_TIME_FMT_12H_SEC` | "02:35:08 PM" |
| `BSP_TIME_FMT_12H_MIN` | "02:35 PM" |

---

{#bsp_date_format_t}

### bsp_date_format_t

```cpp
enum bsp_date_format_t
```

Defined in bsp/bsp_time.h:54

Formatted Date Representation Modes.

| Value | Description |
|-------|-------------|
| `BSP_DATE_FMT_MM_DD_YY` | "09/26/26" |
| `BSP_DATE_FMT_DOW` | "Saturday" |
| `BSP_DATE_FMT_MM_DD_YY_DOW` | "09/26/26 Saturday" |

---

{#bsp_wake_source_mask_t}

### bsp_wake_source_mask_t

```cpp
enum bsp_wake_source_mask_t
```

Defined in bsp/bsp_power.h:51

Wakeup Source Selection Flags.

| Value | Description |
|-------|-------------|
| `BSP_WAKE_SRC_TIMER` | ESP32-S3 Internal RTC Sleep Timer. |
| `BSP_WAKE_SRC_EXTERNAL_RTC` | External PCF85063A RTC INT on GPIO 5. |
| `BSP_WAKE_SRC_BUTTONS` | Hardware BOOT0 (GPIO 0) and POWER (GPIO 18) keys. |
| `BSP_WAKE_SRC_ALL` | BSP_WAKE_SRC_ALL value. |

---

{#bsp_button_t}

### bsp_button_t

```cpp
enum bsp_button_t
```

Defined in bsp/bsp_button.h:40

Hardware Button Identifiers.

| Value | Description |
|-------|-------------|
| `BSP_BUTTON_BOOT` | GPIO 0 User / Boot Button. |
| `BSP_BUTTON_POWER` | GPIO 18 Power / Battery Key. |
| `BSP_BUTTON_COUNT` | BSP_BUTTON_COUNT value. |
| `BSP_BUTTON_MAX` | BSP_BUTTON_MAX value. |

---

{#bsp_button_event_t}

### bsp_button_event_t

```cpp
enum bsp_button_event_t
```

Defined in bsp/bsp_button.h:50

Button Event Types.

| Value | Description |
|-------|-------------|
| `BSP_BUTTON_EVENT_PRESS_DOWN` | Button transitioned to pressed state. |
| `BSP_BUTTON_EVENT_PRESS_UP` | Button transitioned to released state. |
| `BSP_BUTTON_EVENT_SINGLE_CLICK` | Single click completed. |
| `BSP_BUTTON_EVENT_DOUBLE_CLICK` | Double click detected within click timeout window. |
| `BSP_BUTTON_EVENT_LONG_PRESS` | Button held down past long_press_ms threshold. |
| `BSP_BUTTON_EVENT_MAX` |  |

---

{#bsp_splash_type_t}

### bsp_splash_type_t

```cpp
enum bsp_splash_type_t
```

Defined in bsp/bsp_splash.h:39

UI Splash Screen Types.

| Value | Description |
|-------|-------------|
| `BSP_SPLASH_BOOT` | Rendered upon system cold boot. |
| `BSP_SPLASH_WAKE` | Rendered upon resuming from sleep. |
| `BSP_SPLASH_SLEEP` | Rendered prior to entering deep/light sleep. |
| `BSP_SPLASH_SHUTDOWN` | Rendered prior to system power off (e.g. Space Cat). |
| `BSP_SPLASH_MAX` |  |

---

{#bsp_chime_type_t}

### bsp_chime_type_t

```cpp
enum bsp_chime_type_t
```

Defined in bsp/bsp_splash.h:50

Audio Chime / Notification Event Types.

| Value | Description |
|-------|-------------|
| `BSP_CHIME_BOOT` | Bootup melodic chime. |
| `BSP_CHIME_WAKE` | Wake from sleep acoustic cue. |
| `BSP_CHIME_SLEEP` | Sleep / stand-down descending tone. |
| `BSP_CHIME_SHUTDOWN` | Power off tone. |
| `BSP_CHIME_ALARM` | Critical telemetry / threshold alarm. |
| `BSP_CHIME_NOTIFY` | General notification chirp. |
| `BSP_CHIME_EVENT` | User action / UI event click tone. |
| `BSP_CHIME_MAX` |  |

---

{#button_state_t}

### button_state_t

```cpp
enum button_state_t
```

Defined in bsp_button.c:30

| Value | Description |
|-------|-------------|
| `STATE_BOOT_WAIT_RELEASE` | STATE_BOOT_WAIT_RELEASE value. |
| `STATE_IDLE` | STATE_IDLE value. |
| `STATE_DEBOUNCE_PRESS` | STATE_DEBOUNCE_PRESS value. |
| `STATE_PRESSED` | STATE_PRESSED value. |
| `STATE_DEBOUNCE_RELEASE` | STATE_DEBOUNCE_RELEASE value. |
| `STATE_WAIT_DOUBLE_CLICK` | STATE_WAIT_DOUBLE_CLICK value. |

---

{#bsp_display_color_t}

### bsp_display_color_t

```cpp
enum bsp_display_color_t
```

Defined in bsp/bsp_display.h:36

Monochrome Pixel Colors.

| Value | Description |
|-------|-------------|
| `BSP_DISPLAY_COLOR_BLACK` | BSP_DISPLAY_COLOR_BLACK value. |
| `BSP_DISPLAY_COLOR_WHITE` | BSP_DISPLAY_COLOR_WHITE value. |

---

{#bsp_init_mode_t}

### bsp_init_mode_t

```cpp
enum bsp_init_mode_t
```

Defined in bsp/bsp_rtc_mem.h:38

Initialization Modes for Dynamic Hardware Configuration.

| Value | Description |
|-------|-------------|
| `BSP_INIT_MODE_FULL` | Cold boot / all subsystems & LVGL initialized. |
| `BSP_INIT_MODE_FAST` | Wake boot / fast display refresh (no clearing). |

---

{#bsp_sleep_mode_t}

### bsp_sleep_mode_t

```cpp
enum bsp_sleep_mode_t
```

Defined in bsp/bsp_rtc_mem.h:46

Sleep Execution Modes.

| Value | Description |
|-------|-------------|
| `BSP_SLEEP_MODE_LIGHT` | Light sleep (RAM preserved, clock gated). |
| `BSP_SLEEP_MODE_DEEP` | Deep sleep (Power down, RTC slow memory preserved). |
## Typedefs

---

{#bsp_tb_rpc_cb_t}

### bsp_tb_rpc_cb_t

```cpp
using bsp_tb_rpc_cb_t = void(*)
```

Defined in bsp/bsp_tb.h:49

ThingsBoard Server RPC Command Handler Callback.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `request_id` |  | Server RPC request ID string |
| `method` |  | RPC method name (e.g. "getValue", "setLed", "reboot") |
| `params_json` |  | JSON string of parameters |
| `user_data` |  | Optional user context |

---

{#bsp_tb_attr_cb_t}

### bsp_tb_attr_cb_t

```cpp
using bsp_tb_attr_cb_t = void(*)
```

Defined in bsp/bsp_tb.h:57

ThingsBoard Shared Attributes Update Callback.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `json_payload` |  | JSON payload string containing updated shared attributes |
| `user_data` |  | Optional user context |

---

{#bsp_tb_alarm_cb_t}

### bsp_tb_alarm_cb_t

```cpp
using bsp_tb_alarm_cb_t = void(*)
```

Defined in bsp/bsp_tb.h:65

ThingsBoard Alarm / Notification Callback.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `alarm_json` |  | JSON payload string containing alarm data |
| `user_data` |  | Optional user context |

---

{#bsp_ota_progress_cb_t}

### bsp_ota_progress_cb_t

```cpp
using bsp_ota_progress_cb_t = void(*)
```

Defined in bsp/bsp_ota.h:53

OTA Progress Callback Signature.

---

{#bsp_prov_event_cb_t}

### bsp_prov_event_cb_t

```cpp
using bsp_prov_event_cb_t = void(*)
```

Defined in bsp/bsp_prov.h:50

Provisioning Status Callback.

---

{#bsp_audio_done_cb_t}

### bsp_audio_done_cb_t

```cpp
using bsp_audio_done_cb_t = void(*)
```

Defined in bsp/bsp_audio.h:42

Audio playback completion callback type.

---

{#bsp_power_off_cb_t}

### bsp_power_off_cb_t

```cpp
using bsp_power_off_cb_t = void(*)
```

Defined in bsp/bsp_power.h:46

System shutdown / power-off callback function pointer.

---

{#bsp_button_cb_t}

### bsp_button_cb_t

```cpp
using bsp_button_cb_t = void(*)
```

Defined in bsp/bsp_button.h:76

Button Event Callback Signature.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `btn` |  | Originating button (BOOT or POWER) |
| `evt` |  | Triggered event type |
| `user_data` |  | Custom user pointer passed during registration |

---

{#bsp_splash_cb_t}

### bsp_splash_cb_t

```cpp
using bsp_splash_cb_t = void(*)
```

Defined in bsp/bsp_splash.h:67

UI Splash Screen Callback Signature.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` |  | Splash event type |
| `user_data` |  | User context pointer passed during registration |

---

{#bsp_chime_cb_t}

### bsp_chime_cb_t

```cpp
using bsp_chime_cb_t = void(*)
```

Defined in bsp/bsp_splash.h:75

Audio Chime Callback Signature.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` |  | Chime event type |
| `user_data` |  | User context pointer passed during registration |

---

{#bsp_cold_boot_cb_t}

### bsp_cold_boot_cb_t

```cpp
using bsp_cold_boot_cb_t = void(*)
```

Defined in bsp/bsp_lifecycle.h:60

Callback executed on cold boot (Power-On Reset, Brownout, Software Restart, etc.).

---

{#bsp_wake_cb_t}

### bsp_wake_cb_t

```cpp
using bsp_wake_cb_t = void(*)
```

Defined in bsp/bsp_lifecycle.h:65

Callback executed on resume from Deep Sleep or Light Sleep.

---

{#bsp_before_sleep_cb_t}

### bsp_before_sleep_cb_t

```cpp
using bsp_before_sleep_cb_t = void(*)
```

Defined in bsp/bsp_lifecycle.h:70

Callback executed right before entering Deep or Light Sleep (for app cleanup / display badge).

---

{#bsp_shutdown_cb_t}

### bsp_shutdown_cb_t

```cpp
using bsp_shutdown_cb_t = void(*)
```

Defined in bsp/bsp_lifecycle.h:75

Callback executed immediately prior to system power off / clean shutdown.

## Functions

---

{#bsp_board_init}

### bsp_board_init

```cpp
esp_err_t bsp_board_init(void)
```

Defined in bsp/bsp.h:96

Comprehensive Board Initialization.

Applies default configuration, enables power rails, configures I2C, RTC, sensors, buttons, display driver, and starts the LVGL v9 rendering task pinned to Core 1.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_init_mode}

### bsp_init_mode

```cpp
esp_err_t bsp_init_mode(bsp_init_mode_t mode)
```

Defined in bsp/bsp.h:108

Dynamic Hardware Initialization by Mode (FULL, FAST).

* BSP_INIT_MODE_FULL: Cold boot, full peripheral startup, display + LVGL.
* BSP_INIT_MODE_FAST: Wake boot, fast display ready without clear.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `mode` | [`bsp_init_mode_t`](#bsp_init_mode_t) | Target [bsp_init_mode_t](#bsp_init_mode_t) |

---

{#bsp_board_init_with_config}

### bsp_board_init_with_config

```cpp
esp_err_t bsp_board_init_with_config(const bsp_config_t * config)
```

Defined in bsp/bsp.h:117

Custom Board Initialization.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `config` | const [`bsp_config_t`](#bsp_config_t) * | Pointer to custom [bsp_config_t](#bsp_config_t) struct |

---

{#bsp_init_io}

### bsp_init_io

```cpp
esp_err_t bsp_init_io(void)
```

Defined in bsp/bsp.h:125

Initialize all GPIO output pins (power latch, audio PA, display power rail, LED).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_get_device_id}

### bsp_get_device_id

```cpp
esp_err_t bsp_get_device_id(char * buf, size_t max_len)
```

Defined in bsp/bsp.h:135

Retrieve Unique Hardware Device ID string from MAC address (e.g.

"ESP32S3-70041D3B")

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `buf` | `char *` | Destination buffer |
| `max_len` | `size_t` | Buffer length |

---

{#bsp_get_device_name}

### bsp_get_device_name

```cpp
esp_err_t bsp_get_device_name(char * buf, size_t max_len)
```

Defined in bsp/bsp.h:145

Retrieve Human-Readable Device Name string (e.g.

"HumidOS-1D3B")

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `buf` | `char *` | Destination buffer |
| `max_len` | `size_t` | Buffer length |

---

{#bsp_generate_unambiguous_key}

### bsp_generate_unambiguous_key

```cpp
esp_err_t bsp_generate_unambiguous_key(char * buf, size_t len, const char * charset)
```

Defined in bsp/bsp.h:161

Generate a cryptographically random, unambiguous key string (excluding 0, O, o, 1, l, I).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `buf` | `char *` | Destination buffer |
| `len` | `size_t` | Key length (excluding null terminator) |
| `charset` | `const char *` | Custom character set (or NULL for BSP_CHARSET_UNAMBIGUOUS) |

---

{#bsp_system_shutdown}

### bsp_system_shutdown

```cpp
void bsp_system_shutdown(void)
```

Defined in bsp/bsp.h:167

Perform Clean System Shutdown.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_system_deep_sleep}

### bsp_system_deep_sleep

```cpp
void bsp_system_deep_sleep(uint32_t sleep_sec)
```

Defined in bsp/bsp.h:175

Enter Ultra-Low Power Deep Sleep Mode.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `sleep_sec` | `uint32_t` | Duration in seconds (0 for button wakeup only) |

---

{#bsp_delay_ms}

### bsp_delay_ms

```cpp
void bsp_delay_ms(uint32_t ms)
```

Defined in bsp/bsp.h:187

Generic Millisecond Delay Helper.

Suspends execution for the specified duration in milliseconds. Uses FreeRTOS task delay if the scheduler is running, or hardware ROM delay on early boot. Allows delaying execution without including FreeRTOS header files in application code.

Memory ownership: none. Behavior: Blocking / Task Yield. Thread safety: Thread-safe.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `ms` | `uint32_t` | Delay duration in milliseconds |

---

{#bsp_delay_us}

### bsp_delay_us

```cpp
void bsp_delay_us(uint32_t us)
```

Defined in bsp/bsp.h:198

Generic Microsecond Delay Helper.

Pauses execution for the specified duration in microseconds using high-resolution timer. Does not require including FreeRTOS headers.

Memory ownership: none. Behavior: Busy-wait delay. Thread safety: Thread-safe.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `us` | `uint32_t` | Delay duration in microseconds |

---

{#bsp_tb_init}

### bsp_tb_init

```cpp
esp_err_t bsp_tb_init(const bsp_tb_config_t * config)
```

Defined in bsp/bsp_tb.h:114

Initialize ThingsBoard MQTTS Client Engine (Pinned to Core 0).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `config` | const [`bsp_tb_config_t`](#bsp_tb_config_t) * | Pointer to [bsp_tb_config_t](#bsp_tb_config_t) struct |

---

{#bsp_tb_wait_connected}

### bsp_tb_wait_connected

```cpp
esp_err_t bsp_tb_wait_connected(uint32_t timeout_ms)
```

Defined in bsp/bsp_tb.h:123

Wait until ThingsBoard MQTT connection is established.

#### Returns
esp_err_t ESP_OK if connected, ESP_ERR_TIMEOUT on timeout

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `timeout_ms` | `uint32_t` | Maximum time to wait in milliseconds |

---

{#bsp_tb_is_connected}

### bsp_tb_is_connected

```cpp
bool bsp_tb_is_connected(void)
```

Defined in bsp/bsp_tb.h:131

Check if ThingsBoard client is currently connected.

#### Returns
true if connected to broker

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_tb_send_telemetry_entries}

### bsp_tb_send_telemetry_entries

```cpp
esp_err_t bsp_tb_send_telemetry_entries(const bsp_tb_entry_t * entries, size_t count, bool sync, uint32_t timeout_ms)
```

Defined in bsp/bsp_tb.h:143

Publish Configurable Array of Telemetry Key-Value Entries.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `entries` | const [`bsp_tb_entry_t`](#bsp_tb_entry_t) * | Pointer to array of [bsp_tb_entry_t](#bsp_tb_entry_t) items |
| `count` | `size_t` | Number of entries in array |
| `sync` | `bool` | true to wait for QoS 1 broker ACK, false for async QoS 0 |
| `timeout_ms` | `uint32_t` | Maximum time to wait for ACK if sync is true |

---

{#bsp_tb_send_telemetry}

### bsp_tb_send_telemetry

```cpp
esp_err_t bsp_tb_send_telemetry(float temp_k, float rh_pct, uint8_t battery_pct, int rssi_dbm)
```

Defined in bsp/bsp_tb.h:157

Publish Environmental Telemetry JSON to ThingsBoard (Async QoS 0).

Convenience wrapper for: {"temp": Kelvin, "rh": %, "battery": %, "rssi": dBm}

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `temp_k` | `float` | Temperature in Kelvin |
| `rh_pct` | `float` | Relative Humidity percentage |
| `battery_pct` | `uint8_t` | Battery state of charge (0-100%) |
| `rssi_dbm` | `int` | Wi-Fi RSSI in dBm |

---

{#bsp_tb_send_telemetry_sync}

### bsp_tb_send_telemetry_sync

```cpp
esp_err_t bsp_tb_send_telemetry_sync(float temp_k, float rh_pct, uint8_t battery_pct, int rssi_dbm, uint32_t timeout_ms)
```

Defined in bsp/bsp_tb.h:172

Synchronously Publish Telemetry and Wait for Broker ACK (QoS 1).

Critical for Deep Sleep: Guarantees delivery before powering down radios!

#### Returns
esp_err_t ESP_OK on confirmed receipt

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `temp_k` | `float` | Temperature in Kelvin |
| `rh_pct` | `float` | Relative Humidity percentage |
| `battery_pct` | `uint8_t` | Battery % |
| `rssi_dbm` | `int` | Wi-Fi RSSI |
| `timeout_ms` | `uint32_t` | Max wait time for ACK |

---

{#bsp_tb_send_custom_telemetry}

### bsp_tb_send_custom_telemetry

```cpp
esp_err_t bsp_tb_send_custom_telemetry(const char * json_str, bool sync)
```

Defined in bsp/bsp_tb.h:182

Publish Arbitrary JSON Telemetry String.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `json_str` | `const char *` | Formatted JSON string (e.g. "{\"pressure\":1013.25}") |
| `sync` | `bool` | true for QoS 1 synchronous delivery, false for QoS 0 async |

---

{#bsp_tb_report_client_attributes_entries}

### bsp_tb_report_client_attributes_entries

```cpp
esp_err_t bsp_tb_report_client_attributes_entries(const bsp_tb_entry_t * entries, size_t count)
```

Defined in bsp/bsp_tb.h:192

Report Custom Client Attributes Key-Value Array to ThingsBoard.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `entries` | const [`bsp_tb_entry_t`](#bsp_tb_entry_t) * | Array of key-value attributes |
| `count` | `size_t` | Number of entries |

---

{#bsp_tb_report_client_attributes}

### bsp_tb_report_client_attributes

```cpp
esp_err_t bsp_tb_report_client_attributes(void)
```

Defined in bsp/bsp_tb.h:202

Report Standard Client Attributes to ThingsBoard.

Reports FW Version, Device ID, IP, MAC, Battery Voltage, and Uptime.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_tb_request_shared_attributes_keys}

### bsp_tb_request_shared_attributes_keys

```cpp
esp_err_t bsp_tb_request_shared_attributes_keys(const char ** shared_keys, size_t count)
```

Defined in bsp/bsp_tb.h:212

Request Specific Shared Attributes from ThingsBoard Server.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `shared_keys` | `const char **` | Array of attribute key names (or NULL for default OTA/sleep keys) |
| `count` | `size_t` | Number of keys in array |

---

{#bsp_tb_request_shared_attributes}

### bsp_tb_request_shared_attributes

```cpp
esp_err_t bsp_tb_request_shared_attributes(void)
```

Defined in bsp/bsp_tb.h:220

Request Standard Shared Attributes from ThingsBoard Server.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_tb_send_rpc_response}

### bsp_tb_send_rpc_response

```cpp
esp_err_t bsp_tb_send_rpc_response(const char * request_id, const char * response_json)
```

Defined in bsp/bsp_tb.h:230

Send RPC Response Back to ThingsBoard Server.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `request_id` | `const char *` | Request ID passed to RPC callback |
| `response_json` | `const char *` | JSON response string (e.g. "{\"success\":true}") |

---

{#bsp_tb_claim_device}

### bsp_tb_claim_device

```cpp
esp_err_t bsp_tb_claim_device(const char * secret_key, uint32_t duration_ms)
```

Defined in bsp/bsp_tb.h:240

Publish Device Claiming Token (v1/devices/me/claim).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `secret_key` | `const char *` | User claiming secret (or NULL to auto-generate 6-character key) |
| `duration_ms` | `uint32_t` | Claim validity duration in milliseconds |

---

{#bsp_tb_claim_device_auto}

### bsp_tb_claim_device_auto

```cpp
esp_err_t bsp_tb_claim_device_auto(char * out_key, size_t key_len, uint32_t duration_ms)
```

Defined in bsp/bsp_tb.h:251

Generate a 6-Character Unambiguous Claiming Token and Publish to ThingsBoard.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `out_key` | `char *` | Destination buffer to receive generated key (at least 8 bytes) |
| `key_len` | `size_t` | Buffer length |
| `duration_ms` | `uint32_t` | Claim duration in ms (e.g. 180000 / 3 min) |

---

{#bsp_tb_report_ota_state}

### bsp_tb_report_ota_state

```cpp
esp_err_t bsp_tb_report_ota_state(const char * state, const char * error_msg)
```

Defined in bsp/bsp_tb.h:261

Report Current OTA Firmware State to ThingsBoard.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `state` | `const char *` | OTA status string ("DOWNLOADING", "DOWNLOADED", "UPDATING", "SUCCESS", "FAILED") |
| `error_msg` | `const char *` | Optional error message (NULL for success) |

---

{#bsp_tb_disconnect}

### bsp_tb_disconnect

```cpp
esp_err_t bsp_tb_disconnect(void)
```

Defined in bsp/bsp_tb.h:269

Disconnect and Stop ThingsBoard Client.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#handle_rpc_message}

### handle_rpc_message

`static`

```cpp
static void handle_rpc_message(const char * topic, const char * data, int data_len)
```

Defined in bsp_tb.c:38

---

{#handle_attributes_message}

### handle_attributes_message

`static`

```cpp
static void handle_attributes_message(const char * data, int data_len)
```

Defined in bsp_tb.c:73

---

{#mqtt_event_handler}

### mqtt_event_handler

`static`

```cpp
static void mqtt_event_handler(void * handler_args, esp_event_base_t base, int32_t event_id, void * event_data)
```

Defined in bsp_tb.c:119

---

{#bsp_tb_init-1}

### bsp_tb_init

```cpp
esp_err_t bsp_tb_init(const bsp_tb_config_t * config)
```

Defined in bsp_tb.c:161

Initialize ThingsBoard MQTTS Client Engine (Pinned to Core 0).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `config` | const [`bsp_tb_config_t`](#bsp_tb_config_t) * | Pointer to [bsp_tb_config_t](#bsp_tb_config_t) struct |

---

{#bsp_tb_wait_connected-1}

### bsp_tb_wait_connected

```cpp
esp_err_t bsp_tb_wait_connected(uint32_t timeout_ms)
```

Defined in bsp_tb.c:213

Wait until ThingsBoard MQTT connection is established.

#### Returns
esp_err_t ESP_OK if connected, ESP_ERR_TIMEOUT on timeout

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `timeout_ms` | `uint32_t` | Maximum time to wait in milliseconds |

---

{#bsp_tb_is_connected-1}

### bsp_tb_is_connected

```cpp
bool bsp_tb_is_connected(void)
```

Defined in bsp_tb.c:220

Check if ThingsBoard client is currently connected.

#### Returns
true if connected to broker

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_tb_send_custom_telemetry-1}

### bsp_tb_send_custom_telemetry

```cpp
esp_err_t bsp_tb_send_custom_telemetry(const char * json_str, bool sync)
```

Defined in bsp_tb.c:226

Publish Arbitrary JSON Telemetry String.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `json_str` | `const char *` | Formatted JSON string (e.g. "{\"pressure\":1013.25}") |
| `sync` | `bool` | true for QoS 1 synchronous delivery, false for QoS 0 async |

---

{#bsp_tb_entries_to_json}

### bsp_tb_entries_to_json

`static`

```cpp
static cJSON * bsp_tb_entries_to_json(const bsp_tb_entry_t * entries, size_t count)
```

Defined in bsp_tb.c:254

---

{#bsp_tb_send_telemetry_entries-1}

### bsp_tb_send_telemetry_entries

```cpp
esp_err_t bsp_tb_send_telemetry_entries(const bsp_tb_entry_t * entries, size_t count, bool sync, uint32_t timeout_ms)
```

Defined in bsp_tb.c:287

Publish Configurable Array of Telemetry Key-Value Entries.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `entries` | const [`bsp_tb_entry_t`](#bsp_tb_entry_t) * | Pointer to array of [bsp_tb_entry_t](#bsp_tb_entry_t) items |
| `count` | `size_t` | Number of entries in array |
| `sync` | `bool` | true to wait for QoS 1 broker ACK, false for async QoS 0 |
| `timeout_ms` | `uint32_t` | Maximum time to wait for ACK if sync is true |

---

{#bsp_tb_send_telemetry-1}

### bsp_tb_send_telemetry

```cpp
esp_err_t bsp_tb_send_telemetry(float temp_k, float rh_pct, uint8_t battery_pct, int rssi_dbm)
```

Defined in bsp_tb.c:301

Publish Environmental Telemetry JSON to ThingsBoard (Async QoS 0).

Convenience wrapper for: {"temp": Kelvin, "rh": %, "battery": %, "rssi": dBm}

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `temp_k` | `float` | Temperature in Kelvin |
| `rh_pct` | `float` | Relative Humidity percentage |
| `battery_pct` | `uint8_t` | Battery state of charge (0-100%) |
| `rssi_dbm` | `int` | Wi-Fi RSSI in dBm |

---

{#bsp_tb_send_telemetry_sync-1}

### bsp_tb_send_telemetry_sync

```cpp
esp_err_t bsp_tb_send_telemetry_sync(float temp_k, float rh_pct, uint8_t battery_pct, int rssi_dbm, uint32_t timeout_ms)
```

Defined in bsp_tb.c:312

Synchronously Publish Telemetry and Wait for Broker ACK (QoS 1).

Critical for Deep Sleep: Guarantees delivery before powering down radios!

#### Returns
esp_err_t ESP_OK on confirmed receipt

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `temp_k` | `float` | Temperature in Kelvin |
| `rh_pct` | `float` | Relative Humidity percentage |
| `battery_pct` | `uint8_t` | Battery % |
| `rssi_dbm` | `int` | Wi-Fi RSSI |
| `timeout_ms` | `uint32_t` | Max wait time for ACK |

---

{#bsp_tb_report_client_attributes_entries-1}

### bsp_tb_report_client_attributes_entries

```cpp
esp_err_t bsp_tb_report_client_attributes_entries(const bsp_tb_entry_t * entries, size_t count)
```

Defined in bsp_tb.c:323

Report Custom Client Attributes Key-Value Array to ThingsBoard.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `entries` | const [`bsp_tb_entry_t`](#bsp_tb_entry_t) * | Array of key-value attributes |
| `count` | `size_t` | Number of entries |

---

{#bsp_tb_report_client_attributes-1}

### bsp_tb_report_client_attributes

```cpp
esp_err_t bsp_tb_report_client_attributes(void)
```

Defined in bsp_tb.c:338

Report Standard Client Attributes to ThingsBoard.

Reports FW Version, Device ID, IP, MAC, Battery Voltage, and Uptime.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_tb_request_shared_attributes_keys-1}

### bsp_tb_request_shared_attributes_keys

```cpp
esp_err_t bsp_tb_request_shared_attributes_keys(const char ** shared_keys, size_t count)
```

Defined in bsp_tb.c:360

Request Specific Shared Attributes from ThingsBoard Server.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `shared_keys` | `const char **` | Array of attribute key names (or NULL for default OTA/sleep keys) |
| `count` | `size_t` | Number of keys in array |

---

{#bsp_tb_request_shared_attributes-1}

### bsp_tb_request_shared_attributes

```cpp
esp_err_t bsp_tb_request_shared_attributes(void)
```

Defined in bsp_tb.c:386

Request Standard Shared Attributes from ThingsBoard Server.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_tb_send_rpc_response-1}

### bsp_tb_send_rpc_response

```cpp
esp_err_t bsp_tb_send_rpc_response(const char * request_id, const char * response_json)
```

Defined in bsp_tb.c:392

Send RPC Response Back to ThingsBoard Server.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `request_id` | `const char *` | Request ID passed to RPC callback |
| `response_json` | `const char *` | JSON response string (e.g. "{\"success\":true}") |

---

{#bsp_tb_claim_device-1}

### bsp_tb_claim_device

```cpp
esp_err_t bsp_tb_claim_device(const char * secret_key, uint32_t duration_ms)
```

Defined in bsp_tb.c:401

Publish Device Claiming Token (v1/devices/me/claim).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `secret_key` | `const char *` | User claiming secret (or NULL to auto-generate 6-character key) |
| `duration_ms` | `uint32_t` | Claim validity duration in milliseconds |

---

{#bsp_tb_claim_device_auto-1}

### bsp_tb_claim_device_auto

```cpp
esp_err_t bsp_tb_claim_device_auto(char * out_key, size_t key_len, uint32_t duration_ms)
```

Defined in bsp_tb.c:410

Generate a 6-Character Unambiguous Claiming Token and Publish to ThingsBoard.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `out_key` | `char *` | Destination buffer to receive generated key (at least 8 bytes) |
| `key_len` | `size_t` | Buffer length |
| `duration_ms` | `uint32_t` | Claim duration in ms (e.g. 180000 / 3 min) |

---

{#bsp_tb_report_ota_state-1}

### bsp_tb_report_ota_state

```cpp
esp_err_t bsp_tb_report_ota_state(const char * state, const char * error_msg)
```

Defined in bsp_tb.c:420

Report Current OTA Firmware State to ThingsBoard.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `state` | `const char *` | OTA status string ("DOWNLOADING", "DOWNLOADED", "UPDATING", "SUCCESS", "FAILED") |
| `error_msg` | `const char *` | Optional error message (NULL for success) |

---

{#bsp_tb_disconnect-1}

### bsp_tb_disconnect

```cpp
esp_err_t bsp_tb_disconnect(void)
```

Defined in bsp_tb.c:432

Disconnect and Stop ThingsBoard Client.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_get_chip_revision}

### bsp_get_chip_revision

```cpp
esp_err_t bsp_get_chip_revision(uint32_t * major, uint32_t * minor)
```

Defined in bsp/bsp_err.h:70

Get MCU Silicon Revision numbers (Major and Minor).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `major` | `uint32_t *` | Major wafer revision |
| `minor` | `uint32_t *` | Minor wafer revision |

---

{#bsp_get_chip_revision_str}

### bsp_get_chip_revision_str

```cpp
const char * bsp_get_chip_revision_str(void)
```

Defined in bsp/bsp_err.h:78

Get MCU Silicon Revision formatted string (e.g.

"v0.2")

#### Returns
const char* Revision string

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_err_to_name}

### bsp_err_to_name

```cpp
const char * bsp_err_to_name(esp_err_t err)
```

Defined in bsp/bsp_err.h:87

Translate BSP error code to human-readable error name string.

#### Returns
const char* String representation of error

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `err` | `esp_err_t` | Error code |

---

{#bsp_get_diagnostics}

### bsp_get_diagnostics

```cpp
esp_err_t bsp_get_diagnostics(bsp_diag_info_t * diag)
```

Defined in bsp/bsp_err.h:96

Populate a runtime system diagnostics snapshot.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `diag` | [`bsp_diag_info_t`](#bsp_diag_info_t) * | Pointer to [bsp_diag_info_t](#bsp_diag_info_t) struct to populate |

---

{#bsp_diagnostics_dump}

### bsp_diagnostics_dump

```cpp
void bsp_diagnostics_dump(void)
```

Defined in bsp/bsp_err.h:102

Print formatted system diagnostic report to stdout/ESP_LOG.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_i2c_init}

### bsp_i2c_init

```cpp
esp_err_t bsp_i2c_init(void)
```

Defined in bsp/bsp_i2c.h:42

Initialize Shared I2C Master Bus and Create Mutex Guard.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: thread-safe guarantees.

---

{#bsp_i2c_deinit}

### bsp_i2c_deinit

```cpp
esp_err_t bsp_i2c_deinit(void)
```

Defined in bsp/bsp_i2c.h:50

De-initialize Shared I2C Master Bus.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_i2c_get_handle}

### bsp_i2c_get_handle

```cpp
i2c_master_bus_handle_t bsp_i2c_get_handle(void)
```

Defined in bsp/bsp_i2c.h:58

Get the underlying I2C master bus handle.

#### Returns
i2c_master_bus_handle_t Bus handle or NULL

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_i2c_add_device}

### bsp_i2c_add_device

```cpp
esp_err_t bsp_i2c_add_device(const i2c_device_config_t * dev_cfg, i2c_master_dev_handle_t * dev_handle)
```

Defined in bsp/bsp_i2c.h:68

Add a device to the shared I2C master bus.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `dev_cfg` | `const i2c_device_config_t *` | Device configuration struct |
| `dev_handle` | `i2c_master_dev_handle_t *` | Destination pointer to receive device handle |

---

{#bsp_i2c_write}

### bsp_i2c_write

```cpp
esp_err_t bsp_i2c_write(uint8_t addr, const uint8_t * data, size_t len)
```

Defined in bsp/bsp_i2c.h:79

Write Raw Bytes to an I2C Slave Device (Thread-Safe).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: thread-safe guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `addr` | `uint8_t` | 7-bit slave device address |
| `data` | `const uint8_t *` | Data buffer to transmit |
| `len` | `size_t` | Number of bytes to transmit |

---

{#bsp_i2c_read}

### bsp_i2c_read

```cpp
esp_err_t bsp_i2c_read(uint8_t addr, uint8_t * data, size_t len)
```

Defined in bsp/bsp_i2c.h:90

Read Raw Bytes from an I2C Slave Device (Thread-Safe).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: thread-safe guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `addr` | `uint8_t` | 7-bit slave device address |
| `data` | `uint8_t *` | Buffer to receive incoming bytes |
| `len` | `size_t` | Number of bytes to read |

---

{#bsp_i2c_write_reg}

### bsp_i2c_write_reg

```cpp
esp_err_t bsp_i2c_write_reg(uint8_t addr, uint8_t reg, const uint8_t * data, size_t len)
```

Defined in bsp/bsp_i2c.h:102

Write Bytes to a Specific 8-bit Register on an I2C Slave (Thread-Safe).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: thread-safe guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `addr` | `uint8_t` | 7-bit slave device address |
| `reg` | `uint8_t` | 8-bit register address |
| `data` | `const uint8_t *` | Data buffer to write (can be NULL if len == 0) |
| `len` | `size_t` | Number of data bytes |

---

{#bsp_i2c_read_reg}

### bsp_i2c_read_reg

```cpp
esp_err_t bsp_i2c_read_reg(uint8_t addr, uint8_t reg, uint8_t * data, size_t len)
```

Defined in bsp/bsp_i2c.h:114

Read Bytes from a Specific 8-bit Register on an I2C Slave (Thread-Safe).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: thread-safe guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `addr` | `uint8_t` | 7-bit slave device address |
| `reg` | `uint8_t` | 8-bit register address |
| `data` | `uint8_t *` | Buffer to receive data bytes |
| `len` | `size_t` | Number of bytes to read |

---

{#bsp_i2c_probe}

### bsp_i2c_probe

```cpp
esp_err_t bsp_i2c_probe(uint8_t addr)
```

Defined in bsp/bsp_i2c.h:123

Probe whether an I2C slave responds on the bus.

#### Returns
esp_err_t ESP_OK if ACKed, error otherwise

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `addr` | `uint8_t` | 7-bit slave address |

---

{#bsp_nvs_init}

### bsp_nvs_init

```cpp
esp_err_t bsp_nvs_init(void)
```

Defined in bsp/bsp_nvs.h:39

Initialize Non-Volatile Storage (NVS) Subsystem.

Automatically recovers and re-initializes if the partition table is truncated or empty.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_nvs_set_str}

### bsp_nvs_set_str

```cpp
esp_err_t bsp_nvs_set_str(const char * key, const char * value)
```

Defined in bsp/bsp_nvs.h:49

Store a String Value in NVS.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `key` | `const char *` | Key name (up to 15 characters) |
| `value` | `const char *` | Null-terminated string value |

---

{#bsp_nvs_get_str}

### bsp_nvs_get_str

```cpp
esp_err_t bsp_nvs_get_str(const char * key, char * out_val, size_t max_len)
```

Defined in bsp/bsp_nvs.h:60

Retrieve a String Value from NVS.

#### Returns
esp_err_t ESP_OK on success, or ESP_ERR_NVS_NOT_FOUND if key doesn't exist

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `key` | `const char *` | Key name |
| `out_val` | `char *` | Destination character buffer |
| `max_len` | `size_t` | Size of buffer |

---

{#bsp_nvs_set_u32}

### bsp_nvs_set_u32

```cpp
esp_err_t bsp_nvs_set_u32(const char * key, uint32_t value)
```

Defined in bsp/bsp_nvs.h:70

Store an Unsigned 32-bit Integer in NVS.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `key` | `const char *` | Key name |
| `value` | `uint32_t` | 32-bit unsigned integer |

---

{#bsp_nvs_get_u32}

### bsp_nvs_get_u32

```cpp
esp_err_t bsp_nvs_get_u32(const char * key, uint32_t * out_val)
```

Defined in bsp/bsp_nvs.h:80

Retrieve an Unsigned 32-bit Integer from NVS.

#### Returns
esp_err_t ESP_OK on success, or ESP_ERR_NVS_NOT_FOUND if key doesn't exist

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `key` | `const char *` | Key name |
| `out_val` | `uint32_t *` | Pointer to receive 32-bit integer |

---

{#bsp_nvs_set_blob}

### bsp_nvs_set_blob

```cpp
esp_err_t bsp_nvs_set_blob(const char * key, const void * data, size_t length)
```

Defined in bsp/bsp_nvs.h:91

Store a Binary Blob in NVS.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `key` | `const char *` | Key name |
| `data` | `const void *` | Pointer to binary data |
| `length` | `size_t` | Length of data in bytes |

---

{#bsp_nvs_get_blob}

### bsp_nvs_get_blob

```cpp
esp_err_t bsp_nvs_get_blob(const char * key, void * out_data, size_t * length)
```

Defined in bsp/bsp_nvs.h:102

Retrieve a Binary Blob from NVS.

#### Returns
esp_err_t ESP_OK on success, or ESP_ERR_NVS_NOT_FOUND

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `key` | `const char *` | Key name |
| `out_data` | `void *` | Destination buffer |
| `length` | `size_t *` | Pointer to buffer size, updated with actual blob size |

---

{#bsp_nvs_erase_key}

### bsp_nvs_erase_key

```cpp
esp_err_t bsp_nvs_erase_key(const char * key)
```

Defined in bsp/bsp_nvs.h:111

Erase a specific key from NVS.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `key` | `const char *` | Key name to erase |

---

{#bsp_nvs_clear_wifi_credentials}

### bsp_nvs_clear_wifi_credentials

```cpp
esp_err_t bsp_nvs_clear_wifi_credentials(void)
```

Defined in bsp/bsp_nvs.h:119

Clear Stored Wi-Fi SSID and Password from NVS.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_nvs_wipe_all}

### bsp_nvs_wipe_all

```cpp
esp_err_t bsp_nvs_wipe_all(void)
```

Defined in bsp/bsp_nvs.h:129

Perform Complete Factory Wipe of All NVS Keys in Namespace.

Erases all configuration, claiming tokens, and boot history.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_ota_begin}

### bsp_ota_begin

```cpp
esp_err_t bsp_ota_begin(size_t image_size, esp_ota_handle_t * out_handle)
```

Defined in bsp/bsp_ota.h:65

Begin a Chunked OTA Update Session.

Selects next inactive OTA partition and initializes flash erase.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `image_size` | `size_t` | Expected total binary size in bytes (or OTA_SIZE_UNKNOWN) |
| `out_handle` | `esp_ota_handle_t *` | Destination pointer to receive OTA session handle |

---

{#bsp_ota_write}

### bsp_ota_write

```cpp
esp_err_t bsp_ota_write(esp_ota_handle_t handle, const void * data, size_t size)
```

Defined in bsp/bsp_ota.h:76

Write a Data Chunk to the Active OTA Session.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `handle` | `esp_ota_handle_t` | OTA session handle from [bsp_ota_begin()](#bsp_ota_begin) |
| `data` | `const void *` | Chunk buffer pointer |
| `size` | `size_t` | Chunk length in bytes |

---

{#bsp_ota_end}

### bsp_ota_end

```cpp
esp_err_t bsp_ota_end(esp_ota_handle_t handle)
```

Defined in bsp/bsp_ota.h:85

Finalize OTA Session, Validate Header, and Set Boot Partition.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `handle` | `esp_ota_handle_t` | OTA session handle |

---

{#bsp_ota_abort}

### bsp_ota_abort

```cpp
esp_err_t bsp_ota_abort(esp_ota_handle_t handle)
```

Defined in bsp/bsp_ota.h:94

Abort an in-progress OTA Session.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `handle` | `esp_ota_handle_t` | OTA session handle |

---

{#bsp_ota_from_url}

### bsp_ota_from_url

```cpp
esp_err_t bsp_ota_from_url(const char * url, bsp_ota_progress_cb_t cb, void * user_data)
```

Defined in bsp/bsp_ota.h:107

Download and Flash Firmware directly from HTTPS URL.

Uses system TLS certificate bundle and streams chunked download.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `url` | `const char *` | HTTPS URL to firmware .bin image |
| `cb` | [`bsp_ota_progress_cb_t`](#bsp_ota_progress_cb_t) | Progress callback pointer |
| `user_data` | `void *` | Optional user context |

---

{#bsp_ota_mark_valid}

### bsp_ota_mark_valid

```cpp
esp_err_t bsp_ota_mark_valid(void)
```

Defined in bsp/bsp_ota.h:115

Mark the currently running firmware partition as valid (prevents auto-rollback).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_ota_rollback}

### bsp_ota_rollback

```cpp
esp_err_t bsp_ota_rollback(void)
```

Defined in bsp/bsp_ota.h:123

Mark current firmware invalid and trigger rollback to previous working slot.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_ota_get_app_desc}

### bsp_ota_get_app_desc

```cpp
const esp_app_desc_t * bsp_ota_get_app_desc(void)
```

Defined in bsp/bsp_ota.h:131

Retrieve application descriptor for the currently running image.

#### Returns
const esp_app_desc_t* Pointer to app description (version, project name, compile time)

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_rtc_init}

### bsp_rtc_init

```cpp
esp_err_t bsp_rtc_init(void)
```

Defined in bsp/bsp_rtc.h:62

Initialize the PCF85063A RTC and configure the INT GPIO.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

---

{#bsp_rtc_deinit}

### bsp_rtc_deinit

```cpp
esp_err_t bsp_rtc_deinit(void)
```

Defined in bsp/bsp_rtc.h:69

Deinitialize RTC handle and release bus resources.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

---

{#bsp_rtc_software_reset}

### bsp_rtc_software_reset

```cpp
esp_err_t bsp_rtc_software_reset(void)
```

Defined in bsp/bsp_rtc.h:76

Perform a software reset on the PCF85063A (Command 0x58).

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

---

{#bsp_rtc_is_running}

### bsp_rtc_is_running

```cpp
esp_err_t bsp_rtc_is_running(bool * is_running)
```

Defined in bsp/bsp_rtc.h:84

Check if the oscillator is running and time integrity is guaranteed.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `is_running` | `bool *` | true if running, false if power loss/stopped (OS flag set). |

---

{#bsp_rtc_get_datetime}

### bsp_rtc_get_datetime

```cpp
esp_err_t bsp_rtc_get_datetime(bsp_rtc_datetime_t * datetime)
```

Defined in bsp/bsp_rtc.h:92

Read current date and time from the RTC in a single atomic transaction.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `datetime` | [`bsp_rtc_datetime_t`](#bsp_rtc_datetime_t) * | Pointer to datetime struct. |

---

{#bsp_rtc_set_datetime}

### bsp_rtc_set_datetime

```cpp
esp_err_t bsp_rtc_set_datetime(const bsp_rtc_datetime_t * datetime)
```

Defined in bsp/bsp_rtc.h:100

Set current date and time on the RTC and clear the OS (Oscillator Stop) flag.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `datetime` | const [`bsp_rtc_datetime_t`](#bsp_rtc_datetime_t) * | Pointer to datetime struct. |

---

{#bsp_rtc_set_offset}

### bsp_rtc_set_offset

```cpp
esp_err_t bsp_rtc_set_offset(int8_t offset, bsp_rtc_offset_mode_t mode)
```

Defined in bsp/bsp_rtc.h:109

Configure the PCF85063A offset calibration register.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `offset` | `int8_t` | Signed step count (-64 to +63). |
| `mode` | [`bsp_rtc_offset_mode_t`](#bsp_rtc_offset_mode_t) | Correction interval mode (2 hours or 4 minutes). |

---

{#bsp_rtc_set_alarm}

### bsp_rtc_set_alarm

```cpp
esp_err_t bsp_rtc_set_alarm(const bsp_rtc_alarm_t * alarm)
```

Defined in bsp/bsp_rtc.h:117

Configure the PCF85063A hardware alarm.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `alarm` | const [`bsp_rtc_alarm_t`](#bsp_rtc_alarm_t) * | Target alarm struct (pass -1 to ignore any field). |

---

{#bsp_rtc_clear_alarm}

### bsp_rtc_clear_alarm

```cpp
esp_err_t bsp_rtc_clear_alarm(void)
```

Defined in bsp/bsp_rtc.h:124

Disable and clear the RTC alarm.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

---

{#bsp_rtc_set_countdown_timer}

### bsp_rtc_set_countdown_timer

```cpp
esp_err_t bsp_rtc_set_countdown_timer(uint8_t seconds)
```

Defined in bsp/bsp_rtc.h:132

Configure the PCF85063A 1Hz periodic countdown timer.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `seconds` | `uint8_t` | Countdown value (1 to 255 seconds). |

---

{#bsp_rtc_clear_countdown_timer}

### bsp_rtc_clear_countdown_timer

```cpp
esp_err_t bsp_rtc_clear_countdown_timer(void)
```

Defined in bsp/bsp_rtc.h:139

Disable the countdown timer.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

---

{#bsp_rtc_get_and_clear_interrupts}

### bsp_rtc_get_and_clear_interrupts

```cpp
esp_err_t bsp_rtc_get_and_clear_interrupts(bool * alarm_flag, bool * timer_flag)
```

Defined in bsp/bsp_rtc.h:148

Read and clear interrupt flags (AF / TF) while preserving control registers.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `alarm_flag` | `bool *` | True if an alarm generated the interrupt. |
| `timer_flag` | `bool *` | True if the timer generated the interrupt. |

---

{#bsp_rtc_disable_clkout}

### bsp_rtc_disable_clkout

```cpp
esp_err_t bsp_rtc_disable_clkout(void)
```

Defined in bsp/bsp_rtc.h:158

Disable CLKOUT square wave output on PCF85063A (COF = 0x07).

Ensures external CLKOUT output pin is high-impedance to eliminate noise and save power.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_rtc_stop_oscillator}

### bsp_rtc_stop_oscillator

```cpp
esp_err_t bsp_rtc_stop_oscillator(void)
```

Defined in bsp/bsp_rtc.h:168

Stop the PCF85063A 32.768 kHz quartz crystal oscillator.

Sets STOP=1 in Control_1 register to halt the oscillator and divider chain.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_rtc_start_oscillator}

### bsp_rtc_start_oscillator

```cpp
esp_err_t bsp_rtc_start_oscillator(void)
```

Defined in bsp/bsp_rtc.h:178

Start/Resume the PCF85063A 32.768 kHz quartz crystal oscillator.

Sets STOP=0 in Control_1 register to resume active quartz timekeeping.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_rtc_enable_wakeup}

### bsp_rtc_enable_wakeup

```cpp
esp_err_t bsp_rtc_enable_wakeup(bool deep_sleep)
```

Defined in bsp/bsp_rtc.h:186

Configure ESP32-S3 sleep wakeup source from the RTC INT line (GPIO 5).

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `deep_sleep` | `bool` | true for Deep Sleep (EXT1), false for Light Sleep (GPIO wakeup). |

---

{#bsp_rtc_ram_read}

### bsp_rtc_ram_read

```cpp
esp_err_t bsp_rtc_ram_read(uint8_t * val)
```

Defined in bsp/bsp_rtc.h:197

Read the PCF85063A 8-bit general storage RAM byte (Register 0x03).

Stays powered as long as battery rail is active.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `val` | `uint8_t *` | Destination pointer for byte value |

---

{#bsp_rtc_ram_write}

### bsp_rtc_ram_write

```cpp
esp_err_t bsp_rtc_ram_write(uint8_t val)
```

Defined in bsp/bsp_rtc.h:206

Write the PCF85063A 8-bit general storage RAM byte (Register 0x03).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `val` | `uint8_t` | Byte value to store |

---

{#get_or_create_dev_handle}

### get_or_create_dev_handle

`static`

```cpp
static esp_err_t get_or_create_dev_handle(uint8_t addr, i2c_master_dev_handle_t * out_handle)
```

Defined in bsp_i2c.c:42

---

{#bsp_i2c_init-1}

### bsp_i2c_init

```cpp
esp_err_t bsp_i2c_init(void)
```

Defined in bsp_i2c.c:82

Initialize Shared I2C Master Bus and Create Mutex Guard.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: thread-safe guarantees.

---

{#bsp_i2c_deinit-1}

### bsp_i2c_deinit

```cpp
esp_err_t bsp_i2c_deinit(void)
```

Defined in bsp_i2c.c:154

De-initialize Shared I2C Master Bus.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_i2c_get_handle-1}

### bsp_i2c_get_handle

```cpp
i2c_master_bus_handle_t bsp_i2c_get_handle(void)
```

Defined in bsp_i2c.c:182

Get the underlying I2C master bus handle.

#### Returns
i2c_master_bus_handle_t Bus handle or NULL

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_i2c_add_device-1}

### bsp_i2c_add_device

```cpp
esp_err_t bsp_i2c_add_device(const i2c_device_config_t * dev_cfg, i2c_master_dev_handle_t * dev_handle)
```

Defined in bsp_i2c.c:190

Add a device to the shared I2C master bus.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `dev_cfg` | `const i2c_device_config_t *` | Device configuration struct |
| `dev_handle` | `i2c_master_dev_handle_t *` | Destination pointer to receive device handle |

---

{#bsp_i2c_write-1}

### bsp_i2c_write

```cpp
esp_err_t bsp_i2c_write(uint8_t addr, const uint8_t * data, size_t len)
```

Defined in bsp_i2c.c:200

Write Raw Bytes to an I2C Slave Device (Thread-Safe).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: thread-safe guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `addr` | `uint8_t` | 7-bit slave device address |
| `data` | `const uint8_t *` | Data buffer to transmit |
| `len` | `size_t` | Number of bytes to transmit |

---

{#bsp_i2c_read-1}

### bsp_i2c_read

```cpp
esp_err_t bsp_i2c_read(uint8_t addr, uint8_t * data, size_t len)
```

Defined in bsp_i2c.c:220

Read Raw Bytes from an I2C Slave Device (Thread-Safe).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: thread-safe guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `addr` | `uint8_t` | 7-bit slave device address |
| `data` | `uint8_t *` | Buffer to receive incoming bytes |
| `len` | `size_t` | Number of bytes to read |

---

{#bsp_i2c_write_reg-1}

### bsp_i2c_write_reg

```cpp
esp_err_t bsp_i2c_write_reg(uint8_t addr, uint8_t reg, const uint8_t * data, size_t len)
```

Defined in bsp_i2c.c:240

Write Bytes to a Specific 8-bit Register on an I2C Slave (Thread-Safe).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: thread-safe guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `addr` | `uint8_t` | 7-bit slave device address |
| `reg` | `uint8_t` | 8-bit register address |
| `data` | `const uint8_t *` | Data buffer to write (can be NULL if len == 0) |
| `len` | `size_t` | Number of data bytes |

---

{#bsp_i2c_read_reg-1}

### bsp_i2c_read_reg

```cpp
esp_err_t bsp_i2c_read_reg(uint8_t addr, uint8_t reg, uint8_t * data, size_t len)
```

Defined in bsp_i2c.c:259

Read Bytes from a Specific 8-bit Register on an I2C Slave (Thread-Safe).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: thread-safe guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `addr` | `uint8_t` | 7-bit slave device address |
| `reg` | `uint8_t` | 8-bit register address |
| `data` | `uint8_t *` | Buffer to receive data bytes |
| `len` | `size_t` | Number of bytes to read |

---

{#bsp_i2c_probe-1}

### bsp_i2c_probe

```cpp
esp_err_t bsp_i2c_probe(uint8_t addr)
```

Defined in bsp_i2c.c:279

Probe whether an I2C slave responds on the bus.

#### Returns
esp_err_t ESP_OK if ACKed, error otherwise

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `addr` | `uint8_t` | 7-bit slave address |

---

{#bsp_nvs_init-1}

### bsp_nvs_init

```cpp
esp_err_t bsp_nvs_init(void)
```

Defined in bsp_nvs.c:27

Initialize Non-Volatile Storage (NVS) Subsystem.

Automatically recovers and re-initializes if the partition table is truncated or empty.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_nvs_set_str-1}

### bsp_nvs_set_str

```cpp
esp_err_t bsp_nvs_set_str(const char * key, const char * value)
```

Defined in bsp_nvs.c:49

Store a String Value in NVS.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `key` | `const char *` | Key name (up to 15 characters) |
| `value` | `const char *` | Null-terminated string value |

---

{#bsp_nvs_get_str-1}

### bsp_nvs_get_str

```cpp
esp_err_t bsp_nvs_get_str(const char * key, char * out_val, size_t max_len)
```

Defined in bsp_nvs.c:67

Retrieve a String Value from NVS.

#### Returns
esp_err_t ESP_OK on success, or ESP_ERR_NVS_NOT_FOUND if key doesn't exist

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `key` | `const char *` | Key name |
| `out_val` | `char *` | Destination character buffer |
| `max_len` | `size_t` | Size of buffer |

---

{#bsp_nvs_set_u32-1}

### bsp_nvs_set_u32

```cpp
esp_err_t bsp_nvs_set_u32(const char * key, uint32_t value)
```

Defined in bsp_nvs.c:83

Store an Unsigned 32-bit Integer in NVS.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `key` | `const char *` | Key name |
| `value` | `uint32_t` | 32-bit unsigned integer |

---

{#bsp_nvs_get_u32-1}

### bsp_nvs_get_u32

```cpp
esp_err_t bsp_nvs_get_u32(const char * key, uint32_t * out_val)
```

Defined in bsp_nvs.c:101

Retrieve an Unsigned 32-bit Integer from NVS.

#### Returns
esp_err_t ESP_OK on success, or ESP_ERR_NVS_NOT_FOUND if key doesn't exist

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `key` | `const char *` | Key name |
| `out_val` | `uint32_t *` | Pointer to receive 32-bit integer |

---

{#bsp_nvs_set_blob-1}

### bsp_nvs_set_blob

```cpp
esp_err_t bsp_nvs_set_blob(const char * key, const void * data, size_t length)
```

Defined in bsp_nvs.c:116

Store a Binary Blob in NVS.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `key` | `const char *` | Key name |
| `data` | `const void *` | Pointer to binary data |
| `length` | `size_t` | Length of data in bytes |

---

{#bsp_nvs_get_blob-1}

### bsp_nvs_get_blob

```cpp
esp_err_t bsp_nvs_get_blob(const char * key, void * out_data, size_t * length)
```

Defined in bsp_nvs.c:134

Retrieve a Binary Blob from NVS.

#### Returns
esp_err_t ESP_OK on success, or ESP_ERR_NVS_NOT_FOUND

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `key` | `const char *` | Key name |
| `out_data` | `void *` | Destination buffer |
| `length` | `size_t *` | Pointer to buffer size, updated with actual blob size |

---

{#bsp_nvs_erase_key-1}

### bsp_nvs_erase_key

```cpp
esp_err_t bsp_nvs_erase_key(const char * key)
```

Defined in bsp_nvs.c:149

Erase a specific key from NVS.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `key` | `const char *` | Key name to erase |

---

{#bsp_nvs_clear_wifi_credentials-1}

### bsp_nvs_clear_wifi_credentials

```cpp
esp_err_t bsp_nvs_clear_wifi_credentials(void)
```

Defined in bsp_nvs.c:167

Clear Stored Wi-Fi SSID and Password from NVS.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_nvs_wipe_all-1}

### bsp_nvs_wipe_all

```cpp
esp_err_t bsp_nvs_wipe_all(void)
```

Defined in bsp_nvs.c:184

Perform Complete Factory Wipe of All NVS Keys in Namespace.

Erases all configuration, claiming tokens, and boot history.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_ota_begin-1}

### bsp_ota_begin

```cpp
esp_err_t bsp_ota_begin(size_t image_size, esp_ota_handle_t * out_handle)
```

Defined in bsp_ota.c:26

Begin a Chunked OTA Update Session.

Selects next inactive OTA partition and initializes flash erase.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `image_size` | `size_t` | Expected total binary size in bytes (or OTA_SIZE_UNKNOWN) |
| `out_handle` | `esp_ota_handle_t *` | Destination pointer to receive OTA session handle |

---

{#bsp_ota_write-1}

### bsp_ota_write

```cpp
esp_err_t bsp_ota_write(esp_ota_handle_t handle, const void * data, size_t size)
```

Defined in bsp_ota.c:51

Write a Data Chunk to the Active OTA Session.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `handle` | `esp_ota_handle_t` | OTA session handle from [bsp_ota_begin()](#bsp_ota_begin) |
| `data` | `const void *` | Chunk buffer pointer |
| `size` | `size_t` | Chunk length in bytes |

---

{#bsp_ota_end-1}

### bsp_ota_end

```cpp
esp_err_t bsp_ota_end(esp_ota_handle_t handle)
```

Defined in bsp_ota.c:59

Finalize OTA Session, Validate Header, and Set Boot Partition.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `handle` | `esp_ota_handle_t` | OTA session handle |

---

{#bsp_ota_abort-1}

### bsp_ota_abort

```cpp
esp_err_t bsp_ota_abort(esp_ota_handle_t handle)
```

Defined in bsp_ota.c:78

Abort an in-progress OTA Session.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `handle` | `esp_ota_handle_t` | OTA session handle |

---

{#bsp_ota_from_url-1}

### bsp_ota_from_url

```cpp
esp_err_t bsp_ota_from_url(const char * url, bsp_ota_progress_cb_t cb, void * user_data)
```

Defined in bsp_ota.c:84

Download and Flash Firmware directly from HTTPS URL.

Uses system TLS certificate bundle and streams chunked download.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `url` | `const char *` | HTTPS URL to firmware .bin image |
| `cb` | [`bsp_ota_progress_cb_t`](#bsp_ota_progress_cb_t) | Progress callback pointer |
| `user_data` | `void *` | Optional user context |

---

{#bsp_ota_mark_valid-1}

### bsp_ota_mark_valid

```cpp
esp_err_t bsp_ota_mark_valid(void)
```

Defined in bsp_ota.c:145

Mark the currently running firmware partition as valid (prevents auto-rollback).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_ota_rollback-1}

### bsp_ota_rollback

```cpp
esp_err_t bsp_ota_rollback(void)
```

Defined in bsp_ota.c:154

Mark current firmware invalid and trigger rollback to previous working slot.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_ota_get_app_desc-1}

### bsp_ota_get_app_desc

```cpp
const esp_app_desc_t * bsp_ota_get_app_desc(void)
```

Defined in bsp_ota.c:160

Retrieve application descriptor for the currently running image.

#### Returns
const esp_app_desc_t* Pointer to app description (version, project name, compile time)

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bcd_to_binary}

### bcd_to_binary

`static` `inline`

```cpp
static inline uint8_t bcd_to_binary(uint8_t val)
```

Defined in bsp_rtc.c:85

---

{#binary_to_bcd}

### binary_to_bcd

`static` `inline`

```cpp
static inline uint8_t binary_to_bcd(uint8_t val)
```

Defined in bsp_rtc.c:90

---

{#datetime_is_valid}

### datetime_is_valid

`static`

```cpp
static bool datetime_is_valid(const bsp_rtc_datetime_t * datetime)
```

Defined in bsp_rtc.c:95

---

{#bsp_rtc_init-1}

### bsp_rtc_init

```cpp
esp_err_t bsp_rtc_init(void)
```

Defined in bsp_rtc.c:106

Initialize the PCF85063A RTC and configure the INT GPIO.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

---

{#bsp_rtc_deinit-1}

### bsp_rtc_deinit

```cpp
esp_err_t bsp_rtc_deinit(void)
```

Defined in bsp_rtc.c:152

Deinitialize RTC handle and release bus resources.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

---

{#bsp_rtc_software_reset-1}

### bsp_rtc_software_reset

```cpp
esp_err_t bsp_rtc_software_reset(void)
```

Defined in bsp_rtc.c:157

Perform a software reset on the PCF85063A (Command 0x58).

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

---

{#bsp_rtc_is_running-1}

### bsp_rtc_is_running

```cpp
esp_err_t bsp_rtc_is_running(bool * is_running)
```

Defined in bsp_rtc.c:163

Check if the oscillator is running and time integrity is guaranteed.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `is_running` | `bool *` | true if running, false if power loss/stopped (OS flag set). |

---

{#bsp_rtc_get_datetime-1}

### bsp_rtc_get_datetime

```cpp
esp_err_t bsp_rtc_get_datetime(bsp_rtc_datetime_t * datetime)
```

Defined in bsp_rtc.c:176

Read current date and time from the RTC in a single atomic transaction.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `datetime` | [`bsp_rtc_datetime_t`](#bsp_rtc_datetime_t) * | Pointer to datetime struct. |

---

{#bsp_rtc_set_datetime-1}

### bsp_rtc_set_datetime

```cpp
esp_err_t bsp_rtc_set_datetime(const bsp_rtc_datetime_t * datetime)
```

Defined in bsp_rtc.c:200

Set current date and time on the RTC and clear the OS (Oscillator Stop) flag.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `datetime` | const [`bsp_rtc_datetime_t`](#bsp_rtc_datetime_t) * | Pointer to datetime struct. |

---

{#bsp_rtc_set_offset-1}

### bsp_rtc_set_offset

```cpp
esp_err_t bsp_rtc_set_offset(int8_t offset, bsp_rtc_offset_mode_t mode)
```

Defined in bsp_rtc.c:232

Configure the PCF85063A offset calibration register.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `offset` | `int8_t` | Signed step count (-64 to +63). |
| `mode` | [`bsp_rtc_offset_mode_t`](#bsp_rtc_offset_mode_t) | Correction interval mode (2 hours or 4 minutes). |

---

{#bsp_rtc_set_alarm-1}

### bsp_rtc_set_alarm

```cpp
esp_err_t bsp_rtc_set_alarm(const bsp_rtc_alarm_t * alarm)
```

Defined in bsp_rtc.c:243

Configure the PCF85063A hardware alarm.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `alarm` | const [`bsp_rtc_alarm_t`](#bsp_rtc_alarm_t) * | Target alarm struct (pass -1 to ignore any field). |

---

{#bsp_rtc_clear_alarm-1}

### bsp_rtc_clear_alarm

```cpp
esp_err_t bsp_rtc_clear_alarm(void)
```

Defined in bsp_rtc.c:266

Disable and clear the RTC alarm.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

---

{#bsp_rtc_set_countdown_timer-1}

### bsp_rtc_set_countdown_timer

```cpp
esp_err_t bsp_rtc_set_countdown_timer(uint8_t seconds)
```

Defined in bsp_rtc.c:277

Configure the PCF85063A 1Hz periodic countdown timer.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `seconds` | `uint8_t` | Countdown value (1 to 255 seconds). |

---

{#bsp_rtc_clear_countdown_timer-1}

### bsp_rtc_clear_countdown_timer

```cpp
esp_err_t bsp_rtc_clear_countdown_timer(void)
```

Defined in bsp_rtc.c:299

Disable the countdown timer.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

---

{#bsp_rtc_get_and_clear_interrupts-1}

### bsp_rtc_get_and_clear_interrupts

```cpp
esp_err_t bsp_rtc_get_and_clear_interrupts(bool * alarm_flag, bool * timer_flag)
```

Defined in bsp_rtc.c:310

Read and clear interrupt flags (AF / TF) while preserving control registers.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `alarm_flag` | `bool *` | True if an alarm generated the interrupt. |
| `timer_flag` | `bool *` | True if the timer generated the interrupt. |

---

{#bsp_rtc_disable_clkout-1}

### bsp_rtc_disable_clkout

```cpp
esp_err_t bsp_rtc_disable_clkout(void)
```

Defined in bsp_rtc.c:324

Disable CLKOUT square wave output on PCF85063A (COF = 0x07).

Ensures external CLKOUT output pin is high-impedance to eliminate noise and save power.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_rtc_stop_oscillator-1}

### bsp_rtc_stop_oscillator

```cpp
esp_err_t bsp_rtc_stop_oscillator(void)
```

Defined in bsp_rtc.c:335

Stop the PCF85063A 32.768 kHz quartz crystal oscillator.

Sets STOP=1 in Control_1 register to halt the oscillator and divider chain.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_rtc_start_oscillator-1}

### bsp_rtc_start_oscillator

```cpp
esp_err_t bsp_rtc_start_oscillator(void)
```

Defined in bsp_rtc.c:349

Start/Resume the PCF85063A 32.768 kHz quartz crystal oscillator.

Sets STOP=0 in Control_1 register to resume active quartz timekeeping.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_rtc_enable_wakeup-1}

### bsp_rtc_enable_wakeup

```cpp
esp_err_t bsp_rtc_enable_wakeup(bool deep_sleep)
```

Defined in bsp_rtc.c:364

Configure ESP32-S3 sleep wakeup source from the RTC INT line (GPIO 5).

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `deep_sleep` | `bool` | true for Deep Sleep (EXT1), false for Light Sleep (GPIO wakeup). |

---

{#bsp_rtc_ram_read-1}

### bsp_rtc_ram_read

```cpp
esp_err_t bsp_rtc_ram_read(uint8_t * val)
```

Defined in bsp_rtc.c:377

Read the PCF85063A 8-bit general storage RAM byte (Register 0x03).

Stays powered as long as battery rail is active.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `val` | `uint8_t *` | Destination pointer for byte value |

---

{#bsp_rtc_ram_write-1}

### bsp_rtc_ram_write

```cpp
esp_err_t bsp_rtc_ram_write(uint8_t val)
```

Defined in bsp_rtc.c:383

Write the PCF85063A 8-bit general storage RAM byte (Register 0x03).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `val` | `uint8_t` | Byte value to store |

---

{#bsp_lvgl_init}

### bsp_lvgl_init

```cpp
esp_err_t bsp_lvgl_init(void)
```

Defined in bsp/bsp_lvgl.h:39

Initialize LVGL v9 Graphics Subsystem and Register Display Port.

Allocates draw buffers and registers the SSD1681 1-bit monochrome flush callback.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_lvgl_start}

### bsp_lvgl_start

```cpp
esp_err_t bsp_lvgl_start(int priority, int core_id)
```

Defined in bsp/bsp_lvgl.h:49

Start LVGL Background FreeRTOS Execution Task.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `priority` | `int` | Task priority (Default: 5) |
| `core_id` | `int` | CPU Core affinity (Default: 1 - Core 1) |

---

{#bsp_lvgl_stop}

### bsp_lvgl_stop

```cpp
esp_err_t bsp_lvgl_stop(void)
```

Defined in bsp/bsp_lvgl.h:57

Stop LVGL Background FreeRTOS Execution Task.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_lvgl_lock}

### bsp_lvgl_lock

```cpp
bool bsp_lvgl_lock(void)
```

Defined in bsp/bsp_lvgl.h:67

Acquire LVGL Reentrant Mutex Lock.

Must be called prior to modifying any UI widgets or invoking lvgl functions from external tasks.

#### Returns
true if mutex was successfully acquired

Memory ownership: none. Behavior: Blocking. Thread safety: thread-safe guarantees.

---

{#bsp_lvgl_unlock}

### bsp_lvgl_unlock

```cpp
void bsp_lvgl_unlock(void)
```

Defined in bsp/bsp_lvgl.h:73

Release LVGL Reentrant Mutex Lock.

Memory ownership: none. Behavior: Blocking. Thread safety: thread-safe guarantees.

---

{#bsp_lvgl_lock_isr}

### bsp_lvgl_lock_isr

```cpp
bool bsp_lvgl_lock_isr(void)
```

Defined in bsp/bsp_lvgl.h:81

Acquire LVGL Reentrant Mutex Lock from ISR Context.

#### Returns
true if mutex was successfully acquired from ISR

Memory ownership: none. Behavior: Non-blocking. Thread safety: ISR-safe guarantees.

---

{#bsp_lvgl_set_first_flush_mode}

### bsp_lvgl_set_first_flush_mode

```cpp
void bsp_lvgl_set_first_flush_mode(bool full_refresh)
```

Defined in bsp/bsp_lvgl.h:89

Configure whether the next LVGL display flush performs a full OTP refresh or fast partial update.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `full_refresh` | `bool` | true for full OTP clear/refresh (e.g. on cold boot), false for fast partial refresh (e.g. on wake) |

---

{#bsp_prov_start}

### bsp_prov_start

```cpp
esp_err_t bsp_prov_start(const char * custom_service_name, const char * pop, bsp_prov_event_cb_t cb, void * user_data)
```

Defined in bsp/bsp_prov.h:62

Start BLE GATT Wi-Fi Provisioning Service.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `custom_service_name` | `const char *` | Optional custom name (pass NULL for default "PROV_ESP32S3-XXXX") |
| `pop` | `const char *` | Optional Proof-of-Possession string (pass NULL for open pairing) |
| `cb` | [`bsp_prov_event_cb_t`](#bsp_prov_event_cb_t) | Event callback function pointer |
| `user_data` | `void *` | Optional user context |

---

{#bsp_prov_is_running}

### bsp_prov_is_running

```cpp
bool bsp_prov_is_running(void)
```

Defined in bsp/bsp_prov.h:70

Check if Provisioning is currently active.

#### Returns
true if BLE is advertising/provisioning

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_prov_stop}

### bsp_prov_stop

```cpp
void bsp_prov_stop(void)
```

Defined in bsp/bsp_prov.h:76

Stop Provisioning Service and release Bluetooth memory.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_prov_get_qr_payload}

### bsp_prov_get_qr_payload

```cpp
esp_err_t bsp_prov_get_qr_payload(const char * pop, char * dest, size_t max_len)
```

Defined in bsp/bsp_prov.h:87

Generate JSON QR Code Payload string for standard Espressif Provisioning apps.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `pop` | `const char *` | Proof-of-Possession PIN (or NULL) |
| `dest` | `char *` | Destination string buffer |
| `max_len` | `size_t` | Buffer size |

---

{#bsp_prov_calc_qr_code_size}

### bsp_prov_calc_qr_code_size

```cpp
int32_t bsp_prov_calc_qr_code_size(const char * pop, int32_t max_boundary, int32_t min_scale)
```

Defined in bsp/bsp_prov.h:101

Calculate optimal QR code canvas size for a given payload and constraints.

Calculates the smallest QR version required for the payload string, then multiplies by an integer scale factor (pixels per module) to compute an exact whole-factor canvas size. This guarantees 1:1, 2:2, 3:3, etc. integer pixel scaling without fractional distortion.

#### Returns
int32_t Optimal whole-factor canvas pixel size

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `pop` | `const char *` | Proof-of-Possession PIN (or NULL) |
| `max_boundary` | `int32_t` | Maximum display boundary size in pixels (pass <= 0 for default 180) |
| `min_scale` | `int32_t` | Minimum allowed pixels per QR module (pass <= 0 for default 3) |

---

{#bsp_prov_get_qr_raw_bits}

### bsp_prov_get_qr_raw_bits

```cpp
esp_err_t bsp_prov_get_qr_raw_bits(const char * pop, int32_t max_boundary, int32_t min_scale, uint8_t * out_buf, size_t out_buf_size, int32_t * out_size)
```

Defined in bsp/bsp_prov.h:114

Generate raw 1-bit packed monochrome bitmap byte array for QR payload.

#### Returns
esp_err_t ESP_OK on success

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `pop` | `const char *` | Proof-of-Possession PIN (or NULL) |
| `max_boundary` | `int32_t` | Target bounding dimension in pixels |
| `min_scale` | `int32_t` | Minimum integer scale factor |
| `out_buf` | `uint8_t *` | Destination buffer for 1-bit packed bitmap |
| `out_buf_size` | `size_t` | Size of out_buf in bytes |
| `out_size` | `int32_t *` | Output width/height in pixels |

---

{#bsp_prov_render_qr_code}

### bsp_prov_render_qr_code

```cpp
lv_obj_t * bsp_prov_render_qr_code(lv_obj_t * parent, int32_t size, const char * pop)
```

Defined in bsp/bsp_prov.h:128

Render Provisioning QR Code directly on LVGL Screen / Canvas.

Renders a high-contrast 1-bit QR code centered on screen, automatically snapped to an exact whole-factor scale.

#### Returns
lv_obj_t* Pointer to created QR code widget or NULL

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `parent` | `lv_obj_t *` | Parent LVGL object (e.g. lv_screen_active()) |
| `size` | `int32_t` | Dimension in pixels or max boundary (pass <= 0 for auto 180px max) |
| `pop` | `const char *` | Proof-of-Possession PIN (or NULL) |

---

{#bsp_time_set_timezone}

### bsp_time_set_timezone

```cpp
esp_err_t bsp_time_set_timezone(const char * tz_str)
```

Defined in bsp/bsp_time.h:67

Configure System Timezone String.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `tz_str` | `const char *` | Standard POSIX Timezone string (e.g. "EST5EDT,M3.2.0,M11.1.0" or "UTC") |

---

{#bsp_time_sntp_sync}

### bsp_time_sntp_sync

```cpp
esp_err_t bsp_time_sntp_sync(uint32_t timeout_ms)
```

Defined in bsp/bsp_time.h:79

Perform SNTP Network Time Synchronization.

Connects to NTP server (pool.ntp.org), sets ESP32 system clock, and synchronizes the external PCF85063A RTC hardware registers.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `timeout_ms` | `uint32_t` | Maximum time to wait for NTP response |

---

{#bsp_time_sync_system_to_rtc}

### bsp_time_sync_system_to_rtc

```cpp
esp_err_t bsp_time_sync_system_to_rtc(void)
```

Defined in bsp/bsp_time.h:87

Synchronize internal ESP32 system time into external PCF85063A RTC.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_time_sync_rtc_to_system}

### bsp_time_sync_rtc_to_system

```cpp
esp_err_t bsp_time_sync_rtc_to_system(void)
```

Defined in bsp/bsp_time.h:95

Synchronize external PCF85063A RTC hardware time into internal ESP32 system time.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_time_get_formatted}

### bsp_time_get_formatted

```cpp
esp_err_t bsp_time_get_formatted(bsp_time_format_t fmt, char * dest, size_t max_len)
```

Defined in bsp/bsp_time.h:106

Format Current Local Time into Specified String Format.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `fmt` | [`bsp_time_format_t`](#bsp_time_format_t) | Target [bsp_time_format_t](#bsp_time_format_t) |
| `dest` | `char *` | Destination string buffer |
| `max_len` | `size_t` | Buffer size |

---

{#bsp_time_get_date_formatted}

### bsp_time_get_date_formatted

```cpp
esp_err_t bsp_time_get_date_formatted(bsp_date_format_t fmt, char * dest, size_t max_len)
```

Defined in bsp/bsp_time.h:117

Format Current Local Date into Specified Date Format.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `fmt` | [`bsp_date_format_t`](#bsp_date_format_t) | Target [bsp_date_format_t](#bsp_date_format_t) |
| `dest` | `char *` | Destination string buffer |
| `max_len` | `size_t` | Buffer size (minimum 24 bytes recommended) |

---

{#bsp_time_get_date_str}

### bsp_time_get_date_str

```cpp
esp_err_t bsp_time_get_date_str(char * dest, size_t max_len)
```

Defined in bsp/bsp_time.h:127

Format Current Date String as "MM/DD/YY" (e.g.

"09/26/26")

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `dest` | `char *` | Destination string buffer |
| `max_len` | `size_t` | Buffer size (minimum 16 bytes recommended) |

---

{#bsp_time_get_dow_str}

### bsp_time_get_dow_str

```cpp
esp_err_t bsp_time_get_dow_str(char * dest, size_t max_len)
```

Defined in bsp/bsp_time.h:137

Format Current Day-of-Week String as "DayOfWeek" (e.g.

"Saturday")

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `dest` | `char *` | Destination string buffer |
| `max_len` | `size_t` | Buffer size (minimum 16 bytes recommended) |

---

{#bsp_time_get_date_dow_str}

### bsp_time_get_date_dow_str

```cpp
esp_err_t bsp_time_get_date_dow_str(char * dest, size_t max_len)
```

Defined in bsp/bsp_time.h:147

Format Current Date and Day-of-Week String as "MM/DD/YY DayOfWeek" (e.g.

"09/26/26 Saturday")

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `dest` | `char *` | Destination string buffer |
| `max_len` | `size_t` | Buffer size (minimum 24 bytes recommended) |

---

{#bsp_time_start_periodic_sync}

### bsp_time_start_periodic_sync

```cpp
esp_err_t bsp_time_start_periodic_sync(uint32_t interval_sec)
```

Defined in bsp/bsp_time.h:156

Start Background Periodic SNTP Synchronization Timer (e.g., every 24 hours).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `interval_sec` | `uint32_t` | Synchronization interval in seconds (Default: 86400 / 24h) |

---

{#bsp_time_stop_periodic_sync}

### bsp_time_stop_periodic_sync

```cpp
void bsp_time_stop_periodic_sync(void)
```

Defined in bsp/bsp_time.h:162

Stop Background Periodic SNTP Timer.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_wifi_init}

### bsp_wifi_init

```cpp
esp_err_t bsp_wifi_init(void)
```

Defined in bsp/bsp_wifi.h:40

Initialize Wi-Fi Subsystem in Station Mode.

Sets up the default network interface (netif), initializes the L2 event loop, and prepares the driver for connection.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_wifi_connect}

### bsp_wifi_connect

```cpp
esp_err_t bsp_wifi_connect(const char * ssid, const char * password, uint32_t timeout_ms)
```

Defined in bsp/bsp_wifi.h:53

Connect to Wi-Fi Network with Specified SSID and Password.

Uses fast RTC cache if matching SSID was previously stored, otherwise performs full scan.

#### Returns
esp_err_t ESP_OK on successful IP acquisition, or error code on timeout/failure

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `ssid` | `const char *` | Target Access Point SSID |
| `password` | `const char *` | Network password (or empty string for open networks) |
| `timeout_ms` | `uint32_t` | Maximum time to wait for IP assignment in milliseconds |

---

{#bsp_wifi_connect_from_nvs}

### bsp_wifi_connect_from_nvs

```cpp
esp_err_t bsp_wifi_connect_from_nvs(uint32_t timeout_ms)
```

Defined in bsp/bsp_wifi.h:64

Connect to Wi-Fi using Credentials Stored in NVS Flash.

Reads "wifi_ssid" and "wifi_pass" from NVS. If missing or invalid, returns ESP_ERR_NVS_NOT_FOUND.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `timeout_ms` | `uint32_t` | Maximum time to wait for IP assignment |

---

{#bsp_wifi_disconnect}

### bsp_wifi_disconnect

```cpp
esp_err_t bsp_wifi_disconnect(void)
```

Defined in bsp/bsp_wifi.h:74

Disconnect and Stop Wi-Fi Station.

Cleanly terminates association and turns off RF circuitry prior to deep sleep.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_wifi_is_connected}

### bsp_wifi_is_connected

```cpp
bool bsp_wifi_is_connected(void)
```

Defined in bsp/bsp_wifi.h:82

Check if Wi-Fi is Currently Connected and has Valid IP Address.

#### Returns
true if connected with valid IP, false otherwise

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_wifi_get_rssi}

### bsp_wifi_get_rssi

```cpp
esp_err_t bsp_wifi_get_rssi(int * out_rssi)
```

Defined in bsp/bsp_wifi.h:91

Retrieve Current Received Signal Strength Indicator (RSSI).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `out_rssi` | `int *` | Pointer to receive RSSI value in dBm (e.g. -55 dBm) |

---

{#bsp_wifi_get_ip_str}

### bsp_wifi_get_ip_str

```cpp
esp_err_t bsp_wifi_get_ip_str(char * out_ip, size_t max_len)
```

Defined in bsp/bsp_wifi.h:101

Retrieve Formatted Local IPv4 Address String.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `out_ip` | `char *` | Destination character buffer |
| `max_len` | `size_t` | Size of buffer (minimum 16 bytes recommended) |

---

{#bsp_wifi_save_credentials}

### bsp_wifi_save_credentials

```cpp
esp_err_t bsp_wifi_save_credentials(const char * ssid, const char * password)
```

Defined in bsp/bsp_wifi.h:111

Save New Wi-Fi Credentials into NVS Flash.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `ssid` | `const char *` | AP SSID string |
| `password` | `const char *` | AP Password string |

---

{#bsp_wifi_scan}

### bsp_wifi_scan

```cpp
esp_err_t bsp_wifi_scan(void * ap_records, uint16_t * ap_count, uint16_t max_aps)
```

Defined in bsp/bsp_wifi.h:122

Scan for Nearby Wi-Fi Access Points.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `ap_records` | `void *` | Optional array to receive AP record metadata (or NULL) |
| `ap_count` | `uint16_t *` | Pointer to receive total number of discovered APs |
| `max_aps` | `uint16_t` | Maximum size of ap_records array (pass 0 if ap_records is NULL) |

---

{#bsp_wifi_invalidate_fast_cache}

### bsp_wifi_invalidate_fast_cache

```cpp
void bsp_wifi_invalidate_fast_cache(void)
```

Defined in bsp/bsp_wifi.h:128

Invalidate and Clear the RTC Fast Reconnect Session Cache.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#prov_event_handler}

### prov_event_handler

`static`

```cpp
static void prov_event_handler(void * user_data, esp_event_base_t event_base, int32_t event_id, void * event_data)
```

Defined in bsp_prov.c:36

---

{#bsp_prov_start-1}

### bsp_prov_start

```cpp
esp_err_t bsp_prov_start(const char * custom_service_name, const char * pop, bsp_prov_event_cb_t cb, void * user_data)
```

Defined in bsp_prov.c:79

Start BLE GATT Wi-Fi Provisioning Service.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `custom_service_name` | `const char *` | Optional custom name (pass NULL for default "PROV_ESP32S3-XXXX") |
| `pop` | `const char *` | Optional Proof-of-Possession string (pass NULL for open pairing) |
| `cb` | [`bsp_prov_event_cb_t`](#bsp_prov_event_cb_t) | Event callback function pointer |
| `user_data` | `void *` | Optional user context |

---

{#bsp_prov_is_running-1}

### bsp_prov_is_running

```cpp
bool bsp_prov_is_running(void)
```

Defined in bsp_prov.c:132

Check if Provisioning is currently active.

#### Returns
true if BLE is advertising/provisioning

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_prov_stop-1}

### bsp_prov_stop

```cpp
void bsp_prov_stop(void)
```

Defined in bsp_prov.c:137

Stop Provisioning Service and release Bluetooth memory.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_prov_get_qr_payload-1}

### bsp_prov_get_qr_payload

```cpp
esp_err_t bsp_prov_get_qr_payload(const char * pop, char * dest, size_t max_len)
```

Defined in bsp_prov.c:147

Generate JSON QR Code Payload string for standard Espressif Provisioning apps.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `pop` | `const char *` | Proof-of-Possession PIN (or NULL) |
| `dest` | `char *` | Destination string buffer |
| `max_len` | `size_t` | Buffer size |

---

{#bsp_prov_calc_qr_code_size-1}

### bsp_prov_calc_qr_code_size

```cpp
int32_t bsp_prov_calc_qr_code_size(const char * pop, int32_t max_boundary, int32_t min_scale)
```

Defined in bsp_prov.c:171

Calculate optimal QR code canvas size for a given payload and constraints.

Calculates the smallest QR version required for the payload string, then multiplies by an integer scale factor (pixels per module) to compute an exact whole-factor canvas size. This guarantees 1:1, 2:2, 3:3, etc. integer pixel scaling without fractional distortion.

#### Returns
int32_t Optimal whole-factor canvas pixel size

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `pop` | `const char *` | Proof-of-Possession PIN (or NULL) |
| `max_boundary` | `int32_t` | Maximum display boundary size in pixels (pass <= 0 for default 180) |
| `min_scale` | `int32_t` | Minimum allowed pixels per QR module (pass <= 0 for default 3) |

---

{#bsp_prov_get_qr_raw_bits-1}

### bsp_prov_get_qr_raw_bits

```cpp
esp_err_t bsp_prov_get_qr_raw_bits(const char * pop, int32_t max_boundary, int32_t min_scale, uint8_t * out_buf, size_t out_buf_size, int32_t * out_size)
```

Defined in bsp_prov.c:211

Generate raw 1-bit packed monochrome bitmap byte array for QR payload.

#### Returns
esp_err_t ESP_OK on success

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `pop` | `const char *` | Proof-of-Possession PIN (or NULL) |
| `max_boundary` | `int32_t` | Target bounding dimension in pixels |
| `min_scale` | `int32_t` | Minimum integer scale factor |
| `out_buf` | `uint8_t *` | Destination buffer for 1-bit packed bitmap |
| `out_buf_size` | `size_t` | Size of out_buf in bytes |
| `out_size` | `int32_t *` | Output width/height in pixels |

---

{#bsp_prov_render_qr_code-1}

### bsp_prov_render_qr_code

```cpp
lv_obj_t * bsp_prov_render_qr_code(lv_obj_t * parent, int32_t size, const char * pop)
```

Defined in bsp_prov.c:280

Render Provisioning QR Code directly on LVGL Screen / Canvas.

Renders a high-contrast 1-bit QR code centered on screen, automatically snapped to an exact whole-factor scale.

#### Returns
lv_obj_t* Pointer to created QR code widget or NULL

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `parent` | `lv_obj_t *` | Parent LVGL object (e.g. lv_screen_active()) |
| `size` | `int32_t` | Dimension in pixels or max boundary (pass <= 0 for auto 180px max) |
| `pop` | `const char *` | Proof-of-Possession PIN (or NULL) |

---

{#bsp_time_set_timezone-1}

### bsp_time_set_timezone

```cpp
esp_err_t bsp_time_set_timezone(const char * tz_str)
```

Defined in bsp_time.c:30

Configure System Timezone String.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `tz_str` | `const char *` | Standard POSIX Timezone string (e.g. "EST5EDT,M3.2.0,M11.1.0" or "UTC") |

---

{#bsp_time_sync_system_to_rtc-1}

### bsp_time_sync_system_to_rtc

```cpp
esp_err_t bsp_time_sync_system_to_rtc(void)
```

Defined in bsp_time.c:41

Synchronize internal ESP32 system time into external PCF85063A RTC.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_time_sync_rtc_to_system-1}

### bsp_time_sync_rtc_to_system

```cpp
esp_err_t bsp_time_sync_rtc_to_system(void)
```

Defined in bsp_time.c:68

Synchronize external PCF85063A RTC hardware time into internal ESP32 system time.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_time_sntp_sync-1}

### bsp_time_sntp_sync

```cpp
esp_err_t bsp_time_sntp_sync(uint32_t timeout_ms)
```

Defined in bsp_time.c:101

Perform SNTP Network Time Synchronization.

Connects to NTP server (pool.ntp.org), sets ESP32 system clock, and synchronizes the external PCF85063A RTC hardware registers.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `timeout_ms` | `uint32_t` | Maximum time to wait for NTP response |

---

{#bsp_time_get_formatted-1}

### bsp_time_get_formatted

```cpp
esp_err_t bsp_time_get_formatted(bsp_time_format_t fmt, char * dest, size_t max_len)
```

Defined in bsp_time.c:131

Format Current Local Time into Specified String Format.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `fmt` | [`bsp_time_format_t`](#bsp_time_format_t) | Target [bsp_time_format_t](#bsp_time_format_t) |
| `dest` | `char *` | Destination string buffer |
| `max_len` | `size_t` | Buffer size |

---

{#bsp_time_get_date_formatted-1}

### bsp_time_get_date_formatted

```cpp
esp_err_t bsp_time_get_date_formatted(bsp_date_format_t fmt, char * dest, size_t max_len)
```

Defined in bsp_time.c:162

Format Current Local Date into Specified Date Format.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `fmt` | [`bsp_date_format_t`](#bsp_date_format_t) | Target [bsp_date_format_t](#bsp_date_format_t) |
| `dest` | `char *` | Destination string buffer |
| `max_len` | `size_t` | Buffer size (minimum 24 bytes recommended) |

---

{#bsp_time_get_date_str-1}

### bsp_time_get_date_str

```cpp
esp_err_t bsp_time_get_date_str(char * dest, size_t max_len)
```

Defined in bsp_time.c:190

Format Current Date String as "MM/DD/YY" (e.g.

"09/26/26")

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `dest` | `char *` | Destination string buffer |
| `max_len` | `size_t` | Buffer size (minimum 16 bytes recommended) |

---

{#bsp_time_get_dow_str-1}

### bsp_time_get_dow_str

```cpp
esp_err_t bsp_time_get_dow_str(char * dest, size_t max_len)
```

Defined in bsp_time.c:195

Format Current Day-of-Week String as "DayOfWeek" (e.g.

"Saturday")

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `dest` | `char *` | Destination string buffer |
| `max_len` | `size_t` | Buffer size (minimum 16 bytes recommended) |

---

{#bsp_time_get_date_dow_str-1}

### bsp_time_get_date_dow_str

```cpp
esp_err_t bsp_time_get_date_dow_str(char * dest, size_t max_len)
```

Defined in bsp_time.c:200

Format Current Date and Day-of-Week String as "MM/DD/YY DayOfWeek" (e.g.

"09/26/26 Saturday")

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `dest` | `char *` | Destination string buffer |
| `max_len` | `size_t` | Buffer size (minimum 24 bytes recommended) |

---

{#periodic_sync_timer_cb}

### periodic_sync_timer_cb

`static`

```cpp
static void periodic_sync_timer_cb(void * arg)
```

Defined in bsp_time.c:205

---

{#bsp_time_start_periodic_sync-1}

### bsp_time_start_periodic_sync

```cpp
esp_err_t bsp_time_start_periodic_sync(uint32_t interval_sec)
```

Defined in bsp_time.c:213

Start Background Periodic SNTP Synchronization Timer (e.g., every 24 hours).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `interval_sec` | `uint32_t` | Synchronization interval in seconds (Default: 86400 / 24h) |

---

{#bsp_time_stop_periodic_sync-1}

### bsp_time_stop_periodic_sync

```cpp
void bsp_time_stop_periodic_sync(void)
```

Defined in bsp_time.c:235

Stop Background Periodic SNTP Timer.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#wifi_event_handler}

### wifi_event_handler

`static`

```cpp
static void wifi_event_handler(void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data)
```

Defined in bsp_wifi.c:62

---

{#bsp_wifi_init-1}

### bsp_wifi_init

```cpp
esp_err_t bsp_wifi_init(void)
```

Defined in bsp_wifi.c:99

Initialize Wi-Fi Subsystem in Station Mode.

Sets up the default network interface (netif), initializes the L2 event loop, and prepares the driver for connection.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_wifi_connect-1}

### bsp_wifi_connect

```cpp
esp_err_t bsp_wifi_connect(const char * ssid, const char * password, uint32_t timeout_ms)
```

Defined in bsp_wifi.c:134

Connect to Wi-Fi Network with Specified SSID and Password.

Uses fast RTC cache if matching SSID was previously stored, otherwise performs full scan.

#### Returns
esp_err_t ESP_OK on successful IP acquisition, or error code on timeout/failure

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `ssid` | `const char *` | Target Access Point SSID |
| `password` | `const char *` | Network password (or empty string for open networks) |
| `timeout_ms` | `uint32_t` | Maximum time to wait for IP assignment in milliseconds |

---

{#bsp_wifi_connect_from_nvs-1}

### bsp_wifi_connect_from_nvs

```cpp
esp_err_t bsp_wifi_connect_from_nvs(uint32_t timeout_ms)
```

Defined in bsp_wifi.c:195

Connect to Wi-Fi using Credentials Stored in NVS Flash.

Reads "wifi_ssid" and "wifi_pass" from NVS. If missing or invalid, returns ESP_ERR_NVS_NOT_FOUND.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `timeout_ms` | `uint32_t` | Maximum time to wait for IP assignment |

---

{#bsp_wifi_disconnect-1}

### bsp_wifi_disconnect

```cpp
esp_err_t bsp_wifi_disconnect(void)
```

Defined in bsp_wifi.c:214

Disconnect and Stop Wi-Fi Station.

Cleanly terminates association and turns off RF circuitry prior to deep sleep.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_wifi_is_connected-1}

### bsp_wifi_is_connected

```cpp
bool bsp_wifi_is_connected(void)
```

Defined in bsp_wifi.c:224

Check if Wi-Fi is Currently Connected and has Valid IP Address.

#### Returns
true if connected with valid IP, false otherwise

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_wifi_get_rssi-1}

### bsp_wifi_get_rssi

```cpp
esp_err_t bsp_wifi_get_rssi(int * out_rssi)
```

Defined in bsp_wifi.c:229

Retrieve Current Received Signal Strength Indicator (RSSI).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `out_rssi` | `int *` | Pointer to receive RSSI value in dBm (e.g. -55 dBm) |

---

{#bsp_wifi_get_ip_str-1}

### bsp_wifi_get_ip_str

```cpp
esp_err_t bsp_wifi_get_ip_str(char * out_ip, size_t max_len)
```

Defined in bsp_wifi.c:242

Retrieve Formatted Local IPv4 Address String.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `out_ip` | `char *` | Destination character buffer |
| `max_len` | `size_t` | Size of buffer (minimum 16 bytes recommended) |

---

{#bsp_wifi_save_credentials-1}

### bsp_wifi_save_credentials

```cpp
esp_err_t bsp_wifi_save_credentials(const char * ssid, const char * password)
```

Defined in bsp_wifi.c:254

Save New Wi-Fi Credentials into NVS Flash.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `ssid` | `const char *` | AP SSID string |
| `password` | `const char *` | AP Password string |

---

{#bsp_wifi_scan-1}

### bsp_wifi_scan

```cpp
esp_err_t bsp_wifi_scan(void * ap_records, uint16_t * ap_count, uint16_t max_aps)
```

Defined in bsp_wifi.c:264

Scan for Nearby Wi-Fi Access Points.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `ap_records` | `void *` | Optional array to receive AP record metadata (or NULL) |
| `ap_count` | `uint16_t *` | Pointer to receive total number of discovered APs |
| `max_aps` | `uint16_t` | Maximum size of ap_records array (pass 0 if ap_records is NULL) |

---

{#bsp_wifi_invalidate_fast_cache-1}

### bsp_wifi_invalidate_fast_cache

```cpp
void bsp_wifi_invalidate_fast_cache(void)
```

Defined in bsp_wifi.c:304

Invalidate and Clear the RTC Fast Reconnect Session Cache.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_audio_power_enable}

### bsp_audio_power_enable

```cpp
void bsp_audio_power_enable(bool enable)
```

Defined in bsp/bsp_audio.h:50

Control Power Rail for Audio Subsystem (GPIO 42).

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `enable` | `bool` | true to power on audio domain (sets GPIO 42 LOW), false to power off (GPIO 42 HIGH) |

---

{#bsp_audio_init}

### bsp_audio_init

```cpp
esp_err_t bsp_audio_init(void)
```

Defined in bsp/bsp_audio.h:61

Initialize I2S Master Channel & ES8311 Audio Codec.

Configures I2S0 master TX channel at 16 kHz 16-bit mono, initializes the ES8311 codec via the shared I2C bus, enables the NS4168 power amplifier, and starts in muted state.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_audio_play}

### bsp_audio_play

```cpp
esp_err_t bsp_audio_play(const void * data, size_t len, size_t * bytes_written)
```

Defined in bsp/bsp_audio.h:72

Play Raw PCM Audio Buffer.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `data` | `const void *` | Pointer to 16-bit PCM audio samples |
| `len` | `size_t` | Size of data buffer in bytes |
| `bytes_written` | `size_t *` | Optional pointer to receive actual bytes transmitted |

---

{#bsp_audio_set_volume}

### bsp_audio_set_volume

```cpp
esp_err_t bsp_audio_set_volume(float volume)
```

Defined in bsp/bsp_audio.h:81

Set Software Audio Gain / Volume Scaling.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `volume` | `float` | Volume level from 0.0 (mute) to 100.0 (maximum) |

---

{#bsp_audio_play_tone}

### bsp_audio_play_tone

```cpp
esp_err_t bsp_audio_play_tone(uint32_t freq_hz, uint32_t duration_ms, float volume_pct)
```

Defined in bsp/bsp_audio.h:95

Play Synthesized Sine Tone.

Generates a smooth sine wave tone using a 256-point lookup table with 5ms attack/decay envelope ramps to eliminate acoustic popping.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `freq_hz` | `uint32_t` | Frequency in Hertz (e.g. 440, 523, 1046) |
| `duration_ms` | `uint32_t` | Duration in milliseconds |
| `volume_pct` | `float` | Volume percentage from 0.0 (muted) to 100.0 (maximum) |

---

{#bsp_audio_play_chime}

### bsp_audio_play_chime

```cpp
esp_err_t bsp_audio_play_chime(bsp_chime_type_t type)
```

Defined in bsp/bsp_audio.h:113

Play Built-in Synthesized Acoustic System Chime / Notification Sound.

Synthesizes structured melodic tone sequences tailored for system events:

* BSP_CHIME_BOOT: Ascending 4-tone melodic arpeggio (C5 -> E5 -> G5 -> C6)
* BSP_CHIME_WAKE: Quick rising wake cue (G5 -> C6)
* BSP_CHIME_SLEEP: Descending stand-down cadence (C6 -> G5 -> E5)
* BSP_CHIME_SHUTDOWN: Warm descending shutdown tone (G5 -> E5 -> C5)
* BSP_CHIME_ALARM: High-urgency alternating warning warble (1760 Hz / 880 Hz)
* BSP_CHIME_NOTIFY: Dual-ping notification chirp (1046 Hz -> 1318 Hz)
* BSP_CHIME_EVENT: Tactile click feedback blip (1200 Hz)

#### Returns
esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG on unknown chime type

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` | [`bsp_chime_type_t`](#bsp_chime_type_t) | Chime event type |

---

{#bsp_audio_register_default_chimes}

### bsp_audio_register_default_chimes

```cpp
esp_err_t bsp_audio_register_default_chimes(void)
```

Defined in bsp/bsp_audio.h:124

Register Built-in Chimes with the System Notification Dispatcher.

Automatically connects all [bsp_chime_type_t](#bsp_chime_type_t) event types to [bsp_audio_play_chime()](#bsp_audio_play_chime), enabling out-of-the-box acoustic feedback on boot, wake, sleep, shutdown, alarms, and UI clicks.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_audio_stop}

### bsp_audio_stop

```cpp
esp_err_t bsp_audio_stop(void)
```

Defined in bsp/bsp_audio.h:132

Stop Active Audio Playback and Mute Amplifier.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_audio_standby}

### bsp_audio_standby

```cpp
esp_err_t bsp_audio_standby(void)
```

Defined in bsp/bsp_audio.h:143

Enter Ultra-Low Power Standby Mode (~15 uA).

Mutes power amplifier, powers down ES8311 analog/digital blocks via I2C, and halts I2S output channel while leaving codec power rail powered (to prevent I2C bus clamping).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_audio_resume}

### bsp_audio_resume

```cpp
esp_err_t bsp_audio_resume(void)
```

Defined in bsp/bsp_audio.h:154

Resume ES8311 Codec from Standby Mode.

Restores ES8311 analog/digital power registers, configures low power mode, re-enables I2S TX channel, and enables NS4168 power amplifier.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_audio_set_low_power_mode}

### bsp_audio_set_low_power_mode

```cpp
esp_err_t bsp_audio_set_low_power_mode(bool enable)
```

Defined in bsp/bsp_audio.h:165

Enable or Disable ES8311 Low-Power Playback Mode (Reg 0x0F).

Sets LPDAC and LPDACVRP bits in ES8311 Reg 0x0F to reduce active DAC current draw by ~30%.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `enable` | `bool` | true to enable low power playback mode, false for full power |

---

{#bsp_audio_deinit}

### bsp_audio_deinit

```cpp
esp_err_t bsp_audio_deinit(void)
```

Defined in bsp/bsp_audio.h:175

Completely De-initialize Audio Subsystem and Release Resources.

Places ES8311 into standby, disables and deletes I2S channel, and frees codec device handle.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_power_init}

### bsp_power_init

```cpp
esp_err_t bsp_power_init(void)
```

Defined in bsp/bsp_power.h:87

Initialize Power Subsystem & Battery ADC Monitor.

Configures GPIO 17 HIGH to keep the board powered, configures status LED (GPIO 3), and calibrates ADC1 Channel 3 for battery voltage sensing.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_power_hold}

### bsp_power_hold

```cpp
esp_err_t bsp_power_hold(void)
```

Defined in bsp/bsp_power.h:95

Assert Power Latch (GPIO 17 HIGH) to keep LDO active.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_power_release}

### bsp_power_release

```cpp
esp_err_t bsp_power_release(void)
```

Defined in bsp/bsp_power.h:103

Release Power Latch (GPIO 17 LOW) to shut off battery power.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_power_register_shutdown_cb}

### bsp_power_register_shutdown_cb

```cpp
esp_err_t bsp_power_register_shutdown_cb(bsp_power_off_cb_t cb, void * user_data)
```

Defined in bsp/bsp_power.h:115

Register custom shutdown callback hook.

Invoked during [bsp_power_off()](#bsp_power_off) before power latch drops.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `cb` | [`bsp_power_off_cb_t`](#bsp_power_off_cb_t) | Callback function |
| `user_data` | `void *` | Custom user data pointer |

---

{#bsp_power_unregister_shutdown_cb}

### bsp_power_unregister_shutdown_cb

```cpp
esp_err_t bsp_power_unregister_shutdown_cb(void)
```

Defined in bsp/bsp_power.h:123

Unregister shutdown callback hook.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_power_off}

### bsp_power_off

```cpp
void bsp_power_off(void)
```

Defined in bsp/bsp_power.h:133

Turn board completely off.

Executes shutdown splash & chimes, lifecycle on_shutdown hooks, stops background tasks, isolates power rails, and drops the BAT_CTRL power hold latch. If external power (USB) is present, reboots cleanly.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_led_set}

### bsp_led_set

```cpp
void bsp_led_set(bool state)
```

Defined in bsp/bsp_power.h:141

Set Status LED Output State.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `state` | `bool` | true to turn LED on, false to turn LED off |

---

{#bsp_led_toggle}

### bsp_led_toggle

```cpp
void bsp_led_toggle(void)
```

Defined in bsp/bsp_power.h:147

Toggle Status LED Output State.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_battery_get_voltage}

### bsp_battery_get_voltage

```cpp
esp_err_t bsp_battery_get_voltage(uint32_t * out_mv, uint32_t * out_raw)
```

Defined in bsp/bsp_power.h:159

Read Raw and Calibrated Battery Terminal Voltage.

Samples ADC1 CH3 (GPIO 4), applies calibration curve, and compensates for the 1:2 divider.

#### Returns
esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG if out_mv is NULL

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `out_mv` | `uint32_t *` | Calculated battery voltage in millivolts (e.g. 4150 mV = 4.15V) |
| `out_raw` | `uint32_t *` | Optional pointer to receive raw ADC reading (can be NULL) |

---

{#bsp_battery_get_percentage}

### bsp_battery_get_percentage

```cpp
uint8_t bsp_battery_get_percentage(void)
```

Defined in bsp/bsp_power.h:169

Calculate Approximate Battery Remaining Percentage (0 - 100%).

Uses non-linear Li-Po state-of-charge curve mapped between 3.30V (0%) and 4.20V (100%).

#### Returns
uint8_t State of charge percentage (0 to 100)

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_battery_is_low}

### bsp_battery_is_low

```cpp
bool bsp_battery_is_low(uint8_t threshold_pct)
```

Defined in bsp/bsp_power.h:178

Check if Battery is in Low Warning / Critical Condition.

#### Returns
true if battery percentage is less than or equal to threshold

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `threshold_pct` | `uint8_t` | Low battery warning threshold percentage (e.g. 20%) |

---

{#bsp_enter_sleep}

### bsp_enter_sleep

```cpp
esp_err_t bsp_enter_sleep(const bsp_sleep_config_t * config)
```

Defined in bsp/bsp_power.h:190

Low-Level Sleep Execution Driver (Light or Deep Sleep).

Configures hardware wake sources, enables RTC GPIO hold, and executes esp_light_sleep_start() or esp_deep_sleep_start(). Applications should call [bsp_lifecycle_enter_sleep()](#bsp_lifecycle_enter_sleep) instead.

#### Returns
esp_err_t ESP_OK (returns upon wake if Light Sleep)

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `config` | const [`bsp_sleep_config_t`](#bsp_sleep_config_t) * | Sleep configuration parameters |

---

{#bsp_get_reset_reason}

### bsp_get_reset_reason

```cpp
esp_reset_reason_t bsp_get_reset_reason(void)
```

Defined in bsp/bsp_power.h:198

Get the system reset reason reported by ESP-IDF.

#### Returns
esp_reset_reason_t Reset reason

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_get_wakeup_cause}

### bsp_get_wakeup_cause

```cpp
esp_sleep_wakeup_cause_t bsp_get_wakeup_cause(void)
```

Defined in bsp/bsp_power.h:206

Get the sleep wakeup cause reported by ESP-IDF.

#### Returns
esp_sleep_wakeup_cause_t Wakeup cause

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_get_recommended_init_mode}

### bsp_get_recommended_init_mode

```cpp
bsp_init_mode_t bsp_get_recommended_init_mode(void)
```

Defined in bsp/bsp_power.h:214

Determine the recommended hardware initialization mode based on reset & wake history.

#### Returns
[bsp_init_mode_t](#bsp_init_mode_t) Recommended init mode (FULL, FAST, or MIN)

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_audio_power_enable-1}

### bsp_audio_power_enable

```cpp
void bsp_audio_power_enable(bool enable)
```

Defined in bsp_audio.c:79

Control Power Rail for Audio Subsystem (GPIO 42).

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `enable` | `bool` | true to power on audio domain (sets GPIO 42 LOW), false to power off (GPIO 42 HIGH) |

---

{#bsp_audio_init-1}

### bsp_audio_init

```cpp
esp_err_t bsp_audio_init(void)
```

Defined in bsp_audio.c:85

Initialize I2S Master Channel & ES8311 Audio Codec.

Configures I2S0 master TX channel at 16 kHz 16-bit mono, initializes the ES8311 codec via the shared I2C bus, enables the NS4168 power amplifier, and starts in muted state.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_audio_set_volume-1}

### bsp_audio_set_volume

```cpp
esp_err_t bsp_audio_set_volume(float volume)
```

Defined in bsp_audio.c:211

Set Software Audio Gain / Volume Scaling.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `volume` | `float` | Volume level from 0.0 (mute) to 100.0 (maximum) |

---

{#bsp_audio_play-1}

### bsp_audio_play

```cpp
esp_err_t bsp_audio_play(const void * data, size_t len, size_t * bytes_written)
```

Defined in bsp_audio.c:228

Play Raw PCM Audio Buffer.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `data` | `const void *` | Pointer to 16-bit PCM audio samples |
| `len` | `size_t` | Size of data buffer in bytes |
| `bytes_written` | `size_t *` | Optional pointer to receive actual bytes transmitted |

---

{#bsp_audio_stop-1}

### bsp_audio_stop

```cpp
esp_err_t bsp_audio_stop(void)
```

Defined in bsp_audio.c:249

Stop Active Audio Playback and Mute Amplifier.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_audio_set_low_power_mode-1}

### bsp_audio_set_low_power_mode

```cpp
esp_err_t bsp_audio_set_low_power_mode(bool enable)
```

Defined in bsp_audio.c:257

Enable or Disable ES8311 Low-Power Playback Mode (Reg 0x0F).

Sets LPDAC and LPDACVRP bits in ES8311 Reg 0x0F to reduce active DAC current draw by ~30%.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `enable` | `bool` | true to enable low power playback mode, false for full power |

---

{#bsp_audio_standby-1}

### bsp_audio_standby

```cpp
esp_err_t bsp_audio_standby(void)
```

Defined in bsp_audio.c:276

Enter Ultra-Low Power Standby Mode (~15 uA).

Mutes power amplifier, powers down ES8311 analog/digital blocks via I2C, and halts I2S output channel while leaving codec power rail powered (to prevent I2C bus clamping).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_audio_resume-1}

### bsp_audio_resume

```cpp
esp_err_t bsp_audio_resume(void)
```

Defined in bsp_audio.c:310

Resume ES8311 Codec from Standby Mode.

Restores ES8311 analog/digital power registers, configures low power mode, re-enables I2S TX channel, and enables NS4168 power amplifier.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_audio_deinit-1}

### bsp_audio_deinit

```cpp
esp_err_t bsp_audio_deinit(void)
```

Defined in bsp_audio.c:349

Completely De-initialize Audio Subsystem and Release Resources.

Places ES8311 into standby, disables and deletes I2S channel, and frees codec device handle.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_audio_play_tone-1}

### bsp_audio_play_tone

```cpp
esp_err_t bsp_audio_play_tone(uint32_t freq_hz, uint32_t duration_ms, float volume_pct)
```

Defined in bsp_audio.c:374

Play Synthesized Sine Tone.

Generates a smooth sine wave tone using a 256-point lookup table with 5ms attack/decay envelope ramps to eliminate acoustic popping.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `freq_hz` | `uint32_t` | Frequency in Hertz (e.g. 440, 523, 1046) |
| `duration_ms` | `uint32_t` | Duration in milliseconds |
| `volume_pct` | `float` | Volume percentage from 0.0 (muted) to 100.0 (maximum) |

---

{#bsp_audio_play_chime-1}

### bsp_audio_play_chime

```cpp
esp_err_t bsp_audio_play_chime(bsp_chime_type_t type)
```

Defined in bsp_audio.c:452

Play Built-in Synthesized Acoustic System Chime / Notification Sound.

Synthesizes structured melodic tone sequences tailored for system events:

* BSP_CHIME_BOOT: Ascending 4-tone melodic arpeggio (C5 -> E5 -> G5 -> C6)
* BSP_CHIME_WAKE: Quick rising wake cue (G5 -> C6)
* BSP_CHIME_SLEEP: Descending stand-down cadence (C6 -> G5 -> E5)
* BSP_CHIME_SHUTDOWN: Warm descending shutdown tone (G5 -> E5 -> C5)
* BSP_CHIME_ALARM: High-urgency alternating warning warble (1760 Hz / 880 Hz)
* BSP_CHIME_NOTIFY: Dual-ping notification chirp (1046 Hz -> 1318 Hz)
* BSP_CHIME_EVENT: Tactile click feedback blip (1200 Hz)

#### Returns
esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG on unknown chime type

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` | [`bsp_chime_type_t`](#bsp_chime_type_t) | Chime event type |

---

{#bsp_default_chime_dispatcher}

### bsp_default_chime_dispatcher

`static`

```cpp
static void bsp_default_chime_dispatcher(bsp_chime_type_t type, void * user_data)
```

Defined in bsp_audio.c:515

---

{#bsp_audio_register_default_chimes-1}

### bsp_audio_register_default_chimes

```cpp
esp_err_t bsp_audio_register_default_chimes(void)
```

Defined in bsp_audio.c:521

Register Built-in Chimes with the System Notification Dispatcher.

Automatically connects all [bsp_chime_type_t](#bsp_chime_type_t) event types to [bsp_audio_play_chime()](#bsp_audio_play_chime), enabling out-of-the-box acoustic feedback on boot, wake, sleep, shutdown, alarms, and UI clicks.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#init_adc_calibration}

### init_adc_calibration

`static`

```cpp
static bool init_adc_calibration(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t * out_handle)
```

Defined in bsp_power.c:91

---

{#bsp_power_hold-1}

### bsp_power_hold

```cpp
esp_err_t bsp_power_hold(void)
```

Defined in bsp_power.c:135

Assert Power Latch (GPIO 17 HIGH) to keep LDO active.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_power_release-1}

### bsp_power_release

```cpp
esp_err_t bsp_power_release(void)
```

Defined in bsp_power.c:140

Release Power Latch (GPIO 17 LOW) to shut off battery power.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_power_register_shutdown_cb-1}

### bsp_power_register_shutdown_cb

```cpp
esp_err_t bsp_power_register_shutdown_cb(bsp_power_off_cb_t cb, void * user_data)
```

Defined in bsp_power.c:146

Register custom shutdown callback hook.

Invoked during [bsp_power_off()](#bsp_power_off) before power latch drops.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `cb` | [`bsp_power_off_cb_t`](#bsp_power_off_cb_t) | Callback function |
| `user_data` | `void *` | Custom user data pointer |

---

{#bsp_power_unregister_shutdown_cb-1}

### bsp_power_unregister_shutdown_cb

```cpp
esp_err_t bsp_power_unregister_shutdown_cb(void)
```

Defined in bsp_power.c:153

Unregister shutdown callback hook.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_power_off-1}

### bsp_power_off

```cpp
void bsp_power_off(void)
```

Defined in bsp_power.c:160

Turn board completely off.

Executes shutdown splash & chimes, lifecycle on_shutdown hooks, stops background tasks, isolates power rails, and drops the BAT_CTRL power hold latch. If external power (USB) is present, reboots cleanly.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_power_init-1}

### bsp_power_init

```cpp
esp_err_t bsp_power_init(void)
```

Defined in bsp_power.c:217

Initialize Power Subsystem & Battery ADC Monitor.

Configures GPIO 17 HIGH to keep the board powered, configures status LED (GPIO 3), and calibrates ADC1 Channel 3 for battery voltage sensing.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_led_set-1}

### bsp_led_set

```cpp
void bsp_led_set(bool state)
```

Defined in bsp_power.c:254

Set Status LED Output State.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `state` | `bool` | true to turn LED on, false to turn LED off |

---

{#bsp_led_toggle-1}

### bsp_led_toggle

```cpp
void bsp_led_toggle(void)
```

Defined in bsp_power.c:261

Toggle Status LED Output State.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_battery_get_voltage-1}

### bsp_battery_get_voltage

```cpp
esp_err_t bsp_battery_get_voltage(uint32_t * out_mv, uint32_t * out_raw)
```

Defined in bsp_power.c:266

Read Raw and Calibrated Battery Terminal Voltage.

Samples ADC1 CH3 (GPIO 4), applies calibration curve, and compensates for the 1:2 divider.

#### Returns
esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG if out_mv is NULL

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `out_mv` | `uint32_t *` | Calculated battery voltage in millivolts (e.g. 4150 mV = 4.15V) |
| `out_raw` | `uint32_t *` | Optional pointer to receive raw ADC reading (can be NULL) |

---

{#bsp_battery_get_percentage-1}

### bsp_battery_get_percentage

```cpp
uint8_t bsp_battery_get_percentage(void)
```

Defined in bsp_power.c:309

Calculate Approximate Battery Remaining Percentage (0 - 100%).

Uses non-linear Li-Po state-of-charge curve mapped between 3.30V (0%) and 4.20V (100%).

#### Returns
uint8_t State of charge percentage (0 to 100)

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_battery_is_low-1}

### bsp_battery_is_low

```cpp
bool bsp_battery_is_low(uint8_t threshold_pct)
```

Defined in bsp_power.c:346

Check if Battery is in Low Warning / Critical Condition.

#### Returns
true if battery percentage is less than or equal to threshold

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `threshold_pct` | `uint8_t` | Low battery warning threshold percentage (e.g. 20%) |

---

{#bsp_get_reset_reason-1}

### bsp_get_reset_reason

```cpp
esp_reset_reason_t bsp_get_reset_reason(void)
```

Defined in bsp_power.c:351

Get the system reset reason reported by ESP-IDF.

#### Returns
esp_reset_reason_t Reset reason

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_get_wakeup_cause-1}

### bsp_get_wakeup_cause

```cpp
esp_sleep_wakeup_cause_t bsp_get_wakeup_cause(void)
```

Defined in bsp_power.c:356

Get the sleep wakeup cause reported by ESP-IDF.

#### Returns
esp_sleep_wakeup_cause_t Wakeup cause

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_get_recommended_init_mode-1}

### bsp_get_recommended_init_mode

```cpp
bsp_init_mode_t bsp_get_recommended_init_mode(void)
```

Defined in bsp_power.c:364

Determine the recommended hardware initialization mode based on reset & wake history.

#### Returns
[bsp_init_mode_t](#bsp_init_mode_t) Recommended init mode (FULL, FAST, or MIN)

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_enter_sleep-1}

### bsp_enter_sleep

```cpp
esp_err_t bsp_enter_sleep(const bsp_sleep_config_t * config)
```

Defined in bsp_power.c:379

Low-Level Sleep Execution Driver (Light or Deep Sleep).

Configures hardware wake sources, enables RTC GPIO hold, and executes esp_light_sleep_start() or esp_deep_sleep_start(). Applications should call [bsp_lifecycle_enter_sleep()](#bsp_lifecycle_enter_sleep) instead.

#### Returns
esp_err_t ESP_OK (returns upon wake if Light Sleep)

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `config` | const [`bsp_sleep_config_t`](#bsp_sleep_config_t) * | Sleep configuration parameters |

---

{#bsp_button_init}

### bsp_button_init

```cpp
esp_err_t bsp_button_init(const bsp_button_config_t * config)
```

Defined in bsp/bsp_button.h:85

Initialize Hardware Button Interrupts and Debounce Timer.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `config` | const [`bsp_button_config_t`](#bsp_button_config_t) * | Pointer to timing config, or NULL for defaults |

---

{#bsp_button_stop}

### bsp_button_stop

```cpp
esp_err_t bsp_button_stop(void)
```

Defined in bsp/bsp_button.h:93

Stop Button Polling and Debounce Timer.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_button_register_cb}

### bsp_button_register_cb

```cpp
esp_err_t bsp_button_register_cb(bsp_button_t button, bsp_button_event_t event, bsp_button_cb_t cb, void * user_data)
```

Defined in bsp/bsp_button.h:105

Register Callback for Button Event.

#### Returns
esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG on invalid button/event

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `button` | [`bsp_button_t`](#bsp_button_t) | Target button (BOOT or POWER) |
| `event` | [`bsp_button_event_t`](#bsp_button_event_t) | Target event type |
| `cb` | [`bsp_button_cb_t`](#bsp_button_cb_t) | Callback function |
| `user_data` | `void *` | Custom user pointer passed to callback |

---

{#bsp_button_unregister_cb}

### bsp_button_unregister_cb

```cpp
esp_err_t bsp_button_unregister_cb(bsp_button_t button, bsp_button_event_t event)
```

Defined in bsp/bsp_button.h:115

Unregister Callback for Button Event.

#### Returns
esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG on invalid button/event

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `button` | [`bsp_button_t`](#bsp_button_t) | Target button |
| `event` | [`bsp_button_event_t`](#bsp_button_event_t) | Target event type |

---

{#bsp_button_is_pressed}

### bsp_button_is_pressed

```cpp
bool bsp_button_is_pressed(bsp_button_t button)
```

Defined in bsp/bsp_button.h:124

Check if button is currently pressed down.

#### Returns
true if pressed (GPIO level LOW), false otherwise

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `button` | [`bsp_button_t`](#bsp_button_t) | Target button |

---

{#bsp_button_wait_for_click}

### bsp_button_wait_for_click

```cpp
esp_err_t bsp_button_wait_for_click(bsp_button_t button, uint32_t timeout_ms)
```

Defined in bsp/bsp_button.h:134

Synchronously block and wait for a button click (press + release).

#### Returns
esp_err_t ESP_OK on click, ESP_ERR_TIMEOUT on timeout, ESP_ERR_INVALID_ARG on bad button

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `button` | [`bsp_button_t`](#bsp_button_t) | Target button (BOOT or POWER) |
| `timeout_ms` | `uint32_t` | Maximum time to wait in milliseconds (0 for indefinite blocking) |

---

{#bsp_sdcard_mount}

### bsp_sdcard_mount

```cpp
esp_err_t bsp_sdcard_mount(void)
```

Defined in bsp/bsp_sdcard.h:38

Mount MicroSD Card over 1-bit SDMMC FATFS Subsystem.

#### Returns
esp_err_t ESP_OK on success, ESP_ERR_NOT_FOUND if card missing

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_sdcard_unmount}

### bsp_sdcard_unmount

```cpp
esp_err_t bsp_sdcard_unmount(void)
```

Defined in bsp/bsp_sdcard.h:46

Unmount MicroSD Card and Release SDMMC Resources.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_sdcard_is_mounted}

### bsp_sdcard_is_mounted

```cpp
bool bsp_sdcard_is_mounted(void)
```

Defined in bsp/bsp_sdcard.h:54

Check if MicroSD Card is Currently Mounted.

#### Returns
true if mounted, false otherwise

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_sdcard_get_capacity_gb}

### bsp_sdcard_get_capacity_gb

```cpp
float bsp_sdcard_get_capacity_gb(void)
```

Defined in bsp/bsp_sdcard.h:62

Retrieve total capacity of mounted MicroSD Card in Gigabytes.

#### Returns
float Capacity in GB

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_register_splash_cb}

### bsp_register_splash_cb

```cpp
esp_err_t bsp_register_splash_cb(bsp_splash_type_t type, bsp_splash_cb_t cb, void * user_data)
```

Defined in bsp/bsp_splash.h:86

Register a callback for a specific UI Splash event.

#### Returns
esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG on invalid type

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` | [`bsp_splash_type_t`](#bsp_splash_type_t) | Splash type (BOOT, WAKE, SLEEP, SHUTDOWN) |
| `cb` | [`bsp_splash_cb_t`](#bsp_splash_cb_t) | Callback function pointer |
| `user_data` | `void *` | Optional user context pointer |

---

{#bsp_unregister_splash_cb}

### bsp_unregister_splash_cb

```cpp
esp_err_t bsp_unregister_splash_cb(bsp_splash_type_t type)
```

Defined in bsp/bsp_splash.h:95

Unregister a callback for a specific UI Splash event.

#### Returns
esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG on invalid type

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` | [`bsp_splash_type_t`](#bsp_splash_type_t) | Splash type (BOOT, WAKE, SLEEP, SHUTDOWN) |

---

{#bsp_register_chime_cb}

### bsp_register_chime_cb

```cpp
esp_err_t bsp_register_chime_cb(bsp_chime_type_t type, bsp_chime_cb_t cb, void * user_data)
```

Defined in bsp/bsp_splash.h:106

Register a callback for a specific Audio Chime event.

#### Returns
esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG on invalid type

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` | [`bsp_chime_type_t`](#bsp_chime_type_t) | Chime type (BOOT, WAKE, SLEEP, SHUTDOWN, ALARM, NOTIFY, EVENT) |
| `cb` | [`bsp_chime_cb_t`](#bsp_chime_cb_t) | Callback function pointer |
| `user_data` | `void *` | Optional user context pointer |

---

{#bsp_unregister_chime_cb}

### bsp_unregister_chime_cb

```cpp
esp_err_t bsp_unregister_chime_cb(bsp_chime_type_t type)
```

Defined in bsp/bsp_splash.h:115

Unregister a callback for a specific Audio Chime event.

#### Returns
esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG on invalid type

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` | [`bsp_chime_type_t`](#bsp_chime_type_t) | Chime type (BOOT, WAKE, SLEEP, SHUTDOWN, ALARM, NOTIFY, EVENT) |

---

{#bsp_has_splash_cb}

### bsp_has_splash_cb

```cpp
bool bsp_has_splash_cb(bsp_splash_type_t type)
```

Defined in bsp/bsp_splash.h:124

Check if a UI Splash callback is registered for a given event type.

#### Returns
true if callback is registered, false otherwise

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` | [`bsp_splash_type_t`](#bsp_splash_type_t) | Splash type |

---

{#bsp_has_chime_cb}

### bsp_has_chime_cb

```cpp
bool bsp_has_chime_cb(bsp_chime_type_t type)
```

Defined in bsp/bsp_splash.h:133

Check if an Audio Chime callback is registered for a given event type.

#### Returns
true if callback is registered, false otherwise

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` | [`bsp_chime_type_t`](#bsp_chime_type_t) | Chime type |

---

{#bsp_trigger_splash}

### bsp_trigger_splash

```cpp
esp_err_t bsp_trigger_splash(bsp_splash_type_t type)
```

Defined in bsp/bsp_splash.h:142

Trigger the registered UI Splash callback.

#### Returns
esp_err_t ESP_OK if callback was executed, ESP_ERR_NOT_FOUND if no callback registered

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` | [`bsp_splash_type_t`](#bsp_splash_type_t) | Splash type to invoke |

---

{#bsp_trigger_chime}

### bsp_trigger_chime

```cpp
esp_err_t bsp_trigger_chime(bsp_chime_type_t type)
```

Defined in bsp/bsp_splash.h:151

Trigger the registered Audio Chime callback.

#### Returns
esp_err_t ESP_OK if callback was executed, ESP_ERR_NOT_FOUND if no callback registered

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` | [`bsp_chime_type_t`](#bsp_chime_type_t) | Chime type to invoke |

---

{#fire_event}

### fire_event

`static`

```cpp
static void fire_event(bsp_button_t btn, bsp_button_event_t event)
```

Defined in bsp_button.c:59

---

{#button_timer_cb}

### button_timer_cb

`static`

```cpp
static void button_timer_cb(void * arg)
```

Defined in bsp_button.c:72

---

{#bsp_button_stop-1}

### bsp_button_stop

```cpp
esp_err_t bsp_button_stop(void)
```

Defined in bsp_button.c:151

Stop Button Polling and Debounce Timer.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_button_init-1}

### bsp_button_init

```cpp
esp_err_t bsp_button_init(const bsp_button_config_t * config)
```

Defined in bsp_button.c:159

Initialize Hardware Button Interrupts and Debounce Timer.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `config` | const [`bsp_button_config_t`](#bsp_button_config_t) * | Pointer to timing config, or NULL for defaults |

---

{#bsp_button_register_cb-1}

### bsp_button_register_cb

```cpp
esp_err_t bsp_button_register_cb(bsp_button_t button, bsp_button_event_t event, bsp_button_cb_t cb, void * user_data)
```

Defined in bsp_button.c:215

Register Callback for Button Event.

#### Returns
esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG on invalid button/event

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `button` | [`bsp_button_t`](#bsp_button_t) | Target button (BOOT or POWER) |
| `event` | [`bsp_button_event_t`](#bsp_button_event_t) | Target event type |
| `cb` | [`bsp_button_cb_t`](#bsp_button_cb_t) | Callback function |
| `user_data` | `void *` | Custom user pointer passed to callback |

---

{#bsp_button_unregister_cb-1}

### bsp_button_unregister_cb

```cpp
esp_err_t bsp_button_unregister_cb(bsp_button_t button, bsp_button_event_t event)
```

Defined in bsp_button.c:225

Unregister Callback for Button Event.

#### Returns
esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG on invalid button/event

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `button` | [`bsp_button_t`](#bsp_button_t) | Target button |
| `event` | [`bsp_button_event_t`](#bsp_button_event_t) | Target event type |

---

{#bsp_button_is_pressed-1}

### bsp_button_is_pressed

```cpp
bool bsp_button_is_pressed(bsp_button_t button)
```

Defined in bsp_button.c:230

Check if button is currently pressed down.

#### Returns
true if pressed (GPIO level LOW), false otherwise

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `button` | [`bsp_button_t`](#bsp_button_t) | Target button |

---

{#bsp_button_wait_for_click-1}

### bsp_button_wait_for_click

```cpp
esp_err_t bsp_button_wait_for_click(bsp_button_t button, uint32_t timeout_ms)
```

Defined in bsp_button.c:238

Synchronously block and wait for a button click (press + release).

#### Returns
esp_err_t ESP_OK on click, ESP_ERR_TIMEOUT on timeout, ESP_ERR_INVALID_ARG on bad button

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `button` | [`bsp_button_t`](#bsp_button_t) | Target button (BOOT or POWER) |
| `timeout_ms` | `uint32_t` | Maximum time to wait in milliseconds (0 for indefinite blocking) |

---

{#bsp_get_version}

### bsp_get_version

```cpp
const char * bsp_get_version(void)
```

Defined in bsp_common.c:33

Get BSP SemVer semantic version string.

#### Returns
const char* String formatted as "MAJOR.MINOR.PATCH"

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_get_version_val}

### bsp_get_version_val

```cpp
uint32_t bsp_get_version_val(void)
```

Defined in bsp_common.c:38

Get BSP version as integer representation.

#### Returns
uint32_t Version encoded as (MAJOR << 16) | (MINOR << 8) | PATCH

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_get_chip_revision-1}

### bsp_get_chip_revision

```cpp
esp_err_t bsp_get_chip_revision(uint32_t * major, uint32_t * minor)
```

Defined in bsp_common.c:43

Get MCU Silicon Revision numbers (Major and Minor).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `major` | `uint32_t *` | Major wafer revision |
| `minor` | `uint32_t *` | Minor wafer revision |

---

{#bsp_get_chip_revision_str-1}

### bsp_get_chip_revision_str

```cpp
const char * bsp_get_chip_revision_str(void)
```

Defined in bsp_common.c:52

Get MCU Silicon Revision formatted string (e.g.

"v0.2")

#### Returns
const char* Revision string

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_err_to_name-1}

### bsp_err_to_name

```cpp
const char * bsp_err_to_name(esp_err_t err)
```

Defined in bsp_common.c:63

Translate BSP error code to human-readable error name string.

#### Returns
const char* String representation of error

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `err` | `esp_err_t` | Error code |

---

{#bsp_get_diagnostics-1}

### bsp_get_diagnostics

```cpp
esp_err_t bsp_get_diagnostics(bsp_diag_info_t * diag)
```

Defined in bsp_common.c:79

Populate a runtime system diagnostics snapshot.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `diag` | [`bsp_diag_info_t`](#bsp_diag_info_t) * | Pointer to [bsp_diag_info_t](#bsp_diag_info_t) struct to populate |

---

{#bsp_diagnostics_dump-1}

### bsp_diagnostics_dump

```cpp
void bsp_diagnostics_dump(void)
```

Defined in bsp_common.c:115

Print formatted system diagnostic report to stdout/ESP_LOG.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_init_io-1}

### bsp_init_io

```cpp
esp_err_t bsp_init_io(void)
```

Defined in bsp_common.c:141

Initialize all GPIO output pins (power latch, audio PA, display power rail, LED).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_board_init_with_config-1}

### bsp_board_init_with_config

```cpp
esp_err_t bsp_board_init_with_config(const bsp_config_t * config)
```

Defined in bsp_common.c:257

Custom Board Initialization.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `config` | const [`bsp_config_t`](#bsp_config_t) * | Pointer to custom [bsp_config_t](#bsp_config_t) struct |

---

{#bsp_init_mode-1}

### bsp_init_mode

```cpp
esp_err_t bsp_init_mode(bsp_init_mode_t mode)
```

Defined in bsp_common.c:359

Dynamic Hardware Initialization by Mode (FULL, FAST).

* BSP_INIT_MODE_FULL: Cold boot, full peripheral startup, display + LVGL.
* BSP_INIT_MODE_FAST: Wake boot, fast display ready without clear.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `mode` | [`bsp_init_mode_t`](#bsp_init_mode_t) | Target [bsp_init_mode_t](#bsp_init_mode_t) |

---

{#bsp_generate_unambiguous_key-1}

### bsp_generate_unambiguous_key

```cpp
esp_err_t bsp_generate_unambiguous_key(char * buf, size_t len, const char * charset)
```

Defined in bsp_common.c:396

Generate a cryptographically random, unambiguous key string (excluding 0, O, o, 1, l, I).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `buf` | `char *` | Destination buffer |
| `len` | `size_t` | Key length (excluding null terminator) |
| `charset` | `const char *` | Custom character set (or NULL for BSP_CHARSET_UNAMBIGUOUS) |

---

{#bsp_board_init-1}

### bsp_board_init

```cpp
esp_err_t bsp_board_init(void)
```

Defined in bsp_common.c:411

Comprehensive Board Initialization.

Applies default configuration, enables power rails, configures I2C, RTC, sensors, buttons, display driver, and starts the LVGL v9 rendering task pinned to Core 1.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_system_shutdown-1}

### bsp_system_shutdown

```cpp
void bsp_system_shutdown(void)
```

Defined in bsp_common.c:418

Perform Clean System Shutdown.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_system_deep_sleep-1}

### bsp_system_deep_sleep

```cpp
void bsp_system_deep_sleep(uint32_t sleep_sec)
```

Defined in bsp_common.c:425

Enter Ultra-Low Power Deep Sleep Mode.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `sleep_sec` | `uint32_t` | Duration in seconds (0 for button wakeup only) |

---

{#bsp_get_device_id-1}

### bsp_get_device_id

```cpp
esp_err_t bsp_get_device_id(char * buf, size_t max_len)
```

Defined in bsp_common.c:436

Retrieve Unique Hardware Device ID string from MAC address (e.g.

"ESP32S3-70041D3B")

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `buf` | `char *` | Destination buffer |
| `max_len` | `size_t` | Buffer length |

---

{#bsp_get_device_name-1}

### bsp_get_device_name

```cpp
esp_err_t bsp_get_device_name(char * buf, size_t max_len)
```

Defined in bsp_common.c:450

Retrieve Human-Readable Device Name string (e.g.

"HumidOS-1D3B")

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `buf` | `char *` | Destination buffer |
| `max_len` | `size_t` | Buffer length |

---

{#bsp_delay_ms-1}

### bsp_delay_ms

```cpp
void bsp_delay_ms(uint32_t ms)
```

Defined in bsp_common.c:464

Generic Millisecond Delay Helper.

Suspends execution for the specified duration in milliseconds. Uses FreeRTOS task delay if the scheduler is running, or hardware ROM delay on early boot. Allows delaying execution without including FreeRTOS header files in application code.

Memory ownership: none. Behavior: Blocking / Task Yield. Thread safety: Thread-safe.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `ms` | `uint32_t` | Delay duration in milliseconds |

---

{#bsp_delay_us-1}

### bsp_delay_us

```cpp
void bsp_delay_us(uint32_t us)
```

Defined in bsp_common.c:474

Generic Microsecond Delay Helper.

Pauses execution for the specified duration in microseconds using high-resolution timer. Does not require including FreeRTOS headers.

Memory ownership: none. Behavior: Busy-wait delay. Thread safety: Thread-safe.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `us` | `uint32_t` | Delay duration in microseconds |

---

{#bsp_lvgl_port_task}

### bsp_lvgl_port_task

`static`

```cpp
static void bsp_lvgl_port_task(void * pvParameters)
```

Defined in bsp_lvgl.cpp:295

---

{#bsp_lvgl_lock-1}

### bsp_lvgl_lock

```cpp
bool bsp_lvgl_lock(void)
```

Defined in bsp_lvgl.cpp:54

Acquire LVGL Reentrant Mutex Lock.

Must be called prior to modifying any UI widgets or invoking lvgl functions from external tasks.

#### Returns
true if mutex was successfully acquired

Memory ownership: none. Behavior: Blocking. Thread safety: thread-safe guarantees.

---

{#bsp_lvgl_unlock-1}

### bsp_lvgl_unlock

```cpp
void bsp_lvgl_unlock(void)
```

Defined in bsp_lvgl.cpp:59

Release LVGL Reentrant Mutex Lock.

Memory ownership: none. Behavior: Blocking. Thread safety: thread-safe guarantees.

---

{#bsp_lvgl_lock_isr-1}

### bsp_lvgl_lock_isr

```cpp
bool bsp_lvgl_lock_isr(void)
```

Defined in bsp_lvgl.cpp:63

Acquire LVGL Reentrant Mutex Lock from ISR Context.

#### Returns
true if mutex was successfully acquired from ISR

Memory ownership: none. Behavior: Non-blocking. Thread safety: ISR-safe guarantees.

---

{#lvgl_tick_get_cb}

### lvgl_tick_get_cb

`static`

```cpp
static uint32_t lvgl_tick_get_cb(void)
```

Defined in bsp_lvgl.cpp:67

---

{#lvgl_display_flush_cb}

### lvgl_display_flush_cb

`static`

```cpp
static void lvgl_display_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
```

Defined in bsp_lvgl.cpp:72

---

{#bsp_lvgl_init-1}

### bsp_lvgl_init

```cpp
esp_err_t bsp_lvgl_init(void)
```

Defined in bsp_lvgl.cpp:174

Initialize LVGL v9 Graphics Subsystem and Register Display Port.

Allocates draw buffers and registers the SSD1681 1-bit monochrome flush callback.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_lvgl_set_first_flush_mode-1}

### bsp_lvgl_set_first_flush_mode

```cpp
void bsp_lvgl_set_first_flush_mode(bool full_refresh)
```

Defined in bsp_lvgl.cpp:232

Configure whether the next LVGL display flush performs a full OTP refresh or fast partial update.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `full_refresh` | `bool` | true for full OTP clear/refresh (e.g. on cold boot), false for fast partial refresh (e.g. on wake) |

---

{#bsp_lvgl_start-1}

### bsp_lvgl_start

```cpp
esp_err_t bsp_lvgl_start(int priority, int core_id)
```

Defined in bsp_lvgl.cpp:237

Start LVGL Background FreeRTOS Execution Task.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `priority` | `int` | Task priority (Default: 5) |
| `core_id` | `int` | CPU Core affinity (Default: 1 - Core 1) |

---

{#bsp_lvgl_stop-1}

### bsp_lvgl_stop

```cpp
esp_err_t bsp_lvgl_stop(void)
```

Defined in bsp_lvgl.cpp:282

Stop LVGL Background FreeRTOS Execution Task.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_sdcard_mount-1}

### bsp_sdcard_mount

```cpp
esp_err_t bsp_sdcard_mount(void)
```

Defined in bsp_sdcard.c:27

Mount MicroSD Card over 1-bit SDMMC FATFS Subsystem.

#### Returns
esp_err_t ESP_OK on success, ESP_ERR_NOT_FOUND if card missing

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_sdcard_unmount-1}

### bsp_sdcard_unmount

```cpp
esp_err_t bsp_sdcard_unmount(void)
```

Defined in bsp_sdcard.c:78

Unmount MicroSD Card and Release SDMMC Resources.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_sdcard_get_capacity_gb-1}

### bsp_sdcard_get_capacity_gb

```cpp
float bsp_sdcard_get_capacity_gb(void)
```

Defined in bsp_sdcard.c:90

Retrieve total capacity of mounted MicroSD Card in Gigabytes.

#### Returns
float Capacity in GB

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_sdcard_is_mounted-1}

### bsp_sdcard_is_mounted

```cpp
bool bsp_sdcard_is_mounted(void)
```

Defined in bsp_sdcard.c:96

Check if MicroSD Card is Currently Mounted.

#### Returns
true if mounted, false otherwise

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_register_splash_cb-1}

### bsp_register_splash_cb

```cpp
esp_err_t bsp_register_splash_cb(bsp_splash_type_t type, bsp_splash_cb_t cb, void * user_data)
```

Defined in bsp_splash.c:34

Register a callback for a specific UI Splash event.

#### Returns
esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG on invalid type

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` | [`bsp_splash_type_t`](#bsp_splash_type_t) | Splash type (BOOT, WAKE, SLEEP, SHUTDOWN) |
| `cb` | [`bsp_splash_cb_t`](#bsp_splash_cb_t) | Callback function pointer |
| `user_data` | `void *` | Optional user context pointer |

---

{#bsp_unregister_splash_cb-1}

### bsp_unregister_splash_cb

```cpp
esp_err_t bsp_unregister_splash_cb(bsp_splash_type_t type)
```

Defined in bsp_splash.c:45

Unregister a callback for a specific UI Splash event.

#### Returns
esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG on invalid type

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` | [`bsp_splash_type_t`](#bsp_splash_type_t) | Splash type (BOOT, WAKE, SLEEP, SHUTDOWN) |

---

{#bsp_register_chime_cb-1}

### bsp_register_chime_cb

```cpp
esp_err_t bsp_register_chime_cb(bsp_chime_type_t type, bsp_chime_cb_t cb, void * user_data)
```

Defined in bsp_splash.c:55

Register a callback for a specific Audio Chime event.

#### Returns
esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG on invalid type

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` | [`bsp_chime_type_t`](#bsp_chime_type_t) | Chime type (BOOT, WAKE, SLEEP, SHUTDOWN, ALARM, NOTIFY, EVENT) |
| `cb` | [`bsp_chime_cb_t`](#bsp_chime_cb_t) | Callback function pointer |
| `user_data` | `void *` | Optional user context pointer |

---

{#bsp_unregister_chime_cb-1}

### bsp_unregister_chime_cb

```cpp
esp_err_t bsp_unregister_chime_cb(bsp_chime_type_t type)
```

Defined in bsp_splash.c:66

Unregister a callback for a specific Audio Chime event.

#### Returns
esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG on invalid type

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` | [`bsp_chime_type_t`](#bsp_chime_type_t) | Chime type (BOOT, WAKE, SLEEP, SHUTDOWN, ALARM, NOTIFY, EVENT) |

---

{#bsp_has_splash_cb-1}

### bsp_has_splash_cb

```cpp
bool bsp_has_splash_cb(bsp_splash_type_t type)
```

Defined in bsp_splash.c:76

Check if a UI Splash callback is registered for a given event type.

#### Returns
true if callback is registered, false otherwise

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` | [`bsp_splash_type_t`](#bsp_splash_type_t) | Splash type |

---

{#bsp_has_chime_cb-1}

### bsp_has_chime_cb

```cpp
bool bsp_has_chime_cb(bsp_chime_type_t type)
```

Defined in bsp_splash.c:84

Check if an Audio Chime callback is registered for a given event type.

#### Returns
true if callback is registered, false otherwise

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` | [`bsp_chime_type_t`](#bsp_chime_type_t) | Chime type |

---

{#bsp_trigger_splash-1}

### bsp_trigger_splash

```cpp
esp_err_t bsp_trigger_splash(bsp_splash_type_t type)
```

Defined in bsp_splash.c:92

Trigger the registered UI Splash callback.

#### Returns
esp_err_t ESP_OK if callback was executed, ESP_ERR_NOT_FOUND if no callback registered

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` | [`bsp_splash_type_t`](#bsp_splash_type_t) | Splash type to invoke |

---

{#bsp_trigger_chime-1}

### bsp_trigger_chime

```cpp
esp_err_t bsp_trigger_chime(bsp_chime_type_t type)
```

Defined in bsp_splash.c:105

Trigger the registered Audio Chime callback.

#### Returns
esp_err_t ESP_OK if callback was executed, ESP_ERR_NOT_FOUND if no callback registered

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` | [`bsp_chime_type_t`](#bsp_chime_type_t) | Chime type to invoke |

---

{#bsp_display_init}

### bsp_display_init

```cpp
esp_err_t bsp_display_init(void)
```

Defined in bsp/bsp_display.h:52

Initialize SSD1681 SPI Hardware Interface and Panel Controller.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_display_clear}

### bsp_display_clear

```cpp
void bsp_display_clear(void)
```

Defined in bsp/bsp_display.h:58

Clear Entire In-Memory Framebuffer to Pure White.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_display_draw_pixel}

### bsp_display_draw_pixel

```cpp
void bsp_display_draw_pixel(uint16_t x, uint16_t y, bsp_display_color_t color)
```

Defined in bsp/bsp_display.h:68

Draw Single Pixel to Framebuffer.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `x` | `uint16_t` | Coordinate (0 to 199) |
| `y` | `uint16_t` | Coordinate (0 to 199) |
| `color` | [`bsp_display_color_t`](#bsp_display_color_t) | Pixel color (BLACK or WHITE) |

---

{#bsp_display_get_buffer}

### bsp_display_get_buffer

```cpp
uint8_t * bsp_display_get_buffer(void)
```

Defined in bsp/bsp_display.h:76

Retrieve Pointer to In-Memory Framebuffer (5000 bytes).

#### Returns
uint8_t* Buffer pointer

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_display_flush}

### bsp_display_flush

```cpp
void bsp_display_flush(void)
```

Defined in bsp/bsp_display.h:82

Full OTP Waveform Hardware Refresh of In-Memory Framebuffer to Screen.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_display_flush_partial}

### bsp_display_flush_partial

```cpp
void bsp_display_flush_partial(void)
```

Defined in bsp/bsp_display.h:88

Fast Partial Waveform Refresh of Full Screen Area.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_display_flush_partial_area}

### bsp_display_flush_partial_area

```cpp
void bsp_display_flush_partial_area(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
```

Defined in bsp/bsp_display.h:99

Fast Partial Waveform Refresh of Specific Bounding Box Area.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `x_start` | `uint16_t` | Left coordinate |
| `y_start` | `uint16_t` | Top coordinate |
| `x_end` | `uint16_t` | Right coordinate |
| `y_end` | `uint16_t` | Bottom coordinate |

---

{#bsp_display_write_frame}

### bsp_display_write_frame

```cpp
esp_err_t bsp_display_write_frame(const uint8_t * buffer)
```

Defined in bsp/bsp_display.h:108

Transmit Arbitrary 1-bit Monochrome Framebuffer to SSD1681 Controller.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `buffer` | `const uint8_t *` | Pointer to 5000-byte 1-bit packed bitmap buffer |

---

{#bsp_display_refresh}

### bsp_display_refresh

```cpp
esp_err_t bsp_display_refresh(bool partial_mode)
```

Defined in bsp/bsp_display.h:117

Trigger Physical e-Paper Waveform Refresh Cycle.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `partial_mode` | `bool` | true for fast partial refresh (~0.3s), false for full LUT refresh (~1.5s) |

---

{#bsp_display_deep_sleep}

### bsp_display_deep_sleep

```cpp
void bsp_display_deep_sleep(void)
```

Defined in bsp/bsp_display.h:123

Put SSD1681 Controller into Deep Sleep Mode (< 1 µA).

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_display_sleep}

### bsp_display_sleep

```cpp
esp_err_t bsp_display_sleep(void)
```

Defined in bsp/bsp_display.h:130

Alias for bsp_display_deep_sleep.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

---

{#bsp_display_wait_busy}

### bsp_display_wait_busy

```cpp
esp_err_t bsp_display_wait_busy(uint32_t timeout_ms)
```

Defined in bsp/bsp_display.h:144

Wait until E-Paper BUSY hardware signal goes inactive (Low).

#### Returns
esp_err_t ESP_OK when panel is ready, or ESP_ERR_TIMEOUT 

> [!NOTE]
> This function returns IMMEDIATELY as soon as the physical panel update completes (typically ~0.3s for partial refresh, ~1.5s for full refresh). The timeout_ms argument is a maximum safety limit, NOT a fixed sleep/delay duration.

> [!NOTE]
> To hold a screen visible for a fixed time (e.g. 10 seconds), call vTaskDelay() separately after this function returns.

Memory ownership: none. Behavior: Polling wait with TaskDelay yield. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `timeout_ms` | `uint32_t` | Maximum safety timeout in milliseconds before returning ESP_ERR_TIMEOUT |

---

{#bsp_shtc3_init}

### bsp_shtc3_init

```cpp
esp_err_t bsp_shtc3_init(void)
```

Defined in bsp/bsp_sensors.h:67

Initialize Sensirion SHTC3 Environmental Sensor.

Wakes the sensor from sleep, verifies the hardware Product ID, and puts it into ultra-low power standby.

#### Returns
esp_err_t ESP_OK on success, or ESP_ERR_NOT_FOUND if sensor is absent

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_sensors_init}

### bsp_sensors_init

`static` `inline`

```cpp
static inline esp_err_t bsp_sensors_init(void)
```

Defined in bsp/bsp_sensors.h:74

Generic sensor subsystem initialization alias.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

---

{#bsp_shtc3_read}

### bsp_shtc3_read

```cpp
esp_err_t bsp_shtc3_read(bsp_shtc3_data_t * out_data)
```

Defined in bsp/bsp_sensors.h:91

Read Temperature and Relative Humidity.

Executes a normal-power measurement sequence with clock stretching disabled, validates the 8-bit CRC polynomial (0x31) on both data words, and converts raw ADC words into physical units.

Conversion Equations:

* Temperature: T_Kelvin = 228.15 + (175.0 * raw_temp / 65536.0)
* Humidity: RH_% = 100.0 * (raw_rh / 65536.0)

#### Returns
esp_err_t ESP_OK on successful read and valid CRC

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `out_data` | [`bsp_shtc3_data_t`](#bsp_shtc3_data_t) * | Destination struct to receive telemetry |

---

{#bsp_shtc3_read_lp}

### bsp_shtc3_read_lp

```cpp
esp_err_t bsp_shtc3_read_lp(bsp_shtc3_data_t * out_data)
```

Defined in bsp/bsp_sensors.h:104

Read Temperature and Relative Humidity.

Executes a low-power measurement sequence with clock stretching disabled, validates the 8-bit CRC polynomial (0x31) on both data words, and converts raw ADC words into physical units.

#### Returns
esp_err_t ESP_OK on successful read and valid CRC

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `out_data` | [`bsp_shtc3_data_t`](#bsp_shtc3_data_t) * | Destination struct to receive telemetry |

---

{#bsp_shtc3_sleep}

### bsp_shtc3_sleep

```cpp
esp_err_t bsp_shtc3_sleep(void)
```

Defined in bsp/bsp_sensors.h:112

Put SHTC3 Sensor into Ultra-Low Power Sleep Mode (< 0.6 µA).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_shtc3_wakeup}

### bsp_shtc3_wakeup

```cpp
esp_err_t bsp_shtc3_wakeup(void)
```

Defined in bsp/bsp_sensors.h:122

Wake SHTC3 Sensor from Sleep Mode.

Must be followed by a minimum 240 µs wake-up delay before issuing commands.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_get_version-1}

### bsp_get_version

```cpp
const char * bsp_get_version(void)
```

Defined in bsp/bsp_version.h:41

Get BSP SemVer semantic version string.

#### Returns
const char* String formatted as "MAJOR.MINOR.PATCH"

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_get_version_val-1}

### bsp_get_version_val

```cpp
uint32_t bsp_get_version_val(void)
```

Defined in bsp/bsp_version.h:49

Get BSP version as integer representation.

#### Returns
uint32_t Version encoded as (MAJOR << 16) | (MINOR << 8) | PATCH

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#shtc3_crc8}

### shtc3_crc8

`static`

```cpp
static uint8_t shtc3_crc8(const uint8_t * data, size_t len)
```

Defined in bsp_sensors.c:61

---

{#calc_dew_point}

### calc_dew_point

`static`

```cpp
static float calc_dew_point(float temp_c, float rh)
```

Defined in bsp_sensors.c:80

---

{#calc_absolute_humidity}

### calc_absolute_humidity

`static`

```cpp
static float calc_absolute_humidity(float temp_c, float rh)
```

Defined in bsp_sensors.c:93

---

{#shtc3_execute_measurement}

### shtc3_execute_measurement

`static`

```cpp
static esp_err_t shtc3_execute_measurement(uint16_t cmd, bool low_power, bsp_shtc3_data_t * out_data)
```

Defined in bsp_sensors.c:110

---

{#bsp_shtc3_wakeup-1}

### bsp_shtc3_wakeup

```cpp
esp_err_t bsp_shtc3_wakeup(void)
```

Defined in bsp_sensors.c:185

Wake SHTC3 Sensor from Sleep Mode.

Must be followed by a minimum 240 µs wake-up delay before issuing commands.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_shtc3_sleep-1}

### bsp_shtc3_sleep

```cpp
esp_err_t bsp_shtc3_sleep(void)
```

Defined in bsp_sensors.c:194

Put SHTC3 Sensor into Ultra-Low Power Sleep Mode (< 0.6 µA).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_shtc3_init-1}

### bsp_shtc3_init

```cpp
esp_err_t bsp_shtc3_init(void)
```

Defined in bsp_sensors.c:200

Initialize Sensirion SHTC3 Environmental Sensor.

Wakes the sensor from sleep, verifies the hardware Product ID, and puts it into ultra-low power standby.

#### Returns
esp_err_t ESP_OK on success, or ESP_ERR_NOT_FOUND if sensor is absent

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_shtc3_read-1}

### bsp_shtc3_read

```cpp
esp_err_t bsp_shtc3_read(bsp_shtc3_data_t * out_data)
```

Defined in bsp_sensors.c:241

Read Temperature and Relative Humidity.

Executes a normal-power measurement sequence with clock stretching disabled, validates the 8-bit CRC polynomial (0x31) on both data words, and converts raw ADC words into physical units.

Conversion Equations:

* Temperature: T_Kelvin = 228.15 + (175.0 * raw_temp / 65536.0)
* Humidity: RH_% = 100.0 * (raw_rh / 65536.0)

#### Returns
esp_err_t ESP_OK on successful read and valid CRC

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `out_data` | [`bsp_shtc3_data_t`](#bsp_shtc3_data_t) * | Destination struct to receive telemetry |

---

{#bsp_shtc3_read_lp-1}

### bsp_shtc3_read_lp

```cpp
esp_err_t bsp_shtc3_read_lp(bsp_shtc3_data_t * out_data)
```

Defined in bsp_sensors.c:246

Read Temperature and Relative Humidity.

Executes a low-power measurement sequence with clock stretching disabled, validates the 8-bit CRC polynomial (0x31) on both data words, and converts raw ADC words into physical units.

#### Returns
esp_err_t ESP_OK on successful read and valid CRC

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `out_data` | [`bsp_shtc3_data_t`](#bsp_shtc3_data_t) * | Destination struct to receive telemetry |

---

{#bsp_rtc_mem_init}

### bsp_rtc_mem_init

```cpp
esp_err_t bsp_rtc_mem_init(void)
```

Defined in bsp/bsp_rtc_mem.h:74

Initialize or validate RTC Slow Memory state.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_rtc_mem_get_state}

### bsp_rtc_mem_get_state

```cpp
bsp_rtc_state_t * bsp_rtc_mem_get_state(void)
```

Defined in bsp/bsp_rtc_mem.h:82

Get direct pointer to RTC state struct.

#### Returns
bsp_rtc_state_t* Pointer to RTC Slow Memory struct

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_rtc_mem_get_boot_count}

### bsp_rtc_mem_get_boot_count

```cpp
uint32_t bsp_rtc_mem_get_boot_count(void)
```

Defined in bsp/bsp_rtc_mem.h:90

Get total boot count.

#### Returns
uint32_t Boot count

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_rtc_mem_set_next_init_mode}

### bsp_rtc_mem_set_next_init_mode

```cpp
void bsp_rtc_mem_set_next_init_mode(bsp_init_mode_t mode)
```

Defined in bsp/bsp_rtc_mem.h:98

Set the desired initialization mode for the next wake cycle.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `mode` | [`bsp_init_mode_t`](#bsp_init_mode_t) | Target init mode |

---

{#bsp_rtc_mem_get_next_init_mode}

### bsp_rtc_mem_get_next_init_mode

```cpp
bsp_init_mode_t bsp_rtc_mem_get_next_init_mode(void)
```

Defined in bsp/bsp_rtc_mem.h:106

Get the configured initialization mode for the current/next cycle.

#### Returns
[bsp_init_mode_t](#bsp_init_mode_t) Init mode

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_rtc_mem_read_scratchpad}

### bsp_rtc_mem_read_scratchpad

```cpp
esp_err_t bsp_rtc_mem_read_scratchpad(uint8_t * dest, size_t len)
```

Defined in bsp/bsp_rtc_mem.h:116

Read arbitrary user scratchpad bytes from RTC memory.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `dest` | `uint8_t *` | Destination buffer |
| `len` | `size_t` | Number of bytes (max 32) |

---

{#bsp_rtc_mem_write_scratchpad}

### bsp_rtc_mem_write_scratchpad

```cpp
esp_err_t bsp_rtc_mem_write_scratchpad(const uint8_t * src, size_t len)
```

Defined in bsp/bsp_rtc_mem.h:126

Write arbitrary user scratchpad bytes to RTC memory.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `src` | `const uint8_t *` | Source buffer |
| `len` | `size_t` | Number of bytes (max 32) |

---

{#bsp_rtc_mem_save_display_frame}

### bsp_rtc_mem_save_display_frame

```cpp
esp_err_t bsp_rtc_mem_save_display_frame(const uint8_t * frame, size_t len)
```

Defined in bsp/bsp_rtc_mem.h:136

Save 200x200 1-bit EPD frame buffer to RTC Slow Memory for partial refresh persistence.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `frame` | `const uint8_t *` | Pointer to 5000-byte frame buffer |
| `len` | `size_t` | Size of buffer in bytes |

---

{#bsp_rtc_mem_load_display_frame}

### bsp_rtc_mem_load_display_frame

```cpp
esp_err_t bsp_rtc_mem_load_display_frame(uint8_t * dest, size_t len)
```

Defined in bsp/bsp_rtc_mem.h:146

Load 200x200 1-bit EPD frame buffer from RTC Slow Memory.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `dest` | `uint8_t *` | Destination buffer (5000 bytes) |
| `len` | `size_t` | Size of buffer in bytes |

---

{#bsp_rtc_mem_has_display_frame}

### bsp_rtc_mem_has_display_frame

```cpp
bool bsp_rtc_mem_has_display_frame(void)
```

Defined in bsp/bsp_rtc_mem.h:154

Check if a valid display frame is stored in RTC Slow Memory.

#### Returns
true if valid frame exists

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_rtc_mem_reset}

### bsp_rtc_mem_reset

```cpp
void bsp_rtc_mem_reset(void)
```

Defined in bsp/bsp_rtc_mem.h:160

Reset RTC memory structure to defaults.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_rtc_mem_init-1}

### bsp_rtc_mem_init

```cpp
esp_err_t bsp_rtc_mem_init(void)
```

Defined in bsp_rtc_mem.c:27

Initialize or validate RTC Slow Memory state.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_rtc_mem_get_state-1}

### bsp_rtc_mem_get_state

```cpp
bsp_rtc_state_t * bsp_rtc_mem_get_state(void)
```

Defined in bsp_rtc_mem.c:43

Get direct pointer to RTC state struct.

#### Returns
bsp_rtc_state_t* Pointer to RTC Slow Memory struct

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_rtc_mem_get_boot_count-1}

### bsp_rtc_mem_get_boot_count

```cpp
uint32_t bsp_rtc_mem_get_boot_count(void)
```

Defined in bsp_rtc_mem.c:48

Get total boot count.

#### Returns
uint32_t Boot count

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_rtc_mem_set_next_init_mode-1}

### bsp_rtc_mem_set_next_init_mode

```cpp
void bsp_rtc_mem_set_next_init_mode(bsp_init_mode_t mode)
```

Defined in bsp_rtc_mem.c:53

Set the desired initialization mode for the next wake cycle.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `mode` | [`bsp_init_mode_t`](#bsp_init_mode_t) | Target init mode |

---

{#bsp_rtc_mem_get_next_init_mode-1}

### bsp_rtc_mem_get_next_init_mode

```cpp
bsp_init_mode_t bsp_rtc_mem_get_next_init_mode(void)
```

Defined in bsp_rtc_mem.c:58

Get the configured initialization mode for the current/next cycle.

#### Returns
[bsp_init_mode_t](#bsp_init_mode_t) Init mode

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_rtc_mem_read_scratchpad-1}

### bsp_rtc_mem_read_scratchpad

```cpp
esp_err_t bsp_rtc_mem_read_scratchpad(uint8_t * dest, size_t len)
```

Defined in bsp_rtc_mem.c:63

Read arbitrary user scratchpad bytes from RTC memory.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `dest` | `uint8_t *` | Destination buffer |
| `len` | `size_t` | Number of bytes (max 32) |

---

{#bsp_rtc_mem_write_scratchpad-1}

### bsp_rtc_mem_write_scratchpad

```cpp
esp_err_t bsp_rtc_mem_write_scratchpad(const uint8_t * src, size_t len)
```

Defined in bsp_rtc_mem.c:72

Write arbitrary user scratchpad bytes to RTC memory.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `src` | `const uint8_t *` | Source buffer |
| `len` | `size_t` | Number of bytes (max 32) |

---

{#bsp_rtc_mem_save_display_frame-1}

### bsp_rtc_mem_save_display_frame

```cpp
esp_err_t bsp_rtc_mem_save_display_frame(const uint8_t * frame, size_t len)
```

Defined in bsp_rtc_mem.c:81

Save 200x200 1-bit EPD frame buffer to RTC Slow Memory for partial refresh persistence.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `frame` | `const uint8_t *` | Pointer to 5000-byte frame buffer |
| `len` | `size_t` | Size of buffer in bytes |

---

{#bsp_rtc_mem_load_display_frame-1}

### bsp_rtc_mem_load_display_frame

```cpp
esp_err_t bsp_rtc_mem_load_display_frame(uint8_t * dest, size_t len)
```

Defined in bsp_rtc_mem.c:91

Load 200x200 1-bit EPD frame buffer from RTC Slow Memory.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `dest` | `uint8_t *` | Destination buffer (5000 bytes) |
| `len` | `size_t` | Size of buffer in bytes |

---

{#bsp_rtc_mem_has_display_frame-1}

### bsp_rtc_mem_has_display_frame

```cpp
bool bsp_rtc_mem_has_display_frame(void)
```

Defined in bsp_rtc_mem.c:100

Check if a valid display frame is stored in RTC Slow Memory.

#### Returns
true if valid frame exists

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_rtc_mem_reset-1}

### bsp_rtc_mem_reset

```cpp
void bsp_rtc_mem_reset(void)
```

Defined in bsp_rtc_mem.c:105

Reset RTC memory structure to defaults.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_app_start}

### bsp_app_start

```cpp
esp_err_t bsp_app_start(const bsp_app_lifecycle_t * lifecycle)
```

Defined in bsp/bsp_lifecycle.h:99

Start BSP Application Engine with Lifecycle Hooks.

Automatically initializes RTC memory, inspects reset reason, selects the optimal hardware initialization mode (FULL on cold boot, FAST/MIN on wake), populates the wake context, and dispatches to on_cold_boot or on_wake.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `lifecycle` | const [`bsp_app_lifecycle_t`](#bsp_app_lifecycle_t) * | Pointer to [bsp_app_lifecycle_t](#bsp_app_lifecycle_t) configuration |

---

{#bsp_lifecycle_get_context}

### bsp_lifecycle_get_context

```cpp
esp_err_t bsp_lifecycle_get_context(bsp_wake_context_t * ctx)
```

Defined in bsp/bsp_lifecycle.h:108

Retrieve current wake context.

#### Returns
esp_err_t ESP_OK on success, ESP_ERR_INVALID_STATE if context is not available

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `ctx` | [`bsp_wake_context_t`](#bsp_wake_context_t) * | Pointer to [bsp_wake_context_t](#bsp_wake_context_t) destination |

---

{#bsp_lifecycle_set_stage}

### bsp_lifecycle_set_stage

```cpp
esp_err_t bsp_lifecycle_set_stage(uint8_t stage)
```

Defined in bsp/bsp_lifecycle.h:117

Set application stage index in RTC Slow Memory.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `stage` | `uint8_t` | Stage identifier (0..255) |

---

{#bsp_lifecycle_get_stage}

### bsp_lifecycle_get_stage

```cpp
uint8_t bsp_lifecycle_get_stage(void)
```

Defined in bsp/bsp_lifecycle.h:125

Get application stage index from RTC Slow Memory.

#### Returns
uint8_t Current stage identifier

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_lifecycle_save_state}

### bsp_lifecycle_save_state

```cpp
esp_err_t bsp_lifecycle_save_state(const void * data, size_t len)
```

Defined in bsp/bsp_lifecycle.h:135

Save custom application state struct to RTC Slow Memory scratchpad (max 31 bytes).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `data` | `const void *` | Source buffer |
| `len` | `size_t` | Byte count (max 31) |

---

{#bsp_lifecycle_load_state}

### bsp_lifecycle_load_state

```cpp
esp_err_t bsp_lifecycle_load_state(void * out_data, size_t len)
```

Defined in bsp/bsp_lifecycle.h:145

Load custom application state struct from RTC Slow Memory scratchpad.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `out_data` | `void *` | Destination buffer |
| `len` | `size_t` | Byte count (max 31) |

---

{#bsp_lifecycle_enter_sleep}

### bsp_lifecycle_enter_sleep

```cpp
esp_err_t bsp_lifecycle_enter_sleep(const bsp_sleep_config_t * config)
```

Defined in bsp/bsp_lifecycle.h:154

Enter sleep with automatic before_sleep lifecycle hook and splash/chime execution.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `config` | const [`bsp_sleep_config_t`](#bsp_sleep_config_t) * | Sleep configuration (mode, duration, wake sources, next init mode) |

---

{#bsp_lifecycle_power_off}

### bsp_lifecycle_power_off

```cpp
void bsp_lifecycle_power_off(void)
```

Defined in bsp/bsp_lifecycle.h:162

Perform clean hardware power off and system shutdown.

Executes registered on_shutdown callback, drops BAT_CTRL power latch, and stops peripherals.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_lifecycle_invoke_shutdown}

### bsp_lifecycle_invoke_shutdown

```cpp
void bsp_lifecycle_invoke_shutdown(void)
```

Defined in bsp/bsp_lifecycle.h:168

Internal lifecycle dispatcher hook invoked by [bsp_power_off()](#bsp_power_off).

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#epd_set_cs}

### epd_set_cs

`static` `inline`

```cpp
static inline void epd_set_cs(uint8_t level)
```

Defined in bsp_display.cpp:69

---

{#epd_set_dc}

### epd_set_dc

`static` `inline`

```cpp
static inline void epd_set_dc(uint8_t level)
```

Defined in bsp_display.cpp:70

---

{#epd_set_rst}

### epd_set_rst

`static` `inline`

```cpp
static inline void epd_set_rst(uint8_t level)
```

Defined in bsp_display.cpp:71

---

{#bsp_display_wait_busy-1}

### bsp_display_wait_busy

```cpp
esp_err_t bsp_display_wait_busy(uint32_t timeout_ms)
```

Defined in bsp_display.cpp:73

Wait until E-Paper BUSY hardware signal goes inactive (Low).

#### Returns
esp_err_t ESP_OK when panel is ready, or ESP_ERR_TIMEOUT 

> [!NOTE]
> This function returns IMMEDIATELY as soon as the physical panel update completes (typically ~0.3s for partial refresh, ~1.5s for full refresh). The timeout_ms argument is a maximum safety limit, NOT a fixed sleep/delay duration.

> [!NOTE]
> To hold a screen visible for a fixed time (e.g. 10 seconds), call vTaskDelay() separately after this function returns.

Memory ownership: none. Behavior: Polling wait with TaskDelay yield. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `timeout_ms` | `uint32_t` | Maximum safety timeout in milliseconds before returning ESP_ERR_TIMEOUT |

---

{#epd_send_byte}

### epd_send_byte

`static`

```cpp
static void epd_send_byte(uint8_t data)
```

Defined in bsp_display.cpp:93

---

{#epd_send_cmd}

### epd_send_cmd

`static`

```cpp
static void epd_send_cmd(uint8_t cmd)
```

Defined in bsp_display.cpp:103

---

{#epd_send_data}

### epd_send_data

`static`

```cpp
static void epd_send_data(uint8_t data)
```

Defined in bsp_display.cpp:111

---

{#epd_write_bytes}

### epd_write_bytes

`static`

```cpp
static void epd_write_bytes(const uint8_t * data, size_t len)
```

Defined in bsp_display.cpp:119

---

{#epd_set_windows}

### epd_set_windows

`static`

```cpp
static void epd_set_windows(uint16_t x_start_byte, uint16_t y_start, uint16_t x_end_byte, uint16_t y_end)
```

Defined in bsp_display.cpp:158

---

{#epd_set_cursor}

### epd_set_cursor

`static`

```cpp
static void epd_set_cursor(uint16_t x_start_byte, uint16_t y_start)
```

Defined in bsp_display.cpp:171

---

{#epd_load_custom_lut}

### epd_load_custom_lut

`static`

```cpp
static void epd_load_custom_lut(const uint8_t * lut_buffer)
```

Defined in bsp_display.cpp:181

---

{#epd_apply_core_registers}

### epd_apply_core_registers

`static`

```cpp
static void epd_apply_core_registers(bool is_wake_init)
```

Defined in bsp_display.cpp:203

---

{#bsp_display_init-1}

### bsp_display_init

```cpp
esp_err_t bsp_display_init(void)
```

Defined in bsp_display.cpp:235

Initialize SSD1681 SPI Hardware Interface and Panel Controller.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_display_clear-1}

### bsp_display_clear

```cpp
void bsp_display_clear(void)
```

Defined in bsp_display.cpp:320

Clear Entire In-Memory Framebuffer to Pure White.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_display_flush-1}

### bsp_display_flush

```cpp
void bsp_display_flush(void)
```

Defined in bsp_display.cpp:327

Full OTP Waveform Hardware Refresh of In-Memory Framebuffer to Screen.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_display_flush_partial-1}

### bsp_display_flush_partial

```cpp
void bsp_display_flush_partial(void)
```

Defined in bsp_display.cpp:351

Fast Partial Waveform Refresh of Full Screen Area.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_display_flush_partial_area-1}

### bsp_display_flush_partial_area

```cpp
void bsp_display_flush_partial_area(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
```

Defined in bsp_display.cpp:356

Fast Partial Waveform Refresh of Specific Bounding Box Area.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `x_start` | `uint16_t` | Left coordinate |
| `y_start` | `uint16_t` | Top coordinate |
| `x_end` | `uint16_t` | Right coordinate |
| `y_end` | `uint16_t` | Bottom coordinate |

---

{#bsp_display_write_frame-1}

### bsp_display_write_frame

```cpp
esp_err_t bsp_display_write_frame(const uint8_t * buffer)
```

Defined in bsp_display.cpp:409

Transmit Arbitrary 1-bit Monochrome Framebuffer to SSD1681 Controller.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `buffer` | `const uint8_t *` | Pointer to 5000-byte 1-bit packed bitmap buffer |

---

{#bsp_display_refresh-1}

### bsp_display_refresh

```cpp
esp_err_t bsp_display_refresh(bool partial_mode)
```

Defined in bsp_display.cpp:420

Trigger Physical e-Paper Waveform Refresh Cycle.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `partial_mode` | `bool` | true for fast partial refresh (~0.3s), false for full LUT refresh (~1.5s) |

---

{#bsp_display_deep_sleep-1}

### bsp_display_deep_sleep

```cpp
void bsp_display_deep_sleep(void)
```

Defined in bsp_display.cpp:430

Put SSD1681 Controller into Deep Sleep Mode (< 1 µA).

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_display_sleep-1}

### bsp_display_sleep

```cpp
esp_err_t bsp_display_sleep(void)
```

Defined in bsp_display.cpp:437

Alias for bsp_display_deep_sleep.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees. 
#### Returns
esp_err_t ESP_OK on success, or appropriate ESP error code.

---

{#bsp_display_draw_pixel-1}

### bsp_display_draw_pixel

```cpp
void bsp_display_draw_pixel(uint16_t x, uint16_t y, bsp_display_color_t color)
```

Defined in bsp_display.cpp:443

Draw Single Pixel to Framebuffer.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `x` | `uint16_t` | Coordinate (0 to 199) |
| `y` | `uint16_t` | Coordinate (0 to 199) |
| `color` | [`bsp_display_color_t`](#bsp_display_color_t) | Pixel color (BLACK or WHITE) |

---

{#bsp_display_get_buffer-1}

### bsp_display_get_buffer

```cpp
uint8_t * bsp_display_get_buffer(void)
```

Defined in bsp_display.cpp:456

Retrieve Pointer to In-Memory Framebuffer (5000 bytes).

#### Returns
uint8_t* Buffer pointer

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_lifecycle_get_context-1}

### bsp_lifecycle_get_context

```cpp
esp_err_t bsp_lifecycle_get_context(bsp_wake_context_t * ctx)
```

Defined in bsp_lifecycle.c:33

Retrieve current wake context.

#### Returns
esp_err_t ESP_OK on success, ESP_ERR_INVALID_STATE if context is not available

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `ctx` | [`bsp_wake_context_t`](#bsp_wake_context_t) * | Pointer to [bsp_wake_context_t](#bsp_wake_context_t) destination |

---

{#bsp_lifecycle_set_stage-1}

### bsp_lifecycle_set_stage

```cpp
esp_err_t bsp_lifecycle_set_stage(uint8_t stage)
```

Defined in bsp_lifecycle.c:42

Set application stage index in RTC Slow Memory.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `stage` | `uint8_t` | Stage identifier (0..255) |

---

{#bsp_lifecycle_get_stage-1}

### bsp_lifecycle_get_stage

```cpp
uint8_t bsp_lifecycle_get_stage(void)
```

Defined in bsp_lifecycle.c:53

Get application stage index from RTC Slow Memory.

#### Returns
uint8_t Current stage identifier

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_lifecycle_save_state-1}

### bsp_lifecycle_save_state

```cpp
esp_err_t bsp_lifecycle_save_state(const void * data, size_t len)
```

Defined in bsp_lifecycle.c:62

Save custom application state struct to RTC Slow Memory scratchpad (max 31 bytes).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `data` | `const void *` | Source buffer |
| `len` | `size_t` | Byte count (max 31) |

---

{#bsp_lifecycle_load_state-1}

### bsp_lifecycle_load_state

```cpp
esp_err_t bsp_lifecycle_load_state(void * out_data, size_t len)
```

Defined in bsp_lifecycle.c:75

Load custom application state struct from RTC Slow Memory scratchpad.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `out_data` | `void *` | Destination buffer |
| `len` | `size_t` | Byte count (max 31) |

---

{#bsp_lifecycle_enter_sleep-1}

### bsp_lifecycle_enter_sleep

```cpp
esp_err_t bsp_lifecycle_enter_sleep(const bsp_sleep_config_t * config)
```

Defined in bsp_lifecycle.c:88

Enter sleep with automatic before_sleep lifecycle hook and splash/chime execution.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `config` | const [`bsp_sleep_config_t`](#bsp_sleep_config_t) * | Sleep configuration (mode, duration, wake sources, next init mode) |

---

{#bsp_lifecycle_power_off-1}

### bsp_lifecycle_power_off

```cpp
void bsp_lifecycle_power_off(void)
```

Defined in bsp_lifecycle.c:104

Perform clean hardware power off and system shutdown.

Executes registered on_shutdown callback, drops BAT_CTRL power latch, and stops peripherals.

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_lifecycle_invoke_shutdown-1}

### bsp_lifecycle_invoke_shutdown

```cpp
void bsp_lifecycle_invoke_shutdown(void)
```

Defined in bsp_lifecycle.c:110

Internal lifecycle dispatcher hook invoked by [bsp_power_off()](#bsp_power_off).

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_app_start-1}

### bsp_app_start

```cpp
esp_err_t bsp_app_start(const bsp_app_lifecycle_t * lifecycle)
```

Defined in bsp_lifecycle.c:118

Start BSP Application Engine with Lifecycle Hooks.

Automatically initializes RTC memory, inspects reset reason, selects the optimal hardware initialization mode (FULL on cold boot, FAST/MIN on wake), populates the wake context, and dispatches to on_cold_boot or on_wake.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `lifecycle` | const [`bsp_app_lifecycle_t`](#bsp_app_lifecycle_t) * | Pointer to [bsp_app_lifecycle_t](#bsp_app_lifecycle_t) configuration |

---

{#bsp_sensor_cal_init}

### bsp_sensor_cal_init

```cpp
esp_err_t bsp_sensor_cal_init(void)
```

Defined in bsp/bsp_sensor_cal.h:68

Initialize Thermal Calibration Subsystem & MCU Internal Temp Sensor.

Instantiates the ESP32-S3 internal TSENS peripheral, sets default K and alpha filter weights from Kconfig, and initializes raw SHTC3 hardware via [bsp_shtc3_init()](#bsp_shtc3_init).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_sensor_cal_read}

### bsp_sensor_cal_read

```cpp
esp_err_t bsp_sensor_cal_read(bsp_sensor_cal_data_t * out_data)
```

Defined in bsp/bsp_sensor_cal.h:81

Read Calibrated & Compensated Environmental Telemetry.

Reads raw SHTC3 telemetry, queries the MCU die temperature, applies EMA filtering, solves the two-node thermal divider model, and equalizes relative humidity. If dynamic compensation is disabled, outputs match 100% raw uncalibrated SHTC3 data.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `out_data` | [`bsp_sensor_cal_data_t`](#bsp_sensor_cal_data_t) * | Destination struct to receive calibrated metrics |

---

{#bsp_mcu_temp_read}

### bsp_mcu_temp_read

```cpp
esp_err_t bsp_mcu_temp_read(float * out_die_temp)
```

Defined in bsp/bsp_sensor_cal.h:90

Read Raw ESP32-S3 MCU Junction Temperature (°C).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `out_die_temp` | `float *` | Pointer to receive die temperature in Celsius |

---

{#bsp_sensor_cal_set_k}

### bsp_sensor_cal_set_k

```cpp
void bsp_sensor_cal_set_k(float k)
```

Defined in bsp/bsp_sensor_cal.h:98

Set Board Thermal Coupling Constant (K).

Memory ownership: none. Behavior: Non-blocking. Thread safety: thread-safe.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `k` | `float` | Thermal coupling ratio R_amb / R_pcb (default 0.380f) |

---

{#bsp_sensor_cal_get_k}

### bsp_sensor_cal_get_k

```cpp
float bsp_sensor_cal_get_k(void)
```

Defined in bsp/bsp_sensor_cal.h:106

Get Active Board Thermal Coupling Constant (K).

#### Returns
float Active K value

Memory ownership: none. Behavior: Non-blocking. Thread safety: thread-safe.

---

{#bsp_sensor_cal_set_alpha}

### bsp_sensor_cal_set_alpha

```cpp
void bsp_sensor_cal_set_alpha(float alpha)
```

Defined in bsp/bsp_sensor_cal.h:114

Set MCU Die Temp EMA Low-Pass Filter Alpha Weight.

Memory ownership: none. Behavior: Non-blocking. Thread safety: thread-safe.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `alpha` | `float` | Filter weight from 0.001 to 1.0 (default 0.050f) |

---

{#bsp_sensor_cal_get_alpha}

### bsp_sensor_cal_get_alpha

```cpp
float bsp_sensor_cal_get_alpha(void)
```

Defined in bsp/bsp_sensor_cal.h:122

Get Active MCU Die Temp EMA Filter Alpha Weight.

#### Returns
float Active alpha value

Memory ownership: none. Behavior: Non-blocking. Thread safety: thread-safe.

---

{#bsp_sensor_cal_enable}

### bsp_sensor_cal_enable

```cpp
void bsp_sensor_cal_enable(bool enable)
```

Defined in bsp/bsp_sensor_cal.h:130

Enable or Disable Dynamic Thermal Compensation.

Memory ownership: none. Behavior: Non-blocking. Thread safety: thread-safe.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `enable` | `bool` | true to apply dynamic MCU thermal compensation, false to return raw uncalibrated data |

---

{#bsp_sensor_cal_is_enabled}

### bsp_sensor_cal_is_enabled

```cpp
bool bsp_sensor_cal_is_enabled(void)
```

Defined in bsp/bsp_sensor_cal.h:138

Check if Dynamic Thermal Compensation is Currently Enabled.

#### Returns
bool true if enabled

Memory ownership: none. Behavior: Non-blocking. Thread safety: thread-safe.

---

{#calc_dew_point-1}

### calc_dew_point

`static`

```cpp
static float calc_dew_point(float temp_c, float rh)
```

Defined in bsp_sensor_cal.c:48

---

{#calc_absolute_humidity-1}

### calc_absolute_humidity

`static`

```cpp
static float calc_absolute_humidity(float temp_c, float rh)
```

Defined in bsp_sensor_cal.c:60

---

{#bsp_mcu_temp_read-1}

### bsp_mcu_temp_read

```cpp
esp_err_t bsp_mcu_temp_read(float * out_die_temp)
```

Defined in bsp_sensor_cal.c:73

Read Raw ESP32-S3 MCU Junction Temperature (°C).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `out_die_temp` | `float *` | Pointer to receive die temperature in Celsius |

---

{#bsp_sensor_cal_init-1}

### bsp_sensor_cal_init

```cpp
esp_err_t bsp_sensor_cal_init(void)
```

Defined in bsp_sensor_cal.c:98

Initialize Thermal Calibration Subsystem & MCU Internal Temp Sensor.

Instantiates the ESP32-S3 internal TSENS peripheral, sets default K and alpha filter weights from Kconfig, and initializes raw SHTC3 hardware via [bsp_shtc3_init()](#bsp_shtc3_init).

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

---

{#bsp_sensor_cal_read-1}

### bsp_sensor_cal_read

```cpp
esp_err_t bsp_sensor_cal_read(bsp_sensor_cal_data_t * out_data)
```

Defined in bsp_sensor_cal.c:121

Read Calibrated & Compensated Environmental Telemetry.

Reads raw SHTC3 telemetry, queries the MCU die temperature, applies EMA filtering, solves the two-node thermal divider model, and equalizes relative humidity. If dynamic compensation is disabled, outputs match 100% raw uncalibrated SHTC3 data.

#### Returns
esp_err_t ESP_OK on success

Memory ownership: none. Behavior: Blocking. Thread safety: no thread safety guarantees.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `out_data` | [`bsp_sensor_cal_data_t`](#bsp_sensor_cal_data_t) * | Destination struct to receive calibrated metrics |

---

{#bsp_sensor_cal_set_k-1}

### bsp_sensor_cal_set_k

```cpp
void bsp_sensor_cal_set_k(float k)
```

Defined in bsp_sensor_cal.c:199

Set Board Thermal Coupling Constant (K).

Memory ownership: none. Behavior: Non-blocking. Thread safety: thread-safe.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `k` | `float` | Thermal coupling ratio R_amb / R_pcb (default 0.380f) |

---

{#bsp_sensor_cal_get_k-1}

### bsp_sensor_cal_get_k

```cpp
float bsp_sensor_cal_get_k(void)
```

Defined in bsp_sensor_cal.c:206

Get Active Board Thermal Coupling Constant (K).

#### Returns
float Active K value

Memory ownership: none. Behavior: Non-blocking. Thread safety: thread-safe.

---

{#bsp_sensor_cal_set_alpha-1}

### bsp_sensor_cal_set_alpha

```cpp
void bsp_sensor_cal_set_alpha(float alpha)
```

Defined in bsp_sensor_cal.c:211

Set MCU Die Temp EMA Low-Pass Filter Alpha Weight.

Memory ownership: none. Behavior: Non-blocking. Thread safety: thread-safe.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `alpha` | `float` | Filter weight from 0.001 to 1.0 (default 0.050f) |

---

{#bsp_sensor_cal_get_alpha-1}

### bsp_sensor_cal_get_alpha

```cpp
float bsp_sensor_cal_get_alpha(void)
```

Defined in bsp_sensor_cal.c:219

Get Active MCU Die Temp EMA Filter Alpha Weight.

#### Returns
float Active alpha value

Memory ownership: none. Behavior: Non-blocking. Thread safety: thread-safe.

---

{#bsp_sensor_cal_enable-1}

### bsp_sensor_cal_enable

```cpp
void bsp_sensor_cal_enable(bool enable)
```

Defined in bsp_sensor_cal.c:224

Enable or Disable Dynamic Thermal Compensation.

Memory ownership: none. Behavior: Non-blocking. Thread safety: thread-safe.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `enable` | `bool` | true to apply dynamic MCU thermal compensation, false to return raw uncalibrated data |

---

{#bsp_sensor_cal_is_enabled-1}

### bsp_sensor_cal_is_enabled

```cpp
bool bsp_sensor_cal_is_enabled(void)
```

Defined in bsp_sensor_cal.c:230

Check if Dynamic Thermal Compensation is Currently Enabled.

#### Returns
bool true if enabled

Memory ownership: none. Behavior: Non-blocking. Thread safety: thread-safe.

## Variables

---

{#tag}

### TAG

`static`

```cpp
const char * TAG = "bsp_tb"
```

Defined in bsp_tb.c:29

---

{#s_mqtt_client}

### s_mqtt_client

`static`

```cpp
esp_mqtt_client_handle_t s_mqtt_client = NULL
```

Defined in bsp_tb.c:31

---

{#s_mqtt_events}

### s_mqtt_events

`static`

```cpp
EventGroupHandle_t s_mqtt_events = NULL
```

Defined in bsp_tb.c:32

---

{#s_tb_cfg}

### s_tb_cfg

`static`

```cpp
bsp_tb_config_t s_tb_cfg = {0}
```

Type: [`bsp_tb_config_t`](#bsp_tb_config_t)

Defined in bsp_tb.c:33

---

{#tag-1}

### TAG

`static`

```cpp
const char * TAG = "bsp_i2c"
```

Defined in bsp_i2c.c:27

---

{#s_i2c_bus_handle}

### s_i2c_bus_handle

`static`

```cpp
i2c_master_bus_handle_t s_i2c_bus_handle = NULL
```

Defined in bsp_i2c.c:29

---

{#s_i2c_mutex}

### s_i2c_mutex

`static`

```cpp
SemaphoreHandle_t s_i2c_mutex = NULL
```

Defined in bsp_i2c.c:30

---

{#i2c_timeout_ms}

### I2C_TIMEOUT_MS

`static`

```cpp
const int I2C_TIMEOUT_MS = 100
```

Defined in bsp_i2c.c:31

---

{#s_dev_cache}

### s_dev_cache

`static`

```cpp
cached_i2c_dev_t s_dev_cache[8]
```

Defined in bsp_i2c.c:39

---

{#s_dev_cache_count}

### s_dev_cache_count

`static`

```cpp
size_t s_dev_cache_count = 0
```

Defined in bsp_i2c.c:40

---

{#tag-2}

### TAG

`static`

```cpp
const char * TAG = "bsp_nvs"
```

Defined in bsp_nvs.c:21

---

{#s_nvs_initialized}

### s_nvs_initialized

`static`

```cpp
bool s_nvs_initialized = false
```

Defined in bsp_nvs.c:25

---

{#tag-3}

### TAG

`static`

```cpp
const char * TAG = "bsp_ota"
```

Defined in bsp_ota.c:22

---

{#s_update_partition}

### s_update_partition

`static`

```cpp
const esp_partition_t * s_update_partition = NULL
```

Defined in bsp_ota.c:24

---

{#tag-4}

### TAG

`static`

```cpp
const char * TAG = "bsp_rtc"
```

Defined in bsp_rtc.c:51

---

{#tag-5}

### TAG

`static`

```cpp
const char * TAG = "bsp_prov"
```

Defined in bsp_prov.c:29

---

{#s_prov_running}

### s_prov_running

`static`

```cpp
bool s_prov_running = false
```

Defined in bsp_prov.c:31

---

{#s_prov_cb}

### s_prov_cb

`static`

```cpp
bsp_prov_event_cb_t s_prov_cb = NULL
```

Type: [`bsp_prov_event_cb_t`](#bsp_prov_event_cb_t)

Defined in bsp_prov.c:32

---

{#s_prov_user_data}

### s_prov_user_data

`static`

```cpp
void * s_prov_user_data = NULL
```

Defined in bsp_prov.c:33

---

{#s_active_serv_name}

### s_active_serv_name

`static`

```cpp
char s_active_serv_name[48]
```

Defined in bsp_prov.c:34

---

{#tag-6}

### TAG

`static`

```cpp
const char * TAG = "bsp_time"
```

Defined in bsp_time.c:26

---

{#s_periodic_sync_timer}

### s_periodic_sync_timer

`static`

```cpp
esp_timer_handle_t s_periodic_sync_timer = NULL
```

Defined in bsp_time.c:28

---

{#tag-7}

### TAG

`static`

```cpp
const char * TAG = "bsp_wifi"
```

Defined in bsp_wifi.c:34

---

{#s_wifi_event_group}

### s_wifi_event_group

`static`

```cpp
EventGroupHandle_t s_wifi_event_group = NULL
```

Defined in bsp_wifi.c:40

---

{#s_sta_netif}

### s_sta_netif

`static`

```cpp
esp_netif_t * s_sta_netif = NULL
```

Defined in bsp_wifi.c:41

---

{#s_is_initialized}

### s_is_initialized

`static`

```cpp
bool s_is_initialized = false
```

Defined in bsp_wifi.c:42

---

{#s_is_connected}

### s_is_connected

`static`

```cpp
bool s_is_connected = false
```

Defined in bsp_wifi.c:43

---

{#s_rtc_cache}

### s_rtc_cache

`static`

```cpp
RTC_DATA_ATTR rtc_wifi_cache_t s_rtc_cache = {0}
```

Type: RTC_DATA_ATTR [`rtc_wifi_cache_t`](#rtc_wifi_cache_t)

Defined in bsp_wifi.c:57

---

{#tag-8}

### TAG

`static`

```cpp
const char * TAG = "bsp_audio"
```

Defined in bsp_audio.c:30

---

{#sine_lut_256}

### SINE_LUT_256

`static`

```cpp
const int16_t SINE_LUT_256[256]
```

Defined in bsp_audio.c:34

---

{#s_tx_chan}

### s_tx_chan

`static`

```cpp
i2s_chan_handle_t s_tx_chan = NULL
```

Defined in bsp_audio.c:69

---

{#s_codec}

### s_codec

`static`

```cpp
esp_codec_dev_handle_t s_codec = NULL
```

Defined in bsp_audio.c:70

---

{#s_audio_inited}

### s_audio_inited

`static`

```cpp
bool s_audio_inited = false
```

Defined in bsp_audio.c:71

---

{#s_audio_in_standby}

### s_audio_in_standby

`static`

```cpp
bool s_audio_in_standby = false
```

Defined in bsp_audio.c:72

---

{#s_low_power_mode}

### s_low_power_mode

`static`

```cpp
bool s_low_power_mode = false
```

Defined in bsp_audio.c:76

---

{#tag-9}

### TAG

`static`

```cpp
const char * TAG = "bsp_power"
```

Defined in bsp_power.c:49

---

{#s_adc_handle}

### s_adc_handle

`static`

```cpp
adc_oneshot_unit_handle_t s_adc_handle = NULL
```

Defined in bsp_power.c:63

---

{#s_cali_handle}

### s_cali_handle

`static`

```cpp
adc_cali_handle_t s_cali_handle = NULL
```

Defined in bsp_power.c:64

---

{#s_calibrated}

### s_calibrated

`static`

```cpp
bool s_calibrated = false
```

Defined in bsp_power.c:65

---

{#s_led_state}

### s_led_state

`static`

```cpp
bool s_led_state = false
```

Defined in bsp_power.c:66

---

{#s_power_inited}

### s_power_inited

`static`

```cpp
bool s_power_inited = false
```

Defined in bsp_power.c:67

---

{#s_shutdown_cb}

### s_shutdown_cb

`static`

```cpp
bsp_power_off_cb_t s_shutdown_cb = NULL
```

Type: [`bsp_power_off_cb_t`](#bsp_power_off_cb_t)

Defined in bsp_power.c:68

---

{#s_shutdown_user_data}

### s_shutdown_user_data

`static`

```cpp
void * s_shutdown_user_data = NULL
```

Defined in bsp_power.c:69

---

{#s_battery_ocv_lut}

### s_battery_ocv_lut

`static`

```cpp
const battery_lut_point_t s_battery_ocv_lut[]
```

Defined in bsp_power.c:72

---

{#tag-10}

### TAG

`static`

```cpp
const char * TAG = "bsp_button"
```

Defined in bsp_button.c:26

---

{#s_buttons}

### s_buttons

`static`

```cpp
button_dev_t s_buttons[BSP_BUTTON_COUNT]
```

Defined in bsp_button.c:54

---

{#s_cfg}

### s_cfg

`static`

```cpp
bsp_button_config_t s_cfg
```

Type: [`bsp_button_config_t`](#bsp_button_config_t)

Defined in bsp_button.c:55

---

{#s_timer_handle}

### s_timer_handle

`static`

```cpp
esp_timer_handle_t s_timer_handle = NULL
```

Defined in bsp_button.c:56

---

{#s_inited}

### s_inited

`static`

```cpp
bool s_inited = false
```

Defined in bsp_button.c:57

---

{#tag-11}

### TAG

`static`

```cpp
const char * TAG = "bsp_common"
```

Defined in bsp_common.c:31

---

{#tag-12}

### TAG

`static`

```cpp
const char * TAG = "bsp_lvgl"
```

Defined in bsp_lvgl.cpp:28

---

{#s_lv_display}

### s_lv_display

`static`

```cpp
lv_display_t * s_lv_display = NULL
```

Defined in bsp_lvgl.cpp:30

---

{#s_lvgl_task_handle}

### s_lvgl_task_handle

`static`

```cpp
TaskHandle_t s_lvgl_task_handle = NULL
```

Defined in bsp_lvgl.cpp:31

---

{#s_lvgl_task_running}

### s_lvgl_task_running

`static`

```cpp
bool s_lvgl_task_running = false
```

Defined in bsp_lvgl.cpp:32

---

{#s_flush_counter}

### s_flush_counter

`static`

```cpp
uint32_t s_flush_counter = 0
```

Defined in bsp_lvgl.cpp:34

---

{#s_first_boot_flush}

### s_first_boot_flush

`static`

```cpp
bool s_first_boot_flush = true
```

Defined in bsp_lvgl.cpp:35

---

{#s_dirty_x1}

### s_dirty_x1

`static`

```cpp
int16_t s_dirty_x1 = 32767
```

Defined in bsp_lvgl.cpp:41

---

{#s_dirty_y1}

### s_dirty_y1

`static`

```cpp
int16_t s_dirty_y1 = 32767
```

Defined in bsp_lvgl.cpp:41

---

{#s_dirty_x2}

### s_dirty_x2

`static`

```cpp
int16_t s_dirty_x2 = -1
```

Defined in bsp_lvgl.cpp:42

---

{#s_dirty_y2}

### s_dirty_y2

`static`

```cpp
int16_t s_dirty_y2 = -1
```

Defined in bsp_lvgl.cpp:42

---

{#tag-13}

### TAG

`static`

```cpp
const char * TAG = "bsp_sdcard"
```

Defined in bsp_sdcard.c:23

---

{#s_sd_card}

### s_sd_card

`static`

```cpp
sdmmc_card_t * s_sd_card = NULL
```

Defined in bsp_sdcard.c:25

---

{#tag-14}

### TAG

`static`

```cpp
const char * TAG = "bsp_splash"
```

Defined in bsp_splash.c:19

---

{#s_splash_table}

### s_splash_table

`static`

```cpp
splash_entry_t s_splash_table[BSP_SPLASH_MAX]
```

Defined in bsp_splash.c:31

---

{#s_chime_table}

### s_chime_table

`static`

```cpp
chime_entry_t s_chime_table[BSP_CHIME_MAX]
```

Defined in bsp_splash.c:32

---

{#tag-15}

### TAG

`static`

```cpp
const char * TAG = "bsp_sensors"
```

Defined in bsp_sensors.c:34

---

{#tag-16}

### TAG

`static`

```cpp
const char * TAG = "bsp_rtc_mem"
```

Defined in bsp_rtc_mem.c:20

---

{#s_rtc_state}

### s_rtc_state

`static`

```cpp
RTC_DATA_ATTR bsp_rtc_state_t s_rtc_state
```

Type: RTC_DATA_ATTR [`bsp_rtc_state_t`](#bsp_rtc_state_t)

Defined in bsp_rtc_mem.c:22

---

{#s_rtc_frame_buffer}

### s_rtc_frame_buffer

`static`

```cpp
RTC_DATA_ATTR uint8_t s_rtc_frame_buffer[5000]
```

Defined in bsp_rtc_mem.c:23

---

{#s_rtc_frame_valid}

### s_rtc_frame_valid

`static`

```cpp
RTC_DATA_ATTR bool s_rtc_frame_valid = false
```

Defined in bsp_rtc_mem.c:24

---

{#s_boot_counted}

### s_boot_counted

`static`

```cpp
bool s_boot_counted = false
```

Defined in bsp_rtc_mem.c:25

---

{#tag-17}

### TAG

`static`

```cpp
const char * TAG = "bsp_display"
```

Defined in bsp_display.cpp:28

---

{#wf_partial_1in54}

### WF_PARTIAL_1IN54

`static`

```cpp
const uint8_t WF_PARTIAL_1IN54[159]
```

Defined in bsp_display.cpp:34

---

{#s_spi_handle}

### s_spi_handle

`static`

```cpp
spi_device_handle_t s_spi_handle = NULL
```

Defined in bsp_display.cpp:64

---

{#s_frame_buffer}

### s_frame_buffer

`static`

```cpp
uint8_t * s_frame_buffer = NULL
```

Defined in bsp_display.cpp:65

---

{#s_prev_frame_buffer}

### s_prev_frame_buffer

`static`

```cpp
uint8_t * s_prev_frame_buffer = NULL
```

Defined in bsp_display.cpp:66

---

{#s_partial_refresh_count}

### s_partial_refresh_count

`static`

```cpp
uint32_t s_partial_refresh_count = 0
```

Defined in bsp_display.cpp:67

---

{#s_dma_bounce_buf}

### s_dma_bounce_buf

`static`

```cpp
uint8_t * s_dma_bounce_buf = NULL
```

Defined in bsp_display.cpp:91

---

{#tag-18}

### TAG

`static`

```cpp
const char * TAG = "bsp_lifecycle"
```

Defined in bsp_lifecycle.c:27

---

{#s_active_lifecycle}

### s_active_lifecycle

`static`

```cpp
bsp_app_lifecycle_t s_active_lifecycle = {0}
```

Type: [`bsp_app_lifecycle_t`](#bsp_app_lifecycle_t)

Defined in bsp_lifecycle.c:29

---

{#s_current_context}

### s_current_context

`static`

```cpp
bsp_wake_context_t s_current_context = {0}
```

Type: [`bsp_wake_context_t`](#bsp_wake_context_t)

Defined in bsp_lifecycle.c:30

---

{#s_context_valid}

### s_context_valid

`static`

```cpp
bool s_context_valid = false
```

Defined in bsp_lifecycle.c:31

---

{#tag-19}

### TAG

`static`

```cpp
const char * TAG = "bsp_sensor_cal"
```

Defined in bsp_sensor_cal.c:21

---

{#s_temp_sensor}

### s_temp_sensor

`static`

```cpp
temperature_sensor_handle_t s_temp_sensor = NULL
```

Defined in bsp_sensor_cal.c:26

---

{#s_mcu_temp_inited}

### s_mcu_temp_inited

`static`

```cpp
bool s_mcu_temp_inited = false
```

Defined in bsp_sensor_cal.c:27

---

{#s_die_temp_filt}

### s_die_temp_filt

`static`

```cpp
float s_die_temp_filt = -999.0f
```

Defined in bsp_sensor_cal.c:28

---

{#s_comp_enabled}

### s_comp_enabled

`static`

```cpp
bool s_comp_enabled = false
```

Defined in bsp_sensor_cal.c:33

---

{#s_coupling_k}

### s_coupling_k

`static`

```cpp
float s_coupling_k = 0.380f
```

Defined in bsp_sensor_cal.c:39

---

{#s_filter_alpha}

### s_filter_alpha

`static`

```cpp
float s_filter_alpha = 0.050f
```

Defined in bsp_sensor_cal.c:45

{#bsp_config_t}

## bsp_config_t

```cpp
#include <bsp/bsp.h>
```

```cpp
struct bsp_config_t
```

Defined in bsp/bsp.h:56

Modular Hardware Initialization Configuration.

### Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| `bool` | [`init_power`](#init_power)  | Hold LDO power rail HIGH and calibrate ADC battery monitor (Default: true). |
| `bool` | [`init_i2c`](#init_i2c)  | Initialize shared I2C bus at 400kHz with mutex protection (Default: true). |
| `bool` | [`init_sensors`](#init_sensors)  | Initialize Sensirion SHTC3 environmental sensor (Default: true). |
| `bool` | [`init_rtc`](#init_rtc)  | Initialize PCF85063A hardware real-time clock (Default: true). |
| `bool` | [`init_buttons`](#init_buttons)  | Initialize debounced interrupt handlers for BOOT and POWER keys (Default: true). |
| `bool` | [`init_audio`](#init_audio)  | Initialize ES8311 I2S audio codec & NS4168 amp (Default: true). |
| `float` | [`audio_volume`](#audio_volume)  | Initial audio volume 0-100 (Default: 80.0). |
| `bool` | [`init_sdcard`](#init_sdcard)  | Mount MicroSD card over SDMMC FATFS (Default: false). |
| `bool` | [`init_display`](#init_display)  | Initialize SSD1681 1.54" SPI e-Paper display (Default: true). |
| `bool` | [`init_nvs`](#init_nvs)  | Initialize non-volatile flash storage (Default: true). |
| `bool` | [`start_lvgl`](#start_lvgl)  | Spawn LVGL v9 FreeRTOS render task pinned to Core 1 (Default: true). |

---

{#init_power}

#### init_power

```cpp
bool init_power
```

Defined in bsp/bsp.h:57

Hold LDO power rail HIGH and calibrate ADC battery monitor (Default: true).

---

{#init_i2c}

#### init_i2c

```cpp
bool init_i2c
```

Defined in bsp/bsp.h:58

Initialize shared I2C bus at 400kHz with mutex protection (Default: true).

---

{#init_sensors}

#### init_sensors

```cpp
bool init_sensors
```

Defined in bsp/bsp.h:59

Initialize Sensirion SHTC3 environmental sensor (Default: true).

---

{#init_rtc}

#### init_rtc

```cpp
bool init_rtc
```

Defined in bsp/bsp.h:60

Initialize PCF85063A hardware real-time clock (Default: true).

---

{#init_buttons}

#### init_buttons

```cpp
bool init_buttons
```

Defined in bsp/bsp.h:61

Initialize debounced interrupt handlers for BOOT and POWER keys (Default: true).

---

{#init_audio}

#### init_audio

```cpp
bool init_audio
```

Defined in bsp/bsp.h:62

Initialize ES8311 I2S audio codec & NS4168 amp (Default: true).

---

{#audio_volume}

#### audio_volume

```cpp
float audio_volume
```

Defined in bsp/bsp.h:63

Initial audio volume 0-100 (Default: 80.0).

---

{#init_sdcard}

#### init_sdcard

```cpp
bool init_sdcard
```

Defined in bsp/bsp.h:64

Mount MicroSD card over SDMMC FATFS (Default: false).

---

{#init_display}

#### init_display

```cpp
bool init_display
```

Defined in bsp/bsp.h:65

Initialize SSD1681 1.54" SPI e-Paper display (Default: true).

---

{#init_nvs}

#### init_nvs

```cpp
bool init_nvs
```

Defined in bsp/bsp.h:66

Initialize non-volatile flash storage (Default: true).

---

{#start_lvgl}

#### start_lvgl

```cpp
bool start_lvgl
```

Defined in bsp/bsp.h:67

Spawn LVGL v9 FreeRTOS render task pinned to Core 1 (Default: true).

{#button_dev_t}

## button_dev_t

```cpp
struct button_dev_t
```

Defined in bsp_button.c:44

### Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| `gpio_num_t` | [`gpio`](#gpio)  | gpio value |
| [`button_state_t`](#button_state_t) | [`state`](#state)  | state value |
| `uint32_t` | [`press_start_tick`](#press_start_tick)  | press_start_tick value |
| `uint32_t` | [`release_tick`](#release_tick)  | release_tick value |
| `uint32_t` | [`stable_state_ticks`](#stable_state_ticks)  | stable_state_ticks value |
| `bool` | [`long_press_fired`](#long_press_fired)  | long_press_fired value |
| [`button_callback_entry_t`](#button_callback_entry_t) | [`callbacks`](#callbacks)  | callbacks[BSP_BUTTON_EVENT_MAX] value |

---

{#gpio}

#### gpio

```cpp
gpio_num_t gpio
```

Defined in bsp_button.c:45

gpio value

---

{#state}

#### state

```cpp
button_state_t state
```

Type: [`button_state_t`](#button_state_t)

Defined in bsp_button.c:46

state value

---

{#press_start_tick}

#### press_start_tick

```cpp
uint32_t press_start_tick
```

Defined in bsp_button.c:47

press_start_tick value

---

{#release_tick}

#### release_tick

```cpp
uint32_t release_tick
```

Defined in bsp_button.c:48

release_tick value

---

{#stable_state_ticks}

#### stable_state_ticks

```cpp
uint32_t stable_state_ticks
```

Defined in bsp_button.c:49

stable_state_ticks value

---

{#long_press_fired}

#### long_press_fired

```cpp
bool long_press_fired
```

Defined in bsp_button.c:50

long_press_fired value

---

{#callbacks}

#### callbacks

```cpp
button_callback_entry_t callbacks[BSP_BUTTON_EVENT_MAX]
```

Defined in bsp_button.c:51

callbacks[BSP_BUTTON_EVENT_MAX] value

{#chime_entry_t}

## chime_entry_t

```cpp
struct chime_entry_t
```

Defined in bsp_splash.c:26

### Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| [`bsp_chime_cb_t`](#bsp_chime_cb_t) | [`cb`](#cb)  | cb value |
| `void *` | [`user_data`](#user_data)  | user_data value |

---

{#cb}

#### cb

```cpp
bsp_chime_cb_t cb
```

Type: [`bsp_chime_cb_t`](#bsp_chime_cb_t)

Defined in bsp_splash.c:27

cb value

---

{#user_data}

#### user_data

```cpp
void * user_data
```

Defined in bsp_splash.c:28

user_data value

{#splash_entry_t}

## splash_entry_t

```cpp
struct splash_entry_t
```

Defined in bsp_splash.c:21

### Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| [`bsp_splash_cb_t`](#bsp_splash_cb_t) | [`cb`](#cb-1)  | cb value |
| `void *` | [`user_data`](#user_data-1)  | user_data value |

---

{#cb-1}

#### cb

```cpp
bsp_splash_cb_t cb
```

Type: [`bsp_splash_cb_t`](#bsp_splash_cb_t)

Defined in bsp_splash.c:22

cb value

---

{#user_data-1}

#### user_data

```cpp
void * user_data
```

Defined in bsp_splash.c:23

user_data value

{#bsp_tb_entry_t}

## bsp_tb_entry_t

```cpp
#include <bsp/bsp_tb.h>
```

```cpp
struct bsp_tb_entry_t
```

Defined in bsp/bsp_tb.h:81

ThingsBoard Generic Telemetry / Attribute Key-Value Entry.

### Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| `const char *` | [`key`](#key)  | Attribute/Telemetry key name. |
| [`bsp_tb_val_type_t`](#bsp_tb_val_type_t) | [`type`](#type)  | Value type. |
| `union bsp_tb_entry_t::@130170067103220264354311244275220070120372122274` | [`val`](#val)  |  |

---

{#key}

#### key

```cpp
const char * key
```

Defined in bsp/bsp_tb.h:82

Attribute/Telemetry key name.

---

{#type}

#### type

```cpp
bsp_tb_val_type_t type
```

Type: [`bsp_tb_val_type_t`](#bsp_tb_val_type_t)

Defined in bsp/bsp_tb.h:83

Value type.

---

{#val}

#### val

```cpp
union bsp_tb_entry_t::@130170067103220264354311244275220070120372122274 val
```

Defined in bsp/bsp_tb.h:90

{#unionval}

## [union].val

```cpp
union [union].val
```

Defined in bsp/bsp_tb.h:84

### Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| `int64_t` | [`i_val`](#i_val)  | i_val value |
| `float` | [`f_val`](#f_val)  | f_val value |
| `double` | [`d_val`](#d_val)  | d_val value |
| `bool` | [`b_val`](#b_val)  | b_val value |
| `const char *` | [`s_val`](#s_val)  | s_val value |

---

{#i_val}

#### i_val

```cpp
int64_t i_val
```

Defined in bsp/bsp_tb.h:85

i_val value

---

{#f_val}

#### f_val

```cpp
float f_val
```

Defined in bsp/bsp_tb.h:86

f_val value

---

{#d_val}

#### d_val

```cpp
double d_val
```

Defined in bsp/bsp_tb.h:87

d_val value

---

{#b_val}

#### b_val

```cpp
bool b_val
```

Defined in bsp/bsp_tb.h:88

b_val value

---

{#s_val}

#### s_val

```cpp
const char * s_val
```

Defined in bsp/bsp_tb.h:89

s_val value

{#bsp_diag_info_t}

## bsp_diag_info_t

```cpp
#include <bsp/bsp_err.h>
```

```cpp
struct bsp_diag_info_t
```

Defined in bsp/bsp_err.h:43

System Diagnostic Information Snapshot.

### Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| `const char *` | [`bsp_version`](#bsp_version)  | BSP SemVer version string. |
| `const char *` | [`chip_model`](#chip_model)  | MCU Silicon Model (e.g. "ESP32-S3"). |
| `const char *` | [`chip_revision_str`](#chip_revision_str)  | MCU Silicon Revision string (e.g. "v0.2"). |
| `uint16_t` | [`chip_revision`](#chip_revision)  | MCU Silicon Revision (major * 100 + minor). |
| `uint8_t` | [`chip_cores`](#chip_cores)  | MCU CPU Core count. |
| `uint32_t` | [`free_internal_heap`](#free_internal_heap)  | Free internal SRAM heap in bytes. |
| `uint32_t` | [`min_free_internal_heap`](#min_free_internal_heap)  | Minimum historical free internal SRAM in bytes. |
| `uint32_t` | [`free_psram_heap`](#free_psram_heap)  | Free external PSRAM in bytes. |
| `uint32_t` | [`uptime_seconds`](#uptime_seconds)  | Time elapsed since system boot in seconds. |
| `uint32_t` | [`battery_mv`](#battery_mv)  | Measured battery voltage in millivolts. |
| `int8_t` | [`battery_percentage`](#battery_percentage)  | Calculated battery state of charge (0-100%). |
| `bool` | [`power_rail_good`](#power_rail_good)  | Power latch active state. |
| `bool` | [`i2c_bus_healthy`](#i2c_bus_healthy)  | SHTC3 & RTC I2C response state. |
| `bool` | [`display_ready`](#display_ready)  | Display controller SPI initialization state. |
| `bool` | [`wifi_connected`](#wifi_connected)  | Wi-Fi station link state. |
| `int8_t` | [`wifi_rssi`](#wifi_rssi)  | Wi-Fi RSSI signal strength (dBm). |

---

{#bsp_version}

#### bsp_version

```cpp
const char * bsp_version
```

Defined in bsp/bsp_err.h:44

BSP SemVer version string.

---

{#chip_model}

#### chip_model

```cpp
const char * chip_model
```

Defined in bsp/bsp_err.h:45

MCU Silicon Model (e.g. "ESP32-S3").

---

{#chip_revision_str}

#### chip_revision_str

```cpp
const char * chip_revision_str
```

Defined in bsp/bsp_err.h:46

MCU Silicon Revision string (e.g. "v0.2").

---

{#chip_revision}

#### chip_revision

```cpp
uint16_t chip_revision
```

Defined in bsp/bsp_err.h:47

MCU Silicon Revision (major * 100 + minor).

---

{#chip_cores}

#### chip_cores

```cpp
uint8_t chip_cores
```

Defined in bsp/bsp_err.h:48

MCU CPU Core count.

---

{#free_internal_heap}

#### free_internal_heap

```cpp
uint32_t free_internal_heap
```

Defined in bsp/bsp_err.h:49

Free internal SRAM heap in bytes.

---

{#min_free_internal_heap}

#### min_free_internal_heap

```cpp
uint32_t min_free_internal_heap
```

Defined in bsp/bsp_err.h:50

Minimum historical free internal SRAM in bytes.

---

{#free_psram_heap}

#### free_psram_heap

```cpp
uint32_t free_psram_heap
```

Defined in bsp/bsp_err.h:51

Free external PSRAM in bytes.

---

{#uptime_seconds}

#### uptime_seconds

```cpp
uint32_t uptime_seconds
```

Defined in bsp/bsp_err.h:52

Time elapsed since system boot in seconds.

---

{#battery_mv}

#### battery_mv

```cpp
uint32_t battery_mv
```

Defined in bsp/bsp_err.h:53

Measured battery voltage in millivolts.

---

{#battery_percentage}

#### battery_percentage

```cpp
int8_t battery_percentage
```

Defined in bsp/bsp_err.h:54

Calculated battery state of charge (0-100%).

---

{#power_rail_good}

#### power_rail_good

```cpp
bool power_rail_good
```

Defined in bsp/bsp_err.h:55

Power latch active state.

---

{#i2c_bus_healthy}

#### i2c_bus_healthy

```cpp
bool i2c_bus_healthy
```

Defined in bsp/bsp_err.h:56

SHTC3 & RTC I2C response state.

---

{#display_ready}

#### display_ready

```cpp
bool display_ready
```

Defined in bsp/bsp_err.h:57

Display controller SPI initialization state.

---

{#wifi_connected}

#### wifi_connected

```cpp
bool wifi_connected
```

Defined in bsp/bsp_err.h:58

Wi-Fi station link state.

---

{#wifi_rssi}

#### wifi_rssi

```cpp
int8_t wifi_rssi
```

Defined in bsp/bsp_err.h:59

Wi-Fi RSSI signal strength (dBm).

{#bsp_rtc_alarm_t}

## bsp_rtc_alarm_t

```cpp
#include <bsp/bsp_rtc.h>
```

```cpp
struct bsp_rtc_alarm_t
```

Defined in bsp/bsp_rtc.h:44

### Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| `int8_t` | [`second`](#second)  | 0 - 59, or -1 to ignore |
| `int8_t` | [`minute`](#minute)  | 0 - 59, or -1 to ignore |
| `int8_t` | [`hour`](#hour)  | 0 - 23, or -1 to ignore |
| `int8_t` | [`day`](#day)  | 1 - 31, or -1 to ignore |
| `int8_t` | [`weekday`](#weekday)  | 0 - 6, or -1 to ignore |

---

{#second}

#### second

```cpp
int8_t second
```

Defined in bsp/bsp_rtc.h:45

0 - 59, or -1 to ignore

---

{#minute}

#### minute

```cpp
int8_t minute
```

Defined in bsp/bsp_rtc.h:46

0 - 59, or -1 to ignore

---

{#hour}

#### hour

```cpp
int8_t hour
```

Defined in bsp/bsp_rtc.h:47

0 - 23, or -1 to ignore

---

{#day}

#### day

```cpp
int8_t day
```

Defined in bsp/bsp_rtc.h:48

1 - 31, or -1 to ignore

---

{#weekday}

#### weekday

```cpp
int8_t weekday
```

Defined in bsp/bsp_rtc.h:49

0 - 6, or -1 to ignore

{#bsp_rtc_state_t}

## bsp_rtc_state_t

```cpp
#include <bsp/bsp_rtc_mem.h>
```

```cpp
struct bsp_rtc_state_t
```

Defined in bsp/bsp_rtc_mem.h:54

Persistent RTC State Struct (Stored in RTC Slow Memory).

### Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| `uint32_t` | [`magic`](#magic)  | Validation magic token. |
| `uint32_t` | [`boot_count`](#boot_count)  | Total system boot counter. |
| `uint32_t` | [`deep_sleep_count`](#deep_sleep_count)  | Total deep sleep cycles. |
| `uint32_t` | [`light_sleep_count`](#light_sleep_count)  | Total light sleep cycles. |
| `uint8_t` | [`last_init_mode`](#last_init_mode)  | Last executed [bsp_init_mode_t](#bsp_init_mode_t). |
| `uint8_t` | [`next_init_mode`](#next_init_mode)  | Planned next [bsp_init_mode_t](#bsp_init_mode_t) on wake. |
| `uint8_t` | [`last_sleep_mode`](#last_sleep_mode)  | Last executed [bsp_sleep_mode_t](#bsp_sleep_mode_t). |
| `uint8_t` | [`flags`](#flags)  | System runtime status flags. |
| `uint32_t` | [`last_sleep_duration_sec`](#last_sleep_duration_sec)  | Duration of previous sleep period. |
| `uint32_t` | [`last_wake_epoch`](#last_wake_epoch)  | Epoch timestamp of previous wake event. |
| `uint8_t` | [`scratchpad`](#scratchpad)  | Application telemetry / scratch data buffer. |

---

{#magic}

#### magic

```cpp
uint32_t magic
```

Defined in bsp/bsp_rtc_mem.h:55

Validation magic token.

---

{#boot_count}

#### boot_count

```cpp
uint32_t boot_count
```

Defined in bsp/bsp_rtc_mem.h:56

Total system boot counter.

---

{#deep_sleep_count}

#### deep_sleep_count

```cpp
uint32_t deep_sleep_count
```

Defined in bsp/bsp_rtc_mem.h:57

Total deep sleep cycles.

---

{#light_sleep_count}

#### light_sleep_count

```cpp
uint32_t light_sleep_count
```

Defined in bsp/bsp_rtc_mem.h:58

Total light sleep cycles.

---

{#last_init_mode}

#### last_init_mode

```cpp
uint8_t last_init_mode
```

Defined in bsp/bsp_rtc_mem.h:59

Last executed [bsp_init_mode_t](#bsp_init_mode_t).

---

{#next_init_mode}

#### next_init_mode

```cpp
uint8_t next_init_mode
```

Defined in bsp/bsp_rtc_mem.h:60

Planned next [bsp_init_mode_t](#bsp_init_mode_t) on wake.

---

{#last_sleep_mode}

#### last_sleep_mode

```cpp
uint8_t last_sleep_mode
```

Defined in bsp/bsp_rtc_mem.h:61

Last executed [bsp_sleep_mode_t](#bsp_sleep_mode_t).

---

{#flags}

#### flags

```cpp
uint8_t flags
```

Defined in bsp/bsp_rtc_mem.h:62

System runtime status flags.

---

{#last_sleep_duration_sec}

#### last_sleep_duration_sec

```cpp
uint32_t last_sleep_duration_sec
```

Defined in bsp/bsp_rtc_mem.h:63

Duration of previous sleep period.

---

{#last_wake_epoch}

#### last_wake_epoch

```cpp
uint32_t last_wake_epoch
```

Defined in bsp/bsp_rtc_mem.h:64

Epoch timestamp of previous wake event.

---

{#scratchpad}

#### scratchpad

```cpp
uint8_t scratchpad[32]
```

Defined in bsp/bsp_rtc_mem.h:65

Application telemetry / scratch data buffer.

{#bsp_tb_config_t}

## bsp_tb_config_t

```cpp
#include <bsp/bsp_tb.h>
```

```cpp
struct bsp_tb_config_t
```

Defined in bsp/bsp_tb.h:96

ThingsBoard Client Configuration Struct.

### Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| `const char *` | [`broker_uri`](#broker_uri)  | MQTTS Broker URI (e.g. "mqtts://thingsboard.cloud:8883"). |
| `const char *` | [`access_token`](#access_token)  | Device access token (or NULL if using client claiming). |
| `const char *` | [`ca_cert_pem`](#ca_cert_pem)  | Optional custom CA certificate (NULL uses system cert bundle). |
| [`bsp_tb_rpc_cb_t`](#bsp_tb_rpc_cb_t) | [`rpc_cb`](#rpc_cb)  | RPC command handler callback. |
| [`bsp_tb_attr_cb_t`](#bsp_tb_attr_cb_t) | [`attr_cb`](#attr_cb)  | Shared attributes update callback. |
| [`bsp_tb_alarm_cb_t`](#bsp_tb_alarm_cb_t) | [`alarm_cb`](#alarm_cb)  | Alarm / notification callback. |
| [`bsp_ota_progress_cb_t`](#bsp_ota_progress_cb_t) | [`ota_cb`](#ota_cb)  | OTA progress callback. |
| `void *` | [`user_data`](#user_data-2)  | User context pointer. |

---

{#broker_uri}

#### broker_uri

```cpp
const char * broker_uri
```

Defined in bsp/bsp_tb.h:97

MQTTS Broker URI (e.g. "mqtts://thingsboard.cloud:8883").

---

{#access_token}

#### access_token

```cpp
const char * access_token
```

Defined in bsp/bsp_tb.h:98

Device access token (or NULL if using client claiming).

---

{#ca_cert_pem}

#### ca_cert_pem

```cpp
const char * ca_cert_pem
```

Defined in bsp/bsp_tb.h:99

Optional custom CA certificate (NULL uses system cert bundle).

---

{#rpc_cb}

#### rpc_cb

```cpp
bsp_tb_rpc_cb_t rpc_cb
```

Type: [`bsp_tb_rpc_cb_t`](#bsp_tb_rpc_cb_t)

Defined in bsp/bsp_tb.h:100

RPC command handler callback.

---

{#attr_cb}

#### attr_cb

```cpp
bsp_tb_attr_cb_t attr_cb
```

Type: [`bsp_tb_attr_cb_t`](#bsp_tb_attr_cb_t)

Defined in bsp/bsp_tb.h:101

Shared attributes update callback.

---

{#alarm_cb}

#### alarm_cb

```cpp
bsp_tb_alarm_cb_t alarm_cb
```

Type: [`bsp_tb_alarm_cb_t`](#bsp_tb_alarm_cb_t)

Defined in bsp/bsp_tb.h:102

Alarm / notification callback.

---

{#ota_cb}

#### ota_cb

```cpp
bsp_ota_progress_cb_t ota_cb
```

Type: [`bsp_ota_progress_cb_t`](#bsp_ota_progress_cb_t)

Defined in bsp/bsp_tb.h:103

OTA progress callback.

---

{#user_data-2}

#### user_data

```cpp
void * user_data
```

Defined in bsp/bsp_tb.h:104

User context pointer.

{#bsp_shtc3_data_t}

## bsp_shtc3_data_t

```cpp
#include <bsp/bsp_sensors.h>
```

```cpp
struct bsp_shtc3_data_t
```

Defined in bsp/bsp_sensors.h:46

SHTC3 Environmental Sensor Telemetry Data.

### Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| `float` | [`temperature_c`](#temperature_c)  | Temperature in Celsius (°C). |
| `float` | [`temperature_f`](#temperature_f)  | Temperature in Fahrenheit (°F). |
| `float` | [`temperature_k`](#temperature_k)  | Temperature in Kelvin (K). |
| `float` | [`humidity_percent`](#humidity_percent)  | Relative Humidity (RH). |
| `float` | [`dew_point_c`](#dew_point_c)  | Dew point in Celsius (°C). |
| `float` | [`dew_point_f`](#dew_point_f)  | Dew point in Fahrenheit (°F). |
| `float` | [`dew_point_k`](#dew_point_k)  | Dew point in Kelvin (K). |
| `float` | [`absolute_humidity_g`](#absolute_humidity_g)  | Absolute Humidity in (g/m³). |
| `bool` | [`valid`](#valid)  | valid value |

---

{#temperature_c}

#### temperature_c

```cpp
float temperature_c
```

Defined in bsp/bsp_sensors.h:47

Temperature in Celsius (°C).

---

{#temperature_f}

#### temperature_f

```cpp
float temperature_f
```

Defined in bsp/bsp_sensors.h:48

Temperature in Fahrenheit (°F).

---

{#temperature_k}

#### temperature_k

```cpp
float temperature_k
```

Defined in bsp/bsp_sensors.h:49

Temperature in Kelvin (K).

---

{#humidity_percent}

#### humidity_percent

```cpp
float humidity_percent
```

Defined in bsp/bsp_sensors.h:50

Relative Humidity (RH).

---

{#dew_point_c}

#### dew_point_c

```cpp
float dew_point_c
```

Defined in bsp/bsp_sensors.h:51

Dew point in Celsius (°C).

---

{#dew_point_f}

#### dew_point_f

```cpp
float dew_point_f
```

Defined in bsp/bsp_sensors.h:52

Dew point in Fahrenheit (°F).

---

{#dew_point_k}

#### dew_point_k

```cpp
float dew_point_k
```

Defined in bsp/bsp_sensors.h:53

Dew point in Kelvin (K).

---

{#absolute_humidity_g}

#### absolute_humidity_g

```cpp
float absolute_humidity_g
```

Defined in bsp/bsp_sensors.h:54

Absolute Humidity in (g/m³).

---

{#valid}

#### valid

```cpp
bool valid
```

Defined in bsp/bsp_sensors.h:55

valid value

{#cached_i2c_dev_t}

## cached_i2c_dev_t

```cpp
struct cached_i2c_dev_t
```

Defined in bsp_i2c.c:34

### Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| `uint8_t` | [`addr`](#addr)  | addr value |
| `i2c_master_dev_handle_t` | [`handle`](#handle)  | handle value |

---

{#addr}

#### addr

```cpp
uint8_t addr
```

Defined in bsp_i2c.c:35

addr value

---

{#handle}

#### handle

```cpp
i2c_master_dev_handle_t handle
```

Defined in bsp_i2c.c:36

handle value

{#rtc_wifi_cache_t}

## rtc_wifi_cache_t

```cpp
struct rtc_wifi_cache_t
```

Defined in bsp_wifi.c:49

RTC Fast Reconnect Session Cache Structure Preserved across Deep Sleep cycles in RTC Slow/Fast memory.

### Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| `uint32_t` | [`magic`](#magic-1)  | magic value |
| `uint8_t` | [`bssid`](#bssid)  | bssid[6] value |
| `uint8_t` | [`channel`](#channel)  | channel value |
| `char` | [`ssid`](#ssid)  | ssid[33] value |

---

{#magic-1}

#### magic

```cpp
uint32_t magic
```

Defined in bsp_wifi.c:50

magic value

---

{#bssid}

#### bssid

```cpp
uint8_t bssid[6]
```

Defined in bsp_wifi.c:51

bssid[6] value

---

{#channel}

#### channel

```cpp
uint8_t channel
```

Defined in bsp_wifi.c:52

channel value

---

{#ssid}

#### ssid

```cpp
char ssid[33]
```

Defined in bsp_wifi.c:53

ssid[33] value

{#bsp_rtc_datetime_t}

## bsp_rtc_datetime_t

```cpp
#include <bsp/bsp_rtc.h>
```

```cpp
struct bsp_rtc_datetime_t
```

Defined in bsp/bsp_rtc.h:34

### Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| `uint16_t` | [`year`](#year)  | Year (2000 - 2099). |
| `uint8_t` | [`month`](#month)  | Month (1 - 12). |
| `uint8_t` | [`day`](#day-1)  | Day of month (1 - 31). |
| `uint8_t` | [`weekday`](#weekday-1)  | Day of week (0 = Sunday, 1 = Monday, ... 6 = Saturday). |
| `uint8_t` | [`hour`](#hour-1)  | Hour (0 - 23, 24-hour mode). |
| `uint8_t` | [`minute`](#minute-1)  | Minute (0 - 59). |
| `uint8_t` | [`second`](#second-1)  | Second (0 - 59). |

---

{#year}

#### year

```cpp
uint16_t year
```

Defined in bsp/bsp_rtc.h:35

Year (2000 - 2099).

---

{#month}

#### month

```cpp
uint8_t month
```

Defined in bsp/bsp_rtc.h:36

Month (1 - 12).

---

{#day-1}

#### day

```cpp
uint8_t day
```

Defined in bsp/bsp_rtc.h:37

Day of month (1 - 31).

---

{#weekday-1}

#### weekday

```cpp
uint8_t weekday
```

Defined in bsp/bsp_rtc.h:38

Day of week (0 = Sunday, 1 = Monday, ... 6 = Saturday).

---

{#hour-1}

#### hour

```cpp
uint8_t hour
```

Defined in bsp/bsp_rtc.h:39

Hour (0 - 23, 24-hour mode).

---

{#minute-1}

#### minute

```cpp
uint8_t minute
```

Defined in bsp/bsp_rtc.h:40

Minute (0 - 59).

---

{#second-1}

#### second

```cpp
uint8_t second
```

Defined in bsp/bsp_rtc.h:41

Second (0 - 59).

{#bsp_sleep_config_t}

## bsp_sleep_config_t

```cpp
#include <bsp/bsp_power.h>
```

```cpp
struct bsp_sleep_config_t
```

Defined in bsp/bsp_power.h:61

Unified Sleep Configuration.

### Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| [`bsp_sleep_mode_t`](#bsp_sleep_mode_t) | [`mode`](#mode)  | Target sleep mode (Light or Deep). |
| `uint32_t` | [`duration_sec`](#duration_sec)  | Sleep duration in seconds (0 for indefinite / button only). |
| [`bsp_wake_source_mask_t`](#bsp_wake_source_mask_t) | [`wake_sources`](#wake_sources)  | Bitmask of enabled wake triggers. |
| [`bsp_init_mode_t`](#bsp_init_mode_t) | [`next_init_mode`](#next_init_mode-1)  | Hardware initialization mode to perform on wake. |

---

{#mode}

#### mode

```cpp
bsp_sleep_mode_t mode
```

Type: [`bsp_sleep_mode_t`](#bsp_sleep_mode_t)

Defined in bsp/bsp_power.h:62

Target sleep mode (Light or Deep).

---

{#duration_sec}

#### duration_sec

```cpp
uint32_t duration_sec
```

Defined in bsp/bsp_power.h:63

Sleep duration in seconds (0 for indefinite / button only).

---

{#wake_sources}

#### wake_sources

```cpp
bsp_wake_source_mask_t wake_sources
```

Type: [`bsp_wake_source_mask_t`](#bsp_wake_source_mask_t)

Defined in bsp/bsp_power.h:64

Bitmask of enabled wake triggers.

---

{#next_init_mode-1}

#### next_init_mode

```cpp
bsp_init_mode_t next_init_mode
```

Type: [`bsp_init_mode_t`](#bsp_init_mode_t)

Defined in bsp/bsp_power.h:65

Hardware initialization mode to perform on wake.

{#bsp_wake_context_t}

## bsp_wake_context_t

```cpp
#include <bsp/bsp_lifecycle.h>
```

```cpp
struct bsp_wake_context_t
```

Defined in bsp/bsp_lifecycle.h:42

Structured Wake Context passed to application on_wake callback.

### Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| `esp_reset_reason_t` | [`reset_reason`](#reset_reason)  | Reset reason (e.g. ESP_RST_DEEPSLEEP, ESP_RST_POWERON). |
| `esp_sleep_wakeup_cause_t` | [`wake_cause`](#wake_cause)  | Wakeup cause (e.g. EXT1, TIMER, GPIO). |
| `uint64_t` | [`ext1_wakeup_pins`](#ext1_wakeup_pins)  | GPIO mask of pins that triggered EXT1 wakeup. |
| `bool` | [`woke_from_button`](#woke_from_button)  | True if wake was triggered by BOOT or POWER button. |
| [`bsp_button_t`](#bsp_button_t) | [`wake_button`](#wake_button)  | Which button triggered wakeup (if button wake). |
| [`bsp_init_mode_t`](#bsp_init_mode_t) | [`init_mode`](#init_mode)  | Initialization profile executed (FULL, FAST, MIN). |
| `uint32_t` | [`sleep_duration_sec`](#sleep_duration_sec)  | Configured sleep duration from previous cycle. |
| `uint32_t` | [`boot_count`](#boot_count-1)  | Monotonic system boot count. |
| `uint32_t` | [`deep_sleep_count`](#deep_sleep_count-1)  | Total deep sleep cycles. |
| `uint32_t` | [`light_sleep_count`](#light_sleep_count-1)  | Total light sleep cycles. |
| `uint8_t` | [`app_stage`](#app_stage)  | Persistent application stage code (from RTC memory). |
| `void *` | [`user_data`](#user_data-3)  | User data pointer passed during lifecycle start. |

---

{#reset_reason}

#### reset_reason

```cpp
esp_reset_reason_t reset_reason
```

Defined in bsp/bsp_lifecycle.h:43

Reset reason (e.g. ESP_RST_DEEPSLEEP, ESP_RST_POWERON).

---

{#wake_cause}

#### wake_cause

```cpp
esp_sleep_wakeup_cause_t wake_cause
```

Defined in bsp/bsp_lifecycle.h:44

Wakeup cause (e.g. EXT1, TIMER, GPIO).

---

{#ext1_wakeup_pins}

#### ext1_wakeup_pins

```cpp
uint64_t ext1_wakeup_pins
```

Defined in bsp/bsp_lifecycle.h:45

GPIO mask of pins that triggered EXT1 wakeup.

---

{#woke_from_button}

#### woke_from_button

```cpp
bool woke_from_button
```

Defined in bsp/bsp_lifecycle.h:46

True if wake was triggered by BOOT or POWER button.

---

{#wake_button}

#### wake_button

```cpp
bsp_button_t wake_button
```

Type: [`bsp_button_t`](#bsp_button_t)

Defined in bsp/bsp_lifecycle.h:47

Which button triggered wakeup (if button wake).

---

{#init_mode}

#### init_mode

```cpp
bsp_init_mode_t init_mode
```

Type: [`bsp_init_mode_t`](#bsp_init_mode_t)

Defined in bsp/bsp_lifecycle.h:48

Initialization profile executed (FULL, FAST, MIN).

---

{#sleep_duration_sec}

#### sleep_duration_sec

```cpp
uint32_t sleep_duration_sec
```

Defined in bsp/bsp_lifecycle.h:49

Configured sleep duration from previous cycle.

---

{#boot_count-1}

#### boot_count

```cpp
uint32_t boot_count
```

Defined in bsp/bsp_lifecycle.h:50

Monotonic system boot count.

---

{#deep_sleep_count-1}

#### deep_sleep_count

```cpp
uint32_t deep_sleep_count
```

Defined in bsp/bsp_lifecycle.h:51

Total deep sleep cycles.

---

{#light_sleep_count-1}

#### light_sleep_count

```cpp
uint32_t light_sleep_count
```

Defined in bsp/bsp_lifecycle.h:52

Total light sleep cycles.

---

{#app_stage}

#### app_stage

```cpp
uint8_t app_stage
```

Defined in bsp/bsp_lifecycle.h:53

Persistent application stage code (from RTC memory).

---

{#user_data-3}

#### user_data

```cpp
void * user_data
```

Defined in bsp/bsp_lifecycle.h:54

User data pointer passed during lifecycle start.

{#battery_lut_point_t}

## battery_lut_point_t

```cpp
struct battery_lut_point_t
```

Defined in bsp_power.c:58

### Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| `uint16_t` | [`voltage_mv`](#voltage_mv)  | voltage_mv value |
| `uint8_t` | [`percentage`](#percentage)  | percentage value |

---

{#voltage_mv}

#### voltage_mv

```cpp
uint16_t voltage_mv
```

Defined in bsp_power.c:59

voltage_mv value

---

{#percentage}

#### percentage

```cpp
uint8_t percentage
```

Defined in bsp_power.c:60

percentage value

{#bsp_app_lifecycle_t}

## bsp_app_lifecycle_t

```cpp
#include <bsp/bsp_lifecycle.h>
```

```cpp
struct bsp_app_lifecycle_t
```

Defined in bsp/bsp_lifecycle.h:80

Comprehensive Application Lifecycle Configuration.

### Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| [`bsp_cold_boot_cb_t`](#bsp_cold_boot_cb_t) | [`on_cold_boot`](#on_cold_boot)  | Handler for initial cold boot. |
| [`bsp_wake_cb_t`](#bsp_wake_cb_t) | [`on_wake`](#on_wake)  | Handler for sleep wake events. |
| [`bsp_before_sleep_cb_t`](#bsp_before_sleep_cb_t) | [`on_before_sleep`](#on_before_sleep)  | Hook called immediately prior to sleep entry. |
| [`bsp_shutdown_cb_t`](#bsp_shutdown_cb_t) | [`on_shutdown`](#on_shutdown)  | Hook called immediately prior to power off. |
| `void *` | [`user_data`](#user_data-4)  | Custom application context pointer. |

---

{#on_cold_boot}

#### on_cold_boot

```cpp
bsp_cold_boot_cb_t on_cold_boot
```

Type: [`bsp_cold_boot_cb_t`](#bsp_cold_boot_cb_t)

Defined in bsp/bsp_lifecycle.h:81

Handler for initial cold boot.

---

{#on_wake}

#### on_wake

```cpp
bsp_wake_cb_t on_wake
```

Type: [`bsp_wake_cb_t`](#bsp_wake_cb_t)

Defined in bsp/bsp_lifecycle.h:82

Handler for sleep wake events.

---

{#on_before_sleep}

#### on_before_sleep

```cpp
bsp_before_sleep_cb_t on_before_sleep
```

Type: [`bsp_before_sleep_cb_t`](#bsp_before_sleep_cb_t)

Defined in bsp/bsp_lifecycle.h:83

Hook called immediately prior to sleep entry.

---

{#on_shutdown}

#### on_shutdown

```cpp
bsp_shutdown_cb_t on_shutdown
```

Type: [`bsp_shutdown_cb_t`](#bsp_shutdown_cb_t)

Defined in bsp/bsp_lifecycle.h:84

Hook called immediately prior to power off.

---

{#user_data-4}

#### user_data

```cpp
void * user_data
```

Defined in bsp/bsp_lifecycle.h:85

Custom application context pointer.

{#bsp_button_config_t}

## bsp_button_config_t

```cpp
#include <bsp/bsp_button.h>
```

```cpp
struct bsp_button_config_t
```

Defined in bsp/bsp_button.h:62

Button Timing & Feature Configuration.

### Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| `uint32_t` | [`debounce_ms`](#debounce_ms)  | Debounce settling time in milliseconds (Default: 20 ms). |
| `uint32_t` | [`click_timeout_ms`](#click_timeout_ms)  | Max delay between double clicks in ms (Default: 280 ms). |
| `uint32_t` | [`long_press_ms`](#long_press_ms)  | Duration to trigger long press in ms (Default: 2500 ms). |
| `bool` | [`auto_power_off_on_hold`](#auto_power_off_on_hold)  | Auto power off system on POWER long press (Default: true). |

---

{#debounce_ms}

#### debounce_ms

```cpp
uint32_t debounce_ms
```

Defined in bsp/bsp_button.h:63

Debounce settling time in milliseconds (Default: 20 ms).

---

{#click_timeout_ms}

#### click_timeout_ms

```cpp
uint32_t click_timeout_ms
```

Defined in bsp/bsp_button.h:64

Max delay between double clicks in ms (Default: 280 ms).

---

{#long_press_ms}

#### long_press_ms

```cpp
uint32_t long_press_ms
```

Defined in bsp/bsp_button.h:65

Duration to trigger long press in ms (Default: 2500 ms).

---

{#auto_power_off_on_hold}

#### auto_power_off_on_hold

```cpp
bool auto_power_off_on_hold
```

Defined in bsp/bsp_button.h:66

Auto power off system on POWER long press (Default: true).

{#bsp_sensor_cal_data_t}

## bsp_sensor_cal_data_t

```cpp
#include <bsp/bsp_sensor_cal.h>
```

```cpp
struct bsp_sensor_cal_data_t
```

Defined in bsp/bsp_sensor_cal.h:39

Calibrated Environmental & Diagnostic Sensor Telemetry.

### Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| `float` | [`temperature_c`](#temperature_c-1)  | Temperature in Celsius (°C). |
| `float` | [`temperature_f`](#temperature_f-1)  | Temperature in Fahrenheit (°F). |
| `float` | [`temperature_k`](#temperature_k-1)  | Temperature in Kelvin (K). |
| `float` | [`humidity_percent`](#humidity_percent-1)  | Relative Humidity (RH). |
| `float` | [`dew_point_c`](#dew_point_c-1)  | Dew Point in Celsius (°C). |
| `float` | [`dew_point_f`](#dew_point_f-1)  | Dew Point in Fahrenheit (°F). |
| `float` | [`dew_point_k`](#dew_point_k-1)  | Dew Point in Kelvin (K). |
| `float` | [`absolute_humidity_g`](#absolute_humidity_g-1)  | Absolute Humidity (g/m³). |
| `float` | [`raw_temperature_c`](#raw_temperature_c)  | Pure Uncompensated Raw SHTC3 Temperature (°C). |
| `float` | [`raw_humidity_percent`](#raw_humidity_percent)  | Pure Uncompensated Raw SHTC3 Relative Humidity (RH). |
| `float` | [`die_temp_c`](#die_temp_c)  | Filtered ESP32-S3 MCU Junction Temperature (°C). |
| `float` | [`thermal_offset_c`](#thermal_offset_c)  | Applied Thermal Offset (°C) (0.0°C when deactivated). |
| `bool` | [`compensated`](#compensated)  | True if thermal compensation was active, false if bypassed. |
| `bool` | [`valid`](#valid-1)  | True if SHTC3 CRC verified. |

---

{#temperature_c-1}

#### temperature_c

```cpp
float temperature_c
```

Defined in bsp/bsp_sensor_cal.h:41

Temperature in Celsius (°C).

---

{#temperature_f-1}

#### temperature_f

```cpp
float temperature_f
```

Defined in bsp/bsp_sensor_cal.h:42

Temperature in Fahrenheit (°F).

---

{#temperature_k-1}

#### temperature_k

```cpp
float temperature_k
```

Defined in bsp/bsp_sensor_cal.h:43

Temperature in Kelvin (K).

---

{#humidity_percent-1}

#### humidity_percent

```cpp
float humidity_percent
```

Defined in bsp/bsp_sensor_cal.h:44

Relative Humidity (RH).

---

{#dew_point_c-1}

#### dew_point_c

```cpp
float dew_point_c
```

Defined in bsp/bsp_sensor_cal.h:45

Dew Point in Celsius (°C).

---

{#dew_point_f-1}

#### dew_point_f

```cpp
float dew_point_f
```

Defined in bsp/bsp_sensor_cal.h:46

Dew Point in Fahrenheit (°F).

---

{#dew_point_k-1}

#### dew_point_k

```cpp
float dew_point_k
```

Defined in bsp/bsp_sensor_cal.h:47

Dew Point in Kelvin (K).

---

{#absolute_humidity_g-1}

#### absolute_humidity_g

```cpp
float absolute_humidity_g
```

Defined in bsp/bsp_sensor_cal.h:48

Absolute Humidity (g/m³).

---

{#raw_temperature_c}

#### raw_temperature_c

```cpp
float raw_temperature_c
```

Defined in bsp/bsp_sensor_cal.h:51

Pure Uncompensated Raw SHTC3 Temperature (°C).

---

{#raw_humidity_percent}

#### raw_humidity_percent

```cpp
float raw_humidity_percent
```

Defined in bsp/bsp_sensor_cal.h:52

Pure Uncompensated Raw SHTC3 Relative Humidity (RH).

---

{#die_temp_c}

#### die_temp_c

```cpp
float die_temp_c
```

Defined in bsp/bsp_sensor_cal.h:53

Filtered ESP32-S3 MCU Junction Temperature (°C).

---

{#thermal_offset_c}

#### thermal_offset_c

```cpp
float thermal_offset_c
```

Defined in bsp/bsp_sensor_cal.h:54

Applied Thermal Offset (°C) (0.0°C when deactivated).

---

{#compensated}

#### compensated

```cpp
bool compensated
```

Defined in bsp/bsp_sensor_cal.h:55

True if thermal compensation was active, false if bypassed.

---

{#valid-1}

#### valid

```cpp
bool valid
```

Defined in bsp/bsp_sensor_cal.h:56

True if SHTC3 CRC verified.

{#button_callback_entry_t}

## button_callback_entry_t

```cpp
struct button_callback_entry_t
```

Defined in bsp_button.c:39

### Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| [`bsp_button_cb_t`](#bsp_button_cb_t) | [`cb`](#cb-2)  | cb value |
| `void *` | [`user_data`](#user_data-5)  | user_data value |

---

{#cb-2}

#### cb

```cpp
bsp_button_cb_t cb
```

Type: [`bsp_button_cb_t`](#bsp_button_cb_t)

Defined in bsp_button.c:40

cb value

---

{#user_data-5}

#### user_data

```cpp
void * user_data
```

Defined in bsp_button.c:41

user_data value

{#unionval}

## [union].val

```cpp
union [union].val
```

Defined in bsp/bsp_tb.h:84

### Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| `int64_t` | [`i_val`](#i_val)  | i_val value |
| `float` | [`f_val`](#f_val)  | f_val value |
| `double` | [`d_val`](#d_val)  | d_val value |
| `bool` | [`b_val`](#b_val)  | b_val value |
| `const char *` | [`s_val`](#s_val)  | s_val value |

---

{#i_val}

#### i_val

```cpp
int64_t i_val
```

Defined in bsp/bsp_tb.h:85

i_val value

---

{#f_val}

#### f_val

```cpp
float f_val
```

Defined in bsp/bsp_tb.h:86

f_val value

---

{#d_val}

#### d_val

```cpp
double d_val
```

Defined in bsp/bsp_tb.h:87

d_val value

---

{#b_val}

#### b_val

```cpp
bool b_val
```

Defined in bsp/bsp_tb.h:88

b_val value

---

{#s_val}

#### s_val

```cpp
const char * s_val
```

Defined in bsp/bsp_tb.h:89

s_val value

Generated by [Moxygen](https://0state.com/moxygen)