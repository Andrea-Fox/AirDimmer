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
  </style>
</head>
<body>

<div class="container">
  <h1>AirDimmer Dashboard</h1>

  <div class="section">
    <p><span class="label">Mano Rilevata:</span> 
       <span class="value"><span id="hand-text">NO</span><span id="hand-indicator" class="indicator"></span></span>
    </p>
    <p><span class="label">Raw Dist:</span> <span id="raw-dist" class="value">--</span></p>
    <p><span class="label">Filtered:</span> <span id="filt-dist" class="value">--</span></p>
  </div>

  <div class="section">
    <p><span class="label">Luminosità:</span> <span id="bright-val" class="value">0</span>%</p>
    <div class="bar"><div class="bar-fill" id="bright-bar"></div></div>
  </div>

  <button onclick="fetch('/control?cmd=toggleArmed')">Toggle Armed</button>
</div>

<script>
  let isFastPolling = false;

  async function updateLoop() {
    try {
      const response = await fetch('/readData');
      const data = await response.json();

      // 1. Aggiorna numeri
      document.getElementById('raw-dist').innerText = data.raw ? data.raw.toFixed(1) : "--";
      document.getElementById('filt-dist').innerText = data.filtered ? data.filtered.toFixed(1) : "--";
      document.getElementById('bright-val').innerText = data.brightness || 0;
      document.getElementById('bright-bar').style.width = (data.brightness || 0) + '%';

      // 2. Controllo Mano (supporta sia boolean che stringa)
      const isHand = (data.hand === true || data.hand === "true" || data.hand === 1);
      
      const handText = document.getElementById('hand-text');
      const handInd = document.getElementById('hand-indicator');

      if (isHand) {
        handText.innerText = "SI";
        handInd.classList.add('active');
        isFastPolling = true;
      } else {
        handText.innerText = "NO";
        handInd.classList.remove('active');
        isFastPolling = false;
      }

    } catch (e) {
      console.error("Errore lettura dati");
      isFastPolling = false;
    }

    // 3. Calcola prossimo intervallo: 50ms se mano c'è, 1000ms se non c'è
    let nextCheck = isFastPolling ? 250 : 1000;
    setTimeout(updateLoop, nextCheck);
  }

  // Avvio
  updateLoop();
</script>
</body>
</html>
)rawliteral";

#endif