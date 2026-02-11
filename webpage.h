#ifndef WEBPAGE_H
#define WEBPAGE_H

const char webpageHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>AirDimmer Dashboard</title>
  <style>
    body { font-family: sans-serif; margin: 20px; background-color: #f4f4f4; text-align: center; }
    .container { background: white; padding: 20px; border-radius: 10px; shadow: 0 2px 5px rgba(0,0,0,0.1); max-width: 400px; margin: auto; }
    .section { margin-bottom: 20px; border-bottom: 1px solid #eee; padding-bottom: 10px; text-align: left; }
    .label { font-weight: bold; }
    .value { float: right; font-family: monospace; }
    
    .bar { width: 100%; height: 30px; background: #ddd; border-radius: 15px; margin-top: 10px; overflow: hidden; }
    .bar-fill { height: 100%; background: #4caf50; width: 0%; transition: width 0.1s linear; }

    .indicator { height: 15px; width: 15px; background-color: #bbb; border-radius: 50%; display: inline-block; margin-left: 10px; transition: 0.2s; }
    .indicator.active { background-color: #ff4757; box-shadow: 0 0 10px #ff4757; }
    
    /* Toggle Switch */
    .switch { position: relative; display: inline-block; width: 50px; height: 24px; }
    .switch input { opacity: 0; width: 0; height: 0; }
    .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; transition: 0.4s; border-radius: 24px; }
    .slider:before { position: absolute; content: ""; height: 18px; width: 18px; left: 3px; bottom: 3px; background-color: white; transition: 0.4s; border-radius: 50%; }
    input:checked + .slider { background-color: #4caf50; }
    input:checked + .slider:before { transform: translateX(26px); }
    .slider:hover { opacity: 0.8; }
    
    /* Input fields */
    input[type="number"] { width: 80px; padding: 5px; border: 1px solid #ccc; border-radius: 4px; font-family: monospace; }
    button.update-btn { padding: 8px 16px; background-color: #4caf50; color: white; border: none; border-radius: 4px; cursor: pointer; font-size: 13px; margin-left: 10px; }
    button.update-btn:hover { background-color: #45a049; }
    .threshold-row { display: flex; justify-content: space-between; align-items: center; margin: 8px 0; }
  </style>
</head>
<body>

<div class="container">
  <h1>AirDimmer Dashboard</h1>

  <div class="section">
    <p><span class="label">Hand Detected:</span> 
       <span class="value"><span id="hand-text">NO</span><span id="hand-indicator" class="indicator"></span></span>
    </p>
    <p><span class="label">Raw Distance:</span> <span id="raw-dist" class="value">--</span> cm</p>
    <p><span class="label">Last Change Sent:</span> <span id="last-change" class="value">---</span></p>
  </div>

  <div class="section">
    <p><span class="label">Brightness:</span> <span id="bright-val" class="value">0</span>%</p>
    <div class="bar"><div class="bar-fill" id="bright-bar"></div></div>
  </div>

  <div class="section">
    <h3>Settings</h3>
    <div style="display: flex; justify-content: space-between; align-items: center; margin: 10px 0;">
      <span class="label">Update Raw Measurements:</span>
      <label class="switch">
        <input type="checkbox" id="toggle-raw-measurements" onclick="toggleRawMeasurements()">
        <span class="slider"></span>
      </label>
    </div>
    
    <div class="threshold-row">
      <span class="label">Upper Threshold:</span>
      <div>
        <input type="number" id="upper-threshold" min="0" max="1" step="0.01" value="0.9" onchange="updateThresholds()">
      </div>
    </div>
    
    <div class="threshold-row">
      <span class="label">Lower Threshold:</span>
      <div>
        <input type="number" id="lower-threshold" min="0" max="1" step="0.01" value="0.1" onchange="updateThresholds()">
      </div>
    </div>
  </div>

  <div class="section">
    <h3>Actions</h3>
    <button onclick="calibrateThreshold()" style="width: 100%; padding: 10px; background-color: #2196F3; color: white; border: none; border-radius: 5px; cursor: pointer; font-size: 14px; margin-bottom: 10px;">Recalibrate Distance Threshold</button>
  </div>
</div>

<script>
  let isFastPolling = false;

  async function toggleRawMeasurements() {
    try {
      await fetch('/toggle/rawMeasurements');
      // Update will happen on next poll
    } catch (e) {
      console.error("Error toggling raw measurements");
    }
  }

  async function calibrateThreshold() {
    if (confirm("This will recalibrate the distance threshold. Make sure no hand is near the sensor. Continue?")) {
      try {
        await fetch('/calibrate/threshold');
        alert("Calibration started. Please wait...");
      } catch (e) {
        console.error("Error calibrating threshold");
        alert("Error starting calibration");
      }
    }
  }

  async function updateThresholds() {
    const upperVal = parseFloat(document.getElementById('upper-threshold').value);
    const lowerVal = parseFloat(document.getElementById('lower-threshold').value);
    
    // Validate values are in [0, 1] range
    if (isNaN(upperVal) || isNaN(lowerVal) || upperVal < 0 || upperVal > 1 || lowerVal < 0 || lowerVal > 1) {
      console.error("Threshold values must be between 0 and 1");
      return;
    }
    
    try {
      const response = await fetch(`/update/thresholds?upper=${upperVal}&lower=${lowerVal}`);
      if (response.ok) {
        console.log("Thresholds updated successfully");
      } else {
        const errorText = await response.text();
        console.error("Error updating thresholds: " + errorText);
      }
    } catch (e) {
      console.error("Error updating thresholds", e);
    }
  }

  async function updateLoop() {
    try {
      const response = await fetch('/readData');
      const data = await response.json();

      // 1. Update distance values
      document.getElementById('raw-dist').innerText = data.raw ? data.raw.toFixed(1) : "--";
      document.getElementById('bright-val').innerText = data.brightness || 0;
      document.getElementById('bright-bar').style.width = (data.brightness || 0) + '%';

      // 2. Hand detection
      const isHand = (data.hand === true || data.hand === "true" || data.hand === 1);
      
      const handText = document.getElementById('hand-text');
      const handInd = document.getElementById('hand-indicator');
      const lastChangeVal = document.getElementById('last-change');

      if (isHand) {
        handText.innerText = "YES";
        handInd.classList.add('active');
        isFastPolling = true;
        
        // Show last change value if available
        if (data.lastChange && data.lastChange !== 0) {
          lastChangeVal.innerText = (data.lastChange > 0 ? "+" : "") + data.lastChange.toFixed(1);
        } else {
          lastChangeVal.innerText = "---";
        }
      } else {
        handText.innerText = "NO";
        handInd.classList.remove('active');
        isFastPolling = false;
        lastChangeVal.innerText = "---";  // Show --- when no hand
      }

      // 3. Update toggle state and threshold values
      const rawMeasToggle = document.getElementById('toggle-raw-measurements');
      if (rawMeasToggle && data.update_raw_measurements !== undefined) {
        rawMeasToggle.checked = (data.update_raw_measurements === true || data.update_raw_measurements === "true" || data.update_raw_measurements === 1);
      }
      
      // Update threshold input fields
      if (data.upper_threshold !== undefined) {
        document.getElementById('upper-threshold').value = parseFloat(data.upper_threshold).toFixed(2);
      }
      if (data.lower_threshold !== undefined) {
        document.getElementById('lower-threshold').value = parseFloat(data.lower_threshold).toFixed(2);
      }

    } catch (e) {
      console.error("Error reading data");
      isFastPolling = false;
    }

    // 4. Calculate next interval: 250ms if hand detected, 1000ms otherwise
    let nextCheck = isFastPolling ? 250 : 1000;
    setTimeout(updateLoop, nextCheck);
  }

  // Start
  updateLoop();
</script>
</body>
</html>
)rawliteral";

#endif