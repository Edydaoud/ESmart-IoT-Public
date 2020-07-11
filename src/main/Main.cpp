#include <Arduino.h>
#include "ESmart.hpp"
#include "ESmartUpdate.hpp"
#include "Config.h"

#define EEPROM_SIZE 512

#define DEBUG

ESmart esmart;
ESmartUpdate updateHelper;

const int memBase = 350;

void callback(StreamData data) { esmart.streamCallback(data); }

void updateCallback(String data)
{
  updateHelper.handleUpdate(data);
}

void setup()
{
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
  }

  esmart.initLocalTime();

  updateHelper.checkForUpdate();

  Firebase.begin(FIREBASE_URL, FIREBASE_KEY);
  Firebase.reconnectWiFi(true);

  Firebase.setMaxRetry(esmart.firebaseStreamData, 10);
  Firebase.setMaxErrorQueue(esmart.firebaseStreamData, 30);

  esmart.firebaseWriteData.setBSSLBufferSize(1024, 1024);
  esmart.firebaseWriteData.setResponseSize(1024);

  esmart.firebaseStreamData.setResponseSize(1024);
  esmart.firebaseStreamData.setBSSLBufferSize(1024, 1024);

  Firebase.setStreamCallback(esmart.firebaseStreamData, callback);
  Firebase.beginStream(esmart.firebaseStreamData, esmart.getUserPath(""));

  Firebase.getJSON(esmart.firebaseWriteData, "/firmware");
  updateCallback(esmart.firebaseWriteData.jsonString());
  Serial.print(WiFi.macAddress());
}

void loop()
{
  esmart.tickButtons();
}
