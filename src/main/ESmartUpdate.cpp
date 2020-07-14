#include "ESmartUpdate.hpp"
#include "Config.hpp"

// #define DEBUG

ESmartUpdate::ESmartUpdate(){};

void ESmartUpdate::handleUpdate(String jsonStr)
{
  DynamicJsonDocument object(1024);
  deserializeJson(object, jsonStr);

  int version = object["version"];
  String url = object["url"];
  String host = object["host"];

  DEBUG_PRINT("handeling update old version: ");
  DEBUG_PRINT(VERSION);
  DEBUG_PRINT(" new version: ");
  DEBUG_PRINTLN(version);
  Updates update = Updates{version};

  strcpy(update.url, url.c_str());
  strcpy(update.host, host.c_str());
  DEBUG_PRINT(update.host);
  DEBUG_PRINTLN(update.url);
  saveUpdates(update);
}

void ESmartUpdate::checkForUpdate()
{
  Updates update;
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(address, update);
  EEPROM.end();
  DEBUG_PRINT(update.host);
  DEBUG_PRINTLN(update.url);
  if (VERSION < update.version)
  {
    DEBUG_PRINT("Update found old version: ");
    DEBUG_PRINT(VERSION);
    DEBUG_PRINT(" new version: ");
    DEBUG_PRINTLN(update.url);
    startUpdate(update.host, update.url);
  }
}

void ESmartUpdate::startUpdate(char *downloadHost, char *firmwarelink)
{
  int size = strlen(downloadHost) + strlen(firmwarelink) + 10;
  char link[size];
  snprintf(link, size, "https://%s%s", downloadHost, firmwarelink);
  DEBUG_PRINT("Starting OTA from: ");
  DEBUG_PRINTLN(link);
  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(1000);
  DEBUG_PRINT("connecting to ");
  DEBUG_PRINTLN(downloadHost);
  if (!client.connect(downloadHost, httpsPort))
  {
    DEBUG_PRINTLN("connection failed");
    return;
  }
  else
  {
    DEBUG_PRINTLN("connected!");
  }
  auto ret = ESPhttpUpdate.update(client, link);
  DEBUG_PRINT("update failed, result: ");
  DEBUG_PRINTLN((int)ret);
  return;
}

void ESmartUpdate::saveUpdates(const Updates &update)
{
  if (update.host != NULL && update.url != NULL && update.host != "null" && update.url != "null")
  {
    DEBUG_PRINTLN("Saving update");
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(address, update);
    EEPROM.commit();
    EEPROM.end();
  }
}
