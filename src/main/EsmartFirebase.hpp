#include <ArduinoJson.h>
#include "Config.h"

class EsmartFirebase : public Printable
{
public:
  EsmartFirebase();
  EsmartFirebase(DynamicJsonDocument *const &doc);

  int state;
  int pin;
  String id;
  int defaultState;
  int relayState;
  int buttonPin;
  int startTime;
  int endTime;

  void init(String key, DynamicJsonDocument * const &doc);
  void init(DynamicJsonDocument * const &doc);

  void setState(int firebaseState);
  void setId(String firebaseId);
  void setPin(int firebasePin);
  void setButtonPin(int firebaseButtonPin);
  void setDefaultState(int firebaseDefaultState);
  void setRelayState(int firebaseRelayState);
  void setEndTime(double firebaseEndTime);
  void setStartTime(double firebaseStartTime);

  int getDefaultState();

public:
  size_t printTo(Print &p) const
  {
    size_t t = 0;
    t += p.print("state: ");
    t += p.println(state);
    t += p.print("pin: ");
    t += p.println(pin);
    t += p.print("buttonPin: ");
    t += p.println(buttonPin);
    t += p.print("id: ");
    t += p.println(id);
    t += p.print("defaultState: ");
    t += p.println(defaultState);
    t += p.print("startTime: ");
    t += p.println(startTime);
    t += p.print("endTime: ");
    t += p.println(endTime);
    return t;
  }
};
