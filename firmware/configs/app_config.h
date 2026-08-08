#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "driver/gpio.h"

#define APP_WIFI_SSID "ROHIT2IN1"
#define APP_WIFI_PASSWORD "Rohit0707"
#define APP_ALERT_URL "http://example.invalid/aeromesh-alert"

#define APP_I2C_SDA_GPIO ((gpio_num_t)5)
#define APP_I2C_SCL_GPIO ((gpio_num_t)4)
#define APP_RGB_LED_GPIO ((gpio_num_t)8)
#define APP_RGB_LED_COUNT 1
#define APP_I2C_FREQ_HZ 400000
#define APP_MPU6050_ADDRESS 0x68
#define APP_I2C_TIMEOUT_MS 100

#define APP_SENSOR_PERIOD_MS 20
#define APP_WEBSOCKET_PERIOD_MS 50
#define APP_ALERT_COOLDOWN_MS 5000
#define APP_ALERT_QUEUE_LENGTH 8
#define APP_HTTP_TIMEOUT_MS 5000
/* ESP-IDF HTTPD reserves three internal sockets; keep total below LWIP's default limit of 7. */
#define APP_MAX_HTTP_CLIENTS 4
#define APP_COMPLEMENTARY_ALPHA 0.98f

#endif /* APP_CONFIG_H */
