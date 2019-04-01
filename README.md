{:target="_blank"}

# LinkIt_SDK_V4.6.2_HOUNDIFY [![Build Status](https://travis-ci.org/luvinland/LinkIt_SDK_V4.6.2_HOUNDIFY.svg?branch=master)](https://travis-ci.org/luvinland/LinkIt_SDK_V4.6.2_HOUNDIFY){:target="_blank"} [![GitHub release](https://img.shields.io/github/release/luvinland/LinkIt_SDK_V4.6.2_HOUNDIFY.svg)](https://github.com/luvinland/LinkIt_SDK_V4.6.2_HOUNDIFY/releases) 


## Overview
MediaTek LinkIt(TM) Development Platform for RTOS provides a Houndify AI Platform (SoundHound Inc.) software solution for devices based on the MT7686 SOCs.


## Video
* Environment : 48dB Quite and 86dB noise.
* Detected "Alexa" trigger and voice response from cloud service, and then light turn on/off.

**Noise**|**48dB Quite**|**86dB Noise**
---|---|---
**Video**|[![Video Label](http://img.youtube.com/vi/BgAyqwcSkXE/0.jpg)](https://youtu.be/BgAyqwcSkXE?t=0s)|[![Video Label](http://img.youtube.com/vi/AKwL_wjtzu0/0.jpg)](https://youtu.be/AKwL_wjtzu0?t=0s)


## Block diagram
![Block Diagram](https://user-images.githubusercontent.com/26864945/54342151-0af30380-467f-11e9-8a83-520df05f149e.png)


## Hardware
* Main SoC : [MediaTek MT7686](https://labs.mediatek.com/en/chipset/MT7686)
* Audio Codec : [AKM AK4951EN](https://www.akm.com/akm/en/aboutus/news/20140910AK4951_001/)


## Build Enviroment
Previous [Commits](https://github.com/luvinland/LinkIt_SDK_V4.6.2_HOUNDIFY/tree/badb18cef48afdd6ae00fcf46cde56b2127da091)


## Configuration
* WiFi SSID / PASSWORD
```c
Wifi_default.h (project\mt7686_hdk\apps\bse_hound\inc)

#define HOUND_WIFI_DEFAULT_STA_SSID               ("{wifi_ssid}")
#define HOUND_WIFI_DEFAULT_STA_SSID_LEN           ("{ssid_length}")
...
#define HOUND_WIFI_DEFAULT_STA_WPA_PSK            ("{wifi_password}")
#define HOUND_WIFI_DEFAULT_STA_WPA_PSK_LEN        ("{password_length}")

```

* SoundHound Inc. Houndify account data. 
```c
Main.cpp (project\mt7686_hdk\apps\bse_hound\src)

#define CLIENT_ID         "{your_id}"
#define CLIENT_KEY        "{your_key}"

```
