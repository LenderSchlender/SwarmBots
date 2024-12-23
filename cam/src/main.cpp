/*
 * Webserver for the ESP-CAM AI-Thinker board providing a multipart jpeg stream.
 * This code only works for a single client at a time (and the ESP can't handle much more anyways)
 * If we want to show this stream to multiple clients we need to distribute it on an external server
 */

#include <Arduino.h>
#include <WiFi.h>
// For disabling the brownout detection
#include <soc/soc.h>
#include <soc/rtc_cntl_reg.h>
// Camera
#include <esp_camera.h>
#include <esp_timer.h>
// HTTP server
#include <esp_http_server.h>

// Following two macros are just placeholders, in reality they're externally defined in the .env file
#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif

#ifndef WIFI_PASS
#define WIFI_PASS ""
#endif

// Camera options
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y2_GPIO_NUM 5
#define Y3_GPIO_NUM 18
#define Y4_GPIO_NUM 19
#define Y5_GPIO_NUM 21
#define Y6_GPIO_NUM 36
#define Y7_GPIO_NUM 39
#define Y8_GPIO_NUM 34
#define Y9_GPIO_NUM 35
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

// Max framerate
static const uint32_t MAX_FPS = 30;
// Minimum time (ms) between two frames
static const uint32_t MIN_TIME = 1000 / MAX_FPS;

// https://www.w3.org/Protocols/rfc1341/7_2_Multipart.html
#define PART_BOUNDARY "SwarmBotsCamMultipartMJPEGBoundary"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t stream_httpd = NULL;

static esp_err_t stream_handler(httpd_req_t *req)
{
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    char *part_buf[64];

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);

    int32_t last = millis();
    while (res == ESP_OK)
    {
        // limit fps
        if (last + MIN_TIME > millis())
        {
            continue;
        }
        last = millis();

        // get actual image
        fb = esp_camera_fb_get();
        if (!fb)
        {
            Serial.println("Camera capture failed");
            res = ESP_FAIL;
            break;
        }

        // Send frame header
        if (res == ESP_OK)
        {
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, fb->len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }

        // Send frame image data
        if (res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
        }

        // Send frame boundary
        if (res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }

        // clear frame buffer again for re-use
        esp_camera_fb_return(fb);
        fb = NULL;
    }

    return res;
}

void setup()
{
    // disable brownout detection because the WiFi module can pull quite a bit of current on startup
    // this *can* lead to other issues, but is a software "fix" for the issue
    // a "proper" hardware fix would be to provide better decoupling capacitors
    // https://arduino.stackexchange.com/a/76692
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    Serial.begin(115200);
    Serial.println("Starting up!");

    // Get rid of that weird flimmer of the flash led
    pinMode(4, OUTPUT);
    // turn it off once it's ready
    analogWrite(4, 50);

    /* Camera setup */

    // New camera config object
    camera_config_t config;
    // Generating the camera clock signal
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.xclk_freq_hz = 20000000;
    // Pin config
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;

    // Quality and format
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 20;
    config.fb_count = 2;

    // Camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    /* Setting up WLAN */
    Serial.printf("Connecting to WLAN network '%s' using password '%s'", WIFI_SSID, WIFI_PASS);

    // Set hostname
#ifdef CAM_ID
    // sbcam = SwarmBots cam
    WiFi.setHostname("sbcam-" CAM_ID);
#else
    // Set hostname based on last 3 octets of MAC address
    uint8_t mac[6];
    char hostname[12] = {0};
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    sprintf(hostname, "sbcam-%02x%02x%02x", mac[3], mac[4], mac[5]);
    WiFi.setHostname(hostname);
#endif

    // Set mode and connect to AP
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    // Wait until connected
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.printf("\nConnected to the WLAN network '%s' as '%s' (MAC %s)\n", WIFI_SSID, WiFi.getHostname(), WiFi.macAddress().c_str());
    Serial.print("Camera stream ready at http://");
    Serial.println(WiFi.localIP());
    // Connection has been established
    digitalWrite(4, LOW);

    /* Start serving the stream */

    // default port is 80
    httpd_config_t srv_config = HTTPD_DEFAULT_CONFIG();

    httpd_uri_t index_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = stream_handler,
        .user_ctx = NULL};

    if (httpd_start(&stream_httpd, &srv_config) == ESP_OK)
    {
        httpd_register_uri_handler(stream_httpd, &index_uri);
    }
}

void loop()
{
    // do nothing
}