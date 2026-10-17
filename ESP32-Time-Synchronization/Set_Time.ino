#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <time.h>

const char* ssid = "WIFI_NAME";
const char* password = "PASSWORD";


bool syncTime() {

  Serial.println("Getting time...");

  WiFiClient client;
  HTTPClient http;

  if (!http.begin(client, "http://worldtime.timezone.io/api/timezone/Etc/UTC")) {
    Serial.println("HTTP begin failed!");
    return false;
  }

  int httpCode = http.GET();

  Serial.print("HTTP code: ");
  Serial.println(httpCode);

  if (httpCode != 200) {
    Serial.println("Time request failed!");
    http.end();
    return false;
  }

  String response = http.getString();

  http.end();

  // پیدا کردن unixtime
  int pos = response.indexOf("\"unixtime\":");

  if (pos == -1) {
    Serial.println("unixtime not found!");
    return false;
  }

  pos += 11;

  int endPos = response.indexOf(",", pos);

  String unixString =
    response.substring(pos, endPos);

  unsigned long unixTime =
    unixString.toInt();

  Serial.print("Unix time: ");
  Serial.println(unixTime);

  // تنظیم ساعت ESP32
  struct timeval tv;

  tv.tv_sec = unixTime;
  tv.tv_usec = 0;

  settimeofday(&tv, NULL);

  Serial.println("ESP32 clock synchronized!");

  // بررسی ساعت
  struct tm timeinfo;

  if (getLocalTime(&timeinfo)) {

    Serial.print("ESP32 UTC time: ");

    Serial.println(
      &timeinfo,
      "%Y-%m-%d %H:%M:%S"
    );

  } else {

    Serial.println("Could not read clock!");
    return false;
  }

  return true;
}




void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(9600);
  Serial.println("Connecting to WiFi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  if (syncTime()) {
    Serial.println("Time setup completed!");
  } else {
    Serial.println("Time setup FAILED!");
  }
  struct tm timeinfo;

  if (getLocalTime(&timeinfo)) {

    Serial.print("Time: ");

    Serial.println(
      &timeinfo,
      "%Y-%m-%d %H:%M:%S"
    );
  }  
  
}
void loop() {
  // put your main code here, to run repeatedly:

}
