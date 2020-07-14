#include "EsmartFirebase.hpp"
#include <unordered_map>

#include "ezTime.h"
#include "OneButton.h"
#include "TimeAlarms.h"

#include <Time.h>
#include "Config.hpp"
#include "FirebaseESP8266.h"
#include "GPIO.hpp"

class ESmart
{

private:
    bool alarmEnabled = false;
    Config _config;

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
    void click(int pin, int statusPin, String id);
    void createButton(int &pin, int &buttonPin, int &statusPin, String &id);
    void changeRelayState(DynamicJsonDocument doc);
    void streamCallback(StreamData data);
    void scheduleOff(int pin, int statusPin, String id);
    void scheduleOn(int pin, int statusPin, String id);
    void creatOffAlarm(int pin, int statusPin, int time, String id);
    void creatOnAlarm(int pin, int statusPin, int time, String id);
    void tickButtons();
    void click();
    void initLocalTime();
    void setConfig(Config config);
    void setData(EsmartFirebase esmart);
    void doWork(FutureJob work);
    Config getConfig();

    bool hasAny(int num)
    {
        if (std::find(std::begin({0, 1}), std::end({0, 1}), 1) != std::end({0, 1}))
            return true;
        else
            return false;
    }

    EsmartFirebase getData(String id);
};