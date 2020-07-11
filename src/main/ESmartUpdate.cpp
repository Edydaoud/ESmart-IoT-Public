#include "ESmartUpdate.hpp"
#include "Config.h"

#define DEBUG

ESmartUpdate::ESmartUpdate(){};

void ESmartUpdate::handleUpdate(String jsonStr)
{
  DynamicJsonDocument object(1024);
  deserializeJson(object, jsonStr);

  int version = object["version"];
  String url = object["url"];
  String host = object["host"];

#ifdef DEBUG
  Serial.print("handeling update old version: ");
  Serial.print(VERSION);
  Serial.print(" new version: ");
  Serial.println(version);
#endif
  Updates update = Updates{version};

  strcpy(update.url, url.c_str());
  strcpy(update.host, host.c_str());

  Serial.println(update.host);
  Serial.println(update.url);
  saveUpdates(update);
}

void ESmartUpdate::checkForUpdate()
{
  Updates update;
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(address, update);
  EEPROM.end();
  Serial.println(update.host);
  Serial.println(update.url);

  if (VERSION < update.version)
  {
#ifdef DEBUG
    Serial.print("Update found old version: ");
    Serial.println(VERSION);
    Serial.print("new version: ");
    Serial.println(update.url);
#endif
    startUpdate(update.host, update.url);
  }
}

void ESmartUpdate::startUpdate(char *downloadHost, char *firmwarelink)
{
  int size = strlen(downloadHost) + strlen(firmwarelink) + 10;
  char link[size];
  snprintf(link, size, "https://%s%s", downloadHost, firmwarelink);
#ifdef DEBUG
  Serial.print("Starting OTA from: ");
  Serial.println(link);
#endif
  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(1000);
#ifdef DEBUG
  Serial.print("connecting to ");
  Serial.println(downloadHost);
#endif
  if (!client.connect(downloadHost, httpsPort))
  {
#ifdef DEBUG
    Serial.println("connection failed");
#endif
    return;
  }
  else
  {
#ifdef DEBUG
    Serial.println("connected!");
#endif
  }
  auto ret = ESPhttpUpdate.update(client, link);
#ifdef DEBUG
  Serial.println("update failed, result: ");
  Serial.print((int)ret);
#endif
  return;
}

void ESmartUpdate::saveUpdates(const Updates &update)
{
  if (update.host != NULL && update.url != NULL)
  {
#ifdef DEBUG
    Serial.println("Saving update");
#endif
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(address, update);
    EEPROM.commit();
    EEPROM.end();
  }
}
