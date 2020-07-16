#include "Config.hpp"

Config::Config(){};

void Config::setHasInternet(bool hasInternet) { _hasInternet = hasInternet; };
bool Config::isConnected() { return _hasInternet; };

bool Config::loadConfig() {
    if (!LittleFS.begin()) {
        DEBUG_PRINTLN("Failed to mount file system");
        return false;
    }

    File configFile = LittleFS.open("/config.json", "r");
    File localDataFile = LittleFS.open("/data.json", "r");

    if (!configFile) {
        DEBUG_PRINTLN("Failed to open config file");
        return false;
    }
    if (!localDataFile) DEBUG_PRINTLN("Failed to open data file");

    StaticJsonDocument<1024> configDoc;
    StaticJsonDocument<1024> esmartDoc;

    auto error1 = deserializeJson(configDoc, configFile);
    auto error2 = deserializeJson(esmartDoc, localDataFile);

    localData = std::make_shared<StaticJsonDocument<1024>>(esmartDoc);

    if (error1) {
        DEBUG_PRINTLN("Failed to parse config file");
        return false;
    }
    if (error2) {
        DEBUG_PRINTLN("Failed to parse data file");
    }

    setWifiAp(configDoc["wifiAp"]);
    setWifiPass(configDoc["wifiPass"]);
    setUserId(configDoc["userId"]);
    setFirebaseUrl(configDoc["firebaseUrl"]);
    setFirebaseKey(configDoc["firebaseKey"]);

    configFile.close();
    localDataFile.close();
    configDoc.clear();

    return true;
}

String Config::wifiAp() { return _wifiAp; };

String Config::wifiPass() { return _wifiPass; };
String Config::userId() { return _userId; };
String Config::firebaseUrl() { return _firebaseUrl; };
String Config::firebaseKey() { return _firebaseKey; };

String Config::pingPath() { return "status/" + WiFi.macAddress() + "/lastSeen"; }

Updates Config::getLocalUpdateData() { return update; }

void Config::setWifiAp(String wifiAp) { _wifiAp = wifiAp; };
void Config::setWifiPass(String wifiPass) { _wifiPass = wifiPass; };
void Config::setUserId(String userId) { _userId = userId; };
void Config::setFirebaseUrl(String firebaseUrl) { _firebaseUrl = firebaseUrl; };
void Config::setFirebaseKey(String firebaseKey) { _firebaseKey = firebaseKey; };

void Config::setLocalData(std::shared_ptr<DynamicJsonDocument> newData) {
    JsonObject s = newData.get()->as<JsonObject>();
    File localDataFile = LittleFS.open("/data.json", "w+");
    if (!localDataFile) DEBUG_PRINTLN("Failed to open data file");
    serializeJson(s, localDataFile);
    newData.reset();
    localDataFile.close();
    s.clear();
};

void Config::setLocalSingleData(String &id, std::shared_ptr<DynamicJsonDocument> newData) {
    DEBUG_PRINTLN("setLocalSingleData");

    JsonObject s = newData.get()->as<JsonObject>();

    File localDataFile = LittleFS.open("/data.json", "r+");
    DynamicJsonDocument doc(1024);

    if (!localDataFile) DEBUG_PRINTLN("Failed to open data file");

    if (deserializeJson(doc, localDataFile)) DEBUG_PRINTLN("Failed to deserialize data file");

    doc[id] = s;
    setLocalData(std::make_shared<DynamicJsonDocument>(doc));
    newData.reset();
    localDataFile.close();
    doc.clear();
};

void Config::changeLocalState(String id, int newState) {
    DEBUG_PRINTLN("changeLocalState");
    DEBUG_PRINTLN(id);
    DEBUG_PRINTLN(newState);
    File localDataFile = LittleFS.open("/data.json", "r+");
    DynamicJsonDocument doc(1024);

    if (!localDataFile) DEBUG_PRINTLN("Failed to open data file");

    if (deserializeJson(doc, localDataFile)) DEBUG_PRINTLN("Failed to deserialize data file");

    doc[id]["state"] = newState;
    doc[id]["relayState"] = newState;

    serializeJson(doc, Serial);
    setLocalData(std::make_shared<DynamicJsonDocument>(doc));
    localDataFile.close();
};
