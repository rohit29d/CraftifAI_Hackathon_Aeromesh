# Aeromesh

Aeromesh is an ESP32-based firmware project designed to detect and monitor severe turbulence using motion tracking data. It leverages an MPU6050 sensor to collect 6-axis motion data (accelerometer and gyroscope) and processes the orientation (roll, pitch, yaw) to monitor environmental conditions. 

## Demonstration

### Images
![Working Setup 1](./aeromesh_media/IMG_20260808_162942865_HDR_AE.jpg)
![Working Setup 2](./aeromesh_media/IMG_20260808_164841109_HDR_AE.jpg)

### Videos
- [Video Demonstration 1](./aeromesh_media/VID_20260808_162605257.mp4)
- [Video Demonstration 2](./aeromesh_media/VID_20260808_163537660.mp4)

## Features

- **MPU6050 Sensor Integration:** High-speed I2C communication (400kHz) to capture real-time motion data.
- **Wi-Fi Connectivity:** Connects to standard networks to transmit telemetry or alert statuses.
- **HTTP Alert System:** Sends alerts to a remote HTTP endpoint with internal cooldown mechanisms.
- **RGB LED Feedback:** Onboard RGB LED indicator for quick visual status updates.
- **Machine Learning Ready:** Includes a `logger.py` script to seamlessly record device serial output and save it as standard CSV datasets suitable for Edge Impulse.
- **ESP-IDF Framework:** Built using the official Espressif IoT Development Framework (ESP-IDF) for maximum performance and reliability.

## Hardware Requirements

- **Microcontroller:** ESP32 (or compatible ESP-IDF supported board)
- **Sensor:** MPU6050 (6-axis IMU)
- **Additional:** RGB LED (Optional)

### Default Pin Configuration

| Component | Pin (GPIO) | Notes |
| :--- | :--- | :--- |
| **I2C SDA** | `GPIO 5` | MPU6050 Data |
| **I2C SCL** | `GPIO 4` | MPU6050 Clock |
| **RGB LED** | `GPIO 8` | Status Indicator |

*(Note: Pins and settings can be modified in `firmware/configs/app_config.h`)*

## Project Structure

```text
aeromesh/
├── .gitignore              # Git ignore rules
├── CMakeLists.txt          # Main CMake configuration
├── sdkconfig               # ESP-IDF project configuration
├── aeromesh_media/         # Media files and demonstrations
├── firmware/
│   └── configs/
│       └── app_config.h    # Application-specific configurations (Wi-Fi, Pins, Endpoints)
├── main/                   # Source files and application entry point
└── logger.py               # Python utility for Edge Impulse data collection
```

## Getting Started

### 1. Set Up the ESP-IDF Environment
Ensure you have the Espressif IoT Development Framework (ESP-IDF) installed and configured on your system.

### 2. Configure the Project
Before building, update the application configuration to match your environment.
Open `firmware/configs/app_config.h` and update the following definitions:

```c
#define APP_WIFI_SSID "YOUR_WIFI_SSID"
#define APP_WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define APP_ALERT_URL "http://your-alert-endpoint/aeromesh-alert"
```

### 3. Build and Flash
Navigate to the project root directory and run the following commands in your ESP-IDF terminal:

```bash
# Build the project
idf.py build

# Flash the firmware and open the serial monitor (replace COM_PORT with your actual port)
idf.py -p COM_PORT flash monitor
```

## Data Logging for Machine Learning

The `logger.py` script makes it easy to record the motion data coming from the ESP32. It formats the data into a CSV file with the exact header required by Edge Impulse (`timestamp, roll, pitch, yaw`).

1. Open `logger.py` and modify the script variables:
   ```python
   COM_PORT = 'COM5'            # Your ESP32 COM port
   BAUD_RATE = 115200           # Match the ESP-IDF monitor baud rate
   LABEL = 'severe_turbulence'  # The label for the current recording session
   DURATION_SECONDS = 10        # How long to record
   ```

2. Run the script:
   ```bash
   python logger.py
   ```
   
3. The script will wait for data and record for the specified duration. Once complete, it saves the file as `<LABEL>_<timestamp>.csv` which can be uploaded directly to Edge Impulse.

## License

This project is licensed under the MIT License - see the LICENSE file for details.
