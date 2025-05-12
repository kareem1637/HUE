#include <Arduino.h>
#include "esp_camera.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "WiFi.h"
#include <HTTPClient.h>
#include <WebServer.h>
const char* ssid = "ESP32-CAM";
const char* password = "12345678";
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22


WebServer server(80);

void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 1;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);

  IPAddress local_ip(192, 168, 4, 1);  // Fixed IP for ESP
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(local_ip, gateway, subnet);

  Serial.println("AP started");
  Serial.println(WiFi.softAPIP());

  initCamera();
  startCameraServer();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
}

void startCameraServer() {
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", "<html><body><img src='/stream'></body></html>");
  });
server.on("/capture", HTTP_GET, []() {
    camera_fb_t* fb = captureImage();
    if (!fb) {
        Serial.println("Failed to capture image");
        server.send(500, "text/plain", "Camera capture failed");
        return;
    }

    Serial.println("Sending image...");
    server.setContentLength(fb->len);
    server.send(200, "image/jpeg", "");

    WiFiClient client = server.client();
    client.write(fb->buf, fb->len);  // Send the image in binary format
    esp_camera_fb_return(fb);  // Return the frame buffer
});

  server.on("/stream", HTTP_GET, []() {
    WiFiClient client = server.client();

    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
    server.sendContent(response);

    while (1) {
      camera_fb_t * fb = esp_camera_fb_get();
      if (!fb) break;

      client.printf("--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %d\r\n\r\n", fb->len);
      client.write(fb->buf, fb->len);
      client.write("\r\n");
      esp_camera_fb_return(fb);
      if (!client.connected()) break;
    }
  });

  server.begin();
}
camera_fb_t* captureImage() {
  // Skip initial noisy frames
  for (int i = 0; i < 2; i++) {
    camera_fb_t* temp_fb = esp_camera_fb_get();
    if (temp_fb) {
      esp_camera_fb_return(temp_fb);
    } else {
      Serial.println("Warm-up capture failed");
    }
  }

  // Final capture
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Final camera capture failed");
    return nullptr;
  }

  Serial.println("Image captured successfully.");
  return fb;
}
