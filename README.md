# 🧤 AirDimmer v1.0

AirDimmer is an intuitive, mid-air gesture controller for smart home lighting. Using a **VL53L1X Time-of-Flight sensor** and an **ESP32**, it enables precision brightness control through simple hand movements—eliminating the need for physical switches or buttons.

---

## ✋ Magic at your Fingertips

AirDimmer transforms the space above it into a seamless dimming surface. 

1. **Hand Detection**: The high-precision ToF sensor detects your hand entering the active zone.
2. **Intuitive Dimming**: Your hand's vertical position directly maps to light intensity. Raise to brighten, lower to dim.
3. **Visual Feedback**: The integrated web-dashboard provides a real-time "mirror" of your actions, showing the exact distance and the resulting brightness change.
4. **Instant Response**: The system automatically increases its polling frequency ("Fast Polling") as soon as you bring your hand near, providing a zero-lag experience.

---

## ✨ Features

### 🎯 Proximity-Based Precision

* **Safety Thresholds**: Configure exact boundaries to ensure reliable detection.
* **Invertible Control**: Choose the direction of control:
  * **Standard**: Closer = Brighter, Further = Dimmer.
  * **Inverted**: Further = Brighter, Closer = Dimmer.
* **Sensitivity Control**: Adjust the response scaling (1x, 2x, or 3x divider).
* **Persistent Calibration**: Self-calibrates to the mounting surface on boot, ensuring reliable performance in different environments.

### 🔌 Seamless Integration

* **MQTT-Native**: Works out of the box with Home Assistant, Node-RED, and other MQTT brokers.
* **Armed Safety**: Explicit "Armed" toggle to enable/disable light control while keeping the sensor active for monitoring.
* **Hostname Discovery**: Reachable anywhere on your network at `http://airdimmer-(suffix).local`.

---

## 📡 MQTT Topic Structure

AirDimmer uses a structured MQTT hierarchy. The `(suffix)` defaults to `setup` until changed.

| Topic                                  | Direction     | Description                                                                         |
| -------------------------------------- | ------------- | ----------------------------------------------------------------------------------- |
| `AirDimmer/(suffix)/brightness_change` | **Publish**   | Sends the relative change (e.g., `-5.5`, `+12.0`) to adjust light intensity.        |
| `AirDimmer/(suffix)/distance`          | **Publish**   | Sends raw distance measurements (in cm) when enabled in settings.                   |
| `AirDimmer/(suffix)/receiver`          | **Subscribe** | Listens for commands (`threshold_calibration`) or brightness sync (values `0-255`). |

---

### 📱 Configuration Dashboard

While designed for hands-free use, AirDimmer includes a Web Suite for setup:

* **Responsive Control**: Full setup via any phone or desktop browser.
* **Themes**: Light and sophisticated Dark Gray modes.
* **Localization**: Native English (UK) and Italian support.

---

## 🛠️ Tech Stack

* **Microcontroller**: ESP32
* **Sensor**: ST VL53L1X (Time-of-Flight)
* **Connectivity**: WiFi, MQTT, mDNS, ArduinoOTA
* **Persistence**: EEPROM (Store thresholds, sensitivity, and naming)

---

## 🛠️ Hardware & Wiring

### Supported Sensors

AirDimmer is optimized for the **VL53L1X Time-of-Flight (ToF) sensor**. Development and testing were primarily conducted using the [SparkFun Distance Sensor](https://www.sparkfun.com/products/14722). While other VL53L1X-based modules (such as the [Pololu VL53L1X](https://www.pololu.com/product/3415)) are likely compatible due to the common chipset, they have not been personally tested.

### Wiring Diagrams

Connect the sensor to your board via the **I2C interface**. 

#### 🔌 ESP32 (Recommended)
The ESP32 is the preferred platform due to its robust WiFi performance and native MQTT support.

| ESP32 Pin | Sensor Pin | Description |
| :--- | :--- | :--- |
| **3V3** | VIN | Power Supply (3.3V) |
| **GND** | GND | Common Ground |
| **GPIO 21 (SDA)** | SDA | I2C Data Line |
| **GPIO 22 (SCL)** | SCL | I2C Clock Line |


#### 🔌 ESP8266
It is also possible to use an ESP8266 (e.g., NodeMCU or Wemos D1 Mini) with the following typical pinout:

| ESP8266 Pin | Sensor Pin |
| :--- | :--- |
| **3V3** | VIN |
| **GND** | GND |
| **D2 (SDA)** | SDA |
| **D1 (SCL)** | SCL |

#### 🔌 5V Boards (Legacy/Prototyping)
For 5V boards like the Arduino Uno, Leonardo, or Mega:

| Arduino Pin | Sensor Pin |
| :--- | :--- |
| **5V** | VIN |
| **GND** | GND |
| **SDA** | SDA |
| **SCL** | SCL |

> [!IMPORTANT]
> To use MQTT and Web features, you **must** use a WiFi-enabled board like the ESP32 or ESP8266.

## 🚀 Getting Started

1. Configure your WiFi and MQTT credentials in `network_information.h`.
2. Flash the device via USB or OTA.
3. Navigate to `http://airdimmer-setup.local` to begin calibration and setup.
