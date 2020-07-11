#include "ESmart.hpp"

ESmart::ESmart(){};

String ESmart::getUserId() { return USER_ID; }

String ESmart::getUserPath(String id) { return "/users/" + getUserId() + "/" + WiFi.macAddress() + "/" + id; }

String ESmart::getUpdatePath() { return "/update/"; }

bool ESmart::isAlarmEnabled() { return alarmEnabled; }

void ESmart::setAlarmEnabled(bool isEnabled) { alarmEnabled = isEnabled; };

EsmartFirebase ESmart::getData(String id) { return data[id.c_str()]; };

void ESmart::setData(EsmartFirebase esmart) { data[esmart.id.c_str()] = esmart; };

void ESmart::initLocalTime()
{
  waitForSync();
  setTime(UTC.now());
}

void ESmart::tickButtons()
{
  for (size_t i = 0; i < buttons.size(); i++)
  {
    buttons[i].tick();
  }
  Alarm.delay(1000);
}

void ESmart::streamCallback(StreamData data)
{
  if (data.dataType() == "json")
  {
    Serial.println(data.jsonString());
    DynamicJsonDocument object(5120);
    deserializeJson(object, data.jsonString());
    changeRelayState(&object);
  }
}

void ESmart::click(int pin, String id)
{
  FirebaseJson json;
  invertPin(pin);
  json.set("relayState", readPin(pin));
  json.set("state", readPin(pin));
  Firebase.updateNode(firebaseWriteData, getUserPath(id), json);
}

void ESmart::createButton(int pin, int buttonPin, String id)
{
  OneButton button(buttonPin, id, true, true);
  button.attachClick([&](int pin, String id) { click(pin, id); }); // Attach with int literal
  buttons.push_back(button);
}

void ESmart::scheduleOff(int pin, String id)
{
  disablePin(pin - 1);

  FirebaseJson json;
  json.set("relayState", readPin(pin - 1));
  json.set("state", readPin(pin - 1));

  Firebase.updateNode(firebaseWriteData, getUserPath(id), json);
}

void ESmart::scheduleOn(int pin, String id)
{
  enablePin(pin);

  FirebaseJson json;
  json.set("relayState", readPin(pin));
  json.set("state", readPin(pin));

  Firebase.updateNode(firebaseWriteData, getUserPath(id), json);
}

void ESmart::creatOffAlarm(int pin, int time, String id)
{
  time_t offTime = static_cast<time_t>(time);
  if (Alarm.isAllocated(pin + 1))
  {
    if (time != 0)
    {
      tmElements_t element;
      breakTime(offTime, element);
      Alarm.write(pin + 1, AlarmHMS(element.Hour, element.Minute, element.Second));
    }
    else
      Alarm.free(pin + 1);
  }
  else if (time > 0)
  {
    tmElements_t element;
    breakTime(offTime, element);
    Alarm.alarmRepeat(element.Hour, element.Minute, element.Second, pin + 1, id, [&](int relayPin, String relayId) {
      scheduleOff(relayPin, relayId);
    });

    setAlarmEnabled(true);
  }
}

void ESmart::creatOnAlarm(int pin, int time, String id)
{
  time_t onTime = static_cast<time_t>(time);
  if (Alarm.isAllocated(pin))
  {
    if (time != 0)
    {
      tmElements_t element;
      breakTime(onTime, element);
      Alarm.write(pin, AlarmHMS(element.Hour, element.Minute, element.Second));
    }
    else
      Alarm.free(pin);
  }
  else
  {
    tmElements_t element;
    breakTime(onTime, element);
    Alarm.alarmRepeat(element.Hour, element.Minute, element.Second, pin, id, [&](int relayPin, String relayId) {
      scheduleOn(relayPin, relayId);
    });
  }
}

void ESmart::changeRelayState(DynamicJsonDocument *const &doc)
{
  if (!doc->operator[]("id").isNull())
  {
    String eid = doc->operator[]("id");
    EsmartFirebase esmartFirebase;
    esmartFirebase.init(doc);
    Serial.println(esmartFirebase);

    if (esmartFirebase.id != "null")
    {
      setData(esmartFirebase);

      setPin(esmartFirebase.pin, esmartFirebase.state);

      if (esmartFirebase.relayState != readPin(esmartFirebase.pin))
      {
        FirebaseJson json;
        json.set("relayState", readPin(esmartFirebase.pin));
        Firebase.updateNode(firebaseWriteData, getUserPath(esmartFirebase.id), json);
      }

      creatOnAlarm(esmartFirebase.pin, esmartFirebase.startTime, esmartFirebase.id);
      creatOffAlarm(esmartFirebase.pin, esmartFirebase.endTime, esmartFirebase.id);
    }
  }
  else if (doc->operator[]("relayState").isNull())
  {
    JsonObject obj = doc->as<JsonObject>();
    for (auto kv : obj)
    {
      EsmartFirebase esmartFirebase;
      Serial.println(esmartFirebase);

      esmartFirebase.init(kv.key().c_str(), doc);

      setData(esmartFirebase);

      createButton(esmartFirebase.pin, esmartFirebase.buttonPin, esmartFirebase.id);

      setOutput(esmartFirebase.pin);

      if (esmartFirebase.defaultState == -1 && esmartFirebase.relayState != esmartFirebase.pin)
      {
        FirebaseJson json;
        setPin(esmartFirebase.pin, esmartFirebase.state);
        json.set("relayState", esmartFirebase.state);
        Firebase.updateNode(firebaseWriteData, getUserPath(esmartFirebase.id), json);
      }
      else if (esmartFirebase.defaultState != -1)
      {
        setPin(esmartFirebase.pin, esmartFirebase.defaultState);
        if (esmartFirebase.relayState != readPin(esmartFirebase.pin))
        {
          FirebaseJson json;
          json.set("relayState", esmartFirebase.defaultState);
          json.set("state", esmartFirebase.defaultState);
          Firebase.updateNode(firebaseWriteData, getUserPath(esmartFirebase.id), json);
        }
      }

      creatOnAlarm(esmartFirebase.pin, esmartFirebase.startTime, esmartFirebase.id);
      creatOffAlarm(esmartFirebase.pin, esmartFirebase.endTime, esmartFirebase.id);
    }
  }
}
