
# ESP32 LoRa Sender/Receiver Firmware

<img src="https://upload.wikimedia.org/wikipedia/commons/7/7b/ESP32_Espressif_ESP-WROOM-32_Shielded.jpg" width="100" alt="ESP32 Logo">

This project is designed for **ESP32** boards using **LoRa** to send sensor data to **Sensor Community** and visualize it with **Metabase**. The firmware includes a **Sender** and **Receiver**, where the Sender sends data via **LoRa** and the Receiver listens for incoming data. The received data is forwarded to **Sensor Community** for storage and visualization.

## Features

- **Sender Firmware**: Reads sensor data and sends it via LoRa.
- **Receiver Firmware**: Receives LoRa data and forwards it to **Sensor Community**.
- **LoRa Communication**: Utilizes LoRa to transmit data over long distances with low power consumption.
- **Sensor Community Integration**: Sends data to the **Sensor Community** platform for cloud storage and analytics.
- **Metabase Integration**: Visualizes the data collected from Sensor Community in **Metabase** dashboards.

---

## Prerequisites

Before you can build and run this project, you'll need to install the following tools and set up your development environment.

### 1. **Install ESP-IDF** (Espressif IoT Development Framework)

The **ESP-IDF** framework is required for developing on the ESP32. Follow the official installation guide for your platform:

- **Linux/Mac**: [Get Started with ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/)
- **Windows**: [Get Started with ESP-IDF on Windows](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started-windows/)

Make sure to set the `IDF_PATH` environment variable correctly.

### 2. **Install Make and Build Tools**

The project uses `make` to build the firmware. Depending on your operating system, install `make` and other required tools.

#### **Linux/Mac**:
```bash
sudo apt-get install make
```

#### **Windows**:
- Install **MSYS2** or **MinGW** to get `make`.
- Follow the steps [here](https://www.msys2.org/) to install MSYS2.
- Ensure `make` is available in the terminal by running `make --version`.

### 3. **Install Doxygen**

The project uses **Doxygen** to automatically generate documentation for the code. Follow the instructions below to install **Doxygen** on your platform.

#### **Linux/Mac**:
```bash
sudo apt-get install doxygen
```

#### **Windows**:
- Download the **Doxygen installer** for Windows from the official website: [Doxygen Downloads](https://www.doxygen.nl/download.html).
- Run the installer and follow the on-screen instructions to complete the installation.

#### **Verify Doxygen Installation**:
After installing **Doxygen**, verify the installation by running the following command in your terminal:
```bash
doxygen --version
```
This should display the installed version of **Doxygen**.


## How to Use

Clone the repository and navigate to the project directory:

```bash
git clone https://github.com/clement-mendes/T-IOT-902
```

### 1. **Set Target and Build Firmware**

You can choose to build the **Sender** or **Receiver** firmware using the provided `Makefile`.

- **Build Sender Firmware** (to send data via LoRa):
  ```bash
  cd Software_sender
  make deploy
  ```

- **Build Receiver Firmware** (to receive data and forward it to Sensor Community):
  ```bash
  cd Software_receiver
  make deploy
  ```

This will compile the firmware, set the target to `esp32`, and flash the device.

### 2. **Clean the Project**

To clean the project and remove previously compiled files, use the following command:

```bash
make clean
```

### 3. **Configure the project**

Before build and flash you can configure some variable (Wifi SSID, wifi password, ect), using:

```bash
make config
```
## Troubleshooting

If you face any issues during the build or flashing process:

1. Ensure your ESP32 device is properly connected via USB and selected in the correct COM port.
2. Verify the correct **Sensor Community** device keys are added in the **Receiver** firmware.
3. Make sure that **LoRa** is correctly wired and initialized in the firmware.
4. Double-check the configuration for **Sensor Community** API and your Metabase connection.


## Links

- **ESP-IDF Documentation**: [ESP-IDF Docs](https://docs.espressif.com/projects/esp-idf/en/latest/)
- **Sensor Community**: [Sensor Community Platform](https://sensor.community/)
- **Metabase**: [Metabase](https://www.metabase.com/)

---


Happy coding with **ESP32**, **LoRa**, **Sensor Community**, and **Metabase**! 🌍📡📊
