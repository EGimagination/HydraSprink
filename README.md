# HydraFlow 💧
### ESP32 4-Channel Smart Sprinkler Controller

A fully local, web-based sprinkler controller with weather intelligence. No cloud. No subscription. No app. Just navigate to an IP address.

---

## Features

- **4 independent zones** — manual on/off, timed runs, and full weekly scheduling
- **Rain skip** — pulls a 24h precipitation forecast from OpenWeatherMap and skips scheduled watering when rain exceeds your threshold
- **Scheduler** — up to 8 schedules per zone, day-of-week selection, persists through reboots
- **OTA updates** — flash new firmware wirelessly via the web UI
- **Built-in manual** — full user documentation served from the device itself
- **AP fallback** — if WiFi is unreachable, the device spins up a hotspot (`HydraFlow-Setup`) so you're never locked out
- **Zero dependencies** — no MQTT broker, no Home Assistant, no external services beyond a free OWM API key

---

## Hardware

| Component | Notes |
|---|---|
| ESP32 (any variant) | Tested on ESP32 Dev Module |
| 4-channel relay module | Active LOW recommended |
| 5V power supply | For relay module VCC |

**Default GPIO mapping** (configurable in firmware):

| Zone | GPIO |
|---|---|
| Zone 1 | 16 |
| Zone 2 | 17 |
| Zone 3 | 18 |
| Zone 4 | 19 |

---

## Dependencies

Install via Arduino Library Manager:

- [`ArduinoJson`](https://arduinojson.org/) — Benoit Blanchon
- [`NTPClient`](https://github.com/arduino-libraries/NTPClient) — Fabrice Weinberg
- [`AsyncTCP`](https://github.com/me-no-dev/AsyncTCP) — ESP32 variant
- [`ESPAsyncWebServer`](https://github.com/mathieucarbou/ESPAsyncWebServer) — mathieucarbou fork recommended

**Board settings:**
- Board: `ESP32 Dev Module`
- Partition Scheme: `Default 4MB with spiffs` or `Minimal SPIFFS`

---

## Configuration

Edit the `USER CONFIGURATION` block at the top of `sprinkler_controller.ino` before flashing:

```cpp
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const int ZONE_PINS[4]    = {16, 17, 18, 19};
const bool ACTIVE_LOW     = true;

const char* OWM_API_KEY   = "YOUR_OWM_API_KEY";
const char* OWM_CITY      = "London";
const char* OWM_COUNTRY   = "GB";

const float RAIN_SKIP_MM  = 2.5;

const bool WEB_AUTH_ENABLED = false;
```

A free OpenWeatherMap API key is sufficient — the device calls the API every 30 minutes, well within the free tier limit.

---

## Web UI

Once flashed, the device's IP is printed to the serial monitor. Open it in any browser on the same network.

| Tab | Description |
|---|---|
| **Zones** | Manual control, timed runs, zone renaming, run-all sequence |
| **Weather** | Live conditions, rain forecast, skip status |
| **Schedules** | Add/remove schedules, day-picker, per-zone configuration |
| **Firmware** | OTA update via `.bin` upload |
| **Manual** | Full user documentation, pin reference, troubleshooting |

---

## License

MIT

