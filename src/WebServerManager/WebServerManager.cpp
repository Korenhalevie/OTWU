// WebServerManager.cpp 
#include "WebServerManager.h"
#include <Preferences.h>
#include "ScheduleManager/ScheduleManager.h"

WebServerManager::WebServerManager(LEDController* led, MQTTManager* mqtt)
    : _server(80), _ledController(led), _mqttManager(mqtt) {}

void WebServerManager::setup() {
    setupRootPage();
    setupColorHandler();
  setupBrightnessHandler();
    setupScheduleHandler();
    setupClearScheduleHandler();
    setupForgetWiFiHandler();
    _server.begin();
}

void WebServerManager::setupRootPage() {
    _server.on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
        String greenWindows = ScheduleManager::getGreenWindows();
  String currentColor = _ledController->getColor();
  // Inject brightness percent into page via placeholder token appended to greenWindows variable (separate param simpler)
  String page = generateHtmlPage(greenWindows, currentColor);
  page.replace("<!--BRIGHTNESS_PLACEHOLDER-->", String(_ledController->getBrightnessPercent()));
  request->send(200, "text/html", page);
    });
}

void WebServerManager::setupColorHandler() {
    _server.on("/setColor", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (request->hasParam("color")) {
            String color = request->getParam("color")->value();
            _ledController->setColor(color);
            _mqttManager->publishColor(color.c_str());
            request->send(200, "text/plain", "Color changed to " + color);
        } else {
            request->send(400, "text/plain", "Missing color parameter");
        }
    });
}

void WebServerManager::setupBrightnessHandler() {
  _server.on("/setBrightness", HTTP_GET, [this](AsyncWebServerRequest* request) {
    if (request->hasParam("value")) {
      int val = request->getParam("value")->value().toInt();
      if (val < 0) val = 0; if (val > 100) val = 100;
      _ledController->setBrightnessPercent((uint8_t)val);
      request->send(200, "text/plain", "Brightness set to " + String(val));
    } else {
      request->send(400, "text/plain", "Missing value parameter");
    }
  });
}

void WebServerManager::setupScheduleHandler() {
    _server.on("/setSchedule", HTTP_GET, [](AsyncWebServerRequest* request) {
        if (request->hasParam("greenWindows")) {
            ScheduleManager::saveGreenWindows(request->getParam("greenWindows")->value());
            request->send(200, "text/plain", "Schedule saved!");
        } else {
            request->send(400, "text/plain", "Missing green windows parameter");
        }
    });
}


void WebServerManager::setupClearScheduleHandler() {
    _server.on("/clearSchedule", HTTP_GET, [](AsyncWebServerRequest* request) {
        ScheduleManager::clearGreenWindows();
        request->send(200, "text/plain", "Schedule cleared!");
    });
}

void WebServerManager::setupForgetWiFiHandler() {
    _server.on("/forgetWiFi", HTTP_GET, [](AsyncWebServerRequest* request) {
        Preferences pref;
        pref.begin("wifi", false);
        pref.clear();
        pref.end();

        WiFi.disconnect(true, true);
        request->send(200, "text/plain", "WiFi cleared. Restarting...");
        delay(1000);
        ESP.restart();
    });
}

String WebServerManager::generateHtmlPage(const String& greenWindows, const String& currentColor) {
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8" />
<meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover" />
<title>OK TO WAKE – Control Panel</title>
<meta name="theme-color" content="#0ea5a5" />
<script src="https://cdn.jsdelivr.net/npm/sweetalert2@11"></script>
<style>
:root{--bg-0:hsl(250,50%,98%);--bg-1:hsl(230,45%,97%);--card:#fff;--fg:hsl(210,20%,18%);--muted:hsl(220,20%,94%);--muted-fg:hsl(215,15%,50%);--border:hsl(220,16%,88%);--ring:hsl(210,100%,65%);--wake:hsl(120,70%,45%);--wake-weak:hsl(120,50%,85%);--sleep:hsl(0,60%,55%);--sleep-weak:hsl(0,40%,88%);--time:hsl(210,80%,58%);--time-weak:hsl(210,60%,90%);--accent:var(--time);--accent-weak:var(--time-weak);--g-aurora:radial-gradient(60% 60% at 20% 20%,hsl(180,90%,90%) 0%,transparent 60%),radial-gradient(50% 50% at 80% 30%,hsl(260,90%,92%) 0%,transparent 60%),radial-gradient(40% 40% at 60% 80%,hsl(120,90%,90%) 0%,transparent 60%);--g-surface:linear-gradient(145deg,#fff,hsl(220,20%,98%));--g-wake:linear-gradient(135deg,hsl(120,70%,45%),hsl(120,60%,65%));--g-sleep:linear-gradient(135deg,hsl(0,60%,55%),hsl(0,50%,70%));--g-accent:linear-gradient(135deg,var(--accent),hsl(210,100%,70%));--radius-lg:18px;--radius-md:12px;--radius-sm:10px;--shadow-soft:0 8px 24px hsl(215 30% 20% / .08);--shadow-float:0 12px 36px hsl(215 30% 20% / .14);--shadow-glow:0 0 40px hsl(210 100% 80% / .4);--ease:cubic-bezier(.22,1,.36,1);--t-fast:180ms var(--ease);--t-med:280ms var(--ease)}
*{box-sizing:border-box}html,body{height:100%}body{margin:0;font-family:ui-sans-serif,system-ui,-apple-system,Segoe UI,Roboto,Noto Sans,Ubuntu,Cantarell,Arial,"Apple Color Emoji","Segoe UI Emoji";background:linear-gradient(180deg,var(--bg-0),var(--bg-1)),var(--g-aurora);color:var(--fg);line-height:1.55;-webkit-tap-highlight-color:transparent}
.aurora{position:fixed;inset:-20vmax;background:var(--g-aurora);filter:blur(40px) saturate(120%);opacity:.5;pointer-events:none;animation:float 20s ease-in-out infinite alternate}
@keyframes float{0%{transform:translate3d(-2%,-1%,0) scale(1)}100%{transform:translate3d(2%,1%,0) scale(1.02)}}
@media (prefers-reduced-motion:reduce){.aurora{animation:none}}
.container{max-width:960px;margin:0 auto;padding:clamp(16px,3.5vw,32px)}
header{text-align:center;margin-bottom:clamp(16px,6vw,32px);position:relative}
.brand-icons{display:flex;justify-content:center;gap:12px;margin-bottom:10px;opacity:.9}
.brand-icons .ico{font-size:clamp(20px,5vw,28px)}
.title{font-size:clamp(28px,6vw,44px);font-weight:800;letter-spacing:.4px;background:var(--g-accent);-webkit-background-clip:text;background-clip:text;-webkit-text-fill-color:transparent;filter:drop-shadow(0 2px 10px hsl(210 90% 85% / .35));margin:0 0 4px}
.subtitle{color:var(--muted-fg);font-size:clamp(14px,3.6vw,18px);margin:0 auto 8px}
.divider{width:88px;height:4px;margin:10px auto 0;background:var(--g-accent);border-radius:999px;box-shadow:var(--shadow-glow);opacity:.9}
.grid{display:grid;gap:clamp(14px,3.5vw,22px);grid-template-columns:1fr}
.card{background:var(--g-surface);border:1px solid var(--border);border-radius:var(--radius-lg);box-shadow:var(--shadow-soft);padding:clamp(16px,4vw,24px);backdrop-filter:saturate(120%) blur(8px);position:relative;overflow:clip}
.card:hover{box-shadow:var(--shadow-float);transform:translateY(-1px);transition:transform var(--t-fast),box-shadow var(--t-med)}
.card-header{display:flex;align-items:center;justify-content:space-between;gap:12px;margin-bottom:14px;flex-wrap:wrap}
.card-title{display:flex;align-items:center;gap:10px;font-size:18px;font-weight:700}
.card-title .ico{font-size:22px}
.card-desc{color:var(--muted-fg);font-size:14px;margin-top:4px}
.clock{text-align:center;background:linear-gradient(180deg,var(--card),hsl(0 0% 100% / .6));border:1px solid var(--border);border-radius:var(--radius-md);padding:clamp(14px,4vw,20px);box-shadow:var(--shadow-glow)}
.clock-time{font-family:ui-monospace,SFMono-Regular,Menlo,Monaco,Consolas,"Liberation Mono","Courier New",monospace;font-size:clamp(28px,10vw,56px);font-weight:800;letter-spacing:.08em;color:var(--time);text-shadow:0 0 18px hsl(210 100% 88% / .35)}
.clock-date{margin-top:6px;color:var(--muted-fg);font-size:clamp(13px,3.4vw,15px)}
.status{display:flex;align-items:center;gap:8px;padding:10px 12px;background:hsl(0 0% 100% / .5);border:1px solid var(--border);border-radius:999px}
.dot{width:10px;height:10px;border-radius:999px;background:var(--muted-fg);box-shadow:0 0 0 3px hsl(0 0% 100% / .6) inset}
.dot.green{background:var(--wake)}.dot.red{background:var(--sleep)}.dot.blue{background:var(--time)}.dot.off{background:var(--muted-fg)}
.status-text{font-weight:700;font-size:15px}.status-sub{color:var(--muted-fg);font-size:12px}
.btns{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:10px}
@media (min-width:520px){.btns{grid-template-columns:repeat(4,minmax(0,1fr))}}
.btn{-webkit-user-select:none;user-select:none;display:flex;align-items:center;justify-content:center;gap:8px;padding:12px 14px;border:2px solid transparent;border-radius:12px;font-weight:700;font-size:16px;background:var(--card);color:var(--fg);transition:transform var(--t-fast),box-shadow var(--t-fast),background var(--t-fast),border-color var(--t-fast),opacity var(--t-fast);box-shadow:0 1px 0 hsl(0 0% 100% / .8) inset,0 8px 20px hsl(215 30% 20% / .06);min-height:44px}
.btn:hover{transform:translateY(-1px)}.btn:active{transform:translateY(0)}.btn:disabled{opacity:.6;transform:none;cursor:not-allowed}
.btn-wake{background:var(--g-wake);color:#fff;border-color:hsl(120,70%,42%);box-shadow:0 8px 24px hsl(120 70% 40% / .22)}
.btn-sleep{background:var(--g-sleep);color:#fff;border-color:hsl(0,60%,50%);box-shadow:0 8px 24px hsl(0 70% 45% / .22)}
.btn-blue {
  background: linear-gradient(135deg, hsl(210, 80%, 58%), hsl(210, 70%, 65%));
  color: white;
  border-color: hsl(210, 80%, 50%);
  box-shadow: 0 8px 24px hsl(210 80% 40% / 0.22);
}
.btn-off{background:var(--muted);color:var(--muted-fg);border-color:var(--border)}
.btn-outline{background:transparent;border-color:var(--border);color:var(--fg)}
.btn-outline:hover{background:hsl(0 0% 100% / .6)}
.btn-danger{background:transparent;color:var(--sleep);border-color:var(--sleep)}
.btn-danger:hover{background:var(--sleep-weak)}
.toolbar{display:flex;justify-content:center;margin-top:8px}.toolbar .btn{min-width:200px}
.spinner{width:18px;height:18px;border:2px solid transparent;border-top-color:currentColor;border-radius:999px;animation:spin 1s linear infinite}
@keyframes spin{to{transform:rotate(360deg)}}
.form-grid{display:grid;gap:16px;grid-template-columns:1fr}
@media (min-width:720px){.form-grid{grid-template-columns:repeat(2,minmax(0,1fr))}}
.group{display:flex;flex-direction:column;gap:8px}.label{display:flex;align-items:center;gap:8px;font-weight:700;color:var(--wake);font-size:15px}
.row{display:flex;gap:10px;align-items:center}.row>.flex-1{flex:1}
.input{width:100%;padding:12px;font-size:16px;border:2px solid var(--border);background:var(--card);border-radius:var(--radius-sm);color:var(--fg);outline:none;transition:border-color var(--t-fast),box-shadow var(--t-fast)}
.input:focus{border-color:var(--accent);box-shadow:0 0 0 3px hsl(210 100% 60% / .12)}
.input.wake{border-color:hsl(120 60% 50% / .35);background:hsl(120 50% 94% / .3)}
.help{color:var(--muted-fg);font-size:12px}
.actions{display:flex;gap:10px;flex-wrap:wrap}.actions .btn{flex:1;min-width:180px}
.flash{margin-top:10px;padding:10px 12px;border-radius:12px;font-weight:700;text-align:center;border:1px solid transparent;opacity:0;transform:translateY(4px);transition:all var(--t-med)}
.flash.show{opacity:1;transform:translateY(0)}
.flash.success{background:var(--wake-weak);color:var(--wake);border-color:var(--wake)}
.flash.error{background:var(--sleep-weak);color:var(--sleep);border-color:var(--sleep)}
.warning{background:hsl(0 60% 55% / .06);border:1px solid hsl(0 60% 55% / .22);border-radius:var(--radius-md);padding:16px;margin-top:10px}
.warning h3{margin:0 0 6px;color:var(--sleep)}.warning p{margin:0 0 10px;color:var(--muted-fg);font-size:13px}
footer{text-align:center;color:var(--muted-fg);font-size:13px;padding:20px 0 8px}
a,button{cursor:pointer}.sr{position:absolute;width:1px;height:1px;padding:0;margin:-1px;overflow:hidden;clip:rect(0,0,0,0);white-space:nowrap;border:0}
</style>
</head>
<body>
<div class="aurora" aria-hidden="true"></div>
<div class="container">
  <header>
    <div class="brand-icons" aria-hidden="true">
      <span class="ico">🌙</span><span class="ico">✨</span><span class="ico">🌅</span>
    </div>
    <h1 class="title">OK TO WAKE</h1>
    <p class="subtitle">Smart Wake-Up Clock</p>
    <div class="divider"></div>
  </header>

  <main class="grid" role="main">
    <!-- Clock -->
    <section class="card" aria-labelledby="clock-title">
      <div class="card-header">
        <div>
          <div class="card-title" id="clock-title"><span class="ico">🕐</span> Current Time</div>
          <p class="card-desc">Device time reference</p>
        </div>
        <div class="status" aria-live="polite">
          <span class="dot blue" id="statusDot"></span>
          <span class="status-text" id="currentStatus">Device Off</span>
        </div>
      </div>
      <div class="clock">
        <div class="clock-time" id="clock">--:--:--</div>
        <div class="clock-date" id="date">Loading...</div>
      </div>
    </section>

  <!-- Light Control -->
  <section class="card" aria-labelledby="light-title">
      <div class="card-header">
        <div>
          <div class="card-title" id="light-title"><span class="ico">💡</span> Light Control</div>
          <p class="card-desc">Quick set current light color</p>
        </div>
      </div>

      <div class="btns" role="group" aria-label="Light color options">
        <button class="btn btn-wake" onclick="changeColor('green')" id="btnGreen"><span>🟢</span> Wake Up</button>
        <button class="btn btn-sleep" onclick="changeColor('red')" id="btnRed"><span>🔴</span> Sleep</button>
        <button class="btn btn-blue" onclick="changeColor('blue')" id="btnBlue"><span>🔵</span> Blue</button>
        <button class="btn btn-off" onclick="changeColor('off')" id="btnOff"><span>⚫</span> Turn Off</button>
      </div>

      <div class="toolbar">
        <button class="btn btn-outline" id="manualControlBtn" onclick="toggleManualMode()" title="Toggle Manual Control">
          <span class="ico">⚙️</span> <span id="manualControlText">Enable Manual</span>
        </button>
      </div>

      <!-- Brightness (independent of mode) -->
      <div style="margin-top:18px">
        <label for="brightnessSlider" class="label" style="color:var(--accent);display:flex;align-items:center;gap:8px"><span class="ico">🔆</span> Brightness: <span id="brightnessValue">--</span>%</label>
        <div class="brightness-wrapper" style="position:relative;height:46px;display:flex;align-items:center;">
          <input type="range" id="brightnessSlider" min="0" max="100" value="0" style="width:100%;appearance:none;height:16px;border-radius:999px;background:linear-gradient(90deg,var(--muted) 0%, var(--muted) 100%);outline:none;border:1px solid var(--border);padding:0;margin:0;"
            aria-label="Brightness" />
        </div>
      </div>
    </section>

    <!-- Schedule -->
    <section class="card" aria-labelledby="sched-title">
      <div>
        <div class="card-title" id="sched-title"><span class="ico">📅</span> Schedule Settings</div>
        <p class="card-desc">Define Green Windows. Outside of these, light is red.</p>
      </div>

      <form id="scheduleForm" novalidate>
        <div class="form-grid" id="greenWindowsContainer"></div>

        <div class="actions" style="margin-top: 4px; margin-bottom: 8px">
          <button type="button" class="btn btn-outline" id="addWindowBtn">
            <span class="ico">➕</span> Add Wake Up Window
          </button>
        </div>

        <div class="actions">
          <button type="submit" class="btn btn-wake" id="saveBtn">
            <span class="ico">💾</span> Save Schedule
          </button>
          <button type="button" class="btn btn-danger" onclick="clearSchedule()" id="clearBtn">
            <span class="ico">🗑️</span> Clear Settings
          </button>
        </div>
        <div class="flash" id="saveStatus" role="status" aria-live="polite"></div>
      </form>
    </section>

    <!-- Network -->
    <section class="card" aria-labelledby="net-title">
      <div class="card-header">
        <div>
          <div class="card-title" id="net-title"><span class="ico">⚙️</span> Network Settings</div>
          <p class="card-desc">Manage device Wi-Fi connection</p>
        </div>
      </div>

      <div class="status">
        <span class="ico" aria-hidden="true">📶</span>
        <div>
          <div class="status-text">Connected to Network</div>
          <div class="status-sub">Device is operating on current network</div>
        </div>
      </div>

      <div class="warning" role="note">
        <h3>Reset Wi-Fi Settings</h3>
        <p>This will delete saved network details and return the device to setup mode.</p>
        <button class="btn btn-danger" onclick="forgetWiFi()" id="wifiBtn">
          <span class="ico">📶</span> Forget Wi-Fi
        </button>
      </div>
    </section>
  </main>

  <footer>© 2025 Koren Halevie. All rights reserved.</footer>
</div>

<script>
let currentColor = 'off';
let isLoading = false;
let greenWindows = [];
let isManualMode = false;
let brightness = <!--BRIGHTNESS_PLACEHOLDER-->; // injected from server

const $ = (id) => document.getElementById(id);

document.addEventListener('DOMContentLoaded', () => {
  updateClock();
  setInterval(updateClock, 1000);

  // Initialize green windows from the parameter passed from the server
  const initialWindows = ')rawliteral" + greenWindows + R"rawliteral(';
  if (
    initialWindows &&
    initialWindows !== '' &&
    initialWindows.trim() !== ''
  ) {
    greenWindows = initialWindows.split(',').map((pair) => {
      const [start, end] = pair.trim().split('-');
      return { start, end };
    });
  } else {
    greenWindows = [];
  }
  renderGreenWindows();

  // Initialize manual mode and Manual Control button
  isManualMode = false;
  updateManualControlButton();

  // Check immediately if the color needs to be updated according to the current schedule
  if (greenWindows.length > 0) {
    checkAndUpdateColorBySchedule();
  }

  // Initialize color status from server parameter
  const initColor = ')rawliteral" + currentColor + R"rawliteral(';
  updateStatus((initColor && initColor !== '') ? initColor : 'off');

  // Initialize brightness slider
  setupBrightnessSlider();

  // To ensure the add button works at all times
  const addBtn = $('addWindowBtn');
  if (addBtn && !addBtn._bound) {
    addBtn.addEventListener('click', addGreenWindow);
    addBtn._bound = true;
  }
});

function updateClock(){
  const now = new Date();
  const time = now.toLocaleTimeString('en-US',{hour12:false, hour:'2-digit', minute:'2-digit', second:'2-digit'});
  const date = now.toLocaleDateString('en-US',{weekday:'long', year:'numeric', month:'long', day:'numeric'});
  $('clock').textContent = time;
  $('date').textContent = date;

  // === Automatic color calculation by schedule ===
  try {
    const minutesNow = now.getHours()*60 + now.getMinutes();
    let inGreen = false;
    for (let i=0;i<greenWindows.length;i++){
      const w = greenWindows[i];
      if (!w || !w.start || !w.end) continue;
      const [sh,sm] = (w.start||'00:00').split(':').map(Number);
      const [eh,em] = (w.end||'00:00').split(':').map(Number);
      const startMin = (sh*60 + sm);
      const endMin = (eh*60 + em);
      if (startMin === endMin) continue; // Empty range
      if (startMin < endMin) {
        if (minutesNow >= startMin && minutesNow < endMin) { inGreen = true; break; }
      } else {
        if (minutesNow >= startMin || minutesNow < endMin) { inGreen = true; break; }
      }
    }

    // Automatic update of the device according to the schedule - only if not in manual mode
    if (!isManualMode) {
      const target = inGreen ? 'green' : 'red';
      if (currentColor !== target) {
        changeColor(target, true); // true = automatic
      }
    }
  } catch (e) {
    console.error(e);
  }
}

function setAccentBy(color){
  let accent = getComputedStyle(document.documentElement).getPropertyValue('--time').trim();
  if(color === 'green') accent = getComputedStyle(document.documentElement).getPropertyValue('--wake').trim();
  else if(color === 'red') accent = getComputedStyle(document.documentElement).getPropertyValue('--sleep').trim();
  else if(color === 'blue') accent = getComputedStyle(document.documentElement).getPropertyValue('--time').trim();
  else accent = 'hsl(215, 15%, 55%)';
  document.documentElement.style.setProperty('--accent', accent);
}

function updateStatus(color){
  const statusTexts = { green:'Time to Wake Up! 🌅', red:'Sleep Time 😴', blue:'Blue Light 💙', off:'Device Off' };
  const dot = $('statusDot');
  dot.className = 'dot ' + (color || 'off');
  $('currentStatus').textContent = statusTexts[color] || 'Unknown';
  currentColor = color || 'off';
  setAccentBy(color);
  updateBrightnessGradient();
}

function setButtonLoading(buttonId, loading){
  const btn = $(buttonId);
  if(!btn) return;
  if(loading){
    btn.disabled = true;
    btn.innerHTML = '<span class="spinner"></span> Working...';
  } else {
    btn.disabled = false;
    const original = {
      btnGreen:'<span>🟢</span> Wake Up',
      btnRed:'<span>🔴</span> Sleep',
      btnBlue:'<span>🔵</span> Blue',
      btnOff:'<span>⚫</span> Turn Off'
    };
    btn.innerHTML = original[buttonId] || btn.innerHTML;
  }
}

// ------- Brightness Control -------
function setupBrightnessSlider(){
  const slider = $('brightnessSlider');
  const valueEl = $('brightnessValue');
  if(!slider || !valueEl) return;
  if (isNaN(brightness)) brightness = 100;
  slider.value = brightness;
  valueEl.textContent = brightness;
  updateBrightnessGradient();
  slider.addEventListener('input', ()=>{
    const val = Number(slider.value);
    valueEl.textContent = val;
    brightness = val;
    updateBrightnessGradient();
    // send continuously (debounced)
    scheduleBrightnessSend(val);
  });
}

function updateBrightnessGradient(){
  const slider = $('brightnessSlider');
  if(!slider) return;
  // Choose color based on currentColor and a soft starting tint
  let c = '#777777', start = 'rgba(120,120,120,0.05)';
  if(currentColor === 'green'){ c = 'hsl(120,70%,45%)'; start = 'rgba(0,255,0,0.12)'; }
  else if(currentColor === 'red'){ c = 'hsl(0,60%,55%)'; start = 'rgba(255,0,0,0.10)'; }
  else if(currentColor === 'blue'){ c = 'hsl(210,80%,58%)'; start = 'rgba(30,144,255,0.12)'; }
  const pct = brightness || 0;
  slider.style.background = `linear-gradient(90deg, ${start} 0%, ${c} ${pct}%, var(--muted) ${pct}%, var(--muted) 100%)`;
  slider.style.opacity = (pct === 0 ? 0.45 : 1);
}

let brightnessTimeout;
function scheduleBrightnessSend(val){
  if (brightnessTimeout) clearTimeout(brightnessTimeout);
  brightnessTimeout = setTimeout(()=> sendBrightness(val), 90); // faster debounce
}

async function sendBrightness(val){
  try {
    const res = await fetch('/setBrightness?value=' + encodeURIComponent(val));
    if(res.ok){
      // no toast spam; show only occasionally
      if (val === 0 || val === 100 || (val % 10 === 0)) {
        showToast('Brightness ' + val + '%','success');
      }
    }
  } catch(e){
    console.error('Brightness error', e);
  }
}

async function changeColor(color, isAutomatic = false){
  if(isLoading && !isAutomatic) return;
  isLoading = true;
  const map = { green:'btnGreen', red:'btnRed', blue:'btnBlue', off:'btnOff' };
  const id = map[color];
  if(id && !isAutomatic) setButtonLoading(id, true);

  // If manual button is pressed, switch to manual mode
  if (!isAutomatic) {
    isManualMode = true;
    updateManualControlButton();
  }
  
  try{
    const res = await fetch('/setColor?color=' + encodeURIComponent(color));
    if(res.ok){
      updateStatus(color);
      if (!isAutomatic) {
        showToast('Color changed successfully! Manual mode enabled ✨','success');
      }
    } else throw new Error('Request failed');
  } catch(err){
    console.error('Color change error:', err);
    if (!isAutomatic) {
      showToast('Failed to change color. Try again.','error');
    }
  } finally {
    if(id && !isAutomatic) setButtonLoading(id, false);
    isLoading = false;
  }
}

function toggleManualMode(){
  isManualMode = !isManualMode;
  updateManualControlButton();
  
  if (isManualMode) {
    showToast('Manual mode enabled. Colors won\'t change automatically.','info');
  } else {
    showToast('Auto mode enabled. Colors will follow schedule.','success');
    // Immediately update according to the current schedule
    checkAndUpdateColorBySchedule();
  }
}

function updateManualControlButton(){
  const btn = $('manualControlBtn');
  const text = $('manualControlText');
  if (!btn || !text) return;
  
  if (isManualMode) {
    btn.classList.remove('btn-outline');
    btn.classList.add('btn-wake');
    text.textContent = 'Auto Mode';
    btn.title = 'Switch back to automatic mode';
  } else {
    btn.classList.remove('btn-wake');
    btn.classList.add('btn-outline');
    text.textContent = 'Manual Mode';
    btn.title = 'Enable manual control';
  }
}

// ------- Schedule UI -------
function renderGreenWindows(){
  const container = $('greenWindowsContainer');
  container.innerHTML = '';
  greenWindows.forEach((w, i) => {
    const el = document.createElement('div');
    el.className = 'group';
    el.innerHTML = `
      <label class="label"><span class="ico">🌅</span> Wake Up Window #${i+1}</label>
      <div class="row">
        <div class="flex-1"><input type="time" class="input wake" value="${w.start}" data-idx="${i}" data-field="start" required></div>
        <span aria-hidden="true">—</span>
        <div class="flex-1"><input type="time" class="input wake" value="${w.end}" data-idx="${i}" data-field="end" required></div>
        <button type="button" class="btn btn-danger" data-remove="${i}" title="Delete window">🗑️</button>
      </div>
      <div class="help">You can span midnight (e.g., 22:00 → 06:30).</div>
    `;
    container.appendChild(el);
  });

  container.querySelectorAll('input[type="time"]').forEach(inp=>{
    inp.addEventListener('change', (e)=>{
      const idx = Number(e.target.getAttribute('data-idx'));
      const field = e.target.getAttribute('data-field');
      greenWindows[idx][field] = e.target.value;

      // Auto-save when changing time
      saveScheduleAutomatically();
    });
  });

  container.querySelectorAll('button[data-remove]').forEach(btn=>{
    btn.addEventListener('click',(e)=>{
      const idx = Number(e.currentTarget.getAttribute('data-remove'));
      greenWindows.splice(idx,1);
      renderGreenWindows();
    });
  });
}

function addGreenWindow(){
  greenWindows.push({start:'', end:''});
  renderGreenWindows();
}

function validateGreenWindows(allowEmpty = false){
  for(const w of greenWindows){
    // If empty values are allowed and both fields are empty, it's okay
    if(allowEmpty && (!w.start || w.start === '') && (!w.end || w.end === '')) continue;
    
    if(!w.start || !w.end) return { ok:false, msg:'Please fill start and end for each window.' };
    if(!/^\d{2}:\d{2}$/.test(w.start) || !/^\d{2}:\d{2}$/.test(w.end)) return { ok:false, msg:'Invalid time format. Use HH:MM.' };
  }
  return {ok:true};
}

function windowsToQueryParam(){
  return greenWindows
    .filter(w => w.start && w.end && w.start !== '' && w.end !== '') // Only valid windows
    .map(w => `${w.start}-${w.end}`)
    .join(',');
}

$('scheduleForm').addEventListener('submit', async (e)=>{
  e.preventDefault();
  if(isLoading) return;
  isLoading = true;
  const saveBtn = $('saveBtn');
  saveBtn.innerHTML = '<span class="spinner"></span> Saving...';
  saveBtn.disabled = true;

  const valid = validateGreenWindows();
  if(!valid.ok){
    showFlash(valid.msg,'error');
    saveBtn.innerHTML = '<span class="ico">💾</span> Save Schedule';
    saveBtn.disabled = false;
    isLoading = false;
    return;
  }

  try{
    const qs = encodeURIComponent(windowsToQueryParam());
    const res = await fetch(`/setSchedule?greenWindows=${qs}`);
    if(res.ok){
      showFlash('✔ Schedule saved successfully!','success');
      showToast('Schedule updated! 📅','success');
      // Check immediately if the current color needs to be changed
      checkAndUpdateColorBySchedule();
    } else throw new Error('Failed');
  } catch(err){
    console.error('Schedule save error:', err);
    showFlash('❌ Failed to save schedule','error');
  } finally {
    saveBtn.innerHTML = '<span class="ico">💾</span> Save Schedule';
    saveBtn.disabled = false;
    isLoading = false;
  }
});

async function clearSchedule(){
  if(isLoading) return;
  isLoading = true;
  const btn = $('clearBtn');
  btn.innerHTML = '<span class="spinner"></span> Clearing...';
  btn.disabled = true;
  try{
    const res = await fetch('/clearSchedule');
    if(res.ok){
      greenWindows = [];
      renderGreenWindows();
      showFlash('✔ Schedule cleared successfully!','success');
      showToast('Schedule cleared! 🗑️','success');
      // When clearing the schedule, the light should turn red
      if (currentColor !== 'red') {
        changeColor('red', true);
      }
    } else throw new Error('Failed');
  } catch(err){
    console.error('Schedule clear error:', err);
    showFlash('❌ Failed to clear schedule','error');
  } finally {
    btn.innerHTML = '<span class="ico">🗑️</span> Clear Settings';
    btn.disabled = false;
    isLoading = false;
  }
}

function forgetWiFi(){
  Swal.fire({
    title: 'Forget Wi-Fi?',
    text: 'Are you sure you want to remove saved Wi-Fi credentials?',
    icon: 'warning',
    showCancelButton: true,
    confirmButtonColor: 'hsl(0, 60%, 55%)',
    cancelButtonColor: 'hsl(210, 15%, 55%)',
    confirmButtonText: 'Yes, forget it',
    cancelButtonText: 'Cancel',
    background: 'var(--card)',
    color: 'var(--fg)'
  }).then(async (r)=>{
    if(r.isConfirmed){
      const wifiBtn = $('wifiBtn');
      wifiBtn.innerHTML = '<span class="spinner"></span> Resetting...';
      wifiBtn.disabled = true;
      try{
        await fetch('/forgetWiFi');
        Swal.fire({ title:'✅ Wi-Fi Cleared!', text:'Device is restarting...', icon:'success', showConfirmButton:false, background:'var(--card)', color:'var(--fg)' });
      } catch(e){
        Swal.fire({ title:'Error', text:'Failed to forget Wi-Fi.', icon:'error', background:'var(--card)', color:'var(--fg)' });
      } finally {
        wifiBtn.innerHTML = '<span class="ico">📶</span> Forget Wi-Fi';
        wifiBtn.disabled = false;
      }
    }
  });
}

async function saveScheduleAutomatically(){
  // Ensure fields are valid before saving (allows empty fields)
  const valid = validateGreenWindows(true);
  if(!valid.ok) return; // Don't save if there are errors

  try{
    const qs = encodeURIComponent(windowsToQueryParam());
    const res = await fetch(`/setSchedule?greenWindows=${qs}`);
    if(res.ok){
      // Check immediately if the current color needs to be changed
      checkAndUpdateColorBySchedule();
      // Small message indicating that the save was successful
      showToast('Schedule auto-saved ✓','success');
    }
  } catch(err){
    console.error('Auto-save error:', err);
  }
}

function checkAndUpdateColorBySchedule(){
  // only if not in manual state
  if (isManualMode) return;
  
  const now = new Date();
  const minutesNow = now.getHours()*60 + now.getMinutes();
  let inGreen = false;
  
  for (let i=0;i<greenWindows.length;i++){
    const w = greenWindows[i];
    if (!w || !w.start || !w.end) continue;
    const [sh,sm] = (w.start||'00:00').split(':').map(Number);
    const [eh,em] = (w.end||'00:00').split(':').map(Number);
    const startMin = (sh*60 + sm);
    const endMin = (eh*60 + em);
    if (startMin === endMin) continue;
    if (startMin < endMin) {
      if (minutesNow >= startMin && minutesNow < endMin) { inGreen = true; break; }
    } else {
      if (minutesNow >= startMin || minutesNow < endMin) { inGreen = true; break; }
    }
  }
  
  const targetColor = inGreen ? 'green' : 'red';
  if (currentColor !== targetColor) {
    changeColor(targetColor, true);
  }
}

function showFlash(message, type){
  const el = $('saveStatus');
  el.textContent = message;
  el.className = 'flash '+type+' show';
  setTimeout(()=>{ el.classList.remove('show'); }, 4200);
}

function showToast(message, type){
  const toast = Swal.mixin({
    toast: true, position: 'top-end', showConfirmButton: false, timer: 3000, timerProgressBar: true,
    background: 'var(--card)', color: 'var(--fg)',
    didOpen: t => { t.addEventListener('mouseenter', Swal.stopTimer); t.addEventListener('mouseleave', Swal.resumeTimer); }
  });
  const iconMap = { success: 'success', error: 'error', info: 'info' };
  toast.fire({ icon: iconMap[type] || 'info', title: message });
}
</script>
</body>
</html>
    )rawliteral";

    return html;
}
