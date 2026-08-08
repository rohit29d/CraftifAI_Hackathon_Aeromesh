# Aeromesh

Aeromesh is an ESP32-based embedded system designed to detect and monitor severe turbulence using real-time motion data. The system uses an MPU6050 6-axis inertial measurement unit (IMU) to capture accelerometer and gyroscope data and processes the motion to estimate orientation and identify sudden movements associated with severe turbulence.

The system combines embedded sensing, real-time telemetry, wireless connectivity, and an Edge AI pipeline for intelligent motion analysis.

## How It Works

The ESP32 continuously acquires 6-axis motion data from the MPU6050 through high-speed I2C communication at 400 kHz.

The collected accelerometer and gyroscope data are processed to estimate the device's orientation, including roll, pitch, and yaw. Sudden changes and abnormal motion patterns are analyzed to identify potential turbulence events.

When a turbulence event is detected, Aeromesh can transmit an alert to a remote HTTP endpoint over Wi-Fi. An internal cooldown mechanism prevents repeated alerts from being transmitted for the same event.

An onboard RGB LED provides immediate visual feedback regarding the system's operating and detection status.

In addition to event detection, Aeromesh provides real-time flight-device telemetry over WebSockets, allowing the device's orientation and motion trajectory to be monitored remotely.

## Edge AI

Aeromesh includes an Edge AI pipeline for analyzing motion data and identifying turbulence-related patterns.

The machine-learning inference pipeline was successfully developed and tested on a laptop using the collected motion data. The intended final architecture was to deploy the trained model directly on the ESP32 so that turbulence classification could be performed locally without relying on a remote system.

Due to time constraints and integration challenges associated with TensorFlow within the ESP-IDF environment, the complete TensorFlow inference pipeline was not deployed on the microcontroller during the buildathon.

The project therefore demonstrates the complete sensing, telemetry, detection, and AI development pipeline while leaving microcontroller-side AI deployment as a future extension.

## Features

* **MPU6050 IMU Integration:** Real-time 6-axis accelerometer and gyroscope data acquisition.
* **High-Speed I2C:** MPU6050 communication at 400 kHz.
* **Motion Processing:** Real-time roll, pitch, and yaw estimation.
* **Turbulence Detection:** Identification of sudden and abnormal motion patterns.
* **Wi-Fi Connectivity:** Wireless communication with remote services.
* **HTTP Alerts:** Remote turbulence-event notification with cooldown protection.
* **WebSocket Telemetry:** Real-time transmission of orientation and motion information.
* **RGB LED Feedback:** Local visual indication of system state and events.
* **Edge AI Pipeline:** Machine-learning inference pipeline developed and tested for motion-data analysis.
* **ESP-IDF Firmware:** Native development using Espressif's ESP-IDF framework.

## System Architecture

```text
                    ┌─────────────────┐
                    │    MPU6050 IMU  │
                    │  Accel + Gyro   │
                    └────────┬────────┘
                             │
                         I2C 400 kHz
                             │
                             ▼
                    ┌─────────────────┐
                    │      ESP32      │
                    │                 │
                    │ Motion Processing│
                    │ Roll/Pitch/Yaw  │
                    │ Turbulence Logic│
                    └───────┬─────────┘
                            │
                ┌───────────┼────────────┐
                │           │            │
                ▼           ▼            ▼
           ┌────────┐  ┌──────────┐  ┌──────────┐
           │ RGB LED│  │ HTTP API │  │WebSocket │
           │ Status │  │  Alerts  │  │Telemetry │
           └────────┘  └──────────┘  └──────────┘
                            │            │
                            └─────┬──────┘
                                  ▼
                           Remote Monitoring

              ┌────────────────────────────┐
              │        Edge AI Pipeline     │
              │ Motion Data → ML Inference  │
              │ Laptop validation / testing │
              └────────────────────────────┘
```

## Hardware Configuration

| Component        | Description        |
| ---------------- | ------------------ |
| Microcontroller  | ESP32              |
| Motion Sensor    | MPU6050 6-axis IMU |
| Connectivity     | Wi-Fi              |
| Status Indicator | RGB LED            |
| Sensor Interface | I2C                |
| I2C Speed        | 400 kHz            |

### Default Pin Configuration

| Component |   GPIO | Description             |
| --------- | -----: | ----------------------- |
| I2C SDA   | GPIO 5 | MPU6050 data            |
| I2C SCL   | GPIO 4 | MPU6050 clock           |
| RGB LED   | GPIO 8 | System status indicator |

> Pin assignments may be modified through the project configuration depending on the target ESP32 board.

## Project Structure

```text
aeromesh/
├── .gitignore
├── CMakeLists.txt
├── sdkconfig
├── README.md
│
├── aeromesh_media/
│   ├── VID_20260808_162605257.mp4
│   ├── VID_20260808_163537660.mp4
│   └── VID_20260808_173541577.mp4
│
├── firmware/
│   └── configs/
│       └── app_config.h
│
└── main/
    └── ...
```

## Demonstration

### Tasklist Overview

The project demonstration covers the complete embedded workflow, including:

* MPU6050 sensor acquisition
* Real-time motion processing
* Orientation tracking
* Turbulence-event detection
* RGB LED status feedback
* Wi-Fi connectivity
* HTTP alert generation
* WebSocket telemetry
* AI pipeline development and validation

### Firmware Topology

The firmware is structured around sensor acquisition, motion processing, event detection, wireless communication, and telemetry.

```text
MPU6050
   │
   ▼
Sensor Acquisition
   │
   ▼
Motion Processing
   │
   ├──► Orientation Tracking
   │
   └──► Turbulence Detection
             │
             ├──► RGB LED
             ├──► HTTP Alert
             └──► WebSocket Telemetry
```

### Hardware Setup

The MPU6050 is connected to the ESP32 through the I2C interface. The RGB LED is connected to the configured GPIO and provides local system feedback.

## Videos

### Video Demonstration 1

[Watch Video Demonstration 1](./aeromesh_media/VID_20260808_162605257.mp4)

### Video Demonstration 2

[Watch Video Demonstration 2](./aeromesh_media/VID_20260808_163537660.mp4)

### Video Demonstration 3

[Watch Video Demonstration 3](./aeromesh_media/VID_20260808_173541577.mp4)

## Technology Stack

* **Microcontroller:** ESP32
* **Firmware Framework:** ESP-IDF
* **Sensor:** MPU6050
* **Communication:** I2C, Wi-Fi, HTTP, WebSockets
* **Machine Learning:** TensorFlow-based development and inference pipeline
* **Programming:** C / Embedded C
* **Development Environment:** Espressif ESP-IDF

## Future Improvements

* Deploy the trained Edge AI model directly on the ESP32.
* Optimize the model for ESP32 memory and computational constraints.
* Add calibrated multi-axis turbulence classification.
* Improve false-positive rejection using temporal motion features.
* Add persistent flight-data logging.
* Develop a dedicated remote telemetry dashboard.
* Integrate additional environmental sensors for improved turbulence classification.

## Project Status

Aeromesh currently demonstrates real-time IMU acquisition, motion processing, turbulence detection, Wi-Fi connectivity, HTTP alerting, RGB status feedback, and WebSocket telemetry on the ESP32.

The Edge AI model and inference pipeline have been developed and validated on a laptop, with direct ESP32 deployment remaining as the primary next-stage implementation.

## License

This project was developed as part of the CraftifAI Buildathon.
