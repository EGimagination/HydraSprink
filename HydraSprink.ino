/****************************************************
   HYDRASPRINK v1.5.0
   4-Channel Sprinkler Controller with OLED display

   Sketch folder must contain:
     HydraSprink.ino   (this file)
     html.h            (web UI)

   Libraries (install via Library Manager):
     ArduinoJson          - Benoit Blanchon (v6 or v7)
     Adafruit SSD1306     - Adafruit
     Adafruit GFX Library - Adafruit
****************************************************/

#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <time.h>
#include <Update.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "html.h"

#define FIRMWARE_VER  "1.5.0"
#define NUM_ZONES     4
#define MAX_SCHEDULES 16

// ─────────────────────────────────────────────────
//  RELAY PINS — edit to match your wiring
// ─────────────────────────────────────────────────
int  ZONE_PINS[NUM_ZONES] = {16, 17, 18, 19};
bool ACTIVE_LOW = true;   // true for most relay boards (LOW = ON)

// ─────────────────────────────────────────────────
//  OLED — I2C defaults: SDA=GPIO21, SCL=GPIO22
//  oledType: 0=None  1=128x32  2=128x64
//  Saved to flash; selectable from the Settings UI.
//  Change OLED_ADDR to 0x3D if display stays blank.
// ─────────────────────────────────────────────────
#define OLED_SDA  21
#define OLED_SCL  22
#define OLED_ADDR 0x3C

Adafruit_SSD1306* oled         = nullptr;
uint8_t           oledType     = 0;
uint8_t           idleScreen   = 0;
unsigned long     lastScreenMs = 0;
#define SCREEN_MS 4000   // rotate idle screens every 4 seconds

// ─────────────────────────────────────────────────
//  GLOBALS
// ─────────────────────────────────────────────────
WebServer    server(80);
Preferences  prefs;

String WIFI_SSID    = "";
String WIFI_PASS    = "";
String OWM_API_KEY  = "";
String OWM_CITY     = "Hollister";
String OWM_COUNTRY  = "US";
bool   USE_IMPERIAL = false;

// ─────────────────────────────────────────────────
//  ZONE STATE
// ─────────────────────────────────────────────────
struct Zone {
  bool   active         = false;
  bool   manualOn       = false;
  int    timerRemaining = 0;
  String name;
};
Zone zones[NUM_ZONES];

// ─────────────────────────────────────────────────
//  SCHEDULES
// ─────────────────────────────────────────────────
struct Schedule {
  int  zone        = 0;
  int  hour        = 6;
  int  minute      = 0;
  int  durationSec = 600;
  int  days        = 0x7F;
  bool enabled     = true;
};
Schedule schedules[MAX_SCHEDULES];
int      scheduleCount = 0;

// ─────────────────────────────────────────────────
//  WEATHER (always stored in metric)
// ─────────────────────────────────────────────────
struct {
  float  tempC       = 0;
  float  humidity    = 0;
  float  rainMm      = 0;
  String description = "No data";
} weather;

unsigned long lastWeatherMs  = 0;
bool          firstFetchDone = false;
unsigned long bootMs         = 0;
const unsigned long WEATHER_INTERVAL = 600000UL;
unsigned long lastSecondMs = 0;

// ═════════════════════════════════════════════════
//  RELAY HELPERS
// ═════════════════════════════════════════════════
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

// ═════════════════════════════════════════════════
//  SETTINGS PERSISTENCE
// ═════════════════════════════════════════════════
void loadSettings() {
  prefs.begin("hydra", true);
  WIFI_SSID    = prefs.getString("ssid",    "");
  WIFI_PASS    = prefs.getString("pass",    "");
  OWM_API_KEY  = prefs.getString("api",     "");
  OWM_CITY     = prefs.getString("city",    "Hollister");
  OWM_COUNTRY  = prefs.getString("country", "US");
  USE_IMPERIAL = prefs.getBool("imperial",  false);
  oledType     = (uint8_t)prefs.getInt("oled", 0);
  prefs.end();
}
void saveSettings() {
  prefs.begin("hydra", false);
  prefs.putString("ssid",    WIFI_SSID);
  prefs.putString("pass",    WIFI_PASS);
  prefs.putString("api",     OWM_API_KEY);
  prefs.putString("city",    OWM_CITY);
  prefs.putString("country", OWM_COUNTRY);
  prefs.putBool("imperial",  USE_IMPERIAL);
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

// ═════════════════════════════════════════════════
//  OLED INIT
// ═════════════════════════════════════════════════
void initOLED() {
  if (oledType == 0) {
    if (oled) { delete oled; oled = nullptr; }
    return;
  }
  Wire.begin(OLED_SDA, OLED_SCL);
  int h = (oledType == 2) ? 64 : 32;
  if (oled) delete oled;
  oled = new Adafruit_SSD1306(128, h, &Wire, -1);
  if (!oled->begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("[OLED] Init failed — check wiring/address");
    delete oled; oled = nullptr; return;
  }
  oled->clearDisplay();
  oled->setTextColor(SSD1306_WHITE);
  oled->setTextSize(1);
  oled->setCursor(0, 0);
  oled->println("  HydraSprink");
  if (oledType == 2) {
    oled->println(); oled->println("  v" FIRMWARE_VER);
    oled->println(); oled->println("  Starting...");
  } else {
    oled->println("  v" FIRMWARE_VER " Starting");
  }
  oled->display();
  Serial.printf("[OLED] Init OK (%dx%d at 0x%02X)\n", 128, h, OLED_ADDR);
}

// ═════════════════════════════════════════════════
//  OLED DRAW HELPERS
// ═════════════════════════════════════════════════
String fmtSec(int sec) {
  if (sec <= 0) return "--";
  if (sec >= 60) return String(sec/60) + "m " + String(sec%60) + "s";
  return String(sec) + "s";
}
int getActiveZones(int* out) {
  int n = 0;
  for (int i = 0; i < NUM_ZONES; i++) if (zones[i].active) out[n++] = i;
  return n;
}

// ── 128x32 ─────────────────────────────────────
void draw32_zones() {
  int active[NUM_ZONES]; int n = getActiveZones(active);
  oled->setTextSize(1);
  oled->setCursor(0, 0);
  for (int i = 0; i < NUM_ZONES; i++) {
    oled->print("Z"); oled->print(i+1);
    oled->print(zones[i].active ? "+" : "-"); oled->print(" ");
  }
  oled->setCursor(0, 12);
  if (n == 0) {
    oled->print("All zones idle");
  } else {
    int z = active[0];
    String nm = zones[z].name; if (nm.length() > 7) nm = nm.substring(0,7);
    oled->print(nm); oled->print(": ");
    oled->print(zones[z].manualOn ? "Manual" : fmtSec(zones[z].timerRemaining));
    if (n > 1) { oled->print(" +"); oled->print(n-1); }
  }
  oled->setCursor(0, 24);
  oled->print(weather.rainMm >= 2.5f ? "! Rain skip ON" : "  Rain skip off");
}
void draw32_weather() {
  oled->setTextSize(1); oled->setCursor(0, 0);
  if (USE_IMPERIAL) { oled->print(weather.tempC*9/5+32, 1); oled->print("F  "); }
  else              { oled->print(weather.tempC, 1);        oled->print("C  "); }
  oled->print((int)weather.humidity); oled->print("% RH");
  oled->setCursor(0, 12);
  String d = weather.description; if (d.length() > 21) d = d.substring(0,21); oled->print(d);
  oled->setCursor(0, 24); oled->print("Rain: ");
  if (USE_IMPERIAL) { oled->print(weather.rainMm/25.4, 2); oled->print("in"); }
  else              { oled->print(weather.rainMm, 1);       oled->print("mm"); }
}
void draw32_network() {
  oled->setTextSize(1);
  oled->setCursor(0, 0); oled->print("HydraSprink");
  oled->setCursor(0, 12);
  oled->print(WiFi.status()==WL_CONNECTED ? WiFi.localIP().toString() : "AP: HydraSprink_AP");
  oled->setCursor(0, 24);
  struct tm ti;
  if (getLocalTime(&ti)) { char buf[16]; strftime(buf,sizeof(buf),"%a %H:%M:%S",&ti); oled->print(buf); }
  else oled->print("Time: syncing...");
}

// ── 128x64 ─────────────────────────────────────
void draw64_zones() {
  int active[NUM_ZONES]; getActiveZones(active);
  oled->setTextSize(1); oled->setCursor(0, 0); oled->print("-- ZONE STATUS --");
  for (int i = 0; i < NUM_ZONES; i++) {
    oled->setCursor(0, 10 + i*12);
    oled->print(zones[i].active ? "[*] " : "[ ] ");
    String nm = zones[i].name; if (nm.length() > 8) nm = nm.substring(0,8); oled->print(nm);
    String st = zones[i].active
      ? (zones[i].manualOn ? "ON" : fmtSec(zones[i].timerRemaining)) : "OFF";
    oled->setCursor(128 - (int)st.length()*6, 10 + i*12); oled->print(st);
  }
  oled->setCursor(0, 56);
  oled->print(weather.rainMm >= 2.5f ? "! Rain skip active" : "Rain skip: off");
}
void draw64_weather() {
  oled->setTextSize(1); oled->setCursor(0, 0); oled->print("-- WEATHER --");
  oled->setTextSize(2); oled->setCursor(0, 12);
  if (USE_IMPERIAL) { oled->print(weather.tempC*9/5+32, 1); oled->print("F"); }
  else              { oled->print(weather.tempC, 1);        oled->print("C"); }
  oled->setTextSize(1);
  oled->setCursor(0, 32);
  String d = weather.description; if (d.length() > 21) d = d.substring(0,21); oled->print(d);
  oled->setCursor(0, 44); oled->print("Humidity: "); oled->print((int)weather.humidity); oled->print("%");
  oled->setCursor(0, 56); oled->print("Rain: ");
  if (USE_IMPERIAL) { oled->print(weather.rainMm/25.4, 2); oled->print("in"); }
  else              { oled->print(weather.rainMm, 1);       oled->print("mm"); }
}
void draw64_time() {
  struct tm ti;
  oled->setTextSize(1); oled->setCursor(0, 0); oled->print("-- TIME & NET --");
  if (getLocalTime(&ti)) {
    char hhmm[6]; strftime(hhmm, sizeof(hhmm), "%H:%M", &ti);
    oled->setTextSize(2); oled->setCursor(20, 14); oled->print(hhmm);
    char ss[4]; strftime(ss, sizeof(ss), ":%S", &ti);
    oled->setTextSize(1); oled->setCursor(96, 22); oled->print(ss);
    char date[20]; strftime(date, sizeof(date), "%a %d %b %Y", &ti);
    oled->setCursor(0, 40); oled->print(date);
  } else {
    oled->setTextSize(1); oled->setCursor(0, 20); oled->print("Syncing NTP...");
  }
  oled->setTextSize(1); oled->setCursor(0, 56);
  oled->print(WiFi.status()==WL_CONNECTED ? WiFi.localIP().toString() : "AP: HydraSprink_AP");
}
void draw64_schedules() {
  oled->setTextSize(1); oled->setCursor(0, 0); oled->print("-- SCHEDULES --");
  if (scheduleCount == 0) { oled->setCursor(0, 24); oled->print("No schedules set"); return; }
  int shown = min(scheduleCount, 4);
  for (int i = 0; i < shown; i++) {
    Schedule& s = schedules[i]; oled->setCursor(0, 12 + i*12);
    char buf[22];
    snprintf(buf, sizeof(buf), "Z%d %02d:%02d %dm %s",
             s.zone+1, s.hour, s.minute, s.durationSec/60, s.enabled ? "ON" : "OFF");
    oled->print(buf);
  }
  if (scheduleCount > 4) {
    oled->setCursor(0, 56); oled->print("+"); oled->print(scheduleCount-4); oled->print(" more");
  }
}

// ── Main update — called every SCREEN_MS ────────
void updateOLED() {
  if (!oled) return;
  int active[NUM_ZONES];
  bool anyActive = getActiveZones(active) > 0;
  oled->clearDisplay();
  if (anyActive) {
    if (oledType == 1) draw32_zones();
    else               draw64_zones();
  } else {
    if (oledType == 1) {
      switch (idleScreen % 3) {
        case 0: draw32_zones();   break;
        case 1: draw32_weather(); break;
        case 2: draw32_network(); break;
      }
    } else {
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

// ═════════════════════════════════════════════════
//  WIFI
// ═════════════════════════════════════════════════
void connectWiFi() {
  if (WIFI_SSID == "") {
    Serial.println("[WiFi] No SSID — AP: HydraSprink_AP");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("HydraSprink_AP");
    Serial.println("[AP] " + WiFi.softAPIP().toString());
    return;
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
  Serial.print("[WiFi] Connecting");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500); Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] " + WiFi.localIP().toString());
  } else {
    Serial.println("\n[WiFi] Failed — AP: HydraSprink_AP");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("HydraSprink_AP");
    Serial.println("[AP] " + WiFi.softAPIP().toString());
  }
}

// ═════════════════════════════════════════════════
//  WEATHER
// ═════════════════════════════════════════════════
void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED || OWM_API_KEY == "") return;
  String url = "http://api.openweathermap.org/data/2.5/weather?q="
               + OWM_CITY + "," + OWM_COUNTRY
               + "&appid=" + OWM_API_KEY + "&units=metric";
  HTTPClient http;
  http.begin(url); http.setTimeout(5000);
  int code = http.GET();
  if (code == 200) {
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, http.getStream());
    weather.tempC       = doc["main"]["temp"]    | 0.0f;
    weather.humidity    = doc["main"]["humidity"] | 0.0f;
    weather.rainMm      = doc["rain"]["1h"]        | 0.0f;
    weather.description = doc["weather"][0]["description"].as<String>();
    Serial.printf("[Weather] %.1fC  Hum:%.0f%%  Rain:%.1fmm\n",
                  weather.tempC, weather.humidity, weather.rainMm);
  } else {
    Serial.printf("[Weather] HTTP error %d\n", code);
    weather.description = "Error " + String(code);
  }
  http.end();
}

// ═════════════════════════════════════════════════
//  1-SECOND UPDATES
// ═════════════════════════════════════════════════
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
  int h = ti.tm_hour, m = ti.tm_min, todayBit = 1 << ti.tm_wday;
  for (int i = 0; i < scheduleCount; i++) {
    Schedule& s = schedules[i];
    if (!s.enabled || !(s.days & todayBit)) continue;
    if (s.hour == h && s.minute == m && !zones[s.zone].active) {
      Serial.printf("[Sched] Zone %d -> %ds\n", s.zone+1, s.durationSec);
      startZone(s.zone, s.durationSec);
    }
  }
}

// ═════════════════════════════════════════════════
//  HTTP HANDLERS
// ═════════════════════════════════════════════════
void handleRoot() { server.send_P(200, "text/html", INDEX_HTML); }

void handleStatus() {
  DynamicJsonDocument doc(2048);
  doc["version"]  = FIRMWARE_VER;
  doc["imperial"] = USE_IMPERIAL;
  doc["oled"]     = (int)oledType;
  doc["ip"]       = WiFi.isConnected() ? WiFi.localIP().toString() : WiFi.softAPIP().toString();

  JsonArray za = doc.createNestedArray("zones");
  for (int i = 0; i < NUM_ZONES; i++) {
    JsonObject z = za.createNestedObject();
    z["active"]         = zones[i].active;
    z["manualOn"]       = zones[i].manualOn;
    z["timerRemaining"] = zones[i].timerRemaining;
    z["name"]           = zones[i].name;
  }
  JsonObject w = doc.createNestedObject("weather");
  w["tempC"] = weather.tempC; w["humidity"] = weather.humidity;
  w["rainMm"] = weather.rainMm; w["description"] = weather.description;

  JsonArray sa = doc.createNestedArray("schedules");
  for (int i = 0; i < scheduleCount; i++) {
    JsonObject s = sa.createNestedObject();
    s["zone"]=schedules[i].zone; s["hour"]=schedules[i].hour;
    s["minute"]=schedules[i].minute; s["durationSec"]=schedules[i].durationSec;
    s["days"]=schedules[i].days; s["enabled"]=schedules[i].enabled;
  }
  String out; serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleSettingsGet() {
  DynamicJsonDocument doc(512);
  doc["ssid"]=WIFI_SSID; doc["api"]=OWM_API_KEY;
  doc["city"]=OWM_CITY; doc["country"]=OWM_COUNTRY;
  doc["imperial"]=USE_IMPERIAL; doc["oled"]=(int)oledType;
  String out; serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleSettingsPost() {
  if (!server.hasArg("plain")) { server.send(400, "text/plain", "No body"); return; }
  DynamicJsonDocument doc(512);
  if (deserializeJson(doc, server.arg("plain"))) { server.send(400,"text/plain","Bad JSON"); return; }

  bool needRestart = false;
  if (doc.containsKey("ssid") && doc["ssid"].as<String>() != WIFI_SSID)
    { WIFI_SSID=doc["ssid"].as<String>(); needRestart=true; }
  if (doc.containsKey("pass") && doc["pass"].as<String>() != "")
    { WIFI_PASS=doc["pass"].as<String>(); needRestart=true; }
  if (doc.containsKey("api") && doc["api"].as<String>() != OWM_API_KEY)
    { OWM_API_KEY=doc["api"].as<String>(); needRestart=true; }
  if (doc.containsKey("city") && doc["city"].as<String>() != OWM_CITY)
    { OWM_CITY=doc["city"].as<String>(); needRestart=true; }
  if (doc.containsKey("country") && doc["country"].as<String>() != OWM_COUNTRY)
    { OWM_COUNTRY=doc["country"].as<String>(); needRestart=true; }
  if (doc.containsKey("imperial"))
    USE_IMPERIAL = doc["imperial"].as<bool>();
  if (doc.containsKey("oled")) {
    uint8_t nv = (uint8_t)doc["oled"].as<int>();
    if (nv != oledType) { oledType=nv; needRestart=true; }
  }
  saveSettings();
  server.send(200, "text/plain", needRestart ? "restart" : "ok");
  if (needRestart) { Serial.println("[Settings] Rebooting..."); delay(300); ESP.restart(); }
}

void handleZonePost() {
  if (!server.hasArg("plain")) { server.send(400); return; }
  DynamicJsonDocument doc(256);
  if (deserializeJson(doc, server.arg("plain"))) { server.send(400,"text/plain","Bad JSON"); return; }
  int    zone   = doc["zone"]   | -1;
  String action = doc["action"] | "";
  if (action == "stopall") {
    for (int i=0; i<NUM_ZONES; i++) stopZone(i);
    server.send(200,"application/json","{\"ok\":true}"); return;
  }
  if (zone < 0 || zone >= NUM_ZONES) { server.send(400,"text/plain","Bad zone"); return; }
  if      (action=="manual") { startZone(zone, 0); }
  else if (action=="stop")   { stopZone(zone); }
  else if (action=="timer")  { startZone(zone, (doc["minutes"]|5)*60); }
  else if (action=="rename") {
    String fb = "Zone "+String(zone+1);
    zones[zone].name = doc["name"] | fb.c_str();
    saveZoneNames();
  }
  server.send(200,"application/json","{\"ok\":true}");
}

void handleSchedulePost() {
  if (scheduleCount >= MAX_SCHEDULES) { server.send(400,"text/plain","Max reached"); return; }
  if (!server.hasArg("plain")) { server.send(400); return; }
  DynamicJsonDocument doc(256);
  if (deserializeJson(doc, server.arg("plain"))) { server.send(400,"text/plain","Bad JSON"); return; }
  Schedule& s = schedules[scheduleCount];
  s.zone=doc["zone"]|0; s.hour=doc["hour"]|6; s.minute=doc["minute"]|0;
  s.durationSec=doc["durationSec"]|600; s.days=doc["days"]|0x7F; s.enabled=doc["enabled"]|true;
  scheduleCount++;
  saveSchedules();
  server.send(200,"application/json","{\"ok\":true}");
}

void doScheduleDelete(int idx) {
  if (idx<0||idx>=scheduleCount) { server.send(400,"text/plain","Bad index"); return; }
  for (int i=idx; i<scheduleCount-1; i++) schedules[i]=schedules[i+1];
  scheduleCount--;
  saveSchedules();
  server.send(200,"application/json","{\"ok\":true}");
}

void handleOTAUpload() {
  HTTPUpload& u = server.upload();
  if      (u.status==UPLOAD_FILE_START)  { if(!Update.begin(UPDATE_SIZE_UNKNOWN)) Update.printError(Serial); }
  else if (u.status==UPLOAD_FILE_WRITE)  { if(Update.write(u.buf,u.currentSize)!=u.currentSize) Update.printError(Serial); }
  else if (u.status==UPLOAD_FILE_END)    { if(Update.end(true)) Serial.printf("[OTA] Done: %u bytes\n",u.totalSize); else Update.printError(Serial); }
}
void handleOTAFinish() {
  if (Update.hasError()) server.send(500,"text/plain","Failed");
  else { server.send(200,"text/plain","OK"); delay(300); ESP.restart(); }
}

void handleNotFound() {
  String uri = server.uri();
  if (server.method()==HTTP_DELETE && uri.startsWith("/api/schedule/")) {
    doScheduleDelete(uri.substring(uri.lastIndexOf('/')+1).toInt()); return;
  }
  server.send(404,"text/plain","Not found");
}

// ═════════════════════════════════════════════════
//  SETUP
// ═════════════════════════════════════════════════
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
  Serial.printf("[Storage] %d schedule(s)\n", scheduleCount);

  initOLED();   // before WiFi so splash shows during connect

  connectWiFi();

  // Show IP on OLED after WiFi connects
  if (oled) {
    oled->clearDisplay();
    oled->setTextSize(1);
    oled->setCursor(0, 0); oled->print("WiFi OK");
    oled->setCursor(0, 12);
    oled->print(WiFi.status()==WL_CONNECTED ? WiFi.localIP().toString() : "AP: HydraSprink_AP");
    oled->display();
    delay(1500);
  }

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  server.on("/",             HTTP_GET,  handleRoot);
  server.on("/api/status",   HTTP_GET,  handleStatus);
  server.on("/api/settings", HTTP_GET,  handleSettingsGet);
  server.on("/api/settings", HTTP_POST, handleSettingsPost);
  server.on("/api/zone",     HTTP_POST, handleZonePost);
  server.on("/api/schedule", HTTP_POST, handleSchedulePost);
  server.on("/api/ota",      HTTP_POST, handleOTAFinish, handleOTAUpload);
  server.onNotFound(handleNotFound);

  server.begin();
  bootMs = millis();
  Serial.println("[Server] Ready");
}

// ═════════════════════════════════════════════════
//  LOOP
// ═════════════════════════════════════════════════
void loop() {
  server.handleClient();
  unsigned long now = millis();

  if (now - lastSecondMs >= 1000) {
    lastSecondMs = now;
    updateZones();
    updateSchedules();
  }

  if (!firstFetchDone && (now - bootMs) > 3000UL) {
    firstFetchDone = true; lastWeatherMs = now;
    fetchWeather();
  }

  if (firstFetchDone && (now - lastWeatherMs) > WEATHER_INTERVAL) {
    lastWeatherMs = now;
    fetchWeather();
  }

  // OLED cycle
  if (oled && (now - lastScreenMs) > SCREEN_MS) {
    lastScreenMs = now;
    idleScreen++;
    updateOLED();
  }
}