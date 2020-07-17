#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <LittleFS.h>
#include <OneButton.h>
#include <ezTime.h>
#include <TimeAlarms.h>

#include <FutureJob.hpp>

#include "EsmartFirebase.hpp"

#define JSON "json"
#define STATE "state"
#define RELAY_STATE "relayState"
#define NEUTRE -1

#ifdef DEBUG_ESMART
#define INFO(fmt, ...) Serial.printf(fmt, __VA_ARGS__)
#define INFOM(...) Serial.println(F(__VA_ARGS__))
#else
#define INFO(fmt, ...)
#define MINFO(...)
#endif

class Configs {
   public:
    String firebaseUrl;
    String firebaseToken;
    String userId;
    String wifiAp;
    String wifiPass;

    Configs(){};

    Configs(String url, String token, String id, String essid, String pass) {
        firebaseUrl = url;
        firebaseToken = token;
        userId = id;
        wifiAp = essid;
        wifiPass = pass;
    }

    String getUserPath(String id = "") {
        return "/users/" + userId + "/" + WiFi.macAddress() + "/" + id;
    }
};

DynamicJsonDocument localData(2048);
Configs configs;

FirebaseData firebaseStreamData;
FirebaseData firebaseJobData;

int longPressReset;
int maxRetry = 30;
std::vector<OneButton> buttons;

void loop();
void setup();

bool loadConfigs();
void connect();
void begin();
void initLocalData();

void streamCallback(StreamData data);
void handleReceivedData(DynamicJsonDocument &document);

bool beginWrite() { return LittleFS.begin(); }
void endWrite() { LittleFS.end(); }

void setLocalData(EsmartFirebase &esmart);
void updateNode(EsmartFirebase &esmart);
void createButton(EsmartFirebase &esmart);

void doWork(FutureJob &work);
void createButton(EsmartFirebase &esmart);
void createOffAlarm(EsmartFirebase &esmart);
void createOnAlarm(EsmartFirebase &esmart);
void createAlarms(EsmartFirebase &esmart);