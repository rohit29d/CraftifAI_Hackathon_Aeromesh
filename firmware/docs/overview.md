# AeroMesh Edge AI Turbulence Monitor

AeroMesh samples an MPU6050 at 50 Hz over I2C, estimates roll, pitch, and startup-relative yaw with a complementary filter, classifies turbulence through a testable Edge AI stub, publishes telemetry over WebSockets at 20 Hz, and sends severe-turbulence HTTP alerts with a five-second cooldown.

## Hardware and configuration

- ESP32-C3
- MPU6050 address `0x68`
- I2C SDA: GPIO5
- I2C SCL: GPIO4
- Wi-Fi credentials: replace placeholders in `firmware/configs/app_config.h`
- Alert URL: replace `APP_ALERT_URL` before deployment

Yaw is relative to startup and drifts because the MPU6050 has no magnetometer or other absolute heading reference.

## Build and runtime

Build with the ESP-IDF project tools for target `esp32c3`. The HTTP server exposes `/`; the WebSocket endpoint is `/ws`. The firmware logs I2C, Wi-Fi, WebSocket, and HTTP alert activity with standard ESP-IDF logging macros.

The classifier normally returns Stable (0), then generates a random test state every ten seconds. State 2 enters the alert queue, subject to the cooldown.

## Source layout

`firmware/app/app.c` contains the application orchestration and ESP-IDF integration. `firmware/configs/app_config.h` contains application constants and pins. `main/entry.c` is the single ESP-IDF entry shim.
