# 💧 Smart Water Tank Monitor

An end-to-end IoT-powered Water Tank Monitoring & Automated Drainage System built with **ESP8266 (NodeMCU)**, **Firebase Realtime Database**, and a responsive **Web Dashboard**.

🌐 **Live Dashboard:** [https://smartwatertank-c24f5.web.app](https://smartwatertank-c24f5.web.app)

---

## 📌 Features

- **Real-Time Monitoring:**
  - **Water Level:** Measured via HC-SR04 ultrasonic sensor with visual fill percentage and distance in cm.
  - **Temperature:** Accurate water temperature monitoring using DS18B20 digital sensor.
  - **Turbidity (Water Clarity):** Analog clarity percentage (100% = Clear, 0% = Turbid).
  - **OLED Display:** Real-time 0.96" SSD1306 OLED readout directly on the hardware unit.

- **Intelligent Pump Automation:**
  - Automatic drainage trigger when water quality drops below 50% clarity.
  - Safe relay control with active-LOW triggering.

- **Instant Remote Control (Firebase Streaming):**
  - Instant manual pump override directly from the web dashboard.
  - Remote power toggles for individual sensors (Ultrasonic, Temp, Turbidity) and the OLED display.

- **Interactive Web Dashboard:**
  - Modern dark-mode UI with SVG arc gauges and dynamic water tank animation.
  - In-app toast popups, browser push notifications, and alert history log for low water, high water, temperature extremes, and poor water clarity.

---

## 🔌 Hardware Pinout & Wiring

| Component | ESP8266 Pin | Interface / Note |
|---|---|---|
| **Ultrasonic Sensor (HC-SR04)** | `D5` (Trig), `D6` (Echo) | Digital GPIO |
| **DS18B20 Temperature** | `D4` | OneWire Data (4.7kΩ pull-up to 3.3V) |
| **Turbidity Sensor** | `A0` | Analog ADC (0 - 640 raw range) |
| **OLED Display (SSD1306 I2C)** | `D1` (SCL), `D2` (SDA) | I2C (Address: `0x3C`) |
| **5V Water Pump Relay** | `D7` (IN) | Active-LOW Relay |

---

## 🛠️ Tech Stack & Dependencies

### Embedded Firmware (NodeMCU ESP8266)
- **Language / IDE:** C++ / Arduino IDE
- **Core:** ESP8266 Arduino Core (`NodeMCU 1.0 ESP-12E`)
- **Key Libraries:**
  - `Firebase ESP8266 Client` (by Mobizt)
  - `Adafruit SSD1306` & `Adafruit GFX Library`
  - `DallasTemperature` & `OneWire`

### Web Dashboard & Cloud
- **Frontend:** Vanilla HTML5, CSS3 (Flexbox/Grid), JavaScript (ES6 Modular)
- **Firebase Web SDK:** v10+ (Realtime Database, Hosting)
- **Hosting:** Firebase Hosting

---

## 🚀 Setup & Installation

### 1. Hardware Firmware
1. Open [`SmartWaterMonitor/SmartWaterMonitor.ino`](SmartWaterMonitor/SmartWaterMonitor.ino) in Arduino IDE.
2. Install the required libraries from Arduino Library Manager.
3. Configure your WiFi credentials and Firebase database settings:
   ```cpp
   #define WIFI_SSID     "YOUR_WIFI_SSID"
   #define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
   #define FIREBASE_HOST "YOUR_FIREBASE_RTDB_URL"
   #define FIREBASE_AUTH "YOUR_FIREBASE_DATABASE_SECRET"
   ```
4. Select board **NodeMCU 1.0 (ESP-12E Module)** and your COM port, then click **Upload**.

### 2. Web Dashboard Deployment
To deploy updates to Firebase Hosting:
```bash
npm install -g firebase-tools
firebase login
firebase deploy --only hosting
```

---

## 📄 License
This project is open-source and available under the [MIT License](LICENSE).
