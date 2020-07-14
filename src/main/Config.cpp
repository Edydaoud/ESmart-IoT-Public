#include "Config.hpp"

Config::Config(){};

bool Config::loadConfig()
{
  if (!LittleFS.begin())
  {
    DEBUG_PRINTLN("Failed to mount file system");
    return false;
  }

  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile)
  {
    DEBUG_PRINTLN("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024)
  {
    DEBUG_PRINTLN("Config file size is too large");
    return false;
  }

  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);

  DynamicJsonDocument doc(200);
  auto error = deserializeJson(doc, buf.get());
  if (error)
  {
    DEBUG_PRINTLN("Failed to parse config file");
    return false;
  }

  setWifiAp(doc["wifiAp"]);
  setWifiPass(doc["wifiPass"]);
  setUserId(doc["userId"]);
  setFirebaseUrl(doc["firebaseUrl"]);
  setFirebaseKey(doc["firebaseKey"]);

  LittleFS.end();

  DEBUG_PRINT("wifiAp: ");
  DEBUG_PRINTLN(wifiAp());
  DEBUG_PRINT("wifiPass: ");
  DEBUG_PRINTLN(wifiPass());
  DEBUG_PRINT("userId: ");
  DEBUG_PRINTLN(userId());
  DEBUG_PRINT("firebaseUrl: ");
  DEBUG_PRINTLN(firebaseUrl());
  DEBUG_PRINT("firebaseKey: ");
  DEBUG_PRINTLN(firebaseKey());

  return true;
}

String Config::wifiAp()
{
  return _wifiAp;
};

String Config::wifiPass()
{
  return _wifiPass;
};
String Config::userId()
{
  return _userId;
};
String Config::firebaseUrl()
{
  return _firebaseUrl;
};
String Config::firebaseKey()
{
  return _firebaseKey;
};

void Config::setWifiAp(String wifiAp)
{
  _wifiAp = wifiAp;
};
void Config::setWifiPass(String wifiPass)
{
  _wifiPass = wifiPass;
};
void Config::setUserId(String userId)
{
  _userId = userId;
};
void Config::setFirebaseUrl(String firebaseUrl)
{
  _firebaseUrl = firebaseUrl;
};
void Config::setFirebaseKey(String firebaseKey)
{
  _firebaseKey = firebaseKey;
};