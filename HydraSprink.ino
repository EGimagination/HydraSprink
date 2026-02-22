/*

           HydraFlow — ESP32 4-Channel Sprinkler Controller        
           Version 1.2.0                                          
                                                                   
    Place both files in the same sketch folder:                     
     HydraSprink.ino   (this file)                        
     html.h                     (web UI)                           
                                                                   
    Board:     ESP32 Dev Module (or ESP32-WROOM-DA etc.)            
    Partition: Default 4MB with spiffs  OR  Minimal SPIFFS          
                                                                   
    Required libraries (install via Library Manager):               
    ArduinoJson          - Benoit Blanchon                        
     NTPClient            - Fabrice Weinberg                       
     AsyncTCP             - mathieucarbou / ESP32 variant          
     ESPAsyncWebServer    - mathieucarbou fork                     

*/

#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <time.h>
#include <Update.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "html.h"

// ── Hardware ──────────────────────────────────────────────────
#define NUM_ZONES     4
#define MAX_SCHEDULES 16
#define FIRMWARE_VER  "1.2.0"

int  ZONE_PINS[NUM_ZONES] = {16, 17, 18, 19};
bool ACTIVE_LOW = true;

// I2C pins (default ESP32)
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_ADDR 0x3C   // Most common; try 0x3D if display blank

// ── OLED types ────────────────────────────────────────────────
// 0 = None, 1 = 128x32, 2 = 128x64
#define OLED_NONE   0
#define OLED_128x32 1
#define OLED_128x64 2

// ── Globals ───────────────────────────────────────────────────
Preferences prefs;
WebServer   server(80);

// OLED state — allocated dynamically based on user setting
Adafruit_SSD1306* oled     = nullptr;
uint8_t           oledType = OLED_NONE;   // loaded from flash

// Idle screen cycling
uint8_t       idleScreen   = 0;
unsigned long lastScreenMs = 0;
#define SCREEN_INTERVAL_MS 4000   // rotate every 4 seconds

// ── Wi-Fi & OWM settings ──────────────────────────────────────
String WIFI_SSID, WIFI_PASS, OWM_API_KEY, OWM_CITY, OWM_COUNTRY;

// ── Zone state ────────────────────────────────────────────────
struct Zone {
  bool   active;
  bool   manualOn;
  int    timerRemaining;
  String name;
};
Zone zones[NUM_ZONES];

// ── Schedule ──────────────────────────────────────────────────
struct Schedule {
  int  zone, hour, minute, durationSec, days;
  bool enabled;
};
Schedule schedules[MAX_SCHEDULES];
int scheduleCount = 0;

// ── Weather ───────────────────────────────────────────────────
struct Weather {
  float  tempC       = 0;
  String description = "No data";
  String icon        = "";
  float  rainMm      = 0;
  int    humidity    = 0;
  bool   skipWatering = false;
  time_t lastChecked  = 0;
} weather;

unsigned long lastWeatherCheck    = 0;
bool          weatherRefreshPending = false;

// ══════════════════════════════════════════════════════════════
//  SETTINGS PERSISTENCE
// ══════════════════════════════════════════════════════════════
void loadSettings() {
  prefs.begin("hydra", true);
  WIFI_SSID   = prefs.getString("ssid",    "");
  WIFI_PASS   = prefs.getString("pass",    "");
  OWM_API_KEY = prefs.getString("api",     "");
  OWM_CITY    = prefs.getString("city",    "San Fransisco");
  OWM_COUNTRY = prefs.getString("country", "US");
  oledType    = (uint8_t)prefs.getInt("oled", OLED_NONE);
  prefs.end();
}

void saveSettings() {
  prefs.begin("hydra", false);
  prefs.putString("ssid",    WIFI_SSID);
  prefs.putString("pass",    WIFI_PASS);
  prefs.putString("api",     OWM_API_KEY);
  prefs.putString("city",    OWM_CITY);
  prefs.putString("country", OWM_COUNTRY);
  prefs.putInt("oled",       (int)oledType);
  prefs.end();
}

void loadZoneNames() {
  prefs.begin("zones", true);
  for (int i = 0; i < NUM_ZONES; i++)
    zones[i].name = prefs.getString(("z"+String(i)).c_str(), "Zone "+String(i+1));
  prefs.end();
}

void saveZoneNames() {
  prefs.begin("zones", false);
  for (int i = 0; i < NUM_ZONES; i++)
    prefs.putString(("z"+String(i)).c_str(), zones[i].name);
  prefs.end();
}

void loadSchedules() {
  prefs.begin("sched", true);
  scheduleCount = prefs.getInt("cnt", 0);
  if (scheduleCount < 0 || scheduleCount > MAX_SCHEDULES) scheduleCount = 0;
  for (int i = 0; i < scheduleCount; i++)
    prefs.getBytes(("s"+String(i)).c_str(), &schedules[i], sizeof(Schedule));
  prefs.end();
}

void saveSchedules() {
  prefs.begin("sched", false);
  prefs.putInt("cnt", scheduleCount);
  for (int i = 0; i < scheduleCount; i++)
    prefs.putBytes(("s"+String(i)).c_str(), &schedules[i], sizeof(Schedule));
  prefs.end();
}

// ══════════════════════════════════════════════════════════════
//  RELAY HELPERS
// ══════════════════════════════════════════════════════════════
void setRelay(int z, bool on) {
  if (z < 0 || z >= NUM_ZONES) return;
  digitalWrite(ZONE_PINS[z], ACTIVE_LOW ? !on : on);
  zones[z].active = on;
}

void stopZone(int z) {
  if (z < 0 || z >= NUM_ZONES) return;
  zones[z].manualOn       = false;
  zones[z].timerRemaining = 0;
  setRelay(z, false);
}

void startZone(int z, int durationSec) {
  if (z < 0 || z >= NUM_ZONES) return;
  setRelay(z, true);
  zones[z].manualOn       = (durationSec == 0);
  zones[z].timerRemaining = durationSec;
}

// ══════════════════════════════════════════════════════════════
//  OLED — INIT
// ══════════════════════════════════════════════════════════════
void initOLED() {
  if (oledType == OLED_NONE) {
    if (oled) { delete oled; oled = nullptr; }
    return;
  }

  Wire.begin(OLED_SDA, OLED_SCL);

  int h = (oledType == OLED_128x64) ? 64 : 32;
  if (oled) delete oled;
  oled = new Adafruit_SSD1306(128, h, &Wire, -1);

  if (!oled->begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("[OLED] Init failed — check wiring and address");
    delete oled;
    oled = nullptr;
    return;
  }

  oled->clearDisplay();
  oled->setTextColor(SSD1306_WHITE);

  // Boot splash
  oled->setTextSize(1);
  oled->setCursor(0, 0);
  oled->println("  HydraSprink");
  if (oledType == OLED_128x64) {
    oled->println();
    oled->println("  v" FIRMWARE_VER);
    oled->println();
    oled->println("  Starting...");
  } else {
    oled->println("  v" FIRMWARE_VER " Starting");
  }
  oled->display();
  Serial.printf("[OLED] Init OK (%dx%d)\n", 128, h);
}

// ══════════════════════════════════════════════════════════════
//  OLED — DRAW HELPERS
// ══════════════════════════════════════════════════════════════

// Returns count of active zones, fills activeIdx[] with their indices
int getActiveZones(int* activeIdx) {
  int n = 0;
  for (int i = 0; i < NUM_ZONES; i++)
    if (zones[i].active) activeIdx[n++] = i;
  return n;
}

// Format seconds as "4m 32s" or "58s"
String fmtTime(int sec) {
  if (sec <= 0) return "--";
  if (sec >= 60) return String(sec/60) + "m " + String(sec%60) + "s";
  return String(sec) + "s";
}

// ── 128x32: compact 2-line layout ─────────────────────────────
void draw32_zones() {
  // Line 1: zone status dots  Z1[+] Z2[+] Z3[ ] Z4[ ]
  // Line 2: active zone countdown or "All idle"
  int active[NUM_ZONES];
  int n = getActiveZones(active);

  oled->setTextSize(1);

  // Row 0: zone indicator blocks
  oled->setCursor(0, 0);
  for (int i = 0; i < NUM_ZONES; i++) {
    oled->print("Z"); oled->print(i+1);
    oled->print(zones[i].active ? "+" : "-");
    oled->print(" ");
  }

  // Row 1: countdown for first active zone, or idle message
  oled->setCursor(0, 12);
  if (n == 0) {
    oled->print("All zones idle");
  } else {
    int z = active[0];
    oled->print(zones[z].name.substring(0, 7));
    oled->print(": ");
    oled->print(zones[z].manualOn ? "Manual" : fmtTime(zones[z].timerRemaining));
    if (n > 1) { oled->print(" +"); oled->print(n-1); }
  }

  // Row 2: rain skip badge
  oled->setCursor(0, 24);
  oled->print(weather.skipWatering ? "! Rain skip ON" : "  Rain skip off");
}

void draw32_weather() {
  oled->setTextSize(1);
  oled->setCursor(0, 0);
  oled->print(weather.tempC, 1); oled->print("C  ");
  oled->print(weather.humidity); oled->print("% RH");
  oled->setCursor(0, 12);
  // Truncate description to fit 21 chars
  String d = weather.description;
  if (d.length() > 21) d = d.substring(0, 21);
  oled->print(d);
  oled->setCursor(0, 24);
  oled->print("Rain: "); oled->print(weather.rainMm, 1); oled->print("mm");
}

void draw32_network() {
  oled->setTextSize(1);
  oled->setCursor(0, 0);
  oled->print("HydraSprink");
  oled->setCursor(0, 12);
  if (WiFi.status() == WL_CONNECTED) {
    oled->print(WiFi.localIP().toString());
  } else {
    oled->print("AP: hydrasprink_AP");
  }
  oled->setCursor(0, 24);
  // Show time
  struct tm ti;
  if (getLocalTime(&ti)) {
    char buf[20];
    strftime(buf, sizeof(buf), "%a %H:%M:%S", &ti);
    oled->print(buf);
  } else {
    oled->print("Time: syncing...");
  }
}

// ── 128x64: full layout, more info ────────────────────────────
void draw64_zones() {
  int active[NUM_ZONES];
  int n = getActiveZones(active);

  // Title bar
  oled->setTextSize(1);
  oled->setCursor(0, 0);
  oled->print("-- ZONE STATUS --");

  // Zone rows: name + status
  for (int i = 0; i < NUM_ZONES; i++) {
    oled->setCursor(0, 10 + i*12);
    // Indicator dot
    if (zones[i].active) oled->print("[*] ");
    else                  oled->print("[ ] ");
    // Name (max 8 chars)
    String nm = zones[i].name;
    if (nm.length() > 8) nm = nm.substring(0, 8);
    oled->print(nm);
    // Countdown / status on right side
    String st;
    if (zones[i].active) {
      st = zones[i].manualOn ? "ON" : fmtTime(zones[i].timerRemaining);
    } else {
      st = "OFF";
    }
    // Right-align status (col 100+)
    int x = 128 - (st.length() * 6);
    oled->setCursor(x, 10 + i*12);
    oled->print(st);
  }

  // Bottom: rain skip
  oled->setCursor(0, 56);
  oled->print(weather.skipWatering ? "! Rain skip active" : "Rain skip: off");
}

void draw64_weather() {
  oled->setTextSize(1);
  oled->setCursor(0, 0);
  oled->print("-- WEATHER --");

  oled->setTextSize(2);
  oled->setCursor(0, 12);
  oled->print(weather.tempC, 1);
  oled->print("C");

  oled->setTextSize(1);
  oled->setCursor(0, 32);
  String d = weather.description;
  if (d.length() > 21) d = d.substring(0, 21);
  oled->print(d);

  oled->setCursor(0, 44);
  oled->print("Humidity: "); oled->print(weather.humidity); oled->print("%");

  oled->setCursor(0, 56);
  oled->print("Rain 1h: "); oled->print(weather.rainMm, 1); oled->print("mm");
}

void draw64_time() {
  struct tm ti;
  oled->setTextSize(1);
  oled->setCursor(0, 0);
  oled->print("-- TIME & NET --");

  if (getLocalTime(&ti)) {
    // Large time
    char hhmm[6];
    strftime(hhmm, sizeof(hhmm), "%H:%M", &ti);
    oled->setTextSize(2);
    oled->setCursor(20, 14);
    oled->print(hhmm);

    // Seconds small
    oled->setTextSize(1);
    char ss[4];
    strftime(ss, sizeof(ss), ":%S", &ti);
    oled->setCursor(96, 22);
    oled->print(ss);

    // Date
    oled->setTextSize(1);
    char date[20];
    strftime(date, sizeof(date), "%a %d %b %Y", &ti);
    oled->setCursor(0, 40);
    oled->print(date);
  } else {
    oled->setTextSize(1);
    oled->setCursor(0, 20);
    oled->print("Syncing NTP...");
  }

  oled->setTextSize(1);
  oled->setCursor(0, 56);
  if (WiFi.status() == WL_CONNECTED) {
    oled->print(WiFi.localIP().toString());
  } else {
    oled->print("AP: hydrasprink_AP");
  }
}

void draw64_schedules() {
  oled->setTextSize(1);
  oled->setCursor(0, 0);
  oled->print("-- SCHEDULES --");

  if (scheduleCount == 0) {
    oled->setCursor(0, 24);
    oled->print("No schedules set");
    return;
  }

  // Show up to 4 schedules
  int shown = min(scheduleCount, 4);
  for (int i = 0; i < shown; i++) {
    Schedule& s = schedules[i];
    oled->setCursor(0, 12 + i*12);
    char buf[22];
    snprintf(buf, sizeof(buf), "Z%d %02d:%02d %dmin %s",
      s.zone+1, s.hour, s.minute, s.durationSec/60,
      s.enabled ? "ON" : "OFF");
    oled->print(buf);
  }
  if (scheduleCount > 4) {
    oled->setCursor(0, 56);
    oled->print("+"); oled->print(scheduleCount-4); oled->print(" more");
  }
}

// ══════════════════════════════════════════════════════════════
//  OLED — MAIN UPDATE (called from loop every SCREEN_INTERVAL_MS)
// ══════════════════════════════════════════════════════════════
void updateOLED() {
  if (!oled) return;

  // If any zone is active: always show zones screen (no cycling)
  int active[NUM_ZONES];
  bool anyActive = getActiveZones(active) > 0;

  oled->clearDisplay();

  if (anyActive) {
    if (oledType == OLED_128x32) draw32_zones();
    else                          draw64_zones();
  } else {
    // Idle: cycle through screens
    if (oledType == OLED_128x32) {
      // 3 screens: zones/idle, weather, network
      switch (idleScreen % 3) {
        case 0: draw32_zones();   break;
        case 1: draw32_weather(); break;
        case 2: draw32_network(); break;
      }
    } else {
      // 4 screens: zones, weather, time, schedules
      switch (idleScreen % 4) {
        case 0: draw64_zones();     break;
        case 1: draw64_weather();   break;
        case 2: draw64_time();      break;
        case 3: draw64_schedules(); break;
      }
    }
  }

  oled->display();
}

// ══════════════════════════════════════════════════════════════
//  WIFI
// ══════════════════════════════════════════════════════════════
void setupWiFi() {

  WiFi.disconnect(true, true);   // wipe old state
  delay(500);

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);          // IMPORTANT: prevents slow UI
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);

  if (WIFI_SSID.length() == 0) {
    Serial.println("[WiFi] No SSID saved → Starting AP mode");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("hydrasprink_AP");
    Serial.printf("[AP] IP: %s\n", WiFi.softAPIP().toString().c_str());
    return;
  }

  Serial.printf("[WiFi] Connecting to \"%s\"\n", WIFI_SSID.c_str());
  WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());

  unsigned long startAttempt = millis();
  const unsigned long timeout = 15000;

  while (WiFi.status() != WL_CONNECTED &&
         millis() - startAttempt < timeout) {
    delay(250);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[WiFi] CONNECTED");
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
  } else {
    Serial.println("[WiFi] FAILED → Starting AP");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("hydrasprink_AP");
    Serial.printf("[AP] IP: %s\n", WiFi.softAPIP().toString().c_str());
  }
}

// ══════════════════════════════════════════════════════════════
//  WEATHER
// ══════════════════════════════════════════════════════════════
void fetchWeather() {
  if (OWM_API_KEY.length() == 0 || WiFi.status() != WL_CONNECTED) return;
  String url = "http://api.openweathermap.org/data/2.5/weather?q="
               + OWM_CITY + "," + OWM_COUNTRY
//              + "&units=metric&appid=" + OWM_API_KEY; // Uncomment to update in metric
               + "&units=imperial&appid=" + OWM_API_KEY; // Comment out to update in metric.

  HTTPClient http;
  http.begin(url);
  http.setTimeout(5000);
  int code = http.GET();
  if (code == 200) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, http.getStream());
    weather.tempC        = doc["main"]["temp"].as<float>();
    weather.description  = doc["weather"][0]["description"].as<String>();
    weather.icon         = doc["weather"][0]["icon"].as<String>();
    weather.humidity     = doc["main"]["humidity"].as<int>();
    weather.rainMm       = doc["rain"]["1h"] | 0.0f;
    weather.skipWatering = (weather.rainMm >= 2.5f);
    weather.lastChecked  = time(nullptr);
    Serial.printf("[Weather] %.1fC  Rain:%.1fmm  Skip:%s\n",
                  weather.tempC, weather.rainMm, weather.skipWatering ? "YES" : "NO");
  } else {
    Serial.printf("[Weather] HTTP error %d\n", code);
    weather.description = "API error " + String(code);
  }
  http.end();
}

// ══════════════════════════════════════════════════════════════
//  ZONE & SCHEDULE UPDATE (every second)
// ══════════════════════════════════════════════════════════════
void updateZones() {
  for (int i = 0; i < NUM_ZONES; i++) {
    if (zones[i].timerRemaining > 0) {
      zones[i].timerRemaining--;
      if (zones[i].timerRemaining == 0) stopZone(i);
    }
  }
}

void updateSchedules() {
  struct tm ti;
  if (!getLocalTime(&ti)) return;
  int h = ti.tm_hour, m = ti.tm_min;
  int todayBit = 1 << ti.tm_wday;
  for (int i = 0; i < scheduleCount; i++) {
    Schedule& s = schedules[i];
    if (!s.enabled) continue;
    if (!(s.days & todayBit)) continue;
    if (s.hour == h && s.minute == m && !zones[s.zone].active) {
      if (weather.skipWatering) {
        Serial.printf("[Sched] Zone%d skipped (rain %.1fmm)\n", s.zone+1, weather.rainMm);
        continue;
      }
      Serial.printf("[Sched] Starting zone%d for %ds\n", s.zone+1, s.durationSec);
      startZone(s.zone, s.durationSec);
    }
  }
}

// ══════════════════════════════════════════════════════════════
//  HTTP HANDLERS
// ══════════════════════════════════════════════════════════════
void handleRoot() { server.send_P(200, "text/html", INDEX_HTML); }

void handleStatus() {
  DynamicJsonDocument doc(1024);
  doc["version"] = FIRMWARE_VER;
  doc["oled"]    = (int)oledType;

  JsonArray za = doc.createNestedArray("zones");
  for (int i = 0; i < NUM_ZONES; i++) {
    JsonObject z = za.createNestedObject();
    z["active"]         = zones[i].active;
    z["manualOn"]       = zones[i].manualOn;
    z["timerRemaining"] = zones[i].timerRemaining;
    z["name"]           = zones[i].name;
  }
  JsonObject w = doc.createNestedObject("weather");
  w["tempC"]        = weather.tempC;
  w["description"]  = weather.description;
  w["icon"]         = weather.icon;
  w["rainMm"]       = weather.rainMm;
  w["humidity"]     = weather.humidity;
  w["skipWatering"] = weather.skipWatering;
  w["lastChecked"]  = (long)weather.lastChecked;
  JsonArray sa = doc.createNestedArray("schedules");
  for (int i = 0; i < scheduleCount; i++) {
    JsonObject s = sa.createNestedObject();
    s["zone"]=schedules[i].zone; s["hour"]=schedules[i].hour;
    s["minute"]=schedules[i].minute; s["durationSec"]=schedules[i].durationSec;
    s["days"]=schedules[i].days; s["enabled"]=schedules[i].enabled;
  }
  String out; serializeJson(doc, out);
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", out);
}

void handleZonePost() {
  if (!server.hasArg("plain")) { server.send(400); return; }
  DynamicJsonDocument doc(256);
  if (deserializeJson(doc, server.arg("plain"))) { server.send(400,"text/plain","Bad JSON"); return; }
  int zone = doc["zone"].as<int>();
  String action = doc["action"].as<String>();
  if (action == "stopall") {
    for (int i = 0; i < NUM_ZONES; i++) stopZone(i);
  } else if (action == "sequence") {
    int min = doc["minutes"].as<int>(); if (min<1) min=5;
    startZone(0, min*60);
  } else if (zone >= 0 && zone < NUM_ZONES) {
    if      (action=="manual") startZone(zone, 0);
    else if (action=="stop")   stopZone(zone);
    else if (action=="timer")  { int min=doc["minutes"].as<int>(); if(min<1)min=1; startZone(zone,min*60); }
    else if (action=="rename") { zones[zone].name=doc["name"].as<String>(); saveZoneNames(); }
  } else { server.send(400,"text/plain","Bad zone"); return; }
  server.send(200,"application/json","{\"ok\":true}");
}

void handleSchedulePost() {
  if (scheduleCount >= MAX_SCHEDULES) { server.send(400,"text/plain","Max reached"); return; }
  if (!server.hasArg("plain")) { server.send(400); return; }
  DynamicJsonDocument doc(256);
  if (deserializeJson(doc, server.arg("plain"))) { server.send(400,"text/plain","Bad JSON"); return; }
  Schedule& s = schedules[scheduleCount];
  s.zone=doc["zone"].as<int>(); s.hour=doc["hour"].as<int>();
  s.minute=doc["minute"].as<int>(); s.durationSec=doc["durationSec"].as<int>();
  s.days=doc["days"].as<int>(); s.enabled=doc["enabled"].as<bool>();
  scheduleCount++;
  saveSchedules();
  server.send(200,"application/json","{\"ok\":true}");
}

void handleScheduleDelete(int idx) {
  if (idx<0||idx>=scheduleCount) { server.send(400,"text/plain","Bad index"); return; }
  for (int i=idx; i<scheduleCount-1; i++) schedules[i]=schedules[i+1];
  scheduleCount--;
  saveSchedules();
  server.send(200,"application/json","{\"ok\":true}");
}

void handleSettingsGet() {
  DynamicJsonDocument doc(512);
  doc["ssid"]=WIFI_SSID; doc["pass"]=WIFI_PASS;
  doc["api"]=OWM_API_KEY; doc["city"]=OWM_CITY; doc["country"]=OWM_COUNTRY;
  doc["oled"]=(int)oledType;
  String out; serializeJson(doc,out);
  server.send(200,"application/json",out);
}

void handleSettingsPost() {
  if (!server.hasArg("plain")) { server.send(400); return; }
  DynamicJsonDocument doc(512);
  if (deserializeJson(doc, server.arg("plain"))) { server.send(400,"text/plain","Bad JSON"); return; }
  if (doc.containsKey("ssid"))    WIFI_SSID   = doc["ssid"].as<String>();
  if (doc.containsKey("pass"))    WIFI_PASS   = doc["pass"].as<String>();
  if (doc.containsKey("api"))     OWM_API_KEY = doc["api"].as<String>();
  if (doc.containsKey("city"))    OWM_CITY    = doc["city"].as<String>();
  if (doc.containsKey("country")) OWM_COUNTRY = doc["country"].as<String>();
  if (doc.containsKey("oled"))    oledType    = (uint8_t)doc["oled"].as<int>();
  saveSettings();
  server.send(200,"text/plain","Saved");
  Serial.println("[Settings] Saved — rebooting");
  delay(300); ESP.restart();
}

void handleWeatherRefresh() {
  weatherRefreshPending = true;
  server.send(200,"application/json","{\"ok\":true}");
}

void handleOTAUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status==UPLOAD_FILE_START) {
    Serial.printf("[OTA] Start: %s\n", upload.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) Update.printError(Serial);
  } else if (upload.status==UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize)!=upload.currentSize)
      Update.printError(Serial);
  } else if (upload.status==UPLOAD_FILE_END) {
    if (Update.end(true)) Serial.printf("[OTA] OK: %u bytes\n", upload.totalSize);
    else Update.printError(Serial);
  }
}
void handleOTAFinish() {
  if (Update.hasError()) server.send(500,"text/plain","Failed");
  else { server.send(200,"text/plain","OK"); delay(300); ESP.restart(); }
}

void handleNotFound() {
  String url = server.uri();
  if (server.method()==HTTP_DELETE && url.startsWith("/api/schedule/")) {
    handleScheduleDelete(url.substring(url.lastIndexOf('/')+1).toInt());
    return;
  }
  server.send(404,"text/plain","Not found");
}

// ══════════════════════════════════════════════════════════════
//  SETUP
// ══════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  Serial.println("\n[Boot] HydraSprink v" FIRMWARE_VER);

  for (int i = 0; i < NUM_ZONES; i++) {
    pinMode(ZONE_PINS[i], OUTPUT);
    zones[i] = {false, false, 0, ""};
    setRelay(i, false);
  }

  loadSettings();
  loadZoneNames();
  loadSchedules();
  Serial.printf("[Storage] %d schedule(s) loaded\n", scheduleCount);

  // Init OLED before WiFi so splash shows during connect
  initOLED();

  setupWiFi();

  // Update OLED to show IP after WiFi connects
  if (oled) {
    oled->clearDisplay();
    oled->setTextSize(1);
    oled->setCursor(0,0);
    oled->print("WiFi OK");
    oled->setCursor(0, oledType==OLED_128x64 ? 12 : 12);
    oled->print(WiFi.status()==WL_CONNECTED
      ? WiFi.localIP().toString()
      : "AP: hydrasprink_AP");
    oled->display();
    delay(1500);
  }

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("[NTP] Sync requested");

  server.on("/",                    HTTP_GET,  handleRoot);
  server.on("/api/status",          HTTP_GET,  handleStatus);
  server.on("/api/zone",            HTTP_POST, handleZonePost);
  server.on("/api/schedule",        HTTP_POST, handleSchedulePost);
  server.on("/api/settings",        HTTP_GET,  handleSettingsGet);
  server.on("/api/settings",        HTTP_POST, handleSettingsPost);
  server.on("/api/weather/refresh", HTTP_POST, handleWeatherRefresh);
  server.on("/api/ota",             HTTP_POST, handleOTAFinish, handleOTAUpload);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("[Server] Ready");
}

// ══════════════════════════════════════════════════════════════
//  LOOP
// ══════════════════════════════════════════════════════════════
unsigned long lastSecond    = 0;
unsigned long bootMs        = millis();
bool          firstFetch    = false;

void loop() {
  server.handleClient();

  unsigned long now = millis();

  // 1Hz tasks
  if (now - lastSecond >= 1000) {
    lastSecond = now;
    updateZones();
    updateSchedules();
  }

  // First weather fetch after 5s
  if (!firstFetch && (now - bootMs) > 5000UL) {
    firstFetch = true;
    lastWeatherCheck = now;
    fetchWeather();
  }

  // Queued weather refresh from UI
  if (firstFetch && weatherRefreshPending) {
    weatherRefreshPending = false;
    fetchWeather();
  }

  // Auto weather refresh every 30 min
  if (firstFetch && (now - lastWeatherCheck) > 1800000UL) {
    lastWeatherCheck = now;
    fetchWeather();
  }

  // OLED cycling: advance screen index and redraw
  if (oled && (now - lastScreenMs) > SCREEN_INTERVAL_MS) {
    lastScreenMs = now;
    idleScreen++;
    updateOLED();
  }
}