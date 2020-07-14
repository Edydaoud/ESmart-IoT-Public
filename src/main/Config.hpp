#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#define EID "id"

#define VERSION 2

#define EEPROM_SIZE 512

#define ON 0
#define OFF 1
#define NEUTRE -1

#ifdef DEBUG_ESMART
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINT(x) Serial.print(x)
#else
#define DEBUG_PRINT(x)
#endif

class Config
{
private:
    void setWifiAp(String wifiAp);
    void setWifiPass(String wifiPass);
    void setUserId(String userId);
    void setFirebaseUrl(String firebaseUrl);
    void setFirebaseKey(String firebaseKey);

public:
    Config();
    String _wifiAp;
    String _wifiPass;
    String _userId;
    String _firebaseUrl;
    String _firebaseKey;

    bool loadConfig();

    String wifiAp();
    String wifiPass();
    String userId();
    String firebaseUrl();
    String firebaseKey();
};
