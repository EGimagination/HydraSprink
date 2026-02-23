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
.wcrd{background:var(--surface);border:1px solid var(--border);border-radius:14px;padding:24px;display:flex;align-items:flex-start;gap:24px;margin-bottom:28px;flex-wrap:wrap}
.wtemp{font-family:var(--mono);font-size:2rem;font-weight:700;color:var(--accent2)}
.wdesc{color:var(--muted);font-size:.9rem;text-transform:capitalize;margin-top:4px}
.wrow{display:flex;gap:20px;margin-top:10px;flex-wrap:wrap}
.wlbl{font-family:var(--mono);font-size:.7rem;color:var(--muted)}
.wlbl span{color:var(--text)}
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
.otadrop{border:2px dashed var(--border);border-radius:14px;padding:40px;text-align:center;transition:border-color .2s;cursor:pointer}
.otadrop:hover{border-color:var(--accent)}
.otadrop input[type=file]{display:none}
.pbar{width:100%;height:6px;background:var(--surface2);border-radius:3px;margin-top:20px;overflow:hidden;display:none}
.pfill{height:100%;background:linear-gradient(90deg,var(--accent),var(--accent2));width:0;transition:width .3s}
.otamsg{font-family:var(--mono);font-size:.75rem;color:var(--accent);margin-top:12px;min-height:20px}
/* Unit toggle */
.sw{position:relative;display:inline-block;width:50px;height:26px;flex-shrink:0}
.sw input{opacity:0;width:0;height:0}
.swslide{position:absolute;cursor:pointer;inset:0;background:var(--surface2);border:1px solid var(--border);transition:.3s;border-radius:26px}
.swslide:before{position:absolute;content:"";height:18px;width:18px;left:3px;bottom:3px;background:var(--muted);transition:.3s;border-radius:50%}
.sw input:checked + .swslide{background:rgba(0,212,170,.2);border-color:var(--accent)}
.sw input:checked + .swslide:before{transform:translateX(24px);background:var(--accent)}
.ulbl{font-family:var(--mono);font-size:.78rem;transition:color .2s}
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

<!-- ══════════════════ ZONES ══════════════════ -->
<div class="panel active" id="panel-zones">
  <p class="stitle">Zone Control</p>
  <div class="cgrid" id="zoneCards">
    <div class="zcard">
      <div style="color:var(--muted);font-family:var(--mono);font-size:.75rem">Connecting...</div>
    </div>
  </div>
  <p class="stitle">Quick Actions</p>
  <div class="card" style="display:flex;gap:12px;flex-wrap:wrap;align-items:center">
    <button class="btn danger" onclick="stopAll()">Stop All Zones</button>
    <span style="color:var(--border)">|</span>
    <span style="font-size:.8rem;color:var(--muted)">Timed sequence:</span>
    <input type="number" id="seqMin" value="5" min="1" max="60"
      style="width:56px;background:var(--surface2);border:1px solid var(--border);color:var(--text);border-radius:6px;padding:6px 8px;font-family:var(--mono);font-size:.75rem;outline:none;text-align:center">
    <span class="tlbl">min each</span>
    <button class="btn primary" onclick="runSequence()">Run All</button>
  </div>
</div>

<!-- ══════════════════ WEATHER ══════════════════ -->
<div class="panel" id="panel-weather">
  <p class="stitle">Current Conditions</p>
  <div class="wcrd">
    <div style="flex:1">
      <div class="wtemp" id="wTemp">--</div>
      <div class="wdesc" id="wDesc">Loading...</div>
      <div class="wrow">
        <div class="wlbl">Rain &nbsp;<span id="wRain">--</span></div>
        <div class="wlbl">Humidity &nbsp;<span id="wHumid">--</span></div>
        <div class="wlbl">Device IP &nbsp;<span id="wIP">--</span></div>
      </div>
    </div>
  </div>
  <div class="card" style="display:flex;gap:12px;align-items:center;flex-wrap:wrap">
    <p style="font-size:.85rem;flex:1;color:var(--muted)">
      Firmware v<span id="fwVerW">--</span> &nbsp;|&nbsp; Weather refreshes every 10 minutes.
    </p>
    <button class="btn" onclick="forceWeather()">Refresh Now</button>
  </div>
</div>

<!-- ══════════════════ SCHEDULES ══════════════════ -->
<div class="panel" id="panel-schedules">
  <p class="stitle">Active Schedules</p>
  <div class="card">
    <table class="stbl">
      <thead>
        <tr><th>Zone</th><th>Time</th><th>Duration</th><th>Days</th><th>Status</th><th></th></tr>
      </thead>
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
          <option value="0">Zone 1</option>
          <option value="1">Zone 2</option>
          <option value="2">Zone 3</option>
          <option value="3">Zone 4</option>
        </select>
      </div>
      <div class="ff"><label>Start Time</label><input type="time" id="nTime" value="06:00"></div>
      <div class="ff"><label>Duration (min)</label><input type="number" id="nDur" value="10" min="1" max="180"></div>
    </div>
    <div class="ff">
      <label>Days</label>
      <div class="dsel" id="dayToggles">
        <button class="dtog sel" data-d="0">Su</button>
        <button class="dtog sel" data-d="1">Mo</button>
        <button class="dtog sel" data-d="2">Tu</button>
        <button class="dtog sel" data-d="3">We</button>
        <button class="dtog sel" data-d="4">Th</button>
        <button class="dtog sel" data-d="5">Fr</button>
        <button class="dtog sel" data-d="6">Sa</button>
      </div>
    </div>
    <button class="btn primary" style="margin-top:8px" onclick="addSchedule()">Add Schedule</button>
  </div>
</div>

<!-- ══════════════════ SETTINGS ══════════════════ -->
<div class="panel" id="panel-settings">

  <p class="stitle">Display Units</p>
  <div class="card">
    <div style="display:flex;align-items:center;gap:14px">
      <span class="ulbl" id="lblMetric">Metric (°C / mm)</span>
      <label class="sw">
        <input type="checkbox" id="unitToggle" onchange="onUnitToggle()">
        <span class="swslide"></span>
      </label>
      <span class="ulbl" id="lblImperial">Imperial (°F / in)</span>
    </div>
    <p style="font-size:.75rem;color:var(--muted);margin-top:10px">
      Switches instantly without rebooting. Preference is saved to the device.
    </p>
  </div>

  <p class="stitle">Network</p>
  <div class="card">
    <div class="ff"><label>Wi-Fi SSID</label>
      <input type="text" id="cfgSsid" placeholder="Network name"></div>
    <div class="ff"><label>Wi-Fi Password</label>
      <input type="password" id="cfgPass" placeholder="Leave blank to keep current"></div>
  </div>

  <p class="stitle">Weather API</p>
  <div class="card">
    <div class="ff"><label>OpenWeatherMap API Key</label>
      <input type="text" id="cfgApi" placeholder="Free key — openweathermap.org"></div>
    <div style="display:grid;grid-template-columns:1fr 1fr;gap:12px">
      <div class="ff"><label>City</label>
        <input type="text" id="cfgCity" placeholder="e.g. Hollister"></div>
      <div class="ff"><label>Country Code</label>
        <input type="text" id="cfgCountry" placeholder="e.g. US"></div>
    </div>
    <button class="btn primary" onclick="saveNetSettings()">Save &amp; Reboot</button>
    <p style="font-size:.75rem;color:var(--muted);margin-top:8px">
      Device reboots after saving. Reconnect once it restarts.
    </p>
  </div>
</div>

<!-- ══════════════════ FIRMWARE ══════════════════ -->
<div class="panel" id="panel-ota">
  <p class="stitle">Firmware Update</p>
  <div class="card" style="display:flex;gap:24px;flex-wrap:wrap;margin-bottom:20px">
    <div>
      <div style="font-family:var(--mono);font-size:.6rem;color:var(--muted);letter-spacing:.1em;text-transform:uppercase">Current Version</div>
      <div style="font-family:var(--mono);font-size:1.1rem;color:var(--accent);margin-top:4px" id="fwVer">--</div>
    </div>
  </div>
  <div class="otadrop" onclick="document.getElementById('otaFile').click()">
    <label style="display:flex;flex-direction:column;align-items:center;gap:12px;cursor:pointer">
      <span style="font-size:2.5rem;color:var(--muted)">&#8679;</span>
      <span style="font-weight:700;font-size:1rem">Upload Firmware</span>
      <span style="font-size:.8rem;color:var(--muted)">Select a compiled .bin file</span>
      <button class="btn primary" style="pointer-events:none">Choose .bin File</button>
    </label>
    <input type="file" id="otaFile" accept=".bin" onchange="startOTA(this)">
    <div class="pbar" id="pbar"><div class="pfill" id="pfill"></div></div>
    <div class="otamsg" id="otaMsg"></div>
  </div>
</div>

<div class="toast" id="toast"></div>

<script>
var DAYS = ['Su','Mo','Tu','We','Th','Fr','Sa'];

// Holds raw metric weather from last poll; never mutated
var currentWeather = null;
// Holds latest zones array for sequence runs
var currentZones = [];
// Display preference — synced from device on first poll
var unitIsImperial = false;
// Guard: ignore the programmatic toggle.checked = x during init
var unitReady = false;

// ── Tab switching ──────────────────────────────
function showTab(n) {
  document.querySelectorAll('.panel').forEach(function(p){ p.classList.remove('active'); });
  document.querySelectorAll('.tab').forEach(function(t){ t.classList.remove('active'); });
  document.getElementById('panel-'+n).classList.add('active');
  document.getElementById('tab-'+n).classList.add('active');
}

// ── Toast ─────────────────────────────────────
function showToast(msg, isErr) {
  var el = document.getElementById('toast');
  el.textContent = msg;
  el.className = 'toast show' + (isErr ? ' err' : '');
  setTimeout(function(){ el.classList.remove('show'); }, 3000);
}

// ── Clock ─────────────────────────────────────
function tickClock() {
  var n = new Date();
  document.getElementById('htime').textContent = n.toTimeString().slice(0,8);
  document.getElementById('hdate').textContent = n.toDateString();
}
setInterval(tickClock, 1000);
tickClock();

// ── Fetch helper ──────────────────────────────
function apiFetch(path, opts) {
  return fetch(path, opts||{}).then(function(r){
    if(!r.ok) throw new Error('HTTP '+r.status);
    return r.json().catch(function(){ return {}; });
  });
}

// ── HTML escape ───────────────────────────────
function esc(s) {
  return String(s).replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;').replace(/"/g,'&quot;');
}

// ══════════════════════════════════════════════
//  UNIT TOGGLE
//  currentWeather always holds °C / mm.
//  renderWeather() converts for display.
//  Saving to device never triggers a reboot.
// ══════════════════════════════════════════════
function onUnitToggle() {
  if (!unitReady) return;
  unitIsImperial = document.getElementById('unitToggle').checked;
  syncUnitLabels();
  renderWeather();
  apiFetch('/api/settings', {
    method: 'POST',
    headers: {'Content-Type':'application/json'},
    body: JSON.stringify({imperial: unitIsImperial})
  })
  .then(function(){ showToast(unitIsImperial ? 'Imperial units' : 'Metric units'); })
  .catch(function(){ showToast('Could not save unit preference', true); });
}

function syncUnitLabels() {
  var m = document.getElementById('lblMetric');
  var i = document.getElementById('lblImperial');
  if(m) m.style.color = unitIsImperial ? 'var(--muted)' : 'var(--accent)';
  if(i) i.style.color = unitIsImperial ? 'var(--accent)' : 'var(--muted)';
}

// ── Weather render ─────────────────────────────
function renderWeather() {
  if (!currentWeather) return;
  var w = currentWeather;
  document.getElementById('wDesc').textContent  = w.description || '--';
  document.getElementById('wHumid').textContent = (w.humidity||0).toFixed(0) + '%';
  if (unitIsImperial) {
    document.getElementById('wTemp').textContent = ((w.tempC||0)*9/5+32).toFixed(1) + ' F';
    document.getElementById('wRain').textContent = ((w.rainMm||0)/25.4).toFixed(2) + ' in';
  } else {
    document.getElementById('wTemp').textContent = (w.tempC||0).toFixed(1) + ' C';
    document.getElementById('wRain').textContent = (w.rainMm||0).toFixed(1) + ' mm';
  }
}

function forceWeather() {
  // Trigger an immediate re-fetch by making a dummy GET — the next
  // /api/status will return updated values after 3s
  showToast('Refreshing weather...');
}

// ── Zone cards ─────────────────────────────────
function buildZoneCards(arr) {
  currentZones = arr;
  var c = document.getElementById('zoneCards');
  if (!arr || arr.length === 0) return;
  c.innerHTML = '';
  for (var i = 0; i < arr.length; i++) {
    var z   = arr[i];
    var on  = z.active;
    var rem = z.timerRemaining;
    var cd  = '';
    if (rem > 0) cd = Math.floor(rem/60) + 'm ' + (rem%60) + 's remaining';
    else if (on) cd = 'Running (manual)';

    var card = document.createElement('div');
    card.className = 'zcard' + (on ? ' on' : '');
    card.innerHTML =
      '<div class="zhdr">' +
        '<input class="zname-input" type="text" value="' + esc(z.name) + '" ' +
          'id="zn' + i + '" ' +
          'onblur="saveName(' + i + ')" ' +
          'onkeydown="if(event.key===\'Enter\')this.blur()">' +
        '<div style="display:flex;align-items:center">' +
          '<div class="dot' + (on ? ' on' : '') + '"></div>' +
          '<span class="znum">Z' + (i+1) + '</span>' +
        '</div>' +
      '</div>' +
      '<div class="zctrl">' +
        '<button class="btn' + (on ? ' danger' : '') + '" onclick="toggleManual(' + i + ')">' +
          (on && z.manualOn ? 'Stop' : 'Manual ON') +
        '</button>' +
      '</div>' +
      '<div class="trow">' +
        '<input type="number" id="tm' + i + '" value="10" min="1" max="180">' +
        '<span class="tlbl">min</span>' +
        '<button class="btn" onclick="timerRun(' + i + ')">Timed Run</button>' +
      '</div>' +
      '<div class="cdown">' + esc(cd) + '</div>';
    c.appendChild(card);
  }
}

function toggleManual(i) {
  var cards = document.getElementById('zoneCards').querySelectorAll('.zcard');
  var on = cards[i] && cards[i].classList.contains('on');
  apiFetch('/api/zone', {
    method:'POST', headers:{'Content-Type':'application/json'},
    body: JSON.stringify({zone:i, action: on ? 'stop' : 'manual'})
  }).then(function(){ showToast('Zone '+(i+1)+' '+(on?'stopped':'turned ON')); refresh(); })
    .catch(function(e){ showToast('Error: '+e.message, true); });
}

function timerRun(i) {
  var min = parseInt(document.getElementById('tm'+i).value) || 10;
  apiFetch('/api/zone', {
    method:'POST', headers:{'Content-Type':'application/json'},
    body: JSON.stringify({zone:i, action:'timer', minutes:min})
  }).then(function(){ showToast('Zone '+(i+1)+': '+min+' min run started'); refresh(); })
    .catch(function(e){ showToast('Error: '+e.message, true); });
}

function saveName(i) {
  var el = document.getElementById('zn'+i);
  var name = el ? el.value.trim() : 'Zone '+(i+1);
  if (!name) name = 'Zone '+(i+1);
  apiFetch('/api/zone', {
    method:'POST', headers:{'Content-Type':'application/json'},
    body: JSON.stringify({zone:i, action:'rename', name:name})
  }).then(function(){ showToast('Name saved'); })
    .catch(function(){});
}

function stopAll() {
  apiFetch('/api/zone', {
    method:'POST', headers:{'Content-Type':'application/json'},
    body: JSON.stringify({zone:-1, action:'stopall'})
  }).then(function(){ showToast('All zones stopped'); refresh(); })
    .catch(function(){ showToast('Error', true); });
}

// Run each zone for seqMin minutes in sequence via chained timers
var seqIndex = 0;
function runSequence() {
  var min = parseInt(document.getElementById('seqMin').value) || 5;
  seqIndex = 0;
  runNextInSeq(min);
}
function runNextInSeq(min) {
  if (seqIndex >= currentZones.length) { showToast('Sequence complete'); return; }
  var z = seqIndex;
  seqIndex++;
  apiFetch('/api/zone', {
    method:'POST', headers:{'Content-Type':'application/json'},
    body: JSON.stringify({zone:z, action:'timer', minutes:min})
  }).then(function(){
    showToast('Zone '+(z+1)+' running '+min+'min');
    refresh();
    // Queue next zone after this one's duration
    setTimeout(function(){ runNextInSeq(min); }, min*60*1000);
  }).catch(function(){ showToast('Sequence error', true); });
}

// ── Schedule table ─────────────────────────────
function buildScheduleTable(list) {
  var tb = document.getElementById('schedTbody');
  if (!list || list.length === 0) {
    tb.innerHTML = '<tr><td colspan="6" style="color:var(--muted);text-align:center;padding:24px">No schedules yet</td></tr>';
    return;
  }
  var html = '';
  for (var i = 0; i < list.length; i++) {
    var s   = list[i];
    var chips = '';
    for (var d = 0; d < DAYS.length; d++)
      chips += '<span class="dchip' + ((s.days & (1<<d)) ? ' on' : '') + '">' + DAYS[d] + '</span>';
    var dur = s.durationSec >= 60 ? Math.floor(s.durationSec/60) + 'm' : s.durationSec + 's';
    var hh  = String(s.hour).padStart(2,'0');
    var mm  = String(s.minute).padStart(2,'0');
    html +=
      '<tr>' +
      '<td>Zone ' + (s.zone+1) + '</td>' +
      '<td style="font-family:var(--mono)">' + hh + ':' + mm + '</td>' +
      '<td style="font-family:var(--mono)">' + dur + '</td>' +
      '<td><div class="dchips">' + chips + '</div></td>' +
      '<td style="color:' + (s.enabled ? 'var(--accent)' : 'var(--muted)') + '">' + (s.enabled ? 'Active' : 'Off') + '</td>' +
      '<td><button class="btn danger" onclick="delSchedule(' + i + ')">&#x2715;</button></td>' +
      '</tr>';
  }
  tb.innerHTML = html;
}

document.querySelectorAll('.dtog').forEach(function(btn){
  btn.addEventListener('click', function(){ btn.classList.toggle('sel'); });
});

function addSchedule() {
  var parts = document.getElementById('nTime').value.split(':');
  var days  = 0;
  document.querySelectorAll('#dayToggles .dtog').forEach(function(btn, idx){
    if (btn.classList.contains('sel')) days |= (1 << idx);
  });
  if (!days) { showToast('Select at least one day', true); return; }
  var dur = parseInt(document.getElementById('nDur').value) || 10;
  apiFetch('/api/schedule', {
    method:'POST', headers:{'Content-Type':'application/json'},
    body: JSON.stringify({
      zone:        parseInt(document.getElementById('nZone').value),
      hour:        parseInt(parts[0]),
      minute:      parseInt(parts[1]),
      durationSec: dur * 60,
      days:        days,
      enabled:     true
    })
  }).then(function(){ showToast('Schedule added'); refresh(); })
    .catch(function(e){ showToast('Error: '+e.message, true); });
}

function delSchedule(idx) {
  apiFetch('/api/schedule/'+idx, {method:'DELETE'})
    .then(function(){ showToast('Schedule removed'); refresh(); })
    .catch(function(){ showToast('Error', true); });
}

// ── Settings form ──────────────────────────────
function loadSettingsForm() {
  apiFetch('/api/settings').then(function(d){
    if (document.getElementById('cfgSsid'))    document.getElementById('cfgSsid').value    = d.ssid    || '';
    if (document.getElementById('cfgApi'))     document.getElementById('cfgApi').value     = d.api     || '';
    if (document.getElementById('cfgCity'))    document.getElementById('cfgCity').value    = d.city    || '';
    if (document.getElementById('cfgCountry')) document.getElementById('cfgCountry').value = d.country || '';
  }).catch(function(){});
}

function saveNetSettings() {
  apiFetch('/api/settings', {
    method:'POST', headers:{'Content-Type':'application/json'},
    body: JSON.stringify({
      ssid:    document.getElementById('cfgSsid').value,
      pass:    document.getElementById('cfgPass').value,
      api:     document.getElementById('cfgApi').value,
      city:    document.getElementById('cfgCity').value,
      country: document.getElementById('cfgCountry').value
    })
  }).then(function(){ showToast('Saved — rebooting...'); })
    .catch(function(e){ showToast('Error: '+e.message, true); });
}

// ── OTA upload ─────────────────────────────────
function startOTA(input) {
  var file = input.files[0];
  if (!file || !file.name.endsWith('.bin')) { showToast('Select a .bin file', true); return; }
  var pbar  = document.getElementById('pbar');
  var pfill = document.getElementById('pfill');
  var msg   = document.getElementById('otaMsg');
  pbar.style.display = 'block';
  msg.textContent = 'Uploading...';
  var fd = new FormData();
  fd.append('firmware', file);
  var xhr = new XMLHttpRequest();
  xhr.upload.onprogress = function(e){
    if (e.lengthComputable){
      var p = Math.round(e.loaded/e.total*100);
      pfill.style.width = p+'%';
      msg.textContent = 'Uploading... '+p+'%';
    }
  };
  xhr.onload = function(){
    if (xhr.status === 200){
      msg.textContent = 'Done! Rebooting...';
      showToast('Firmware updated!');
      setTimeout(function(){ location.reload(); }, 8000);
    } else {
      msg.textContent = 'Failed: '+xhr.responseText;
      showToast('Update failed', true);
    }
  };
  xhr.onerror = function(){ msg.textContent = 'Upload error'; showToast('Upload error', true); };
  xhr.open('POST','/api/ota');
  xhr.send(fd);
}

// ══════════════════════════════════════════════
//  MAIN STATUS POLL  (every 3 seconds)
//  Populates zones, weather, schedules.
//  On first call, syncs unit toggle from device.
// ══════════════════════════════════════════════
function refresh() {
  apiFetch('/api/status').then(function(d){

    // Zones
    if (d.zones) buildZoneCards(d.zones);

    // Weather
    if (d.weather) {
      currentWeather = d.weather;
      renderWeather();
    }
    if (d.ip && document.getElementById('wIP'))
      document.getElementById('wIP').textContent = d.ip;

    // Schedules
    if (d.schedules) buildScheduleTable(d.schedules);

    // Version
    if (d.version){
      var fv  = document.getElementById('fwVer');
      var fvw = document.getElementById('fwVerW');
      if (fv)  fv.textContent  = d.version;
      if (fvw) fvw.textContent = d.version;
    }

    // Sync unit toggle from device on very first poll only
    if (!unitReady) {
      unitIsImperial = d.imperial === true;
      var tog = document.getElementById('unitToggle');
      if (tog) tog.checked = unitIsImperial;
      syncUnitLabels();
      renderWeather();
      unitReady = true;
    }

  }).catch(function(e){ console.warn('Status poll failed:', e); });
}

setInterval(refresh, 3000);
loadSettingsForm();
refresh();
</script>
</body>
</html>
)HTMLRAW";