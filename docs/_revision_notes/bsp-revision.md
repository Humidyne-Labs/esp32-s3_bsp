# New BSP Direction (A Moderately Large Laundry List of Changes & Additions)

1.) a full example test suit to ensure that all periferals are up to snuff. this will be a genaric run test, all API functions should be tested for operations via the test suit. all boot modes, all sleep modes included.

1.b) we also need a versioning mechanism in place for tracking changes and revisions, I've been on V0.0.0 this whole time.

2.) two independent sleep wake modes, configuration setting available during run time. use light sleep and deep sleep methods. if possible both modes should be configureable with the internal or external rtc.

3.) dynamic initialization. we should feature 3 distinct init modes for the hardware, a fast mode, a full mode, and a min mode, fast mode configures the periferals for the wake cycle. full mode is for a cold boot, min mode is a lean set select periferals required for telemetry readings. we can use the systems information that gets reported in the esp-idf to determine if it's a cold boot, wake boot, (or light wake boot?), or min boot. min/wake should be flagged in rtc memory. we should not init the audio codec or amp unless we are playing audio, but all power rails need to come up hot for reliable I2C coms, this is due to a weird hardware condition that clamps the I2C lines if the codec is powered down during I2C transactions.

4.) LVGL, when initialized for the first time, it flashes the screen, on wake, we need to ensure the display driver is initialized, but we shouldn't write anything to memory, or clear the screen. this persists the residual eink state.

4.b) as a bonus, we could save the current screen state on sleep or power down, then reinit the display ram, so that when we send new data, a partial update is done rather than a full screen update. this prevents long screen flicker and faster refresh times.

4.c) I would like splash callbacks for both UI/image display, as well as audio playback callbacks for init, alarms, chimes, and signals. we should have a boot splash, a sleep splash option, and a shutdown slash option. 

5.) I want to incorporate wifi and ble into the bsp for quick use in the main application. this will solidify the "full" Monty, as far as the hardware support is concerned.

5.a) ble should be used for wifi provisioning, we can create a call back function for the UI elements, we should handle the name, the pair key, and have the option of QR code display on button press with the use of LVGL as the barcode generator if needed. we will need to enable QR code support in the sdconfig, if the provision sdk does not provide conversation. the image format will be 1bit, available space for display is 200x200 pixels, "if" that's large enough for a QR. I don't know ahead of time what the data length is, so giving you an estimate is beyond my ability at the current moment.

5.b) we should wrap wifi in the bsp, the use case will be a things board mqtts workflow. we need to handle gracefully init, graceful connect, gracefully disconnect, graceful radio power down, gracefully hot connect on wake. use the internal mem for key storage.

5.c) since we are wrapping wifi, we should have access in the bsp for getting operational attributes, Mac address, device name, ken generation, ip address, maybe up time.

5.d) ideally the BSP should use a preconfigured partition table, this table will have two app slots. if possible, we should extend the bsp to support genaric OTA updates. use case will be things board OTA, but if we can add hooks for the main app, that would be ideal. this creature a conduit for the main application, hopefully reducing app code size, by abstracting everything into the bsp.

6.) since we are wrapping wifi, we can incorporate ntsp sync for the Internal and external rtc. we need to make sure that the timezone is configured and user settable. a periodic sync timer maybe useful for keeping the rtc up to date during extended use, maybe once every 24h we sync the rtc? time mode should be displayed in a 12h format with an/pm designation. mm/dd/yy dow, as a string would also be nice to have. 4 time formats should be accessible, 24h hh:mm:ss, 24h hh:mm, 12h hh:mm:ss am/pm, 12h hh:mm am/pm.

7.) technically the hardware that the bsp is for, does not have a touch controller, we should remove touch support from the bsp, and update the board description, and kconfig.

8.) since my target application is again thingsboard, "maybe" we could incorporate a json parser wrapper into the bsp for quick use.

8.b) again with the target in mind, maybe a general mqtts wrapper as well. maybe we could turn it into a tb framework with the following support, remote procedure calling, server attributes, shared attributes, client, sub/pub, disconnect, claiming, attributes, alarms checking, OTA. again this should be isolated to the bsp for use with the main application, we just need handle logic. we could att the things board support as a conditional config in kconfig. please lup this together with bullet number 8 and 5.d, keeping the partition management as a standard staple in the bsp independent of the configurable tb framework. so... idk if we should have a tb layer built on json, mqtt and OTA or if we should try to consolidate the tb layer as a singleton. the architectural decisions is up to you.

9.) all thingsboard logic and contoll will be handled in the main application logic. we should try to utilize callbacks as much as possible for flexibility in the tb framework. 

10.) power and temp readings are currently not reflective of actually environmental conditions, the batt never sees 100% charge, humidity is too low per my reference measurements, temp is too high per my reference measurements. current environmental state is, 58% relitive humidity, 73.6°F. see if you can find a way to calibrate the shtc3 and confirm if the humidity readings are relitive or absolute. (there's a difference right?)

11.) we should take the time to add our own bsp error handling later as well as a debug diagnostic layer. debug should be enabled during run time for on the fly inspection if needed.

12.) add a wrapper around the Internal nvs rtc memory for easy access.

13.) we will need to inspect the current bsp, take inventory, refactor if needed, and expand the bsp to support the above features/additions/changes and restructuring.

# Points of Interest (Things to Note While in DEV Mode)

- your currently connected to all esp-idfs documentation as well as an LVGL simulator (idk how it works either). you also have the ability to call a chrome browser instance for looking up anything you need on the web. please utilize the tools available to you to aid in development. This is your mission, should you choose to accept it. This communication will self-destruct, in 10 seconds. (kidding, I'm asking for help, I mean, you could decline, if you really wanted too, I guess. I would be a little disappointed if ya did)
- assume the main application that will be using the bsp will be initialized on every wake up event. I know we have the option of resume in place, but if we structure everything to run clean on cold, it should run clean on warm too. build up and take down before sleep where applicable so we don't run into any fun and exciting surprises, like a Room with a Moose! or worse, Nutella!
- the external RTC has a ln 8bit general storage register that stays powered as long as the battery rail is up. idk what we should use it for, or if we even need to. maybe a rnd seed? or crc if the... idk... use your "imagination" *paints a rainbow with my hands in the air in front of me*.
- epaper is 188dpi, 200x200 pixels. I assume the font is scaled appropriately, ie 14point font should be 18.6667px etc ...
- to be blunt text inversion looks like utter shit, we should some how come up with a solution to display a nice looking inversion on epaper.
- I would like to eventually investigate how mp3 would be handled by the codec, and if it's possible to store them on the internal partition for use as sounds bytes and notification alerts.
- to test the partition mounting, we could just render a patern on disk, then display it through our bsp channel. this avoids byte heavy assets in the bsp/example during compilation and testing.
- the documentation shouldn't be forgotten about, once everything is finalized, we/you should update the associated documentation for presentability.
- this is a "hobby" project, but like most, I also do this work professionally. I would appreciate it, if you could hold yourself to the same high expectation. the work being published is a reflection on both of us and our ability.
- please use a minimal amount of doxygen comments for the functions, and do comment the code. also make an effort to include a header/tittle blocks per file. avoid switch/case, do/while, and goto/continue. break is fine, I like break. keep recursion to a minimum if you need to use it. switch/case if you need it, ie in the error handling code. namespace is fine, pragma once in the hpp's is fine, try to pick c over c++ for portability. null is fine, please don't leave dangling pointers, or headless tasks running forever, everything should have a natural start up and shut down with a clean up. please do use tasks in freertos, mux, sema, lock where needed. ensure tasks have enough stack for execution. 
- I'm going to be pulling in the bsp as a registered module from my public repo when it comes time to wright the application code. any additional resources you can provide to bootstart the application code will be appreciated.
- all network junk should be on one core, all UI junk should be on another core, since we have two at our disposal.
- as encouragement, I would like to say, that so far, I'm having an enjoyable time working with you. most of my frustration comes from not knowing what I want, vs what your able to provide. your code edits have been very helpful, and I'm looking forward to the results of this revision.
- if you need a break, or think I should stop and reflect on the work done, just let me know, I will be more than happy to accommodate. I see us as a team, and I would like to encourage you to share the same perspective. little corrections along the way prevents a 90° degree turn down the road, or a fork! or those damn roundabouts... I hate those things, I always end up going around 3-4 times before I figure out how to actually get out. it's maddening sometimes.... why ODOT thinks people wanna drive in an endless circle, I have no idea. maybe it's a metaphor for there design ability.
- I grew a pear! but I only produced one *chuckles to self*. I bought a pear a few years back, it finally produced, I thought the thing was dead. damn thing was 80 bucks, and they shipped it to me! in the mail! alive! I was kinda impressed *nods*.


