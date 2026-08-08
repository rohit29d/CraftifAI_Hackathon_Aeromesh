#include "app.h"
#include "app_config.h"
#include "logger.h"
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "driver/i2c_master.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "led_strip.h"
#include "led_strip_rmt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "nvs_flash.h"

static const char *TAG = "aeromesh";
static i2c_master_bus_handle_t imu_bus;
static i2c_master_dev_handle_t imu_dev;
static bool imu_ready;
static SemaphoreHandle_t sample_mutex;
static QueueHandle_t alert_queue;
static httpd_handle_t http_server;
static led_strip_handle_t status_led;

static const char *html_page =
"<!DOCTYPE html>\n"
"<html lang=\"en\">\n"
"<head>\n"
"    <meta charset=\"UTF-8\">\n"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
"    <title>AeroMesh Live Flight</title>\n"
"    <style>\n"
"        body { margin: 0; overflow: hidden; background-color: #0A0D14; color: #ECEDF0; font-family: 'Courier New', Courier, monospace; }\n"
"        #ui-container { position: absolute; top: 20px; left: 20px; z-index: 100; background: rgba(17, 22, 33, 0.8); padding: 20px; border: 1px solid rgba(255, 255, 255, 0.1); border-radius: 12px; }\n"
"        .data-row { margin: 8px 0; font-size: 14px; }\n"
"        .data-val { color: #F3B63F; font-weight: bold; }\n"
"        #turbulence-status { margin-top: 15px; padding: 10px; text-align: center; font-weight: bold; font-size: 18px; border-radius: 6px; background-color: #c62828; color: white; text-transform: uppercase; transition: background-color 0.3s; }\n"
"    </style>\n"
"    <script src=\"https://cdnjs.cloudflare.com/ajax/libs/three.js/r128/three.min.js\"></script>\n"
"</head>\n"
"<body>\n"
"    <div id=\"ui-container\">\n"
"        <div style=\"color:#0A84FF; font-weight:bold; margin-bottom:15px; font-size:18px;\">AEROMESH LINK ACTIVE</div>\n"
"        <div class=\"data-row\">Roll: <span id=\"roll-val\" class=\"data-val\">0.00</span>&deg;</div>\n"
"        <div class=\"data-row\">Pitch: <span id=\"pitch-val\" class=\"data-val\">0.00</span>&deg;</div>\n"
"        <div class=\"data-row\">Yaw: <span id=\"yaw-val\" class=\"data-val\">0.00</span>&deg;</div>\n"
"        <div id=\"turbulence-status\">Connecting...</div>\n"
"    </div>\n"
"    <script>\n"
"        const scene = new THREE.Scene();\n"
"        const camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);\n"
"        const renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });\n"
"        renderer.setSize(window.innerWidth, window.innerHeight);\n"
"        document.body.appendChild(renderer.domElement);\n"
"        const ambientLight = new THREE.AmbientLight(0xffffff, 0.6);\n"
"        scene.add(ambientLight);\n"
"        const directionalLight = new THREE.DirectionalLight(0xffffff, 0.8);\n"
"        directionalLight.position.set(10, 20, 10);\n"
"        scene.add(directionalLight);\n"
"        const airplane = new THREE.Group();\n"
"        const fuselageGeo = new THREE.CylinderGeometry(0.5, 0.5, 4, 32);\n"
"        fuselageGeo.rotateZ(Math.PI / 2);\n"
"        const bodyMat = new THREE.MeshPhongMaterial({ color: 0xcccccc });\n"
"        const fuselage = new THREE.Mesh(fuselageGeo, bodyMat);\n"
"        airplane.add(fuselage);\n"
"        const wingGeo = new THREE.BoxGeometry(1.5, 0.1, 5);\n"
"        const wingMat = new THREE.MeshPhongMaterial({ color: 0x0A84FF });\n"
"        const wings = new THREE.Mesh(wingGeo, wingMat);\n"
"        wings.position.set(0.5, 0, 0);\n"
"        airplane.add(wings);\n"
"        const tailGeo = new THREE.BoxGeometry(1, 0.1, 2);\n"
"        const tail = new THREE.Mesh(tailGeo, wingMat);\n"
"        tail.position.set(-1.5, 0, 0);\n"
"        airplane.add(tail);\n"
"        const vertGeo = new THREE.BoxGeometry(1, 1.5, 0.1);\n"
"        const vert = new THREE.Mesh(vertGeo, wingMat);\n"
"        vert.position.set(-1.5, 0.75, 0);\n"
"        airplane.add(vert);\n"
"        scene.add(airplane);\n"
"        camera.position.z = 8;\n"
"        camera.position.y = 2;\n"
"        camera.lookAt(0, 0, 0);\n"
"        const gridHelper = new THREE.GridHelper(20, 20, 0x0A84FF, 0x444444);\n"
"        gridHelper.position.y = -2;\n"
"        scene.add(gridHelper);\n"
"        let targetRotation = new THREE.Euler(0, 0, 0, 'XYZ');\n"
"        function animate() {\n"
"            requestAnimationFrame(animate);\n"
"            airplane.rotation.x += (targetRotation.x - airplane.rotation.x) * 0.15;\n"
"            airplane.rotation.y += (targetRotation.y - airplane.rotation.y) * 0.15;\n"
"            airplane.rotation.z += (targetRotation.z - airplane.rotation.z) * 0.15;\n"
"            renderer.render(scene, camera);\n"
"        }\n"
"        animate();\n"
"        window.addEventListener('resize', () => {\n"
"            camera.aspect = window.innerWidth / window.innerHeight;\n"
"            camera.updateProjectionMatrix();\n"
"            renderer.setSize(window.innerWidth, window.innerHeight);\n"
"        });\n"
"        const wsUrl = \"ws://\" + window.location.host + \"/ws\";\n"
"        let ws = new WebSocket(wsUrl);\n"
"        ws.onopen = () => {\n"
"            document.getElementById('turbulence-status').innerText = \"Stable\";\n"
"            document.getElementById('turbulence-status').style.backgroundColor = \"#2e7d32\";\n"
"        };\n"
"        ws.onmessage = (event) => {\n"
"            try {\n"
"                const data = JSON.parse(event.data);\n"
"                document.getElementById('roll-val').innerText = data.r.toFixed(2);\n"
"                document.getElementById('pitch-val').innerText = data.p.toFixed(2);\n"
"                document.getElementById('yaw-val').innerText = data.y.toFixed(2);\n"
"                targetRotation.x = data.p * (Math.PI / 180);\n"
"                targetRotation.y = -data.y * (Math.PI / 180);\n"
"                targetRotation.z = data.r * (Math.PI / 180);\n"
"                if (data.s !== undefined) {\n"
"                    const statusEl = document.getElementById('turbulence-status');\n"
"                    if (data.s === 0) {\n"
"                        statusEl.innerText = \"Stable\";\n"
"                        statusEl.style.backgroundColor = \"#2e7d32\";\n"
"                    } else if (data.s === 1) {\n"
"                        statusEl.innerText = \"Light Chop\";\n"
"                        statusEl.style.backgroundColor = \"#F3B63F\";\n"
"                    } else if (data.s === 2) {\n"
"                        statusEl.innerText = \"SEVERE TURBULENCE!\";\n"
"                        statusEl.style.backgroundColor = \"#c62828\";\n"
"                        camera.position.x = (Math.random() - 0.5) * 0.7;\n"
"                        camera.position.y = 2 + (Math.random() - 0.5) * 0.7;\n"
"                    }\n"
"                }\n"
"            } catch (e) { }\n"
"        };\n"
"        ws.onclose = () => {\n"
"            document.getElementById('turbulence-status').innerText = \"Disconnected\";\n"
"            document.getElementById('turbulence-status').style.backgroundColor = \"#c62828\";\n"
"        };\n"
"    </script>\n"
"</body>\n"
"</html>\n";

typedef struct {
    int16_t ax, ay, az, gx, gy, gz;
    float roll, pitch, yaw;
    int state;
    int64_t timestamp_us;
} imu_sample_t;

static imu_sample_t latest;
static bool filter_ready;

static esp_err_t status_led_init(void)
{
    led_strip_config_t strip_config = {};
    strip_config.strip_gpio_num = APP_RGB_LED_GPIO;
    strip_config.max_leds = APP_RGB_LED_COUNT;
    strip_config.led_model = LED_MODEL_WS2812;
    strip_config.color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB;
    led_strip_rmt_config_t rmt_config = {};
    rmt_config.clk_src = RMT_CLK_SRC_DEFAULT;
    rmt_config.resolution_hz = 10 * 1000 * 1000;
    rmt_config.mem_block_symbols = 0;
    rmt_config.flags.with_dma = false;
    esp_err_t err = led_strip_new_rmt_device(&strip_config, &rmt_config, &status_led);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "RGB LED initialization failed: %s", esp_err_to_name(err));
        return err;
    }
    return led_strip_clear(status_led);
}

static void status_led_set_state(int state)
{
    uint32_t red = 0;
    uint32_t green = 0;
    switch (state) {
    case 1:
        red = 255;
        green = 180;
        break;
    case 2:
        red = 255;
        break;
    case 0:
    default:
        green = 255;
        break;
    }
    if (status_led != NULL) {
        esp_err_t err = led_strip_set_pixel(status_led, 0, red, green, 0);
        if (err == ESP_OK) {
            err = led_strip_refresh(status_led);
        }
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "RGB LED update failed for state %d: %s", state, esp_err_to_name(err));
        }
    }
}

static int64_t last_alert_us;

static esp_err_t imu_write(uint8_t reg, uint8_t value)
{
    uint8_t data[2] = {reg, value};
    return i2c_master_transmit(imu_dev, data, sizeof(data), pdMS_TO_TICKS(APP_I2C_TIMEOUT_MS));
}

static esp_err_t imu_read(uint8_t reg, uint8_t *data, size_t length)
{
    return i2c_master_transmit_receive(imu_dev, &reg, 1, data, length,
                                       pdMS_TO_TICKS(APP_I2C_TIMEOUT_MS));
}

static esp_err_t imu_init(void)
{
    if (imu_ready) {
        return ESP_OK;
    }
    esp_err_t err = ESP_OK;
    if (imu_dev == NULL) {
        i2c_master_bus_config_t bus_config = {};
        bus_config.i2c_port = I2C_NUM_0;
        bus_config.sda_io_num = APP_I2C_SDA_GPIO;
        bus_config.scl_io_num = APP_I2C_SCL_GPIO;
        bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
        bus_config.glitch_ignore_cnt = 7;
        bus_config.flags.enable_internal_pullup = true;
        err = i2c_new_master_bus(&bus_config, &imu_bus);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "I2C bus initialization failed: %s", esp_err_to_name(err));
            return err;
        }
        i2c_device_config_t device_config = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = APP_MPU6050_ADDRESS,
            .scl_speed_hz = APP_I2C_FREQ_HZ,
        };
        err = i2c_master_bus_add_device(imu_bus, &device_config, &imu_dev);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "MPU6050 device attach failed: %s", esp_err_to_name(err));
            return err;
        }
    }
    uint8_t who = 0;
    err = imu_read(0x75, &who, 1);
    if (err != ESP_OK || (who != 0x68 && who != 0x69)) {
        ESP_LOGE(TAG, "MPU6050 WHO_AM_I failed: err=%s value=0x%02x",
                 esp_err_to_name(err), who);
        return err == ESP_OK ? ESP_ERR_NOT_FOUND : err;
    }
    const uint8_t init_regs[][2] = {{0x6B, 0x00}, {0x1A, 0x03}, {0x1B, 0x00}, {0x1C, 0x00}};
    for (size_t i = 0; i < sizeof(init_regs) / sizeof(init_regs[0]); ++i) {
        err = imu_write(init_regs[i][0], init_regs[i][1]);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "MPU6050 register 0x%02x write failed: %s",
                     init_regs[i][0], esp_err_to_name(err));
            return err;
        }
    }
    imu_ready = true;
    ESP_LOGI(TAG, "MPU6050 ready at 0x%02x, SDA=%d SCL=%d", APP_MPU6050_ADDRESS,
             APP_I2C_SDA_GPIO, APP_I2C_SCL_GPIO);
    return ESP_OK;
}

static float classifier_window[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
static size_t classifier_window_count;
static int last_classifier_state;

static int ei_signal_get_data(size_t offset, size_t length, float *out_ptr)
{
    if (offset + length > classifier_window_count) {
        return -1;
    }
    memcpy(out_ptr, classifier_window + offset, length * sizeof(float));
    return 0;
}

static int classify_turbulence(const imu_sample_t *s)
{
    if (classifier_window_count + EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME > EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        classifier_window_count = 0;
    }
    classifier_window[classifier_window_count++] = s->roll;
    classifier_window[classifier_window_count++] = s->pitch;
    classifier_window[classifier_window_count++] = s->yaw;
    if (classifier_window_count < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        return last_classifier_state;
    }

    signal_t signal = {};
    signal.total_length = classifier_window_count;
    signal.get_data = ei_signal_get_data;
    ei_impulse_result_t result = {};
    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false);
    classifier_window_count = 0;
    if (err != EI_IMPULSE_OK) {
        ESP_LOGW(TAG, "Edge Impulse inference failed: %d", (int)err);
        return last_classifier_state;
    }

    float best = -1.0f;
    int state = last_classifier_state;
    for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; ++i) {
        const char *label = result.classification[i].label;
        const float value = result.classification[i].value;
        if (value > best) {
            best = value;
            if (strstr(label, "severe") != NULL) {
                state = 2;
            } else if (strstr(label, "light") != NULL) {
                state = 1;
            } else {
                state = 0;
            }
        }
    }
    last_classifier_state = state;
    ESP_LOGI(TAG, "Edge Impulse state=%d confidence=%.3f", state, best);
    return state;
}

static void sensor_task(void *arg)
{
    (void)arg;
    TickType_t wake = xTaskGetTickCount();
    int errors = 0;
    vTaskDelay(pdMS_TO_TICKS(100));
    while (true) {
        if (!imu_ready) {
            static int64_t last_retry_us;
            int64_t now = esp_timer_get_time();
            if (now - last_retry_us >= 1000000) {
                last_retry_us = now;
                ESP_LOGW(TAG, "IMU unavailable; retrying initialization");
                if (imu_init() != ESP_OK) {
                    vTaskDelayUntil(&wake, pdMS_TO_TICKS(APP_SENSOR_PERIOD_MS));
                    continue;
                }
            }
        }
        uint8_t raw[14];
        esp_err_t err = imu_read(0x3B, raw, sizeof(raw));
        if (err != ESP_OK) {
            errors++;
            if ((errors % 50) == 1) {
                ESP_LOGW(TAG, "I2C IMU read failed (%d): %s", errors, esp_err_to_name(err));
            }
            vTaskDelayUntil(&wake, pdMS_TO_TICKS(APP_SENSOR_PERIOD_MS));
            continue;
        }
        imu_sample_t s = {0};
        s.ax = (int16_t)((raw[0] << 8) | raw[1]); s.ay = (int16_t)((raw[2] << 8) | raw[3]);
        s.az = (int16_t)((raw[4] << 8) | raw[5]); s.gx = (int16_t)((raw[8] << 8) | raw[9]);
        s.gy = (int16_t)((raw[10] << 8) | raw[11]); s.gz = (int16_t)((raw[12] << 8) | raw[13]);
        float ax = s.ax / 16384.0f, ay = s.ay / 16384.0f, az = s.az / 16384.0f;
        float accel_roll = atan2f(ay, az) * 57.29578f;
        float accel_pitch = atan2f(-ax, sqrtf(ay * ay + az * az)) * 57.29578f;
        float gx = s.gx / 131.0f, gy = s.gy / 131.0f, gz = s.gz / 131.0f;
        if (!filter_ready) { latest.roll = accel_roll; latest.pitch = accel_pitch; latest.yaw = 0; filter_ready = true; }
        else {
            latest.roll = APP_COMPLEMENTARY_ALPHA * (latest.roll + gx * 0.02f) + (1.0f - APP_COMPLEMENTARY_ALPHA) * accel_roll;
            latest.pitch = APP_COMPLEMENTARY_ALPHA * (latest.pitch + gy * 0.02f) + (1.0f - APP_COMPLEMENTARY_ALPHA) * accel_pitch;
            latest.yaw += gz * 0.02f;
            if (latest.yaw > 180) {
                latest.yaw -= 360;
            }
            if (latest.yaw < -180) {
                latest.yaw += 360;
            }
        }
        s.roll = latest.roll; s.pitch = latest.pitch; s.yaw = latest.yaw; s.timestamp_us = esp_timer_get_time();
        s.state = classify_turbulence(&s);
        if (xSemaphoreTake(sample_mutex, pdMS_TO_TICKS(5)) == pdTRUE) {
            latest = s;
            xSemaphoreGive(sample_mutex);
        }
        status_led_set_state(s.state);
        if (s.state == 2 && xQueueSend(alert_queue, &s, 0) != pdTRUE) {
            ESP_LOGW(TAG, "Alert queue full");
        }
        printf("%lld,%.2f,%.2f,%.2f\n", s.timestamp_us / 1000, s.roll, s.pitch, s.yaw);
        vTaskDelayUntil(&wake, pdMS_TO_TICKS(APP_SENSOR_PERIOD_MS));
    }
}

static esp_err_t root_handler(httpd_req_t *req) { ESP_LOGI(TAG, "HTTP GET /"); return httpd_resp_send(req, html_page, HTTPD_RESP_USE_STRLEN); }
static esp_err_t ws_handler(httpd_req_t *req)
{
    if (req->method == HTTP_GET) { ESP_LOGI(TAG, "WebSocket client connected fd=%d", httpd_req_to_sockfd(req)); return ESP_OK; }
    httpd_ws_frame_t frame = {.type = HTTPD_WS_TYPE_TEXT};
    esp_err_t err = httpd_ws_recv_frame(req, &frame, 0);
    if (err != ESP_OK) ESP_LOGW(TAG, "WebSocket receive failed: %s", esp_err_to_name(err));
    return err;
}

static void start_server(void)
{
    if (http_server) return;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_open_sockets = APP_MAX_HTTP_CLIENTS;
    if (httpd_start(&http_server, &config) != ESP_OK) { ESP_LOGE(TAG, "HTTP server start failed"); return; }
    static const httpd_uri_t root = {.uri = "/", .method = HTTP_GET, .handler = root_handler};
    static const httpd_uri_t ws = {.uri = "/ws", .method = HTTP_GET, .handler = ws_handler, .is_websocket = true};
    httpd_register_uri_handler(http_server, &root); httpd_register_uri_handler(http_server, &ws);
    ESP_LOGI(TAG, "HTTP/WebSocket server ready");
}

static void websocket_task(void *arg)
{
    (void)arg;
    while (true) {
        if (http_server != NULL && xSemaphoreTake(sample_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            imu_sample_t s = latest; xSemaphoreGive(sample_mutex);
            cJSON *obj = cJSON_CreateObject();
            if (obj) {
                cJSON_AddNumberToObject(obj, "r", s.roll); cJSON_AddNumberToObject(obj, "p", s.pitch);
                cJSON_AddNumberToObject(obj, "y", s.yaw); cJSON_AddNumberToObject(obj, "s", s.state);
                char *json = cJSON_PrintUnformatted(obj);
                if (json) {
                    httpd_ws_frame_t frame = {.type = HTTPD_WS_TYPE_TEXT, .payload = (uint8_t *)json, .len = strlen(json)};
                    int fds[APP_MAX_HTTP_CLIENTS]; size_t count = APP_MAX_HTTP_CLIENTS;
                    if (httpd_get_client_list(http_server, &count, fds) == ESP_OK)
                        for (size_t i = 0; i < count; ++i)
                            if (httpd_ws_get_fd_info(http_server, fds[i]) == HTTPD_WS_CLIENT_WEBSOCKET && httpd_ws_send_frame_async(http_server, fds[i], &frame) != ESP_OK)
                                ESP_LOGW(TAG, "WebSocket send failed fd=%d", fds[i]);
                    free(json);
                }
                cJSON_Delete(obj);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(APP_WEBSOCKET_PERIOD_MS));
    }
}

static void alert_task(void *arg)
{
    (void)arg; imu_sample_t event;
    while (xQueueReceive(alert_queue, &event, portMAX_DELAY) == pdTRUE) {
        int64_t now = esp_timer_get_time();
        if (now - last_alert_us < (int64_t)APP_ALERT_COOLDOWN_MS * 1000) { ESP_LOGW(TAG, "Alert suppressed by cooldown"); continue; }
        last_alert_us = now;
        if (strcmp(APP_ALERT_URL, "http://example.invalid/aeromesh-alert") == 0) { ESP_LOGW(TAG, "Alert URL is still a placeholder"); continue; }
        esp_http_client_config_t cfg = {.url = APP_ALERT_URL, .timeout_ms = APP_HTTP_TIMEOUT_MS};
        esp_http_client_handle_t client = esp_http_client_init(&cfg);
        if (!client) { ESP_LOGE(TAG, "HTTP alert client init failed"); continue; }
        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK) ESP_LOGI(TAG, "HTTP alert sent, status=%d", esp_http_client_get_status_code(client));
        else ESP_LOGE(TAG, "HTTP alert failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
    }
}

static void wifi_event(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    (void)arg; (void)data;
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) { ESP_LOGI(TAG, "WiFi station started"); esp_wifi_connect(); }
    else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) { ESP_LOGW(TAG, "WiFi disconnected; retrying"); esp_wifi_connect(); }
    else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) { ESP_LOGI(TAG, "WiFi acquired IP"); start_server(); }
}

static void wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event, NULL));
    wifi_config_t wifi = {0}; strncpy((char *)wifi.sta.ssid, APP_WIFI_SSID, sizeof(wifi.sta.ssid));
    strncpy((char *)wifi.sta.password, APP_WIFI_PASSWORD, sizeof(wifi.sta.password));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_start(void)
{
    esp_err_t nvs = nvs_flash_init();
    if (nvs == ESP_ERR_NVS_NO_FREE_PAGES || nvs == ESP_ERR_NVS_NEW_VERSION_FOUND) { ESP_ERROR_CHECK(nvs_flash_erase()); ESP_ERROR_CHECK(nvs_flash_init()); }
    sample_mutex = xSemaphoreCreateMutex(); alert_queue = xQueueCreate(APP_ALERT_QUEUE_LENGTH, sizeof(imu_sample_t));
    if (!sample_mutex || !alert_queue) { ESP_LOGE(TAG, "Synchronization allocation failed"); return; }
    if (status_led_init() != ESP_OK) {
        ESP_LOGW(TAG, "RGB status LED unavailable");
    }
    status_led_set_state(0);
    if (imu_init() != ESP_OK) ESP_LOGW(TAG, "IMU unavailable; sensor task will retry reads");
    wifi_init();
    xTaskCreate(sensor_task, "imu_50hz", 4096, NULL, 8, NULL);
    xTaskCreate(websocket_task, "ws_20hz", 4096, NULL, 5, NULL);
    xTaskCreate(alert_task, "http_alert", 4096, NULL, 4, NULL);
    ESP_LOGI(TAG, "AeroMesh started: sensor 50Hz, WebSocket 20Hz");
}
