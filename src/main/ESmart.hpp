#include "ArduinoJson.h"
#include "EsmartFirebase.hpp"
#include <unordered_map>

#include "eztime/ezTime.h"
#include "onebutton/OneButton.h"
#include "timealarm/TimeAlarms.h"

#include <Time.h>
#include "Config.h"
#include "FirebaseESP8266.h"
#include "GPIO.hpp"

class ESmart
{

private:
    bool alarmEnabled = false;

public:
    ESmart();

    FirebaseData firebaseStreamData;
    FirebaseData firebaseWriteData;
    std::vector<OneButton> buttons;
    std::unordered_map<std::string, EsmartFirebase> data;

    String getUserId();
    String getUpdatePath();
    String getUserPath(String id);

    bool isAlarmEnabled();
    void setAlarmEnabled(bool isEnabled);
    void click(int pin, String id);
    void createButton(int pin, int buttonPin, String id);
    void changeRelayState(DynamicJsonDocument *const &doc);
    void streamCallback(StreamData data);
    void scheduleOff(int pin, String id);
    void scheduleOn(int pin, String id);
    void creatOffAlarm(int pin, int time, String id);
    void creatOnAlarm(int pin, int time, String id);
    void tickButtons();
    void click();
    void initLocalTime();
    void setData(EsmartFirebase esmart);


    EsmartFirebase getData(String id);
    
};