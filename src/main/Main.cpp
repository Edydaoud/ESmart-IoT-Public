#include <Arduino.h>

#include "Config.h"
#include "ESmart.hpp"
#include "ESmartUpdate.hpp"
#include "LittleFS.h"

ESmartUpdate updateHelper;
Config config;
ESmart esmart;

int retry = 50;

void callback(StreamData data) { esmart.streamCallback(data); }

void updateCallback(StreamData data) { updateHelper.handleUpdate(data.jsonString()); }

bool internetCheck() {
    // BearSSL::WiFiClientSecure client;
    // client.setInsecure();
    // client.setTimeout(500);
    // HTTPClient http;
    // http.begin(client, "google.com");

    // if (!http.begin(client, "google.com")) {
    //     DEBUG_PRINTLN("connection failed");
    //     http.end();
    //     return false;
    // } else {
    //     DEBUG_PRINTLN("connected!");
    //     http.end();
    //     return true;
    // }

    return true;
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    DEBUG_PRINTLN("Mounting FS...");

    if (config.loadConfig()) {
        DEBUG_PRINTLN("Config loaded");
        esmart.initLocalState(config.localData);
        config.localData.reset();

        WiFi.begin(config.wifiAp(), config.wifiPass());
        while (retry > 0) {
            if (WiFi.status() == WL_CONNECTED) break;
            delay(100);
            retry--;
        }
    } else {
        DEBUG_PRINTLN("Failed to load config");
        return;
    }

    config.setHasInternet(internetCheck());
    esmart.setConfig(config);

    delay(1000);

    if (config.isConnected()) {
        esmart.initLocalTime();
        // updateHelper.checkForUpdate();

        Firebase.setStreamCallback(esmart.firebaseStreamData, callback);
        Firebase.beginStream(esmart.firebaseStreamData, esmart.getUserPath(""));

        // Firebase.setStreamCallback(esmart.firebaseJobData, updateCallback);
        // Firebase.beginStream(esmart.firebaseJobData, "/firmware");
    }
}

void loop() {
    esmart.handleLoop();
}