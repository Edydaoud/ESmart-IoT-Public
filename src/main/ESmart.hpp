#include <OneButton.h>
#include <Time.h>

#include <unordered_map>

#include "Config.hpp"
#include "EsmartFirebase.hpp"
#include "FirebaseESP8266.h"
#include "GPIO.hpp"
#include "TimeAlarms.h"
#include "ezTime.h"

class ESmart {
   private:
    bool alarmEnabled = false;
    Config _config;
    std::vector<OneButton> buttons;
    time_t oldTime = 0;
    time_t longPressReset;

   public:
    ESmart();

    FirebaseData firebaseStreamData;
    FirebaseData firebaseJobData;
    std::unordered_map<std::string, EsmartFirebase> data;

    String getUserId();
    String getUpdatePath();
    String getUserPath(String id);

    void createButton(int &pin, int &buttonState, int &buttonPin, int &statusPin, String &id);
    void changeRelayState(DynamicJsonDocument &doc);
    void streamCallback(StreamData data);

    void creatOffAlarm(int &pin, int &statusPin, int &time, String &id);
    void creatOnAlarm(int &pin, int &statusPin, int &time, String &id);

    void handleLoop();
    void click();
    void initLocalTime();

    void setConfig(Config &config);

    void doWork(FutureJob &work, bool localData = false);

    void initLocalState(std::shared_ptr<StaticJsonDocument<1024>> doc);

    Config getConfig();

    bool hasAny(int num) {
        if (std::find(std::begin({0, 1}), std::end({0, 1}), 1) != std::end({0, 1}))
            return true;
        else
            return false;
    }

    EsmartFirebase getData(String id);
};