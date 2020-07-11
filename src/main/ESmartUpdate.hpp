#include "ESP8266httpUpdate.h"
#include "EEPROM.h"
#include "ArduinoJson.h"
#include "Config.h"

struct Updates
{
    int version;
    char host[32];
    char url[64];
};

class ESmartUpdate
{

public:
    ESmartUpdate();
    const int address = 0;

    const int16_t httpsPort = 443;
    void startUpdate(char *downloadHost, char *firmwarelink);
    void saveUpdates(const Updates &update);
    void handleUpdate(String jsonStr);
    void checkForUpdate();
};