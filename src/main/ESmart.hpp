#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include <FirebaseESP8266.h>
#include <LittleFS.h>
#include <NTPClient.h>
#include <OneButton.h>
#include <TimeAlarms.h>
#include <WiFiUdp.h>

#include <FutureJob.hpp>

#include "EsmartFirebase.hpp"

#define JSON "json"
#define STATE "state"
#define RELAY_STATE "relayState"
#define NEUTRE -1
#define WIFI_TIMEOUT 50
#define NTP_TIMEOUT 5

#ifdef DEBUG_ESMART
#define INFO(fmt, ...) Serial.printf(fmt, __VA_ARGS__)
#define INFOM(...) Serial.println(F(__VA_ARGS__))
#else
#define INFO(fmt, ...)
#define INFOM(...)
#endif

#ifdef INVERTED_PINS
#define WRITE_OPERATOR 1
#define READ_OPERATOR 1
#else
#define WRITE_OPERATOR 0
#define READ_OPERATOR 0
#endif

#define VERSION 2
#define FIRMWARE_PATH "/firmware/"

class UpdateConfig {
   public:
    String host;
    String url;
    int version;

    UpdateConfig(String serverHost, String serverUrl, int serverVersion) {
        host = serverHost;
        url = serverUrl;
        version = serverVersion;
    }

    UpdateConfig(DynamicJsonDocument document) {
        setHost(document["host"]);
        setUrl(document["url"]);
        setVersion(document["version"]);
    }

    void setHost(String serverHost) { host = serverHost; }
    void setUrl(String serverUrl) { url = serverUrl; }
    void setVersion(int serverVersion) { version = serverVersion; }

    String getUpdateUrl() { return "https://" + host + url; }
};

class Configs {
   public:
    String firebaseUrl;
    String firebaseKey;
    String userId;
    String wifiAp;
    String wifiPass;

    Configs(){};

    Configs(DynamicJsonDocument &document) {
        setUrl(document["firebaseUrl"]);
        setKey(document["firebaseKey"]);
        setUserId(document["userId"]);
        setAp(document["wifiAp"]);
        setPass(document["wifiPass"]);
    }

    String getUserPath(String id = "") { return "/users/" + userId + "/" + WiFi.macAddress() + "/" + id; }

    void setUrl(String url) { firebaseUrl = url; }
    void setKey(String key) { firebaseKey = key; }
    void setUserId(String id) { userId = id; }
    void setAp(String ap) { wifiAp = ap; }
    void setPass(String pass) { wifiPass = pass; }
};

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

Configs configs;

FirebaseData firebaseStreamData;
FirebaseData firebaseJobData;

int longPressReset;
bool isConnected = false;

std::vector<OneButton> buttons;

const int16_t httpsPort = 443;

void loop();
void setup();

bool loadConfigs();
void connect();
void begin();
void initLocalData(DynamicJsonDocument &document);

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

int readPin(int pin);
void writePin(int pin, int statusPin, int val);

bool isInternetConnected() { return isConnected && WiFi.status() == WL_CONNECTED; }

void startUpdate(UpdateConfig &config);
void saveUpdates(DynamicJsonDocument &doc);
void handleUpdate(String jsonStr);
bool checkForNewVersion();
void checkForServerUpdate();