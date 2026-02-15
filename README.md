# 🧤 AirDimmer v1.1

AirDimmer is an intuitive, mid-air gesture controller for smart home lighting. Using a **VL53L1X Time-of-Flight sensor** and an **ESP32**, it enables precision brightness control through simple hand movements—eliminating the need for physical switches or buttons.

---

## ✋ Magic at your Fingertips

AirDimmer transforms the space above it into a seamless dimming surface. 

1.  **Hand Detection**: The high-precision ToF sensor detects your hand entering the active zone.
2.  **Intuitive Dimming**: Your hand's vertical position directly maps to light intensity. Raise to brighten, lower to dim.
3.  **Visual Feedback**: The integrated web-dashboard provides a real-time "mirror" of your actions, showing the exact distance and the resulting brightness change.
4.  **Instant Response**: The system automatically increases its polling frequency ("Fast Polling") as soon as you bring your hand near, providing a zero-lag experience.

---

## ✨ Features

### 🎯 Proximity-Based Precision
*   **Safety Thresholds**: Configure exact boundaries to ensure reliable detection.
*   **Invertible Control**: Choose the direction of control:
    *   **Standard**: Closer = Brighter, Further = Dimmer.
    *   **Inverted**: Further = Brighter, Closer = Dimmer.
*   **Sensitivity Control**: Adjust the response scaling (1x, 2x, or 3x divider).
*   **Persistent Calibration**: Self-calibrates to the mounting surface on boot, ensuring reliable performance in different environments.

### 🔌 Seamless Integration
*   **MQTT-Native**: Works out of the box with Home Assistant, Node-RED, and other MQTT brokers.
*   **Armed Safety**: Explicit "Armed" toggle to enable/disable light control while keeping the sensor active for monitoring.
*   **Hostname Discovery**: Reachable anywhere on your network at `http://airdimmer-(suffix).local`.

---

## 📡 MQTT Topic Structure

AirDimmer uses a structured MQTT hierarchy. The `(suffix)` defaults to `setup` until changed.

| Topic | Direction | Description |
|---|---|---|
| `AirDimmer/(suffix)/brightness_change` | **Publish** | Sends the relative change (e.g., `-5.5`, `+12.0`) to adjust light intensity. |
| `AirDimmer/(suffix)/distance` | **Publish** | Sends raw distance measurements (in cm) when enabled in settings. |
| `AirDimmer/(suffix)/receiver` | **Subscribe** | Listens for commands (`threshold_calibration`) or brightness sync (values `0-255`). |

---

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

## 🚀 Getting Started
1.  Configure your WiFi and MQTT credentials in `network_information.h`.
2.  Flash the device via USB or OTA.
3.  Navigate to `http://airdimmer-setup.local` to begin calibration and setup.
