# BSP-REV-APPEND I
---
- Change date format to include,
```
	1.) mm/dd/yy
	2.) dow
	3.) mm/dd/yy dow
```	
---
- Bsp_tb_send_telemetry, should be config-able via struct or some other element type.
- Bsp_tb_send_telemetry_sync, should also be config-able
- Bsp_tb_report_client_attributes, should be config-able
- Bsp_tb_request_shared_attributes, should be config-able
- I feel like a struct to json parser would be a good addition for sending, bsp_tb_send_custom_telemetry.
- "esp_err_t bsp_tb_send_telemetry(float temp_k, float rh_pct, uint8_t battery_pct, int rssi_dbm);" is too app specific
- Should we sub to the alarms/notification url? because, tracking alarms in the code with hard values, 
	seems like a waste, then tb does it for us.
---
- We need the display in 'MIN' mode I think, the display should reflect the webdash, send telemetry data without alerting the user,
	seems like a cheat to me.
---
- The test application does should contain all the test you spec-ed in the readme, please add any missing tests,
```
2. **`examples/Peripherals_Test_Suite`**: Comprehensive peripheral validation test app exercising:
   - System Diagnostics & SemVer API verification
   - RTC Memory state engine & PCF85063A 8-bit RAM read/write
   - SHTC3 Temperature & Relative Humidity acquisition
   - PCF85063A Calendar RTC time read/write & alarm
   - Battery ADC voltage curve measurement
   - MAX98357A I2S Audio synthesizer tones & chimes
   - SSD1681 1.54" EPD test patterns & partial refresh bit-blits
   - SD Card mount / unmount and FATFS file I/O
   - NVS read / write / erase cycles
   - Wi-Fi scan and RSSI validation
```
---
- We should change the key generation method to something thats easy to read, since the text is so small.
	"abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ23456789" <- "exclude visually ambiguous characters to avoid confusion"
- We should also change the password size for pairing to 8char, and 6char for claiming via dashboard.
---
- Have you found any info on the mp3's by chance?
- Can i store them in the partition and play them as "mp3's"?


# Please Refer Back to The bsp_revision.md for Details About The BSP Revision.
