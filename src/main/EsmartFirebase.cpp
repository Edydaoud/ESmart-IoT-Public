#include "EsmartFirebase.hpp"

EsmartFirebase::EsmartFirebase(){};
EsmartFirebase::EsmartFirebase(DynamicJsonDocument *const &doc){
  init(doc);
};

void EsmartFirebase::init(String key, DynamicJsonDocument *const &doc)
{
  DynamicJsonDocument edoc = doc->operator[](key);
  init(&edoc);
};

void EsmartFirebase::init(DynamicJsonDocument *const &doc)
{
  setState(doc->operator[]("state"));
  setPin(doc->operator[]("pin"));
  setId(doc->operator[]("id"));
  setDefaultState(doc->operator[]("defaultState"));
  setRelayState(doc->operator[]("relayState"));
  setEndTime(doc->operator[]("endTime"));
  setButtonPin(doc->operator[]("buttonPin"));
  setStartTime(doc->operator[]("startTime"));
};

void EsmartFirebase::setState(int firebaseState)
{
  state = firebaseState;
};

void EsmartFirebase::setPin(int firebasePin)
{
  pin = firebasePin;
};

void EsmartFirebase::setButtonPin(int firebaseButtonPin)
{
  buttonPin = firebaseButtonPin;
};

void EsmartFirebase::setId(String firebaseId)
{
  id = firebaseId;
};

void EsmartFirebase::setDefaultState(int firebaseDefaultState)
{
  defaultState = firebaseDefaultState;
};

void EsmartFirebase::setRelayState(int firebaseRelayState)
{
  relayState = firebaseRelayState;
}

void EsmartFirebase::setStartTime(double firebaseStartTime)
{
  startTime = firebaseStartTime;
}

void EsmartFirebase::setEndTime(double firebaseEndTime)
{
  endTime = firebaseEndTime;
}

int EsmartFirebase::getDefaultState()
{
  if (defaultState == -1)
    return state;
  else
    return defaultState;
};
