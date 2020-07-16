#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <StreamUtils.h>

#include "EEPROM.h"

#define EID "id"

#define VERSION 3

#define EEPROM_SIZE 512

#define ON 0
#define OFF 1
#define NEUTRE -1

#ifdef DEBUG_ESMART
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINT(x) Serial.print(x)
#else
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINT(x)
#endif
// namespace

struct Updates {
    int version;
    char host[32];
    char url[64];
};

class Config {
   private:
    String _wifiAp;
    String _wifiPass;
    String _userId;
    String _firebaseUrl;
    String _firebaseKey;
    bool _hasInternet;

   public:
    Config();
    std::shared_ptr<StaticJsonDocument<1024>> localData;

    Updates update;

    bool loadConfig();

    void setWifiAp(String wifiAp);
    void setWifiPass(String wifiPass);
    void setUserId(String userId);
    void setFirebaseUrl(String firebaseUrl);
    void setFirebaseKey(String firebaseKey);

    void setLocalData(std::shared_ptr<DynamicJsonDocument> newData);
    void setLocalSingleData(String &id, std::shared_ptr<DynamicJsonDocument> newData);
    void changeLocalState(String id, int newState);
    void setHasInternet(bool hasInternet);
    bool isConnected();

    String wifiAp();
    String wifiPass();
    String userId();
    String firebaseUrl();
    String firebaseKey();
    String pingPath();

    Updates getLocalUpdateData();
};
