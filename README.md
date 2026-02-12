# 🧤 AirDimmer v1.0

AirDimmer is an intuitive, mid-air gesture controller for smart home lighting. Using a **VL53L1X Time-of-Flight sensor** and an **ESP32**, it enables precision brightness control through simple hand movements—eliminating the need for physical switches or buttons.

---

## ✋ Magic at your Fingertips

AirDimmer transforms the space above it into a seamless dimming surface. 

1.  **Hand Detection**: The high-precision ToF sensor detects your hand entering the active zone.
2.  **Intuitive Dimming**: Your hand's vertical position directly maps to light intensity. Raise to brighten, lower to dim.
3.  **Visual Feedback**: The integrated web-dashboard provides a real-time "mirror" of your actions, showing the exact distance and the resulting brightness change.
4.  **Instant Response**: The system automatically increases its polling frequency ("Fast Polling") as soon as you bring your hand near, providing a zero-lag experience.

---

##  Features

###  Proximity-Based Precision
*   **Custom Thresholds**: Configure exact upper and lower boundaries to match your installation (e.g., under a cabinet or next to a bed).
*   **Sensitivity Control**: Adjust the response scaling (1x, 2x, or 3x divider) to fine-tune how much hand movement is required for dimming.
*   **Persistent Calibration**: Self-calibrates to the mounting surface on boot, ensuring reliable performance in different environments.

### Seamless Integration
*   **MQTT-Native**: Works out of the box with Home Assistant, Node-RED, and other MQTT brokers.
*   **Armed Safety**: Explicit "Armed" toggle to enable/disable light control while keeping the sensor active for monitoring.
*   **Hostname Discovery**: Reachable anywhere on your network at `http://airdimmer-(suffix).local`.

### 📱 Configuration Dashboard
While designed for hands-free use, AirDimmer includes a Web Suite for setup:
*   **Responsive Control**: Full setup via any phone or desktop browser.
*   **Themes**: Light and sophisticated Dark Gray modes.
*   **Localization**: Native English (UK) and Italian support.

---

## 🛠️ Tech Stack
*   **Microcontroller**: ESP32
*   **Sensor**: ST VL53L1X (Time-of-Flight)
*   **Connectivity**: WiFi, MQTT, mDNS, ArduinoOTA
*   **Persistence**: EEPROM (Store thresholds, sensitivity, and naming)

---

##  Getting Started
1.  Configure your WiFi and MQTT credentials in `network_information.h`.
2.  Flash the device via USB or OTA.
3.  Navigate to `http://airdimmer-setup.local` to begin calibration and setup.

---

## 👨‍💻 Author
Created by **Andrea Fox**  
[GitHub Repository](https://github.com/Andrea-Fox/AirDimmer)