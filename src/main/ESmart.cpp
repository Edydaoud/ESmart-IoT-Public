#include "ESmart.hpp"

ESmart::ESmart(){};

Config ESmart::getConfig() { return _config; };
void ESmart::setConfig(Config config) { _config = config; };

String ESmart::getUserId()
{
  return getConfig().userId();
}

String ESmart::getUserPath(String id) { return "/users/" + getUserId() + "/" + WiFi.macAddress() + "/" + id; }

String ESmart::getUpdatePath() { return "/update/"; }

bool ESmart::isAlarmEnabled() { return alarmEnabled; }

void ESmart::setAlarmEnabled(bool isEnabled) { alarmEnabled = isEnabled; };

EsmartFirebase ESmart::getData(String id) { return data[id.c_str()]; };

void ESmart::setData(EsmartFirebase esmart) { data[esmart.id.c_str()] = esmart; };

void ESmart::doWork(FutureJob work)
{
  DEBUG_PRINT("doing work: ");
  DEBUG_PRINTLN(work);
  if (work.getRelayId() != NULL && work.getRelayId() != "null")
  {
    FirebaseJson json;
    if (work.getRelayState() >= 0 && work.getRelayState() <= 1)
    {
      int newState = work.getRelayState();

      setPin(work.getRelayPin(), newState);

      if (work.getStatusPin() >= 0)
        setPin(work.getStatusPin(), newState);

      int state = readPin(work.getRelayPin());
      json.set("relayState", state);
    }
    else
    {
      int newState = !readPin(work.getRelayPin());
      setPin(work.getRelayPin(), newState);

      if (work.getStatusPin() >= 0)
        setPin(work.getStatusPin(), newState);

      int state = readPin(work.getRelayPin());
      json.set("relayState", state);
    }

    if (work.isSetState())
    {
      int state = readPin(work.getRelayPin());
      json.set("state", state);
    }

    Firebase.updateNode(firebaseWriteData, getUserPath(work.getRelayId()), json);
  }
}

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
  Alarm.delay(0);
}

void ESmart::streamCallback(StreamData data)
{
  if (data.dataType() == "json")
  {
    DEBUG_PRINT("Data received");
    DEBUG_PRINTLN(data.jsonString());
    DynamicJsonDocument object(5120);
    deserializeJson(object, data.jsonString());
    changeRelayState(object);
  }
}

void ESmart::createButton(int &pin, int &buttonPin, int &statusPin, String &id)
{
  DEBUG_PRINTLN("creating button");
  OneButton button(buttonPin, FutureJob(id, Job{pin, statusPin, NEUTRE, true}), false, false);
  button.attachClick([&](FutureJob work) { doWork(work); });
  button.attachLongPressStop([&](FutureJob work) { doWork(work); });
  buttons.push_back(button);
}

void ESmart::creatOffAlarm(int pin, int statusPin, int time, String id)
{
  time_t offTime = static_cast<time_t>(time);
  if (Alarm.isAllocated(pin + 1))
  {
    if (time != 0)
    {
      tmElements_t element;
      breakTime(offTime, element);
      Alarm.write(pin + 1, AlarmHMS(element.Hour, element.Minute, element.Second));
      DEBUG_PRINTLN("updating off alarm");
    }
    else
    {
      DEBUG_PRINTLN("deleting off alarm");
      Alarm.free(pin + 1);
    }
  }
  else if (time > 0)
  {
    tmElements_t element;
    breakTime(offTime, element);
    FutureJob work = FutureJob(id, Job{pin, statusPin, OFF, true});

    Alarm.alarmRepeat(element.Hour, element.Minute, element.Second, pin + 1, work, [&](FutureJob work) {
      doWork(work);
    });
    DEBUG_PRINT("creating off alarm");
    setAlarmEnabled(true);
  }
  DEBUG_PRINT("alarm id: ");
  DEBUG_PRINTLN(id);
}

void ESmart::creatOnAlarm(int pin, int statusPin, int time, String id)
{
  time_t onTime = static_cast<time_t>(time);
  if (Alarm.isAllocated(pin))
  {
    if (time != 0)
    {
      DEBUG_PRINT("updating on alarm");
      tmElements_t element;
      breakTime(onTime, element);
      Alarm.write(pin, AlarmHMS(element.Hour, element.Minute, element.Second));
    }
    else
    {
      DEBUG_PRINT("deleting on alarm");
      Alarm.free(pin);
    }
  }
  else
  {
    tmElements_t element;
    breakTime(onTime, element);
    FutureJob work = FutureJob(id, Job{pin, statusPin, ON, true});

    Alarm.alarmRepeat(element.Hour, element.Minute, element.Second, pin, work, [&](FutureJob work) {
      doWork(work);
    });
    DEBUG_PRINTLN("creating on alarm");
  }
  DEBUG_PRINT("alarm id: ");
  DEBUG_PRINTLN(id);
}

void ESmart::changeRelayState(DynamicJsonDocument doc)
{
  if (!doc["id"].isNull())
  {
    String eid = doc["id"];
    EsmartFirebase esmartFirebase;
    esmartFirebase.init(doc);
    DEBUG_PRINTLN(esmartFirebase);

    if (esmartFirebase.id != "null")
    {
      doWork(FutureJob(esmartFirebase.id, Job{esmartFirebase.pin, esmartFirebase.ledPin, esmartFirebase.state, false}));
      creatOnAlarm(esmartFirebase.pin, esmartFirebase.ledPin, esmartFirebase.startTime, esmartFirebase.id);
      creatOffAlarm(esmartFirebase.pin, esmartFirebase.ledPin, esmartFirebase.endTime, esmartFirebase.id);
    }
  }
  else if (doc["relayState"].isNull())
  {
    JsonObject obj = doc.as<JsonObject>();

    for (auto kv : obj)
    {

      EsmartFirebase esmartFirebase;

      esmartFirebase.init(kv.value());

      DEBUG_PRINTLN(esmartFirebase);

      createButton(esmartFirebase.pin, esmartFirebase.buttonPin, esmartFirebase.ledPin, esmartFirebase.id);

      setOutput(esmartFirebase.pin);
      setOutput(esmartFirebase.ledPin);

      if (esmartFirebase.defaultState == -1 && esmartFirebase.relayState != esmartFirebase.pin)
      {
        doWork(FutureJob(esmartFirebase.id, Job{esmartFirebase.pin, esmartFirebase.ledPin, esmartFirebase.state, false}));
      }
      else if (esmartFirebase.defaultState != -1)
      {
        doWork(FutureJob(esmartFirebase.id, Job{esmartFirebase.pin, esmartFirebase.ledPin, esmartFirebase.defaultState, true}));
      }

      creatOnAlarm(esmartFirebase.pin, esmartFirebase.ledPin, esmartFirebase.startTime, esmartFirebase.id);
      creatOffAlarm(esmartFirebase.pin, esmartFirebase.ledPin, esmartFirebase.endTime, esmartFirebase.id);
    }
  }
}
