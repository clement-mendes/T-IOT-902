# 🧪 Functional Test Plan — IoT Sensors

This section describes the functional test procedures for validating each sensor and module in the IoT system. These tests ensure that components are correctly wired and provide expected outputs before full system integration.

## ⚙️ General Requirements

- Microcontroller is powered and flashed (e.g., ESP32).
- Serial console is available (Arduino Serial Monitor, PuTTY, etc.).
- Required libraries are installed:
  - `Wire`, `Adafruit_Sensor`, `Adafruit_BMP280`, `WiFi.h`, `LoRa.h`, etc.
- Wiring matches the hardware schematic.
- Stable and sufficient power supply.

## 🔊 1. Sound Sensor — **SPH0645 (I2S Microphone)**

### 🎯 Objective
Verify that the microphone captures audio input correctly.

### 🧪 Test Procedure
- Initialize I2S interface.
- Capture a buffer of samples (e.g., 1024).
- Calculate and log the RMS or peak signal value.

### ✅ Success Criteria
- Values vary in response to ambient sounds (e.g., clapping, talking).

### 📤 Sample Output

Mic RMS: 145
Mic RMS: 218
Mic RMS: 93

## 🌡️ 2. Temperature Sensor — **BMP280**

### 🎯 Objective
Read accurate ambient temperature via I2C.

### 🧪 Test Procedure
- Use BMP280 library to read temperature.
- Output result to the console.

### ✅ Success Criteria
- Temperature is within a realistic indoor range (10–40°C).

### 📤 Sample Output

Temperature: 23.45 °C

## 🌬️ 3. Pressure Sensor — **BMP280**

### 🎯 Objective
Read atmospheric pressure via I2C.

### 🧪 Test Procedure
- Use BMP280 library to read pressure.
- Display the value in hPa.

### ✅ Success Criteria
- Reading falls within the expected range (950–1050 hPa).

### 📤 Sample Output

Pressure: 1012.34 hPa

## 🌫️ 4. Air Quality Sensor — **Dust Sensor SKU:10500 (Waveshare)**

### 🎯 Objective
Measure and report particulate matter (e.g., PM10).

### 🧪 Test Procedure
- Read analog or digital signal.
- Convert data to µg/m³ using sensor formula.
- Simulate pollution (e.g., moving cloth near sensor) to check responsiveness.

### ✅ Success Criteria
- Values vary with changes in air quality.

### 📤 Sample Output

PM10: 12.3 µg/m³

## 📡 5. LoRa Module

### 🎯 Objective
Test wireless communication via LoRa.

### 🧪 Test Procedure
- Send a test message from one LoRa device.
- Receive it on another device or gateway.
- Log RSSI and SNR values.

### ✅ Success Criteria
- Message is successfully transmitted and received.

### 📤 Sample Output

Message Sent: Hello LoRa
RSSI: -92 dBm | SNR: 8.2 dB

## 🌐 6. WiFi Module

### 🎯 Objective
Validate network connection and HTTP communication.

### 🧪 Test Procedure
- Connect to a known WiFi network.
- Send a POST request to a remote server (e.g., `/data` endpoint).
- Log connection status and HTTP response.

### ✅ Success Criteria
- HTTP response code is 200 (OK) or 201 (Created).

### 📤 Sample Output

Connected to WiFi: MySSID
IP: 192.168.1.23
POST /data -> 201 Created

## 📝 Final Notes

- Perform individual tests before full integration.
- Use Serial Monitor to verify sensor output and diagnose issues.
- Confirm sensor calibration when possible.
- Re-test WiFi and LoRa communication after firmware updates or configuration changes.
