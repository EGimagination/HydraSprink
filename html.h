#pragma once
#include <pgmspace.h>

const char INDEX_HTML[] PROGMEM = R"HTMLRAW(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>HydraSprink</title>
<link href="https://fonts.googleapis.com/css2?family=Space+Mono:wght@400;700&family=DM+Sans:wght@300;400;500;700&display=swap" rel="stylesheet">
<style>
:root{--bg:#0a0f1a;--surface:#111827;--surface2:#1a2436;--border:#1e3050;--accent:#00d4aa;--accent2:#0ea5e9;--danger:#f43f5e;--warn:#f59e0b;--text:#e2e8f0;--muted:#64748b;--mono:'Space Mono',monospace;--body:'DM Sans',sans-serif;}
*{box-sizing:border-box;margin:0;padding:0}
body{background:var(--bg);color:var(--text);font-family:var(--body);min-height:100vh;padding-bottom:60px}
body::before{content:'';position:fixed;inset:0;background-image:linear-gradient(rgba(0,212,170,.03)1px,transparent 1px),linear-gradient(90deg,rgba(0,212,170,.03)1px,transparent 1px);background-size:40px 40px;pointer-events:none;z-index:0}
header{position:sticky;top:0;z-index:100;background:rgba(10,15,26,.96);backdrop-filter:blur(12px);border-bottom:1px solid var(--border);padding:0 24px;display:flex;align-items:center;justify-content:space-between;height:64px}
.logo{font-family:var(--mono);font-size:1.1rem;color:var(--accent);letter-spacing:.05em}
.htime{font-family:var(--mono);font-size:.85rem;color:var(--accent)}
.hdate{font-family:var(--mono);font-size:.7rem;color:var(--muted)}
.tabs{display:flex;padding:0 24px;background:var(--surface);border-bottom:1px solid var(--border);overflow-x:auto}
.tab{font-family:var(--mono);font-size:.72rem;letter-spacing:.08em;text-transform:uppercase;padding:14px 20px;cursor:pointer;color:var(--muted);border-bottom:2px solid transparent;white-space:nowrap;transition:all .2s;background:none;border-top:none;border-left:none;border-right:none}
.tab:hover{color:var(--text)}
.tab.active{color:var(--accent);border-bottom-color:var(--accent)}
.panel{display:none;padding:28px 24px;max-width:960px;margin:0 auto}
.panel.active{display:block}
.stitle{font-family:var(--mono);font-size:.65rem;letter-spacing:.15em;text-transform:uppercase;color:var(--muted);margin-bottom:16px;display:flex;align-items:center;gap:10px}
.stitle::after{content:'';flex:1;height:1px;background:var(--border)}
.card{background:var(--surface);border:1px solid var(--border);border-radius:12px;padding:20px;margin-bottom:16px}
.cgrid{display:grid;grid-template-columns:repeat(auto-fill,minmax(210px,1fr));gap:16px;margin-bottom:28px}
.zcard{background:var(--surface);border:1px solid var(--border);border-radius:14px;padding:20px;display:flex;flex-direction:column;gap:14px;transition:border-color .3s,box-shadow .3s;position:relative;overflow:hidden}
.zcard.on{border-color:var(--accent);box-shadow:0 0 24px rgba(0,212,170,.12)}
.zcard.on::before{content:'';position:absolute;top:0;left:0;right:0;height:3px;background:linear-gradient(90deg,var(--accent),var(--accent2))}
.zhdr{display:flex;align-items:center;justify-content:space-between}
.zname-input{background:none;border:none;border-bottom:1px solid transparent;color:var(--text);font-family:var(--body);font-weight:700;font-size:.95rem;width:100%;outline:none;cursor:text;transition:border-color .2s}
.zname-input:focus{border-bottom-color:var(--accent)}
.znum{font-family:var(--mono);font-size:.65rem;color:var(--muted);background:var(--surface2);border-radius:4px;padding:2px 6px;margin-left:8px;flex-shrink:0}
.dot{width:8px;height:8px;border-radius:50%;background:var(--muted);transition:background .3s;flex-shrink:0}
.dot.on{background:var(--accent);box-shadow:0 0 8px var(--accent)}
.zctrl{display:flex;gap:8px;flex-wrap:wrap}
.btn{padding:8px 14px;border-radius:7px;border:1px solid var(--border);background:var(--surface2);color:var(--text);font-family:var(--mono);font-size:.7rem;letter-spacing:.05em;cursor:pointer;transition:all .15s;white-space:nowrap}
.btn:hover{border-color:var(--accent);color:var(--accent)}
.btn.danger{border-color:var(--danger);color:var(--danger)}
.btn.danger:hover{background:rgba(244,63,94,.1)}
.btn.primary{background:var(--accent);color:#000;border-color:var(--accent);font-weight:700}
.btn.primary:hover{filter:brightness(1.1)}
.trow{display:flex;gap:6px;align-items:center}
.trow input[type=number]{width:62px;background:var(--surface2);border:1px solid var(--border);color:var(--text);border-radius:6px;padding:6px 8px;font-family:var(--mono);font-size:.75rem;outline:none;text-align:center}
.trow input[type=number]:focus{border-color:var(--accent)}
.tlbl{font-family:var(--mono);font-size:.65rem;color:var(--muted)}
.cdown{font-family:var(--mono);font-size:.75rem;color:var(--accent);min-height:16px}
.wcrd{background:var(--surface);border:1px solid var(--border);border-radius:14px;padding:24px;display:flex;align-items:center;gap:24px;margin-bottom:28px;flex-wrap:wrap}
.wtemp{font-family:var(--mono);font-size:2rem;font-weight:700;color:var(--accent2)}
.wdesc{color:var(--muted);font-size:.9rem;text-transform:capitalize;margin-top:4px}
.wstats{display:flex;gap:20px;margin-top:8px;flex-wrap:wrap}
.wstat{font-family:var(--mono);font-size:.7rem;color:var(--muted)}
.wstat span{color:var(--text)}
.skip-badge{background:rgba(245,158,11,.15);border:1px solid var(--warn);color:var(--warn);font-family:var(--mono);font-size:.68rem;padding:6px 12px;border-radius:20px}
.ok-badge{background:rgba(0,212,170,.1);border:1px solid var(--accent);color:var(--accent);font-family:var(--mono);font-size:.68rem;padding:6px 12px;border-radius:20px}
.stbl{width:100%;border-collapse:collapse;font-size:.82rem}
.stbl th{font-family:var(--mono);font-size:.6rem;letter-spacing:.1em;text-transform:uppercase;color:var(--muted);padding:8px 12px;text-align:left;border-bottom:1px solid var(--border)}
.stbl td{padding:10px 12px;border-bottom:1px solid rgba(30,48,80,.5);vertical-align:middle}
.stbl tr:hover td{background:rgba(0,212,170,.03)}
.dchips{display:flex;gap:4px;flex-wrap:wrap}
.dchip{font-family:var(--mono);font-size:.6rem;padding:2px 6px;border-radius:4px;background:var(--surface2);border:1px solid var(--border);color:var(--muted)}
.dchip.on{background:rgba(0,212,170,.15);border-color:var(--accent);color:var(--accent)}
.fgrid{display:grid;grid-template-columns:repeat(auto-fill,minmax(140px,1fr));gap:12px;margin-bottom:16px}
.ff{display:flex;flex-direction:column;gap:6px;margin-bottom:12px}
.ff label{font-family:var(--mono);font-size:.6rem;letter-spacing:.1em;text-transform:uppercase;color:var(--muted)}
.ff input,.ff select{background:var(--surface2);border:1px solid var(--border);color:var(--text);border-radius:7px;padding:8px 10px;font-family:var(--mono);font-size:.75rem;outline:none;width:100%}
.ff input:focus,.ff select:focus{border-color:var(--accent)}
.dsel{display:flex;gap:6px;flex-wrap:wrap}
.dtog{width:34px;height:34px;border-radius:7px;border:1px solid var(--border);background:var(--surface2);color:var(--muted);font-family:var(--mono);font-size:.65rem;cursor:pointer;transition:all .15s;display:inline-flex;align-items:center;justify-content:center}
.dtog.sel{background:rgba(0,212,170,.2);border-color:var(--accent);color:var(--accent)}
.otazone{border:2px dashed var(--border);border-radius:14px;padding:40px;text-align:center;transition:border-color .2s;cursor:pointer}
.otazone:hover{border-color:var(--accent)}
.otazone input[type=file]{display:none}
.pbar{width:100%;height:6px;background:var(--surface2);border-radius:3px;margin-top:20px;overflow:hidden;display:none}
.pfill{height:100%;background:linear-gradient(90deg,var(--accent),var(--accent2));width:0;transition:width .3s}
.otastat{font-family:var(--mono);font-size:.75rem;color:var(--accent);margin-top:12px;min-height:20px}
/* OLED preview card */
.oled-preview{background:#000;border:2px solid var(--border);border-radius:8px;padding:12px;font-family:var(--mono);font-size:.7rem;color:#fff;line-height:1.6;transition:border-color .3s;min-height:60px}
.oled-preview.active{border-color:var(--accent);box-shadow:0 0 16px rgba(0,212,170,.15)}
.oled-screens{display:grid;gap:12px;margin-top:12px}
.oled-screen-label{font-family:var(--mono);font-size:.6rem;letter-spacing:.1em;text-transform:uppercase;color:var(--muted);margin-bottom:6px}
.screen-badge{display:inline-block;font-family:var(--mono);font-size:.6rem;padding:2px 8px;border-radius:4px;background:var(--surface2);border:1px solid var(--border);color:var(--muted);margin-right:6px;margin-bottom:4px}
.screen-badge.on{background:rgba(0,212,170,.1);border-color:var(--accent);color:var(--accent)}
.toast{position:fixed;bottom:24px;right:24px;background:var(--surface2);border:1px solid var(--accent);color:var(--text);padding:12px 20px;border-radius:10px;font-family:var(--mono);font-size:.75rem;z-index:999;transform:translateY(80px);opacity:0;transition:all .3s;pointer-events:none}
.toast.show{transform:translateY(0);opacity:1}
.toast.err{border-color:var(--danger)}
@media(max-width:600px){.panel{padding:20px 14px}.cgrid{grid-template-columns:1fr 1fr}}
</style>
</head>
<body>

<header>
  <div class="logo">HydraSprink</div>
  <div style="text-align:right">
    <div class="htime" id="htime">--:--:--</div>
    <div class="hdate" id="hdate"></div>
  </div>
</header>

<div class="tabs">
  <button class="tab active" onclick="showTab('zones')"     id="tab-zones">Zones</button>
  <button class="tab"        onclick="showTab('weather')"   id="tab-weather">Weather</button>
  <button class="tab"        onclick="showTab('schedules')" id="tab-schedules">Schedules</button>
  <button class="tab"        onclick="showTab('settings')"  id="tab-settings">Settings</button>
  <button class="tab"        onclick="showTab('ota')"       id="tab-ota">Firmware</button>
</div>

<!-- ═══════ ZONES ═══════ -->
<div class="panel active" id="panel-zones">
  <p class="stitle">Zone Control</p>
  <div class="cgrid" id="zoneCards">
    <div class="zcard"><div style="color:var(--muted);font-family:var(--mono);font-size:.75rem">Connecting...</div></div>
  </div>
  <p class="stitle">Quick Actions</p>
  <div class="card" style="display:flex;gap:12px;flex-wrap:wrap;align-items:center">
    <button class="btn danger" onclick="stopAll()">Stop All Zones</button>
    <span style="color:var(--border)">|</span>
    <span style="font-size:.8rem;color:var(--muted)">Run sequence:</span>
    <input type="number" id="seqDur" value="5" min="1" max="120"
      style="width:62px;background:var(--surface2);border:1px solid var(--border);color:var(--text);border-radius:6px;padding:6px 8px;font-family:var(--mono);font-size:.75rem;text-align:center;outline:none">
    <span class="tlbl">min each</span>
    <button class="btn primary" onclick="runSequence()">Run Sequence</button>
  </div>
</div>

<!-- ═══════ WEATHER ═══════ -->
<div class="panel" id="panel-weather">
  <p class="stitle">Current Conditions</p>
  <div class="wcrd">
    <div style="font-size:2.5rem" id="wIco">--</div>
    <div style="flex:1">
      <div class="wtemp" id="wTemp">--</div>
      <div class="wdesc" id="wDesc">Loading...</div>
      <div class="wstats">
        <div class="wstat">Rain 1h <span id="wRain">--</span></div>
        <div class="wstat">Humidity <span id="wHumid">--</span></div>
        <div class="wstat">Updated <span id="wUpd">--</span></div>
      </div>
    </div>
    <div id="wBadge"></div>
  </div>
  <div class="card" style="display:flex;gap:16px;align-items:center;flex-wrap:wrap">
    <p style="font-size:.88rem;flex:1">Scheduled watering skips when recent rainfall exceeds 2.5mm. Manual control is never affected.</p>
    <button class="btn" onclick="refreshWeather()">Refresh Now</button>
  </div>
</div>

<!-- ═══════ SCHEDULES ═══════ -->
<div class="panel" id="panel-schedules">
  <p class="stitle">Active Schedules</p>
  <div class="card">
    <table class="stbl">
      <thead><tr><th>Zone</th><th>Time</th><th>Duration</th><th>Days</th><th>Status</th><th></th></tr></thead>
      <tbody id="schedTbody">
        <tr><td colspan="6" style="color:var(--muted);text-align:center;padding:24px">No schedules yet</td></tr>
      </tbody>
    </table>
  </div>
  <p class="stitle">Add Schedule</p>
  <div class="card">
    <div class="fgrid">
      <div class="ff">
        <label>Zone</label>
        <select id="nZone">
          <option value="0">Zone 1</option><option value="1">Zone 2</option>
          <option value="2">Zone 3</option><option value="3">Zone 4</option>
        </select>
      </div>
      <div class="ff"><label>Start Time</label><input type="time" id="nTime" value="06:00"></div>
      <div class="ff"><label>Duration (min)</label><input type="number" id="nDur" value="15" min="1" max="180"></div>
    </div>
    <div class="ff">
      <label>Days</label>
      <div class="dsel">
        <button class="dtog sel" data-day="0">Su</button>
        <button class="dtog sel" data-day="1">Mo</button>
        <button class="dtog sel" data-day="2">Tu</button>
        <button class="dtog sel" data-day="3">We</button>
        <button class="dtog sel" data-day="4">Th</button>
        <button class="dtog sel" data-day="5">Fr</button>
        <button class="dtog sel" data-day="6">Sa</button>
      </div>
    </div>
    <button class="btn primary" style="margin-top:8px" onclick="addSchedule()">Add Schedule</button>
  </div>
</div>

<!-- ═══════ SETTINGS ═══════ -->
<div class="panel" id="panel-settings">
  <p class="stitle">Network</p>
  <div class="card">
    <div class="ff"><label>Wi-Fi SSID</label><input type="text" id="ssid" placeholder="Your network name"></div>
    <div class="ff"><label>Wi-Fi Password</label><input type="password" id="wpass" placeholder="Your password"></div>
  </div>

  <p class="stitle">Weather</p>
  <div class="card">
    <div class="ff"><label>OpenWeatherMap API Key</label><input type="text" id="apikey" placeholder="Free key from openweathermap.org"></div>
    <div style="display:grid;grid-template-columns:1fr 1fr;gap:12px">
      <div class="ff"><label>City</label><input type="text" id="city" placeholder="e.g. Hollister"></div>
      <div class="ff"><label>Country Code</label><input type="text" id="country" placeholder="e.g. US"></div>
    </div>
  </div>

  <p class="stitle">OLED Display</p>
  <div class="card">
    <div class="ff" style="max-width:280px;margin-bottom:20px">
      <label>Display Type</label>
      <select id="oledType" onchange="updateOledPreview()">
        <option value="0">None — No display</option>
        <option value="1">128x32 — Mini (0.91")</option>
        <option value="2">128x64 — Standard (0.96" / 1.3")</option>
      </select>
    </div>

    <!-- Info cards per display type -->
    <div id="oledNoneInfo">
      <p style="font-size:.82rem;color:var(--muted)">No OLED display connected. Select a display type if you have one wired to SDA (GPIO 21) and SCL (GPIO 22).</p>
    </div>

    <div id="oled32Info" style="display:none">
      <p style="font-size:.82rem;color:var(--muted);margin-bottom:14px">
        128x32 displays 3 screens cycling every 4 seconds when idle.
        During active watering the zone screen is always shown.
      </p>
      <div class="oled-screen-label">Idle screens (cycling)</div>
      <div>
        <span class="screen-badge on">1 Zone Status</span>
        <span class="screen-badge on">2 Weather</span>
        <span class="screen-badge on">3 IP / Time</span>
      </div>
      <div style="margin-top:14px">
        <div class="oled-screen-label">Preview — Zone Status</div>
        <div class="oled-preview active" style="font-size:.65rem;white-space:pre">Z1+ Z2- Z3- Z4-
Backyard: 4m 12s +1
! Rain skip ON</div>
      </div>
    </div>

    <div id="oled64Info" style="display:none">
      <p style="font-size:.82rem;color:var(--muted);margin-bottom:14px">
        128x64 displays 4 screens cycling every 4 seconds when idle.
        During active watering the zone screen is always shown.
      </p>
      <div class="oled-screen-label">Idle screens (cycling)</div>
      <div>
        <span class="screen-badge on">1 Zone Status</span>
        <span class="screen-badge on">2 Weather</span>
        <span class="screen-badge on">3 Time &amp; IP</span>
        <span class="screen-badge on">4 Schedules</span>
      </div>
      <div style="margin-top:14px;display:grid;grid-template-columns:1fr 1fr;gap:12px">
        <div>
          <div class="oled-screen-label">Preview — Zone Status</div>
          <div class="oled-preview active" style="font-size:.6rem;white-space:pre">-- ZONE STATUS --
[*] Backyard    4m12s
[ ] Front         OFF
[ ] Side          OFF
[ ] Garden        OFF
Rain skip: off</div>
        </div>
        <div>
          <div class="oled-screen-label">Preview — Weather</div>
          <div class="oled-preview active" style="font-size:.6rem;white-space:pre">-- WEATHER --

21.4C

partly cloudy
Humidity: 62%
Rain 1h: 0.0mm</div>
        </div>
        <div>
          <div class="oled-screen-label">Preview — Time</div>
          <div class="oled-preview active" style="font-size:.6rem;white-space:pre">-- TIME &amp; NET --

     14:32 :07

Mon 14 Jul 2025
192.168.1.42</div>
        </div>
        <div>
          <div class="oled-screen-label">Preview — Schedules</div>
          <div class="oled-preview active" style="font-size:.6rem;white-space:pre">-- SCHEDULES --
Z1 06:00 15min ON
Z2 06:20 10min ON
Z3 07:00 20min OFF
Z4 07:30 15min ON</div>
        </div>
      </div>
    </div>

    <div style="margin-top:16px;padding-top:16px;border-top:1px solid var(--border)">
      <p style="font-size:.78rem;color:var(--muted)">
        Wiring: SDA to GPIO 21 &nbsp;|&nbsp; SCL to GPIO 22 &nbsp;|&nbsp; VCC to 3.3V &nbsp;|&nbsp; GND to GND<br>
        I2C address: 0x3C (most modules). Change OLED_ADDR in firmware if display stays blank.
      </p>
    </div>
  </div>

  <button class="btn primary" onclick="saveSettings()">Save &amp; Reboot</button>
  <p style="font-size:.75rem;color:var(--muted);margin-top:10px">
    Device will reboot after saving. Reconnect to the same IP once it restarts.
  </p>
</div>

<!-- ═══════ FIRMWARE ═══════ -->
<div class="panel" id="panel-ota">
  <p class="stitle">Firmware Update</p>
  <div class="card" style="display:flex;gap:24px;flex-wrap:wrap;margin-bottom:20px">
    <div><div class="wstat">Version</div><div style="font-family:var(--mono);font-size:1.1rem;color:var(--accent)" id="fwVer">--</div></div>
  </div>
  <div class="otazone" onclick="document.getElementById('otaFile').click()">
    <label style="display:flex;flex-direction:column;align-items:center;gap:12px;cursor:pointer">
      <span style="font-size:2.5rem;color:var(--muted)">^</span>
      <span style="font-weight:700;font-size:1rem">Upload Firmware</span>
      <span style="font-size:.8rem;color:var(--muted)">Select a compiled .bin file</span>
      <button class="btn primary" style="pointer-events:none">Choose .bin File</button>
    </label>
    <input type="file" id="otaFile" accept=".bin" onchange="startOTA(this)">
    <div class="pbar" id="pbar"><div class="pfill" id="pfill"></div></div>
    <div class="otastat" id="otaMsg"></div>
  </div>
</div>

<div class="toast" id="toast"></div>

<script>
var DAYS = ['Su','Mo','Tu','We','Th','Fr','Sa'];

function showTab(n) {
  document.querySelectorAll('.panel').forEach(function(p){ p.classList.remove('active'); });
  document.querySelectorAll('.tab').forEach(function(t){ t.classList.remove('active'); });
  var p = document.getElementById('panel-'+n);
  var t = document.getElementById('tab-'+n);
  if(p) p.classList.add('active');
  if(t) t.classList.add('active');
}

function showToast(msg, isErr) {
  var el = document.getElementById('toast');
  el.textContent = msg;
  el.className = 'toast show' + (isErr ? ' err' : '');
  setTimeout(function(){ el.classList.remove('show'); }, 3000);
}

function updateClock() {
  var now = new Date();
  document.getElementById('htime').textContent = now.toTimeString().slice(0,8);
  document.getElementById('hdate').textContent = now.toDateString();
}
setInterval(updateClock, 3000);
updateClock();

function apiFetch(path, opts) {
  return fetch(path, opts||{})
    .then(function(r){
      if(!r.ok) throw new Error('HTTP '+r.status);
      return r.json().catch(function(){ return {}; });
    });
}

function escHtml(s) {
  return String(s).replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;').replace(/"/g,'&quot;');
}

/* ---- Zone cards ---- */
function buildZoneCards(zonesData) {
  var c = document.getElementById('zoneCards');
  c.innerHTML = '';
  for(var i=0; i<zonesData.length; i++) {
    var z = zonesData[i];
    var on = z.active;
    var rem = z.timerRemaining;
    var cd = rem>0 ? (Math.floor(rem/60)+'m '+(rem%60)+'s remaining') : (on?'Running (manual)':'');
    var card = document.createElement('div');
    card.className = 'zcard'+(on?' on':'');
    card.innerHTML =
      '<div class="zhdr">'
      +'<input class="zname-input" type="text" value="'+escHtml(z.name)+'" id="zn'+i+'" '
      +'onblur="saveName('+i+')" onkeydown="if(event.key===\'Enter\')this.blur()">'
      +'<div style="display:flex;align-items:center">'
      +'<div class="dot'+(on?' on':'')+'" id="dot'+i+'"></div>'
      +'<span class="znum">Z'+(i+1)+'</span>'
      +'</div></div>'
      +'<div class="zctrl">'
      +'<button class="btn'+(on?' danger':'')+'" onclick="toggleManual('+i+')">'+(on&&z.manualOn?'Stop':'Manual ON')+'</button>'
      +'</div>'
      +'<div class="trow">'
      +'<input type="number" id="tm'+i+'" value="5" min="1" max="180">'
      +'<span class="tlbl">min</span>'
      +'<button class="btn" onclick="timerRun('+i+')">Timed Run</button>'
      +'</div>'
      +'<div class="cdown" id="cd'+i+'">'+escHtml(cd)+'</div>';
    c.appendChild(card);
  }
}

function toggleManual(i) {
  var cards = document.getElementById('zoneCards').querySelectorAll('.zcard');
  var on = cards[i] && cards[i].classList.contains('on');
  apiFetch('/api/zone',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({zone:i,action:on?'stop':'manual'})})
    .then(function(){ showToast('Zone '+(i+1)+' '+(on?'stopped':'ON')); refresh(); })
    .catch(function(e){ showToast('Error: '+e.message,true); });
}

function timerRun(i) {
  var min = parseInt(document.getElementById('tm'+i).value)||5;
  apiFetch('/api/zone',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({zone:i,action:'timer',minutes:min})})
    .then(function(){ showToast('Zone '+(i+1)+': '+min+'min run started'); refresh(); })
    .catch(function(e){ showToast('Error: '+e.message,true); });
}

function saveName(i) {
  var el = document.getElementById('zn'+i);
  var name = el ? el.value.trim() : '';
  if(!name) name = 'Zone '+(i+1);
  apiFetch('/api/zone',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({zone:i,action:'rename',name:name})})
    .then(function(){ showToast('Name saved'); })
    .catch(function(){});
}

function stopAll() {
  apiFetch('/api/zone',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({zone:-1,action:'stopall'})})
    .then(function(){ showToast('All zones stopped'); refresh(); })
    .catch(function(){ showToast('Error',true); });
}

function runSequence() {
  var min = parseInt(document.getElementById('seqDur').value)||5;
  apiFetch('/api/zone',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({zone:-1,action:'sequence',minutes:min})})
    .then(function(){ showToast('Sequence started'); refresh(); })
    .catch(function(){ showToast('Error',true); });
}

/* ---- Weather ---- */
var wIcons={'01':'[sun]','02':'[cloud/sun]','03':'[cloud]','04':'[clouds]',
            '09':'[rain]','10':'[showers]','11':'[storm]','13':'[snow]','50':'[fog]'};

function updateWeather(w) {
  if(!w) return;
  var code = w.icon ? w.icon.slice(0,2) : '';
  document.getElementById('wIco').textContent = wIcons[code]||'[--]';
  document.getElementById('wTemp').textContent = w.tempC.toFixed(1)+'C';
  document.getElementById('wDesc').textContent = w.description||'--';
  document.getElementById('wRain').textContent = w.rainMm.toFixed(1)+'mm';
  document.getElementById('wHumid').textContent = w.humidity+'%';
  var upd = w.lastChecked ? new Date(w.lastChecked*1000).toLocaleTimeString() : 'Never';
  document.getElementById('wUpd').textContent = upd;
  document.getElementById('wBadge').innerHTML = w.skipWatering
    ? '<span class="skip-badge">SKIP WATERING</span>'
    : '<span class="ok-badge">OK TO WATER</span>';
}

function refreshWeather() {
  apiFetch('/api/weather/refresh',{method:'POST'})
    .then(function(){ showToast('Refreshing...'); setTimeout(refresh, 4000); })
    .catch(function(){ showToast('Error',true); });
}

/* ---- Schedules ---- */
function updateScheduleTable(list) {
  var tb = document.getElementById('schedTbody');
  if(!list||list.length===0){
    tb.innerHTML='<tr><td colspan="6" style="color:var(--muted);text-align:center;padding:24px">No schedules yet</td></tr>';
    return;
  }
  var html='';
  for(var i=0;i<list.length;i++){
    var s=list[i];
    var chips='';
    for(var j=0;j<DAYS.length;j++)
      chips+='<span class="dchip'+((s.days&(1<<j))?' on':'')+'">'+DAYS[j]+'</span>';
    var dur=s.durationSec>=60?Math.floor(s.durationSec/60)+'m':s.durationSec+'s';
    var hh=String(s.hour).padStart(2,'0'),mm=String(s.minute).padStart(2,'0');
    html+='<tr>'
      +'<td>Zone '+(s.zone+1)+'</td>'
      +'<td style="font-family:var(--mono)">'+hh+':'+mm+'</td>'
      +'<td style="font-family:var(--mono)">'+dur+'</td>'
      +'<td><div class="dchips">'+chips+'</div></td>'
      +'<td style="color:'+(s.enabled?'var(--accent)':'var(--muted)')+'">'+( s.enabled?'Active':'Off')+'</td>'
      +'<td><button class="btn danger" onclick="delSchedule('+i+')">x</button></td>'
      +'</tr>';
  }
  tb.innerHTML=html;
}

document.querySelectorAll('.dtog').forEach(function(btn){
  btn.addEventListener('click',function(){ btn.classList.toggle('sel'); });
});

function addSchedule() {
  var parts=document.getElementById('nTime').value.split(':');
  var days=0;
  document.querySelectorAll('.dtog').forEach(function(btn,idx){
    if(btn.classList.contains('sel')) days|=(1<<idx);
  });
  if(!days){ showToast('Select at least one day',true); return; }
  apiFetch('/api/schedule',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({
      zone:parseInt(document.getElementById('nZone').value),
      hour:parseInt(parts[0]),minute:parseInt(parts[1]),
      durationSec:parseInt(document.getElementById('nDur').value||15)*60,
      days:days,enabled:true
    })})
    .then(function(){ showToast('Schedule added'); refresh(); })
    .catch(function(e){ showToast('Error: '+e.message,true); });
}

function delSchedule(idx) {
  apiFetch('/api/schedule/'+idx,{method:'DELETE'})
    .then(function(){ showToast('Schedule removed'); refresh(); })
    .catch(function(){ showToast('Error',true); });
}

/* ---- Settings ---- */
function updateOledPreview() {
  var v = document.getElementById('oledType').value;
  document.getElementById('oledNoneInfo').style.display = v==='0' ? '' : 'none';
  document.getElementById('oled32Info').style.display   = v==='1' ? '' : 'none';
  document.getElementById('oled64Info').style.display   = v==='2' ? '' : 'none';
}

function loadSettings() {
  apiFetch('/api/settings').then(function(d){
    document.getElementById('ssid').value    = d.ssid||'';
    document.getElementById('wpass').value   = d.pass||'';
    document.getElementById('apikey').value  = d.api||'';
    document.getElementById('city').value    = d.city||'';
    document.getElementById('country').value = d.country||'';
    var osel = document.getElementById('oledType');
    osel.value = String(d.oled||0);
    updateOledPreview();
  }).catch(function(){});
}

function saveSettings() {
  apiFetch('/api/settings',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({
      ssid:document.getElementById('ssid').value,
      pass:document.getElementById('wpass').value,
      api:document.getElementById('apikey').value,
      city:document.getElementById('city').value,
      country:document.getElementById('country').value,
      oled:parseInt(document.getElementById('oledType').value)
    })})
    .then(function(){ showToast('Saved — rebooting...'); })
    .catch(function(e){ showToast('Error: '+e.message,true); });
}

/* ---- OTA ---- */
function startOTA(input) {
  var file=input.files[0];
  if(!file||!file.name.endsWith('.bin')){ showToast('Select a .bin file',true); return; }
  var pbar=document.getElementById('pbar'),pfill=document.getElementById('pfill'),msg=document.getElementById('otaMsg');
  pbar.style.display='block'; msg.textContent='Uploading...';
  var fd=new FormData(); fd.append('firmware',file);
  var xhr=new XMLHttpRequest();
  xhr.upload.onprogress=function(e){
    if(e.lengthComputable){ var p=Math.round(e.loaded/e.total*100); pfill.style.width=p+'%'; msg.textContent='Uploading... '+p+'%'; }
  };
  xhr.onload=function(){
    if(xhr.status===200){ msg.textContent='Done! Rebooting...'; showToast('Firmware updated!'); setTimeout(function(){ location.reload(); },8000); }
    else { msg.textContent='Failed: '+xhr.responseText; showToast('Update failed',true); }
  };
  xhr.onerror=function(){ msg.textContent='Upload error'; showToast('Upload error',true); };
  xhr.open('POST','/api/ota');
  xhr.send(fd);
}

/* ---- Status poll ---- */
function refresh() {
  apiFetch('/api/status')
    .then(function(d){
      if(d.zones)     buildZoneCards(d.zones);
      if(d.weather)   updateWeather(d.weather);
      if(d.schedules) updateScheduleTable(d.schedules);
      if(d.version)   document.getElementById('fwVer').textContent = d.version;
    })
    .catch(function(e){ console.warn('Status error:',e); });
}

setInterval(refresh, 3000);
refresh();
loadSettings();
</script>
</body>
</html>
)HTMLRAW";
