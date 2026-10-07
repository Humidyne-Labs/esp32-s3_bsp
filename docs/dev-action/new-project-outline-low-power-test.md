# The Addition of a New Test Application/Project
A new application needs drafted so we can test the new bsp additions. The current test suite was designed for testing the periferals for functionality. We should remove all un-need tests and functions in the test suite.

The new BSP should be a deep sleep looping function that displays temperature and environment metics to the EPD, as well is the current battery life in volts and percent. 

The two buttons should be used, one button for power on/off/hold from sleep, the boot button should be used for external wake/sleep.

The device should auto power off at 5-10% battery life.

we need to respect and fully utilize the bsp lifecycle mechanism for state management, cold/warm boot, etc ... please refer to bsp_common.c/.h for implementation details.

metrics should be in my native si unit (USA) as I can't visualize Celsius change very well in my head. we should include dew point, abs humidity, rh, temperature. battery value in v/%, up time, via external rtc. the deep sleep wake mech should be the rtc, we will use alarm, not timer. exact time is not needed, I would like to focus on duration, or elapsed time. please include a calculated current consumption metric, this will illustrate approx run time based on, current time duration and batt voltage. the battery capacity is fixed at 400mah.

again this example/demo project is to test the new bsp additions for functionality and bugs. we need to be able to set the deep sleep duration at runtime. screen updates will happen before deep sleep. during deep sleep the device should be in extremely low power mode. only the rtc domain on the esp32 should be in use (for the ulp etc...). all periferals need to be powered down or in a low power state before deep sleep. 

we will not enable the radios in this example application, we also do not need to include the bsp_tb wrapper for things board and mqtts.

***(maybe we do enable wifi to set/sync the time, we can test ble provisioning too, that way we don't have to hardcod any auth keys or tokens)*** I haven't fully decided if this is absolutely needed or not. if this is implemented, we need to add additional button operations for clearing the provisioned device mem at runtime. ideally this should be a minimal example, but I could make the argument for time synchronization if we logged everything. 

speaking of logging, what effects would the micro SD card have on the power consumption!? there's no way to gate the SD card power rail. if the card remains powered, what would be the estimated nominal current draw at idle? or does it only use power during read and wright operations because it's non volatile flash? there ***has*** to be at least alittle current draw at idle, I can't image that it just sits there contetent on the rail, drawing nothing. please investigate this issue further.

> Gemini says, 300 µA – 800 µA for consumer micro SD cards, I'm inclined to accept the approximate idle current as fact.

we need to make use of the bsps callback hooks for lifecycle management, ensure the default audio tone registration is disabled during init. I'm like 90% sure it's not being implemented in this bsp version, but check to make sure.

at any point during development of the bsp and its demo app, you find a redundant code block, please document it, as during the dev cycle I've tried to consolidate and minimize the bsp foot print. as a result code artifact may be left/right hiding in the code, and standard lifecycle functions may have more than one logical entry point. these are undesirable by products of development and should be recorded for review so that we my prune the source tree in future revisions. IE doc for removal.


