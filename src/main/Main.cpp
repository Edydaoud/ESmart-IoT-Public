#include <Arduino.h>
#include "ESmart.hpp"
#include "ESmartUpdate.hpp"
#include "Config.h"
#include "LittleFS.h"

ESmartUpdate updateHelper;
Config config;
ESmart esmart;

const int memBase = 350;

void callback(StreamData data) { esmart.streamCallback(data); }

void updateCallback(String data) { updateHelper.handleUpdate(data); }

void setup()
{
  Serial.begin(115200);
  delay(1000);
  DEBUG_PRINTLN("Mounting FS...");

  if (config.loadConfig())
  {
    DEBUG_PRINTLN("Config loaded");
    WiFi.begin(config.wifiAp(), config.wifiPass());
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(100);
    }
  }
  else
  {
    DEBUG_PRINTLN("Failed to load config");
    return;
  }

  esmart.setConfig(config);
  esmart.initLocalTime();

  updateHelper.checkForUpdate();

  Firebase.begin(config.firebaseUrl(), config.firebaseKey());
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
}

void loop()
{
  esmart.tickButtons();
}