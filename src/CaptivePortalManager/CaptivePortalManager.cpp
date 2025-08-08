#include "CaptivePortalManager.h"

CaptivePortalManager::CaptivePortalManager(const char *ssid, const IPAddress &localIP, const IPAddress &gatewayIP, const String &redirectURL)
    : server(80), ssid(ssid), localIP(localIP), gatewayIP(gatewayIP), subnetMask(255, 255, 255, 0), redirectURL(redirectURL)
{
    connectedMode = false;
}

void CaptivePortalManager::start()
{
    Serial.println("🚀 Starting Captive Portal...");

    WiFi.disconnect(true);
    delay(100);

    Serial.println("🔍 Scanning for WiFi networks...");
    int numNetworks = WiFi.scanNetworks();

    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(localIP, gatewayIP, subnetMask);
    WiFi.softAP(ssid);

    dnsServer.setTTL(3600);
    dnsServer.start(53, "*", localIP);

    connectedMode = (WiFi.status() == WL_CONNECTED);

    // ROOT PAGE
    server.on("/", HTTP_GET, [this, numNetworks](AsyncWebServerRequest *request) {
        if (connectedMode) {
            String html = "<html><body><h1>You're connected!</h1></body></html>";
            request->send(200, "text/html", html);
        } else {
            String html = generateWiFiSetupPage(numNetworks);
            request->send(200, "text/html", html);
        }
    });

    // SAVE WiFi CREDENTIALS
    server.on("/save", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
            String ssid = request->getParam("ssid", true)->value();
            String password = request->getParam("password", true)->value();
            saveWiFiCredentials(ssid, password);
            request->send(200, "text/plain", "✅ WiFi credentials saved! Restarting...");
            delay(2000);
            ESP.restart();
        } else {
            request->send(400, "text/plain", "❌ Missing SSID or Password");
        }
    });

    // CAPTIVE PORTAL DETECTION ROUTES
    server.on("/generate_204", [](AsyncWebServerRequest *request) {
        request->redirect("http://4.3.2.1");
      });
      

    server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", "OK");
    });

    server.on("/ncsi.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Microsoft NCSI");
    });

    server.on("/connecttest.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Success");
    });

    server.on("/success.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Success");
    });

    server.on("/captiveportal", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->redirect("/");
    });

    // CATCH-ALL: FORCE REDIRECT
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->redirect("http://4.3.2.1");
        Serial.print("onnotfound ");
        Serial.print(request->host());
        Serial.print(" ");
        Serial.print(request->url());
        Serial.println(" sent redirect to http://4.3.2.1");
      });
      

    server.begin();
    Serial.println("✅ Captive Portal Started at 192.168.4.1");
}

void CaptivePortalManager::processDNSRequests()
{
    dnsServer.processNextRequest();
}

void CaptivePortalManager::saveWiFiCredentials(const String &ssid, const String &password)
{
    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.end();
}

bool CaptivePortalManager::loadWiFiCredentials(String &ssid, String &password)
{
    preferences.begin("wifi", true);
    ssid = preferences.getString("ssid", "");
    password = preferences.getString("password", "");
    preferences.end();
    return !ssid.isEmpty();
}

bool CaptivePortalManager::connectToWiFi()
{
    WiFi.mode(WIFI_STA);
    String ssid, password;

    if (loadWiFiCredentials(ssid, password)) {
        Serial.printf("📶 Trying to connect to WiFi: %s\n", ssid.c_str());
        WiFi.begin(ssid.c_str(), password.c_str());

        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
            delay(500);
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\n✅ Connected to WiFi!");
            Serial.print("📡 IP Address: ");
            Serial.println(WiFi.localIP());
            return true;
        } else {
            Serial.println("\n❌ Failed to connect within timeout.");
        }
    } else {
        Serial.println("⚠️ No saved WiFi credentials found.");
    }

    Serial.println("🔄 Switching to AP mode and starting Captive Portal...");
    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_AP_STA);
    start();
    return false;
}

String CaptivePortalManager::generateWiFiSetupPage(int numNetworks) {
    // Build networks JSON
    String networksJson = "[";
    if (numNetworks > 0) {
        for (int i = 0; i < numNetworks; i++) {
            if (i > 0) networksJson += ",";
            networksJson += "{";
            networksJson += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
            networksJson += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
            networksJson += "\"open\":" + String(WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "true" : "false");
            networksJson += "}";
        }
    }
    networksJson += "]";

    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8" />
<meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover" />
<title>OK TO WAKE – Wi‑Fi Setup</title>
<meta name="theme-color" content="#0ea5a5" />
<script src="https://cdn.jsdelivr.net/npm/sweetalert2@11"></script>
<style>
:root{
--bg-0:hsl(250,50%,98%);
--bg-1:hsl(230,45%,97%);
--card:#fff;
--fg:hsl(210,20%,18%);
--muted:hsl(220,20%,94%);
--muted-fg:hsl(215,15%,50%);
--border:hsl(220,16%,88%);
--ring:hsl(210,100%,65%);
--wake:hsl(120,70%,45%);
--sleep:hsl(0,60%,55%);
--time:hsl(210,80%,58%);
--accent:var(--time);
--accent-weak:hsl(210,60%,90%);
--g-aurora:radial-gradient(60% 60% at 20% 20%,hsl(180,90%,90%) 0%,transparent 60%),
radial-gradient(50% 50% at 80% 30%,hsl(260,90%,92%) 0%,transparent 60%),
radial-gradient(40% 40% at 60% 80%,hsl(120,90%,90%) 0%,transparent 60%);
--g-surface:linear-gradient(145deg,#fff,hsl(220,20%,98%));
--g-accent:linear-gradient(135deg,var(--accent),hsl(210,100%,70%));
--radius-lg:18px;
--radius-md:12px;
--radius-sm:10px;
--shadow-soft:0 8px 24px hsl(215 30% 20% / .08);
--shadow-float:0 12px 36px hsl(215 30% 20% / .14);
--ease:cubic-bezier(.22,1,.36,1);
--t-fast:180ms var(--ease);
--t-med:280ms var(--ease)
}
*{box-sizing:border-box}
html,body{height:100%}
body{margin:0;font-family:ui-sans-serif,system-ui,-apple-system,Segoe UI,Roboto,Noto Sans,Ubuntu,Cantarell,Arial,"Apple Color Emoji","Segoe UI Emoji";
background:linear-gradient(180deg,var(--bg-0),var(--bg-1)),var(--g-aurora);color:var(--fg);line-height:1.55;-webkit-tap-highlight-color:transparent}
.aurora{position:fixed;inset:-20vmax;background:var(--g-aurora);filter:blur(40px) saturate(120%);opacity:.5;pointer-events:none;animation:float 20s ease-in-out infinite alternate}
@keyframes float{0%{transform:translate3d(-2%,-1%,0) scale(1)}100%{transform:translate3d(2%,1%,0) scale(1.02)}}
@media (prefers-reduced-motion:reduce){.aurora{animation:none}}
.container{max-width:860px;margin:0 auto;padding:clamp(16px,3.5vw,32px)}
header{text-align:center;margin-bottom:clamp(16px,6vw,28px)}
.brand{display:flex;justify-content:center;gap:12px;margin-bottom:10px;opacity:.9}
.brand .ico{font-size:clamp(20px,5vw,28px)}
.title{font-size:clamp(26px,6vw,40px);font-weight:800;letter-spacing:.4px;background:var(--g-accent);
-webkit-background-clip:text;background-clip:text;-webkit-text-fill-color:transparent;filter:drop-shadow(0 2px 10px hsl(210 90% 85% / .35));margin:0 0 4px}
.subtitle{color:var(--muted-fg);font-size:clamp(14px,3.6vw,16px);margin:0 auto 8px}
.divider{width:88px;height:4px;margin:10px auto 0;background:var(--g-accent);border-radius:999px;box-shadow:0 0 30px hsl(210 100% 80% / .35);opacity:.95}
.card{background:var(--g-surface);border:1px solid var(--border);border-radius:var(--radius-lg);box-shadow:var(--shadow-soft);
padding:clamp(16px,4vw,22px);backdrop-filter:saturate(120%) blur(8px)}
.card + .card{margin-top:clamp(12px,3vw,16px)}
.section-title{display:flex;align-items:center;gap:10px;font-weight:800;font-size:clamp(16px,4vw,18px)}
.section-sub{color:var(--muted-fg);font-size:clamp(12px,3vw,13px);margin-top:4px}
.toolbar{display:flex;gap:10px;flex-wrap:wrap;margin-top:12px}
.toolbar .grow{flex:1;min-width:0}
.input{width:100%;padding:12px 14px;font-size:16px;border:2px solid var(--border);background:#fff;border-radius:12px;color:var(--fg);outline:none;
transition:border-color var(--t-fast),box-shadow var(--t-fast)}
.input:focus{border-color:var(--accent);box-shadow:0 0 0 3px hsl(210 100% 60% / .12)}
.btn{display:inline-flex;align-items:center;justify-content:center;gap:8px;padding:12px 14px;border:2px solid var(--border);background:#fff;color:var(--fg);
border-radius:12px;font-weight:800;min-height:44px;box-shadow:0 1px 0 hsl(0 0% 100% / .8) inset,0 8px 20px hsl(215 30% 20% / .06);
transition:transform var(--t-fast),box-shadow var(--t-fast),background var(--t-fast),border-color var(--t-fast);white-space:nowrap}
.btn:hover{transform:translateY(-1px)}
.btn:active{transform:translateY(0)}
.btn:disabled{opacity:.6;cursor:not-allowed}
.btn-accent{background:var(--g-accent);color:#fff;border-color:hsl(210,80%,56%)}
.btn-outline{background:transparent}
.list{margin-top:12px;border:1px solid var(--border);border-radius:14px;overflow:hidden;background:#fff}
.item{display:flex;align-items:center;gap:12px;padding:12px 14px;border-bottom:1px solid var(--border);cursor:pointer;transition:background var(--t-fast)}
.item:hover{background:hsl(0 0% 98%)}
.item:last-child{border-bottom:none}
.ssid{font-weight:700;word-break:break-all;flex:1}
.meta{margin-left:auto;display:flex;align-items:center;gap:8px;color:var(--muted-fg);font-size:12px;flex-shrink:0}
.lock{font-size:14px}
.bars{display:inline-grid;grid-template-columns:repeat(4,5px);gap:2px;align-items:end}
.bar{width:5px;background:var(--accent-weak);border-radius:2px;height:4px;opacity:.45}
.bar.on{background:var(--accent);opacity:1}
.bar.b1{height:6px}.bar.b2{height:9px}.bar.b3{height:12px}.bar.b4{height:15px}
.skeleton{height:56px;display:block;background:linear-gradient(90deg, #f3f4f6, #eceff3, #f3f4f6);
background-size:200% 100%;animation:shimmer 1.2s infinite}
@keyframes shimmer{0%{background-position:200% 0}100%{background-position:-200% 0}}
.pair{display:grid;gap:10px;grid-template-columns:1fr}
.row{display:flex;gap:10px;align-items:center;flex-wrap:wrap}
.password-group{display:flex;flex-direction:column;gap:10px}
.password-row{display:flex;gap:10px;align-items:end}
.password-input{flex:1;min-width:0}
.switch{display:flex;align-items:center;gap:8px;white-space:nowrap;height:44px}
.helper{color:var(--muted-fg);font-size:12px}
footer{text-align:center;color:var(--muted-fg);font-size:13px;padding:18px 0 6px}

@media (max-width: 480px){
.toolbar{flex-direction:column}
.toolbar .grow{min-width:unset}
.btn{font-size:15px;padding:10px 12px}
.password-row{flex-direction:column;align-items:stretch}
.password-input{width:100%}
.switch{justify-content:center;height:auto;padding:8px 0}
.meta{gap:6px;font-size:11px}
.bars{grid-template-columns:repeat(4,4px);gap:1px}
.bar{width:4px}
}
</style>
</head>
<body>
<div class="aurora" aria-hidden="true"></div>
<div class="container">
<header>
<div class="brand" aria-hidden="true"><span class="ico">📶</span><span class="ico">✨</span><span class="ico">🔒</span></div>
<h1 class="title">Wi‑Fi Setup</h1>
<p class="subtitle">Connect your OK TO WAKE device to a Wi‑Fi network</p>
<div class="divider"></div>
</header>

<!-- Networks -->
<section class="card">
<div class="section-title"><span>📡</span> Available Networks</div>
<p class="section-sub">Tap a network to select. Use Refresh to rescan.</p>
<div class="toolbar">
  <input id="search" class="input grow" type="text" placeholder="Search SSID…" />
  <button id="refresh" class="btn btn-outline"><span>🔄</span> Refresh</button>
  <a href="/" class="btn btn-outline"><span>🏠</span> Dashboard</a>
</div>

<div id="list" class="list" role="listbox" aria-label="Wi‑Fi networks">
  <div class="skeleton"></div>
  <div class="skeleton"></div>
  <div class="skeleton"></div>
</div>
</section>

<!-- Connect -->
<section class="card">
<div class="section-title"><span>🔗</span> Connect</div>
<p class="section-sub">Selected SSID is filled automatically; you can also enter a hidden network.</p>
<div class="pair" style="margin-top:10px">
  <label class="helper">SSID</label>
  <input id="ssid" class="input" type="text" placeholder="Select from list or type manually" autocomplete="username" />
</div>

<div class="password-group" style="margin-top:10px">
  <label class="helper">Password</label>
  <div class="password-row">
    <div class="password-input">
      <input id="password" class="input" type="password" placeholder="Enter password (if required)" autocomplete="current-password" />
    </div>
    <label class="switch"><input id="showPass" type="checkbox" /> Show</label>
  </div>
</div>

<div class="row" style="margin-top:12px; display:flex; justify-content:center;">
  <button id="connect" class="btn btn-accent">
    <span>✅</span> Connect
  </button>
</div>

<p class="helper" style="margin-top:6px">Open networks don't require a password.</p>
</section>

<footer>© 2025 Koren Halevie. All rights reserved.</footer>
</div>

<script>
// Server injects networks JSON here:
const INJECTED_NETWORKS = ')rawliteral" + networksJson + R"rawliteral(';

const $ = (id) => document.getElementById(id);
const listEl = $('list'), searchEl = $('search'), ssidEl = $('ssid'), passEl = $('password');
const showPassEl = $('showPass'), refreshEl = $('refresh'), connectEl = $('connect');

let networks = [];
let selected = null;

document.addEventListener('DOMContentLoaded', async () => {
  showPassEl.addEventListener('change', () => {
    passEl.type = showPassEl.checked ? 'text' : 'password';
  });
  refreshEl.addEventListener('click', scanNetworks);
  connectEl.addEventListener('click', connectWifi);
  searchEl.addEventListener('input', () => renderList(filterNetworks(searchEl.value)));
  await loadInitial();
});

async function loadInitial(){
  const injected = parseInjected(INJECTED_NETWORKS);
  if (injected.length) {
    networks = normalize(injected);
    renderList(networks);
  } else {
    await scanNetworks();
  }
}

function parseInjected(str){
  try {
    if (!str || str === 'REPLACE_WIFI_NETWORKS_JSON') return [];
    const data = JSON.parse(str);
    return Array.isArray(data) ? data : [];
  } catch {
    return [];
  }
}

function normalize(arr){
  return arr
    .filter(n => n && n.ssid)
    .map(n => ({
      ssid: String(n.ssid),
      rssi: Number(n.rssi ?? -100),
      open: !!(n.open ?? n.isOpen ?? n.secure === false)
    }));
}

function filterNetworks(q){
  if (!q) return networks;
  q = q.toLowerCase();
  return networks.filter(n => n.ssid.toLowerCase().includes(q));
}

function rssiToBars(rssi){
  if (rssi >= -55) return 4;
  if (rssi >= -65) return 3;
  if (rssi >= -75) return 2;
  if (rssi >= -85) return 1;
  return 0;
}

function renderList(items){
  listEl.innerHTML = '';
  if (!items.length) {
    listEl.innerHTML = `<div class="item" aria-disabled="true"><span>😕</span> No networks found</div>`;
    return;
  }
  
  items.forEach((n, idx) => {
    const li = document.createElement('div');
    li.className = 'item';
    li.setAttribute('role','option');
    li.setAttribute('aria-selected', selected?.ssid === n.ssid ? 'true' : 'false');
    const bars = rssiToBars(n.rssi);
    li.innerHTML = `
      <div class="ssid">${escapeHtml(n.ssid)}</div>
      <div class="meta">
        <span class="lock" title="${n.open ? 'Open' : 'Secured'}">${n.open ? '🔓' : '🔒'}</span>
        <div class="bars" aria-label="Signal strength" title="Signal: ${n.rssi} dBm">
          ${[1,2,3,4].map(i => `<span class="bar b${i} ${i<=bars?'on':''}"></span>`).join('')}
        </div>
      </div>
    `;
    li.addEventListener('click', () => {
      selected = n;
      ssidEl.value = n.ssid;
      if (n.open) {
        passEl.value = '';
        passEl.disabled = true;
        passEl.placeholder = 'Open network';
      } else {
        passEl.disabled = false;
        passEl.placeholder = 'Enter password';
      }
      Array.from(listEl.children).forEach(c => c.setAttribute('aria-selected','false'));
      li.setAttribute('aria-selected','true');
    });
    listEl.appendChild(li);
  });
}

async function scanNetworks(){
  setLoading(refreshEl, true, 'Scanning…');
  listEl.innerHTML = '<div class="skeleton"></div><div class="skeleton"></div><div class="skeleton"></div>';
  
  // Try to rescan - reload the page to get fresh scan
  try {
    window.location.reload();
  } catch(e) {
    console.error('Scan error:', e);
    setLoading(refreshEl, false, 'Refresh');
  }
}

function setLoading(btn, loading, textWhenLoading){
  if (!btn) return;
  if (loading) {
    btn.disabled = true;
    btn.innerHTML = `<span style="width:18px;height:18px;border:2px solid transparent;border-top-color:currentColor;border-radius:999px;animation:spin 1s linear infinite;display:inline-block"></span> ${textWhenLoading}`;
  } else {
    btn.disabled = false;
    if (btn === refreshEl) {
      btn.innerHTML = `<span>🔄</span> Refresh`;
    } else if (btn === connectEl) {
      btn.innerHTML = `<span>✅</span> Connect`;
    }
  }
}

async function connectWifi(){
  const ssid = ssidEl.value.trim();
  const pass = passEl.value;
  
  if (!ssid) {
    toast('Please select or enter an SSID','info');
    return;
  }
  
  setLoading(connectEl, true, 'Connecting…');
  
  try {
    const formData = new FormData();
    formData.append('ssid', ssid);
    formData.append('password', pass);
    
    const res = await fetch('/save', {
      method: 'POST',
      body: formData
    });
    
    const responseText = await res.text();
    console.log('Server response:', responseText);
    
    if (res.ok) {
      await Swal.fire({
        title:'🔗 Connected!',
        text:'Device will attempt to join the network. It may restart.',
        icon:'success',
        timer:2200,
        showConfirmButton:false,
        background:'#fff',
        color:'var(--fg)'
      });
      // Don't reset the button since device will restart
    } else {
      throw new Error(`Server responded with: ${responseText}`);
    }
  } catch(e){
    console.error('Connection error:', e);
    Swal.fire({
      title:'Error',
      text:'Failed to connect. Check credentials and try again.',
      icon:'error',
      background:'#fff',
      color:'var(--fg)'
    });
    setLoading(connectEl, false, 'Connect');
  }
}

function toast(message, type='info'){
  const t = Swal.mixin({
    toast:true,
    position:'top-end',
    showConfirmButton:false,
    timer:2600,
    timerProgressBar:true,
    background:'#fff',
    color:'var(--fg)'
  });
  t.fire({
    icon:type,
    title:message
  });
}

function escapeHtml(s){
  return s.replace(/[&<>"']/g, m => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#039;'}[m]));
}
</script>
</body>
</html>
    )rawliteral";

    return html;
}
