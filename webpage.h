#ifndef WEBPAGE_H
#define WEBPAGE_H

const char webpageHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>AirDimmer Dashboard</title>
  <style>
    :root {
      /* Theme: Light (Default) */
      --bg-gradient: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%);
      --card-bg: rgba(255, 255, 255, 0.7);
      --card-border: rgba(255, 255, 255, 0.3);
      --text-main: #2d3436;
      --text-dim: #636e72;
      --primary: #0984e3;
      --primary-hover: #74b9ff;
      --accent: #00b894;
      --danger: #d63031;
      --shadow: 0 8px 32px 0 rgba(31, 38, 135, 0.15);
      --glass-blur: blur(12px);
    }

    [data-theme="dark"] {
      --bg-gradient: linear-gradient(135deg, #1e272e 0%, #2f3640 50%, #1e272e 100%);
      --card-bg: rgba(25, 25, 25, 0.4);
      --card-border: rgba(255, 255, 255, 0.08);
      --text-main: #f5f6fa;
      --text-dim: #a4b0be;
      --primary: #ffb142;
      --primary-hover: #ff9f43;
      --shadow: 0 8px 32px 0 rgba(0, 0, 0, 0.7);
    }

    * { box-sizing: border-box; transition: all 0.3s ease; }
    
    body { 
      font-family: 'Inter', -apple-system, sans-serif; 
      margin: 0; min-height: 100vh;
      background: var(--bg-gradient); 
      background-attachment: fixed;
      color: var(--text-main);
      display: flex; justify-content: center; align-items: flex-start;
      padding: 20px;
    }

    .container { 
      width: 100%; max-width: 480px;
      background: var(--card-bg);
      backdrop-filter: var(--glass-blur);
      -webkit-backdrop-filter: var(--glass-blur);
      border: 1px solid var(--card-border);
      border-radius: 24px;
      padding: 30px;
      box-shadow: var(--shadow);
    }

    header { 
      display: flex; flex-direction: column; align-items: center; 
      margin-bottom: 30px; position: relative;
    }

    .controls-top {
      position: absolute; top: 0; right: 0; display: flex; gap: 10px;
    }

    .icon-btn {
      background: var(--card-bg); border: 1px solid var(--card-border);
      border-radius: 12px; width: 40px; height: 40px;
      cursor: pointer; display: flex; align-items: center; justify-content: center;
      font-size: 18px; color: var(--text-main);
    }
    .icon-btn:hover { background: var(--primary); color: white; transform: translateY(-2px); }

    h1 { margin: 10px 0; font-size: 26px; font-weight: 800; background: linear-gradient(to right, var(--primary), var(--accent)); -webkit-background-clip: text; -webkit-text-fill-color: transparent; }
    
    .status-bar { display: flex; gap: 10px; margin-bottom: 25px; }
    .status-badge { padding: 6px 14px; border-radius: 30px; font-size: 11px; font-weight: 700; text-transform: uppercase; display: flex; align-items: center; gap: 6px; letter-spacing: 0.5px; border: 1px solid transparent; }
    .status-badge.connected { background: rgba(0, 184, 148, 0.15); color: #00b894; border-color: rgba(0, 184, 148, 0.2); }
    .status-badge.disconnected { background: rgba(214, 48, 49, 0.15); color: #d63031; border-color: rgba(214, 48, 49, 0.2); }
    .dot { width: 8px; height: 8px; border-radius: 50%; box-shadow: 0 0 8px currentColor; }

    .card { background: rgba(255,255,255,0.05); border-radius: 18px; padding: 20px; margin-bottom: 20px; border: 1px solid var(--card-border); }
    .stat-row { display: flex; justify-content: space-between; align-items: center; margin-bottom: 12px; }
    .stat-row:last-child { margin-bottom: 0; }
    .label { font-size: 14px; font-weight: 600; color: var(--text-dim); }
    .value { font-family: 'JetBrains Mono', monospace; font-size: 16px; font-weight: 700; color: var(--text-main); }
    
    .indicator { height: 12px; width: 12px; border-radius: 50%; background: #bbb; display: inline-block; margin-left: 8px; }
    .indicator.active { background: var(--danger); box-shadow: 0 0 12px var(--danger); animation: pulse 1.5s infinite; }
    @keyframes pulse { 0% { opacity: 1; } 50% { opacity: 0.5; } 100% { opacity: 1; } }

    .progress-container { margin-top: 15px; }
    .progress-label-bar { display: flex; justify-content: space-between; font-size: 13px; margin-bottom: 8px; font-weight: 700; }
    .bar-bg { width: 100%; height: 14px; background: rgba(0,0,0,0.1); border-radius: 10px; overflow: hidden; }
    .bar-fill { height: 100%; background: linear-gradient(90deg, var(--primary), var(--accent)); width: 0%; border-radius: 10px; }

    h3 { font-size: 16px; margin: 0 0 15px 0; font-weight: 700; color: var(--text-main); display: flex; align-items: center; gap: 8px; }

    /* Controls */
    .switch { position: relative; display: inline-block; width: 44px; height: 24px; }
    .switch input { opacity: 0; width: 0; height: 0; }
    .slider { position: absolute; cursor: pointer; inset: 0; background: #dfe6e9; border-radius: 34px; border: 1px solid rgba(0,0,0,0.05); }
    .slider:before { position: absolute; content: ""; height: 18px; width: 18px; left: 2px; bottom: 2px; background: white; border-radius: 50%; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
    input:checked + .slider { background: var(--primary); }
    input:checked + .slider:before { transform: translateX(20px); }

    input[type="number"] { width: 100px; }
    input[type="number"], input[type="text"] { 
      background: rgba(0,0,0,0.05); border: 1px solid var(--card-border); border-radius: 8px; color: var(--text-main); padding: 6px 10px; font-family: monospace; outline: none; transition: 0.2s;
    }
    input:focus { border-color: var(--primary); box-shadow: 0 0 0 3px rgba(116, 185, 255, 0.2); }

    /* Segment Control for Sensitivity */
    .segment-control {
      display: flex; background: rgba(0,0,0,0.05); border-radius: 12px; padding: 4px; gap: 4px;
    }
    .segment-btn {
      flex: 1; border: none; background: none; padding: 6px; border-radius: 8px;
      font-size: 11px; font-weight: 700; cursor: pointer; color: var(--text-dim);
      transition: 0.2s;
    }
    .segment-btn.active {
      background: var(--primary); color: white; box-shadow: 0 2px 8px rgba(0,0,0,0.1);
    }
    .segment-btn:hover:not(.active) { background: rgba(0,0,0,0.05); }

    .btn { 
      padding: 12px 20px; border-radius: 12px; border: none; font-weight: 700; font-size: 14px; cursor: pointer; width: 100%; display: flex; align-items: center; justify-content: center; gap: 8px;
    }
    .btn-primary { background: var(--primary); color: white; }
    .btn-primary:hover { background: var(--primary-hover); transform: translateY(-2px); box-shadow: 0 5px 15px rgba(9, 132, 227, 0.3); }
    .btn-danger { background: rgba(214, 48, 49, 0.1); color: var(--danger); border: 1px solid rgba(214, 48, 49, 0.2); }
    .btn-danger:hover { background: var(--danger); color: white; }

    .small-text { font-size: 12px; color: var(--text-dim); margin-top: 5px; }

    /* Footer for language/credits */
    footer { margin-top: 25px; font-size: 11px; color: var(--text-dim); font-weight: 600; opacity: 0.8; line-height: 1.6; }
    footer a { color: var(--text-dim); text-decoration: none; border-bottom: 1px solid transparent; }
    footer a:hover { color: var(--primary); border-bottom-color: var(--primary); }
  </style>
</head>
<body data-theme="light">

<div class="container">
  <header>
    <div class="controls-top">
      <button class="icon-btn" onclick="toggleLanguage()" title="Switch Language" id="lang-toggle">🇮🇹</button>
      <button class="icon-btn" onclick="toggleTheme()" id="theme-icon">🌙</button>
    </div>
    <h1 id="t-title">AirDimmer</h1>
    <div class="status-bar">
      <div id="wifi-status" class="status-badge disconnected">
        <div class="dot"></div><span>WiFi</span>
      </div>
      <div id="mqtt-status" class="status-badge disconnected">
        <div class="dot"></div><span>MQTT</span>
      </div>
    </div>
  </header>

  <!-- Live Stats -->
  <div class="card">
    <div class="stat-row">
      <span class="label" id="t-hand">Hand Detected</span>
      <span class="value"><span id="hand-text">NO</span><span id="hand-indicator" class="indicator"></span></span>
    </div>
    <div class="stat-row">
      <span class="label" id="t-raw">Raw Distance</span>
      <span class="value"><span id="raw-dist">--</span> cm</span>
    </div>
    <div class="stat-row">
      <span class="label" id="t-last-change">Last Change</span>
      <span class="value" id="last-change">---</span>
    </div>
    
    <div class="progress-container">
      <div class="progress-label-bar">
        <span id="t-brightness">Brightness</span>
        <span id="bright-val">0%</span>
      </div>
      <div class="bar-bg"><div class="bar-fill" id="bright-bar"></div></div>
    </div>
  </div>

  <!-- Settings -->
  <div class="card">
    <h3 id="t-settings">Settings</h3>
    
    <div class="stat-row">
      <span class="label" id="t-armed">Enable Control Light</span>
      <label class="switch">
        <input type="checkbox" id="armed-chk" onchange="toggleArmed()">
        <span class="slider"></span>
      </label>
    </div>

    <div class="stat-row">
      <span class="label" id="t-invert">Invert Direction</span>
      <label class="switch">
        <input type="checkbox" id="invert-chk" onchange="toggleInvert()">
        <span class="slider"></span>
      </label>
    </div>

    <!-- Sensitivity Setting -->
    <div style="margin-top: 15px;">
      <div class="stat-row" style="margin-bottom: 8px;">
        <span class="label" id="t-sensitivity">Sensitivity</span>
        <span class="value" id="sensitivity-val">1</span>
      </div>
      <div class="segment-control">
        <button class="segment-btn active" id="s1" onclick="updateSensitivity(1)">1</button>
        <button class="segment-btn" id="s2" onclick="updateSensitivity(2)">2</button>
        <button class="segment-btn" id="s3" onclick="updateSensitivity(3)">3</button>
      </div>
      <p class="small-text" id="t-sens-help">1: High, 3: Low</p>
    </div>

    <div class="stat-row">
      <span class="label" id="t-update-raw">Update Raw Readings</span>
      <label class="switch">
        <input type="checkbox" id="raw-chk" onchange="toggleRawMeasurements()">
        <span class="slider"></span>
      </label>
    </div>

    <div class="stat-row" style="margin-top: 20px;">
      <span class="label" id="t-upper">Upper Threshold</span>
      <input type="number" id="upper-threshold" min="0" max="1" step="0.01" onchange="updateThresholds()">
    </div>
    <div class="stat-row">
      <span class="label" id="t-lower">Lower Threshold</span>
      <input type="number" id="lower-threshold" min="0" max="1" step="0.01" onchange="updateThresholds()">
    </div>

    <div style="margin-top: 15px; padding-top: 15px; border-top: 1px solid var(--card-border);">
      <div class="stat-row">
        <span class="label" id="t-surface">Surface Distance</span>
        <span class="value"><span id="surface-dist">--</span> cm</span>
      </div>
      <button class="btn btn-primary" onclick="calibrateThreshold()" style="margin-top: 10px;">
        <span id="t-recalibrate">Recalibrate Distance</span>
      </button>
    </div>
  </div>

  <!-- Device Config -->
  <div class="card">
    <h3 id="t-dev-name">Device Name</h3>
    <p class="small-text" id="t-dev-help">Sets your .local address and MQTT topics.</p>
    <div class="stat-row" style="margin-top: 10px;">
      <span class="label">airdimmer-</span>
      <div style="display: flex; gap: 8px;">
        <input type="text" id="device-suffix" maxlength="30" style="width: 100px;">
        <button onclick="updateHostname()" class="btn btn-danger" style="width: auto; padding: 6px 12px; font-size: 12px;">Save</button>
      </div>
    </div>
    <p class="small-text">Address: <b id="full-address">airdimmer-setup.local</b></p>
  </div>

  <center><footer>
    AirDimmer v1.0 • 2026<br>
    <a href="https://github.com/Andrea-Fox/AirDimmer" target="_blank" id="t-github">GitHub</a> • 
    <span id="t-credits">Created by Andrea Fox</span>
  </footer></center>
</div>

<script>
  let currentLang = localStorage.getItem('lang') || 'en';
  let currentTheme = localStorage.getItem('theme') || 'light';
  let isFastPolling = false;

  const translations = {
    en: {
      title: "AirDimmer",
      hand: "Hand Detected",
      raw: "Current Distance",
      lastChange: "Last Change Sent",
      brightness: "Brightness",
      settings: "Settings",
      armed: "Enable Light Control",
      invert: "Invert Direction",
      updateRaw: "Send updated measurements",
      upper: "Upper Threshold",
      lower: "Lower Threshold",
      surface: "Surface Distance",
      recalibrate: "Recalibrate Distance",
      devName: "Device Name",
      devHelp: "Sets your .local address and MQTT topics. Device will reboot.",
      rebootPrompt: "Device will rename and reboot. Continue?",
      calPrompt: "Recalibrate distance? Make sure nothing is near the sensor.",
      waitMsg: "Saving and rebooting... Please wait.",
      github: "GitHub",
      credits: "Created by Andrea Fox",
      sensitivity: "Sensitivity",
      sensHelp: "1: High, 3: Low",
      langKey: "🇮🇹"
    },
    it: {
      title: "AirDimmer",
      hand: "Mano Rilevata",
      raw: "Distanza attuale",
      lastChange: "Ultimo Cambio",
      brightness: "Luminosità",
      settings: "Impostazioni",
      armed: "Abilita Controllo Luci",
      invert: "Inverti Direzione",
      updateRaw: "Invia aggiornamenti letture",
      upper: "Soglia Superiore",
      lower: "Soglia Inferiore",
      surface: "Distanza Superficie",
      recalibrate: "Ricalibra Distanza",
      devName: "Nome Dispositivo",
      devHelp: "Imposta l'indirizzo .local e i topic MQTT. Il dispositivo si riavvierà.",
      rebootPrompt: "Il dispositivo cambierà nome e si riavvierà. Continuare?",
      calPrompt: "Ricalibrare la distanza? Assicurati che non ci sia nulla vicino al sensore.",
      waitMsg: "Salvataggio e riavvio... Attendere.",
      github: "GitHub",
      credits: "Creato da Andrea Fox",
      sensitivity: "Sensibilità",
      sensHelp: "1: Alta, 3: Bassa",
      langKey: "🇬🇧"
    }
  };

  function updateUI() {
    const t = translations[currentLang];
    document.getElementById('t-title').innerText = t.title;
    document.getElementById('t-hand').innerText = t.hand;
    document.getElementById('t-raw').innerText = t.raw;
    document.getElementById('t-last-change').innerText = t.lastChange;
    document.getElementById('t-brightness').innerText = t.brightness;
    document.getElementById('t-settings').innerText = t.settings;
    document.getElementById('t-armed').innerText = t.armed;
    document.getElementById('t-invert').innerText = t.invert;
    document.getElementById('t-update-raw').innerText = t.updateRaw;
    document.getElementById('t-upper').innerText = t.upper;
    document.getElementById('t-lower').innerText = t.lower;
    document.getElementById('t-surface').innerText = t.surface;
    document.getElementById('t-recalibrate').innerText = t.recalibrate;
    document.getElementById('t-dev-name').innerText = t.devName;
    document.getElementById('t-dev-help').innerText = t.devHelp;
    document.getElementById('t-github').innerText = t.github;
    document.getElementById('t-credits').innerText = t.credits;
    document.getElementById('t-sensitivity').innerText = t.sensitivity;
    document.getElementById('t-sens-help').innerText = t.sensHelp;
    document.getElementById('lang-toggle').innerText = t.langKey;
    
    document.body.setAttribute('data-theme', currentTheme);
    document.getElementById('theme-icon').innerText = currentTheme === 'light' ? '🌙' : '☀️';
  }

  function updateSensitivity(val) {
    fetch('/update/sensitivity?val=' + val)
      .then(r => { if(r.ok) showToast("Sensitivity updated: " + val); });
  }

  function toggleLanguage() {
    currentLang = currentLang === 'en' ? 'it' : 'en';
    localStorage.setItem('lang', currentLang);
    updateUI();
  }

  function toggleTheme() {
    currentTheme = currentTheme === 'light' ? 'dark' : 'light';
    localStorage.setItem('theme', currentTheme);
    updateUI();
  }

  async function toggleArmed() { try { await fetch('/toggle/armed'); } catch (e) {} }
  async function toggleInvert() { try { await fetch('/toggle/invert'); } catch (e) {} }
  async function toggleRawMeasurements() { try { await fetch('/toggle/rawMeasurements'); } catch (e) {} }

  async function calibrateThreshold() {
    if (confirm(translations[currentLang].calPrompt)) {
      try { await fetch('/calibrate/threshold'); alert("Calibration started..."); } catch (e) {}
    }
  }

  async function updateThresholds() {
    const upper = parseFloat(document.getElementById('upper-threshold').value);
    const lower = parseFloat(document.getElementById('lower-threshold').value);
    if (!isNaN(upper) && !isNaN(lower)) {
      try { await fetch(`/update/thresholds?upper=${upper}&lower=${lower}`); } catch (e) {}
    }
  }

  async function updateHostname() {
    const suffix = document.getElementById('device-suffix').value.trim();
    if (suffix && confirm(translations[currentLang].rebootPrompt)) {
      try { await fetch(`/update/hostname?suffix=${suffix}`); alert(translations[currentLang].waitMsg); } catch (e) {}
    }
  }

  function updateConnectionStatus(wifi, mqtt) {
    const w = document.getElementById('wifi-status');
    const m = document.getElementById('mqtt-status');
    w.className = 'status-badge ' + (wifi ? 'connected' : 'disconnected');
    m.className = 'status-badge ' + (mqtt ? 'connected' : 'disconnected');
  }

  // Helper to sync settings without disturbing user input
  function syncSetting(id, value, type = 'text') {
    const element = document.getElementById(id);
    if (element && document.activeElement !== element) {
      if (type === 'checkbox') {
        element.checked = !!value;
      } else {
        element.value = value;
      }
    }
  }

  async function updateLoop() {
    try {
      const resp = await fetch('/readData');
      const data = await resp.json();

      document.getElementById('raw-dist').innerText = data.raw ? data.raw.toFixed(1) : "--";
      document.getElementById('bright-val').innerText = (data.brightness || 0) + "%";
      document.getElementById('bright-bar').style.width = (data.brightness || 0) + '%';

      const isHand = (data.hand === true || data.hand === "true" || data.hand === 1);
      document.getElementById('hand-text').innerText = isHand ? (currentLang === 'it' ? "SI" : "YES") : "NO";
      document.getElementById('hand-indicator').className = 'indicator ' + (isHand ? 'active' : '');
      
      const lastChange = document.getElementById('last-change');
      if (isHand && data.lastChange) {
        lastChange.innerText = (data.lastChange > 0 ? "+" : "") + data.lastChange.toFixed(1);
      } else {
        lastChange.innerText = "---";
      }

      // Sync settings
      syncSetting('armed-chk', data.armed, 'checkbox');
      syncSetting('invert-chk', data.invert_brightness, 'checkbox');
      syncSetting('raw-chk', data.update_raw_measurements, 'checkbox');
      
      // Update sensitivity UI
      document.getElementById('sensitivity-val').innerText = data.sensitivity;
      [1,2,3].forEach(v => {
        document.getElementById('s'+v).className = (data.sensitivity == v) ? 'segment-btn active' : 'segment-btn';
      });
      
      if (document.activeElement !== document.getElementById('upper-threshold')) 
        document.getElementById('upper-threshold').value = data.upper_threshold.toFixed(2);
      if (document.activeElement !== document.getElementById('lower-threshold')) 
        document.getElementById('lower-threshold').value = data.lower_threshold.toFixed(2);
      
      document.getElementById('surface-dist').innerText = data.surface_distance.toFixed(1);
      
      if (document.activeElement !== document.getElementById('device-suffix')) 
        document.getElementById('device-suffix').value = data.device_suffix;
      
      document.getElementById('full-address').innerText = data.full_hostname;

      updateConnectionStatus(data.wifi_connected == 1, data.mqtt_connected == 1);
      isFastPolling = isHand;

    } catch (e) {
      isFastPolling = false;
    } finally {
      setTimeout(updateLoop, isFastPolling ? 250 : 1000);
    }
  }

  updateUI();
  updateLoop();
</script>
</body>
</html>
)rawliteral";


#endif