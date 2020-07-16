#include <ArduinoJson.h>

#include "Config.hpp"

class EsmartFirebase : public Printable {
   public:
    EsmartFirebase();
    EsmartFirebase(DynamicJsonDocument *const &doc);

    int state = 1;
    int pin = -1;
    String id = "";
    int defaultState = -1;
    int relayState = 1;
    int buttonPin = -1;
    int startTime = 0;
    int endTime = 0;
    int ledPin = -1;
    int buttonState = -1;

    void init(DynamicJsonDocument const &doc);

    void setState(int firebaseState);
    void setId(String firebaseId);
    void setPin(int firebasePin);
    void setButtonPin(int firebaseButtonPin);
    void setDefaultState(int firebaseDefaultState);
    void setRelayState(int firebaseRelayState);
    void setEndTime(double firebaseEndTime);
    void setStartTime(double firebaseStartTime);
    void setLedPin(int ledPin);
    void setButtonState(int buttonState);

    int getDefaultState();

   public:
    size_t printTo(Print &p) const {
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
        t += p.println(startTime);
        t += p.print("ledPin: ");
        t += p.println(ledPin);
        t += p.print("buttonState: ");
        t += p.println(buttonState);
        return t;
    }

    DynamicJsonDocument getJsonDoc() {
        DynamicJsonDocument doc(1024);
        doc["state"] = state;
        doc["pin"] = pin;
        doc["buttonPin"] = buttonPin;
        doc["id"] = id;
        doc["defaultState"] = defaultState;
        doc["relayState"] = state;
        doc["buttonState"] = buttonState;
        return doc;
    }
};
