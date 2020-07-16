#include "EsmartFirebase.hpp"

EsmartFirebase::EsmartFirebase(){};

void EsmartFirebase::init(DynamicJsonDocument const &doc) {
    setState(doc["state"]);
    setPin(doc["pin"]);
    setId(doc["id"]);
    setDefaultState(doc["defaultState"]);
    setRelayState(doc["relayState"]);
    setEndTime(doc["endTime"]);
    setButtonPin(doc["buttonPin"]);
    setStartTime(doc["startTime"]);
    setLedPin(doc["ledPin"]);
    setButtonState(doc["buttonState"]);
};

void EsmartFirebase::setState(int firebaseState) {
    state = firebaseState;
};

void EsmartFirebase::setPin(int firebasePin) {
    pin = firebasePin;
};

void EsmartFirebase::setButtonPin(int firebaseButtonPin) {
    buttonPin = firebaseButtonPin;
};

void EsmartFirebase::setId(String firebaseId) {
    id = firebaseId;
};

void EsmartFirebase::setDefaultState(int firebaseDefaultState) {
    defaultState = firebaseDefaultState;
};

void EsmartFirebase::setRelayState(int firebaseRelayState) {
    relayState = firebaseRelayState;
}

void EsmartFirebase::setStartTime(double firebaseStartTime) {
    startTime = firebaseStartTime;
}

void EsmartFirebase::setEndTime(double firebaseEndTime) {
    endTime = firebaseEndTime;
}

void EsmartFirebase::setLedPin(int firebaseLedPin) {
    ledPin = firebaseLedPin;
}
void EsmartFirebase::setButtonState(int firebaseButtonState) {
    buttonState = firebaseButtonState;
}

int EsmartFirebase::getDefaultState() {
    if (defaultState == -1)
        return state;
    else
        return defaultState;
};
