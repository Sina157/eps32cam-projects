#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
// ==================== WiFi ====================

const char* ssid = "WIFI NAME";
const char* password = "PASSWORD";

// ==================== ESP32-CAM ====================

// AI Thinker ESP32-CAM
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5

#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_pm.h"

void testLinkQuality() {
  wifi_ap_record_t ap_info;
  esp_wifi_sta_get_ap_info(&ap_info);
  
  Serial.printf("RSSI: %d dBm\n", ap_info.rssi);
  Serial.printf("Channel: %d\n", ap_info.primary);
  Serial.printf("Noise floor: %d dBm\n", 
                ap_info.rssi - ap_info.rssi);  // تقریبی
}

void setup() {
  //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  //setCpuFrequencyMhz(80);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  //WiFi.mode(WIFI_STA);

  
  Serial.begin(9600);
  delay(1000);
  // ==================== Camera ====================

  Serial.println("Initializing camera...");

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

  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //config.frame_size = FRAMESIZE_QVGA;   // 320x240
  //config.jpeg_quality = 30;            
  config.frame_size = FRAMESIZE_SVGA; // 800×600
  config.jpeg_quality = 8;
  //config.frame_size = FRAMESIZE_XGA; // 1280×1024
  //config.jpeg_quality = 5;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();

  // نوردهی خودکار
  s->set_exposure_ctrl(s, 1);
  s->set_gain_ctrl(s, 1);
  s->set_awb_gain(s, 1);
  s->set_whitebal(s, 1);

  // تصویر
  s->set_brightness(s, 1);
  s->set_contrast(s, 1);
  s->set_saturation(s, 0);
  s->set_sharpness(s, 1);

  // نوردهی کمی بیشتر
  s->set_ae_level(s, 1);

  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    return;
  }

  Serial.println("Camera initialized!");


  // ==================== WiFi ====================

  Serial.println("Connecting to WiFi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");

  Serial.print("IP address: ");
  Serial.printf("\nWiFi OK | RSSI: %d dBm | IP: %s\n",
                WiFi.RSSI(), WiFi.localIP().toString().c_str());

  // ==================== Take Picture ====================

  Serial.println("Taking picture...");

  camera_fb_t *fb = esp_camera_fb_get();

  if (!fb) {
    Serial.println("Capture failed!");
    return;
  }

  Serial.println("Capture successful!");

  Serial.print("Width: ");
  Serial.println(fb->width);

  Serial.print("Height: ");
  Serial.println(fb->height);

  Serial.print("Format: ");
  Serial.println(fb->format);

  Serial.print("Image size: ");
  Serial.print(fb->len);
  Serial.println(" bytes");
  if (SendByGoogleScript(fb) == false){
  SendByCloudFlare(fb);
  }
  // آزاد کردن حافظه
  esp_camera_fb_return(fb);
}




#include "mbedtls/base64.h"

const char* workerURL =
  "YOUR GOOGLE SCRIPT LINK";

bool SendByGoogleScript(camera_fb_t *fb) {
  bool Success = false;

  Serial.printf("Image size: %d bytes\n", fb->len);

  // -----------------------------
  // محاسبه حجم Base64
  // -----------------------------
  size_t base64Length = 4 * ((fb->len + 2) / 3);
  Serial.printf("Base64 size: %d bytes\n", base64Length);

  // -----------------------------
  // رزرو RAM
  // -----------------------------
  char *base64Image = (char*)ps_malloc(base64Length + 1);  // ← PSRAM!
  if (!base64Image) {
    Serial.println("ERROR: Not enough RAM for Base64!");
    esp_camera_fb_return(fb);
    return false;
  }

  // -----------------------------
  // تبدیل JPEG به Base64
  // -----------------------------
  size_t outputLength = 0;
  int result = mbedtls_base64_encode(
      (unsigned char*)base64Image,
      base64Length + 1,
      &outputLength,
      fb->buf,
      fb->len
  );

  if (result != 0) {
    Serial.printf("Base64 encode failed: %d\n", result);
    free(base64Image);
    esp_camera_fb_return(fb);
    return false;
  }
  base64Image[outputLength] = '\0';
  Serial.printf("Base64 created: %d bytes\n", outputLength);

  // -----------------------------
  // WiFi power کم کن (قبل از اتصال)
  // -----------------------------
  WiFi.setTxPower(WIFI_POWER_8_5dBm);

  // -----------------------------
  // HTTPS
  // -----------------------------
  WiFiClientSecure client;
  client.setInsecure();                          // برای تست
  client.setTimeout(20000);                      // ۲۰ ثانیه تایم‌اوت

  const char* host = "script.google.com";

  Serial.println("Connecting to Google Apps Script...");
  if (!client.connect(host, 443)) {
    Serial.println("Connection failed!");
    free(base64Image);
    esp_camera_fb_return(fb);
    return false;
  }
  Serial.println("Connected!");

  // -----------------------------
  // استخراج مسیر
  // -----------------------------
  String url = String(workerURL);
  int pathStart = url.indexOf("script.google.com");
  if (pathStart < 0) {
    Serial.println("ERROR: Invalid URL!");
    client.stop();
    free(base64Image);
    esp_camera_fb_return(fb);
    return false;
  }
  pathStart += strlen("script.google.com");
  String path = url.substring(pathStart);

  // -----------------------------
  // HTTP Header
  // -----------------------------
  client.print("POST "); client.print(path); client.println(" HTTP/1.1");
  client.println("Host: script.google.com");
  client.println("Content-Type: text/plain");
  client.print("Content-Length: "); client.println(outputLength);
  client.println("Connection: close");
  client.println();

  // ⚡ به TLS فرصت بده header رو بفرسته
  client.flush();
  delay(50);
  yield();

  // -----------------------------
  // ارسال Base64 — با retry واقعی
  // -----------------------------
  const size_t chunkSize = 512;                 // ← کوچیک‌تر
  size_t sent = 0;
  size_t retries = 0;
  const size_t maxRetries = 20;

  Serial.println("Sending Base64 in chunks...");

  while (sent < outputLength) {

    // چک کن اتصال هنوز برقراره
    if (!client.connected()) {
      Serial.println("❌ Connection lost during send!");
      break;
    }

    size_t remaining = outputLength - sent;
    size_t currentChunk = (remaining > chunkSize) ? chunkSize : remaining;

    size_t written = client.write(
      (const uint8_t*)base64Image + sent,
      currentChunk
    );

    if (written == 0) {
      // ⚡ retry کن، نه break
      retries++;
      Serial.printf("⚠️ Write returned 0 (retry %d/%d) at %d/%d\n",
                    retries, maxRetries, sent, outputLength);

      if (retries > maxRetries) {
        Serial.println("❌ Too many retries, aborting!");
        break;
      }

      // ولتاژ و WiFi فرصت برگشتن پیدا کنن
      delay(100);
      yield();
      continue;
    }

    sent += written;
    retries = 0;                                // موفق = reset retry

    // هر ۴KB یه بار log
    if (sent % 4096 == 0 || sent == outputLength) {
      Serial.printf("Sent: %d / %d\n", sent, outputLength);
    }

    // ⚡ به جای delay(20)، yield + delay کوتاه
    yield();
    delay(5);                                   // خیلی کوتاه‌تر
  }

  // -----------------------------
  // بررسی ارسال کامل
  // -----------------------------
  if (sent != outputLength) {
    Serial.printf("❌ Not all sent! %d / %d\n", sent, outputLength);
    client.stop();
    free(base64Image);
    esp_camera_fb_return(fb);
    return false;
  }

  Serial.println("✅ All Base64 bytes sent!");

  // -----------------------------
  // دریافت پاسخ Google — بدون String
  // -----------------------------
  Serial.println("Waiting for response...");

  unsigned long timeout = millis();
  bool headerDone = false;
  char lineBuf[128];
  size_t linePos = 0;

  while (client.connected() && millis() - timeout < 15000) {
    while (client.available()) {
      char c = client.read();

      if (c == '\n') {
        lineBuf[linePos] = '\0';
        Serial.println(lineBuf);
        linePos = 0;
        timeout = millis();

        if (!headerDone && lineBuf[0] == '\r') {
          headerDone = true;
        }
      } else if (c != '\r' && linePos < sizeof(lineBuf) - 1) {
        lineBuf[linePos++] = c;
      }
    }
    yield();
  }

  client.stop();

  free(base64Image);
  esp_camera_fb_return(fb);

  Serial.println("Done.");
  Success = true;
  return Success;
}

void SendByCloudFlare(camera_fb_t *fb) {


  WiFiClientSecure client;
  client.setInsecure();

  const char* host = "CLOUDFLARE WOTKER LINK";
  const int port = 443;

  Serial.println("Connecting to Cloudflare...");

  if (!client.connect(host, port)) {

    Serial.println("Connection to Cloudflare failed!");

    esp_camera_fb_return(fb);
    return;
  }

  Serial.println("Connected!");

  // HTTP Header
  client.println("POST / HTTP/1.1");
  client.print("Host: ");
  client.println(host);
  client.println("Content-Type: image/jpeg");

  client.print("Content-Length: ");
  client.println(fb->len);

  client.println("Connection: close");
  client.println();

  // ارسال تکه تکه
  const size_t chunkSize = 1024;

  size_t sent = 0;

  while (sent < fb->len) {
    delay(20);
    size_t remaining = fb->len - sent;

    size_t currentChunk =
      (remaining > chunkSize)
      ? chunkSize
      : remaining;

    size_t written = client.write(
      fb->buf + sent,
      currentChunk
    );

    if (written == 0) {

      Serial.println("Write failed!");
      break;
    }

    sent += written;

    Serial.print("Sent: ");
    Serial.print(sent);
    Serial.print(" / ");
    Serial.println(fb->len);

    delay(5);
  }

  Serial.println();
  Serial.println("Finished sending.");

  if (sent == fb->len) {
    Serial.println("All image bytes were successfully transmitted by Cloudflare.");
  } else {
    Serial.println("WARNING: Image was not fully sent!");
  }

  // دریافت پاسخ Worker
  unsigned long timeout = millis();

  while (client.connected() &&
         millis() - timeout < 15000) {

    while (client.available()) {

      String line =
        client.readStringUntil('\n');

      Serial.println(line);

      timeout = millis();
    }
  }

  client.stop();

  esp_camera_fb_return(fb);

  Serial.println("Done.");
}


void loop() {

}
