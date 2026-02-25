# ESPHome iBoost Paper

An [ESPHome](https://esphome.io/) firmware for monitoring and controlling an iBoost solar immersion system using a Heltec Wireless Paper (ESP32-S3 with SX1262 radio). Exposes energy data and boost controls to [Home Assistant](https://www.home-assistant.io/).

> **Note**
> This project requires ESPHome 2026.2 or later, which includes the native SX126x radio component.

## Features

- **868MHz packet decoding** — listens for iBoost protocol packets and decodes energy and status data automatically.
- **Auto-discovery** — learns the iBoost system address from received packets, no manual configuration needed.
- **Boost control** — start and cancel manual boost commands directly from Home Assistant.
- **E-ink display** — shows system status on the Heltec Wireless Paper's built-in e-ink screen.
- **All-in-one board** — no separate radio module required; the Heltec Wireless Paper has everything on-board.

## Hardware

| Component | Details |
|-----------|---------|
| Board | Heltec Wireless Paper V1.1 (ESP32-S3, 8MB flash) |
| Radio | Built-in SX1262 (868MHz FSK) |
| Display | Built-in e-ink (landscape) |
| Button | GPIO0 (active low) |
| LED | GPIO18 |

## Components

### esphiBoost

Main radio protocol handler:

- Listens for packets from iBoost system (sender, buddy, main unit)
- Decodes energy data (today, yesterday, 7-day, 28-day, total)
- Sends boost start/cancel commands
- Auto-discovers system address from received packets
- Cycles through data request codes (0xCA-0xCE)

### esphWirelessPaper

E-ink display driver:

- Shows title, status, and data lines on the Heltec e-ink display
- Fast-mode refresh for efficient updates
- Basic status display (further integration planned)

## Home Assistant entities

### Sensors

| Entity | Unit | Description |
|--------|------|-------------|
| iBoost Today | Wh | Energy diverted today |
| iBoost Yesterday | Wh | Energy diverted yesterday |
| iBoost Last 7 Days | Wh | Energy diverted in the last 7 days |
| iBoost Last 28 Days | Wh | Energy diverted in the last 28 days |
| iBoost Total | Wh | Total energy diverted |
| Grid Import | W | Current grid import power |
| iBoost Power | W | Current iBoost power consumption |
| Manual Boost Time Remaining | Min | Time left on active manual boost |

### Diagnostic sensors

| Entity | Description |
|--------|-------------|
| RSSI iBoost | Signal strength from the iBoost main unit |
| RSSI Buddy | Signal strength from the Buddy unit |
| RSSI Sender | Signal strength from the Sender unit |
| Packet Count | Total packets received |
| Last Packet Received | Timestamp of the last decoded packet |

### Controls

| Entity | Description |
|--------|-------------|
| Manual Boost START | Start a manual boost |
| Manual Boost CANCEL | Cancel an active manual boost |
| Manual Boost Time | Set boost duration (0-120 minutes, 15-minute steps) |
| LED Light | Toggle the on-board LED |

### Text sensors

| Entity | Description |
|--------|-------------|
| Mode | Current iBoost system mode |
| Warn | Current warning status |

## Setup

1. Clone this repository.
2. Create a `secrets.yaml` with your WiFi credentials, NTP server, and fallback password.
3. Update the `wifi_ip_address` substitution in `minibuddy-iBoostPaper.yaml` to match your network.
4. Flash with ESPHome: `esphome run minibuddy-iBoostPaper.yaml`

The radio is pre-configured to match the iBoost protocol (868.3MHz, 99.975kbps, sync `[0xD3, 0x91, 0xD3, 0x91]`) and will start receiving packets automatically once in range.

### Radio configuration

| Parameter | Value |
|-----------|-------|
| Frequency | 868.3 MHz |
| Bitrate | 99.975 kbps |
| Deviation | 47.6 kHz |
| Sync word | `0xD3, 0x91, 0xD3, 0x91` |
| CRC | 16-bit, polynomial 0x8005 |

> **Tip**
> If you have a different ESP32 with an SX126x radio, you can use just the `esphiBoost` component without `esphWirelessPaper`. Remove the display-related configuration and it will work as a headless receiver.

## Licence

Released under the [MIT Licence](LICENSE).

### Attribution

This project builds upon the following prior work, released under the MIT Licence:

| Project | Author | Licence |
|---------|--------|---------|
| [ESP-Home-iBoost](https://github.com/JNSwanson/ESP-Home-iBoost) | [@JNSwanson](https://github.com/JNSwanson) | [MIT](https://github.com/JNSwanson/ESP-Home-iBoost/blob/main/LICENSE) |
