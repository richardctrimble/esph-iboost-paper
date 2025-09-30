# esph-iboost-paper

iBoost Buddy implementation using ESPHome on Heltec Wireless Paper with SX126x radio.

its 2 components:

**esphWirelessPaper** - to drive the screen
**esphiBoost** - to drive the radio section and decoders

# Important
Many thanks for the wonderful work and support from this project the original C1101 & ESP8266 implementation  [JMSwanson / ESP-Home-iBoost](https://github.com/JNSwanson/ESP-Home-iBoost) 

## What it does

Decodes iBoost packets on 868MHz and exposes the data to Home Assistant. Can also send boost start/cancel commands. Built it because I had problems with my CC1101 and 8266 and wanted a prebuilt board..  

if you have an esp32 and sx126x you can just drop the screen part that doesnt do much yet as i havent got around writing to the screen

this version is using a preview version of the sx126x esphome component that should be live in 2025.9.2.

## Hardware

- Heltec Wireless Paper V1.1 (ESP32-S3)
- Built-in SX1262 radio (868MHz FSK)
- E-ink display
- Physical button on GPIO0

## Components

### esphiBoost
Main radio protocol handler:
- Listens for packets from iBoost system (sender/buddy/main unit)
- Decodes energy data (today, yesterday, 7/28 days, total)
- Sends boost control commands
- Auto-discovers system address from received packets
- Cycles through data requests (0xCA-0xCE)

### esphWirelessPaper
E-ink display wrapper:
- Shows title, status, and data lines
- Fast refresh for updates
- Currently just shows basic status (display data integration TODO)

## What you get in Home Assistant

- Energy sensors for all time periods
- Power/import readings
- Boost time remaining
- RSSI values for diagnostics
- Boost start/cancel buttons
- Configurable boost duration (15min steps)

## Setup

Just flash `minibuddy_iBoostPaper.yaml` to your Wireless Paper. The radio config matches the iBoost protocol so it should start receiving packets automatically once in range.

im using a base.yaml for some settings, so you will need to update that, it has wifi etc in the secrets file,  also in the main yaml you will need to update as i use a fixed ip address.

Radio settings: 868.3MHz, 99.975kbps, sync `[0xD3, 0x91, 0xD3, 0x91]`
