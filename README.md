# 💧 HydraSprink

**ESP32-based 4-zone smart sprinkler controller with a self-hosted web UI, weather integration, OLED display support, and OTA firmware updates.**

No cloud. No app. No subscription. Just connect to the device's IP in any browser.

---

## Features

### Zone Control
- 4 independently controlled relay zones (GPIO 16–19)
- Manual ON toggle per zone
- Timed runs — set any duration in minutes
- Rename zones with an explicit **Save** button — type freely, periodic refreshes never overwrite an unsaved edit. Names persist across reboots
- Stop all zones with one tap
- Timed sequence — run all zones back-to-back for a set duration each

### Scheduling
- Up to 16 schedules stored in flash
- Per-schedule: zone, start time, duration, day-of-week selection, enable/disable
- Schedules survive reboots
- Reliable 60-second trigger window — won't miss a firing if the ESP32 is briefly busy
- Timezone-aware — set your UTC offset in Settings so schedules fire at true local time
- **Rain skip** — schedules are automatically skipped when OpenWeatherMap reports **any rain** in the last hour (threshold tunable via `RAIN_SKIP_MM` in firmware). Manual and Timed Run actions from the web UI still fire (intentional overrides). Skip events are logged to Serial
- Schedule rows display the current **zone names** (not generic "Zone 1/2/3/4"); rename a zone and every existing schedule + the Add Schedule dropdown updates within one poll cycle (~3 s)

### Weather Integration
- Pulls current conditions from [OpenWeatherMap](https://openweathermap.org) (free API key)
- Displays temperature, humidity, and rainfall
- Weather fetched 3 seconds after boot, then refreshed every 10 minutes
- Configurable city and country code

### OLED Display
- Optional — select display type in Settings; no reboot required to disable
- **Mini 0.91"** — 128×32 SSD1306, 3 rotating idle screens (zones, weather, network/time)
- **Standard 0.96"** — 128×64 SSD1306, 4 rotating idle screens (zones, weather, clock, schedules)
- During active watering always shows zone status regardless of rotation
- Splash screen on boot; IP address shown after WiFi connects
- I²C: SDA = GPIO 21, SCL = GPIO 22 (default address `0x3C`, change to `0x3D` in firmware if blank)

### Settings & Persistence
- All settings stored in ESP32 flash (NVS via `Preferences`) — survive power cycles
- Wi-Fi SSID / password
- OpenWeatherMap API key, city, country
- UTC timezone offset — applied live, no reboot needed
- Imperial / metric toggle — applied live, no reboot needed
- OLED display type — saved to flash, takes effect after reboot

### Display Units
- Toggle between **Metric** (°C, mm) and **Imperial** (°F, inches) from the Settings tab
- Preference saved to the device; both the web UI and OLED respect the setting

### Web UI
- Served directly from the ESP32 — no internet connection required after initial setup
- Tabs: Zones · Weather · Schedules · Settings · Firmware
- Mobile-friendly — tested on iPhone mini (375px); all tabs, inputs, and buttons sized for thumbs
- No iOS auto-zoom bug (inputs correctly sized to prevent Safari from zooming on focus)
- Schedule list switches to a card layout on small screens
- Dark theme with monospace accents

### OTA Firmware Updates
- Upload a compiled `.bin` directly from the Firmware tab
- Progress bar during upload
- Device reboots automatically on success

### Networking
- Connects to your Wi-Fi on boot
- Falls back to AP mode (`HydraSprink_AP`) if no credentials are stored or connection fails
- NTP time sync via `pool.ntp.org` and `time.nist.gov`

### No Authentication
HydraSprink has no username/password login. It is designed for use on a trusted local network. Do not expose the device's web interface to the public internet without placing your own authenticated reverse proxy in front of it.

---

## Hardware

| Component | Details |
|---|---|
| Microcontroller | ESP32 (any standard dev board) |
| Relay board | 4-channel, active-low (most common relay modules) |
| Zone pins | GPIO 16, 17, 18, 19 (configurable in firmware) |
| OLED (optional) | SSD1306 128×32 or 128×64, I²C |
| I²C pins | SDA = GPIO 21, SCL = GPIO 22 |
| Power | 5V via USB or regulated supply to the ESP32 |

> Relay boards with active-high logic: set `ACTIVE_LOW = false` near the top of `HydraSprink.ino`.

---

## Required Libraries

Install via Arduino IDE → Library Manager:

| Library | Author |
|---|---|
| ArduinoJson | Benoit Blanchon (v6 or v7) |
| Adafruit SSD1306 | Adafruit |
| Adafruit GFX Library | Adafruit |

Built-in ESP32 libraries used (no install needed): `WiFi`, `WebServer`, `HTTPClient`, `Preferences`, `Update`, `Wire`

---

## Installation

1. Clone or download this repository.
2. Open `HydraSprink.ino` in the Arduino IDE. Ensure `html.h` is in the same folder.
3. Install the required libraries listed above.
4. Select your ESP32 board under **Tools → Board**.
5. Optionally edit the pin assignments and `ACTIVE_LOW` near the top of `HydraSprink.ino`.
6. Flash to the ESP32.

---

## First Boot

On first boot with no Wi-Fi credentials stored, the device starts in AP mode:

- **SSID:** `HydraSprink_AP`
- **IP:** `192.168.4.1`

Connect to that network, open `http://192.168.4.1` in a browser, go to **Settings → Network**, enter your Wi-Fi credentials, and tap **Save & Reboot**. The device will join your network and display its assigned IP on the OLED (if connected) and the Serial monitor.

---

## Settings Reference

| Setting | Location | Triggers Reboot? |
|---|---|---|
| Wi-Fi SSID / Password | Settings → Network | Yes |
| OpenWeatherMap API Key | Settings → Weather API | Yes |
| City / Country | Settings → Weather API | Yes |
| OLED Display Type | Settings → OLED Display | Yes |
| UTC Timezone Offset | Settings → Timezone | No — applied immediately |
| Imperial / Metric | Settings → Display Units | No — applied immediately |

**Timezone examples:** PST = `-8` · PDT = `-7` · EST = `-5` · EDT = `-4` · GMT = `0` · CET = `1`

---

## OLED Wiring

```
ESP32           SSD1306
─────           ───────
3.3V     →      VCC
GND      →      GND
GPIO 21  →      SDA
GPIO 22  →      SCL
```

If the display stays blank after selecting your type in Settings, try changing `OLED_ADDR` from `0x3C` to `0x3D` in `HydraSprink.ino` and reflashing.

---

## File Structure

```
HydraSprink/
├── HydraSprink.ino   — Firmware (ESP32 logic, API handlers, scheduling, OLED)
└── html.h            — Web UI served as a PROGMEM string from flash
```

---

## Version History

| Version | Summary |
|---|---|
| **v0.7.0** | Rain skip — schedules are automatically skipped whenever OpenWeatherMap reports any rain in the last hour (manual/timer runs unaffected); explicit **Save** button for zone renames (no more focus-snap during periodic polling, unsaved edits preserved across refreshes); OLED selector dropdown actually wired into the Settings UI (was claimed in v1.6.0 but missing); zone-name changes now flow through to the Schedules tab (table rows, mobile cards, and the Add Schedule dropdown all reflect the current name) |
| **v1.6.0** | Timezone UTC offset setting (live, no reboot); schedule reliability fix (60-second window + per-schedule fire tracking); input snap-back fix in web UI; OLED dropdown added to Settings UI; full mobile-responsive UI (iOS zoom fix, schedule card layout, 44px tap targets, single-column zones on small screens) |
| **v1.5.0** | OLED display support — Mini 0.91" (128×32) and Standard 0.96" (128×64); rotating idle screens; boot splash; IP display after WiFi connects |
| **v1.4.0** | ArduinoJson v7 compatibility fix; conditional reboot logic (unit toggle saves silently, network/API changes trigger reboot); imperial/metric toggle |
| **v1.3.0** | 4-zone relay control; schedule persistence in flash; NTP time sync; weather fetch on boot; 1-second zone timer tick |

---

## License

MIT
