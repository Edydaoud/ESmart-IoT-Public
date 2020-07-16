#include "ESmart.hpp"

ESmart::ESmart(){};
void ESmart::setConfig(Config &config) { _config = config; };

Config ESmart::getConfig() { return _config; };

String ESmart::getUserId() { return getConfig().userId(); }

String ESmart::getUserPath(String id) { return "/users/" + getUserId() + "/" + WiFi.macAddress() + "/" + id; }
String ESmart::getUpdatePath() { return "/update/"; }

void ESmart::doWork(FutureJob &work, bool local) {
    DEBUG_PRINT("doing work: ");
    DEBUG_PRINTLN(work);
    if (work.getRelayId() != NULL && work.getRelayId() != "null") {
        FirebaseJson json;
        if (work.getRelayState() >= 0 && work.getRelayState() <= 1) {
            int newState = work.getRelayState();

            setPin(work.getRelayPin(), newState);

            if (work.getStatusPin() >= 0)
                setPin(work.getStatusPin(), newState);

            int state = readPin(work.getRelayPin());
            json.set("relayState", state);
        } else {
            int newState = !readPin(work.getRelayPin());
            setPin(work.getRelayPin(), newState);

            if (work.getStatusPin() >= 0)
                setPin(work.getStatusPin(), newState);

            int state = readPin(work.getRelayPin());
            json.set("relayState", state);
        }

        if (work.isSetState()) {
            int state = readPin(work.getRelayPin());
            json.set("state", state);
        }
        DEBUG_PRINTLN(getConfig().isConnected());
        if (!local) DEBUG_PRINTLN(Firebase.updateNode(firebaseJobData, getUserPath(work.getRelayId()), json));
    }
}

void ESmart::initLocalTime() {
    DEBUG_PRINTLN("Start sync");
    waitForSync(5);
    setTime(UTC.now());
    DEBUG_PRINTLN("Synced");

    Firebase.begin(getConfig().firebaseUrl(), getConfig().firebaseKey());

    Firebase.setMaxRetry(firebaseJobData, 5);
    Firebase.setMaxErrorQueue(firebaseJobData, 10);

    Firebase.setMaxRetry(firebaseStreamData, 5);
    Firebase.setMaxErrorQueue(firebaseStreamData, 10);

    firebaseJobData.setResponseSize(2084);
    firebaseJobData.setBSSLBufferSize(2084, 2084);

    firebaseStreamData.setResponseSize(2084);
    firebaseStreamData.setBSSLBufferSize(2084, 2084);
}

void ESmart::handleLoop() {
    Alarm.delay(0);
    for (size_t i = 0; i < buttons.size(); i++) {
        buttons[i].tick();
    }

    // if (oldTime - ezt::now() > 60 && getConfig().isConnected()) {
    //     Firebase.setInt(firebaseJobData, getConfig().pingPath(), ezt::now());
    //     oldTime = ezt::now();
    // }
}

void ESmart::streamCallback(StreamData data) {
    if (data.dataType() == "json") {
        DEBUG_PRINT("Data received");
        DEBUG_PRINTLN(data.jsonString());
        DynamicJsonDocument object(2048);
        deserializeJson(object, data.jsonString());
        changeRelayState(object);
    }
}

//job id 3
void ESmart::createButton(int &pin, int &buttonState, int &buttonPin, int &statusPin, String &id) {
    DEBUG_PRINT("creating button: ");
    DEBUG_PRINTLN(buttonPin);
    OneButton button(buttonPin, buttonState, FutureJob(id, Job{pin, statusPin, NEUTRE, true, 3}), true, false);

    button.attachPress([&](FutureJob work) {
        doWork(work);
        int newState = readPin(work.getRelayPin());
        getConfig().changeLocalState(work.getRelayId(), newState);
    });

    button.attachClick([&](FutureJob work) {
        doWork(work);
        int newState = readPin(work.getRelayPin());
        getConfig().changeLocalState(work.getRelayId(), newState);
    });

    button.attachLongPressStop([&](FutureJob work) {
        doWork(work);
        int newState = readPin(work.getRelayPin());
        getConfig().changeLocalState(work.getRelayId(), newState);
        longPressReset = 0;
    });

    button.attachDuringLongPress([&](FutureJob work) {
        if (longPressReset == 0) {
            longPressReset = millis();
            DEBUG_PRINTLN("longPressReset: " + longPressReset);
        } else if (millis() - longPressReset > 5000) {
            ESP.reset();
        }
        DEBUG_PRINTLN(millis() - longPressReset);
    });

    buttons.push_back(button);
}

void ESmart::creatOffAlarm(int &pin, int &statusPin, int &time, String &id) {
    time_t offTime = static_cast<time_t>(time);
    if (Alarm.isAllocated(pin + 1)) {
        if (time != 0) {
            tmElements_t element;
            breakTime(offTime, element);
            Alarm.write(pin + 1, AlarmHMS(element.Hour, element.Minute, element.Second));
            DEBUG_PRINTLN("updating off alarm");
        } else {
            DEBUG_PRINTLN("deleting off alarm");
            Alarm.free(pin + 1);
        }
    } else if (time > 0) {
        tmElements_t element;
        breakTime(offTime, element);
        FutureJob work = FutureJob(id, Job{pin, statusPin, OFF, true, 0});

        Alarm.alarmRepeat(element.Hour, element.Minute, element.Second, pin + 1, work, [&](FutureJob work) {
            doWork(work);
            int newState = readPin(work.getRelayPin());
            getConfig().changeLocalState(work.getRelayId(), newState);
        });
        DEBUG_PRINT("creating off alarm");
    }
    DEBUG_PRINT("alarm id: ");
    DEBUG_PRINTLN(id);
}

//job id 1
void ESmart::creatOnAlarm(int &pin, int &statusPin, int &time, String &id) {
    time_t onTime = static_cast<time_t>(time);
    if (Alarm.isAllocated(pin)) {
        if (time != 0) {
            DEBUG_PRINT("updating on alarm");
            tmElements_t element;
            breakTime(onTime, element);
            Alarm.write(pin, AlarmHMS(element.Hour, element.Minute, element.Second));
        } else {
            DEBUG_PRINT("deleting on alarm");
            Alarm.free(pin);
        }
    } else {
        tmElements_t element;
        breakTime(onTime, element);
        FutureJob work = FutureJob(id, Job{pin, statusPin, ON, true, 1});

        Alarm.alarmRepeat(element.Hour, element.Minute, element.Second, pin, work, [&](FutureJob work) {
            doWork(work);
            int newState = readPin(work.getRelayPin());
            getConfig().changeLocalState(work.getRelayId(), newState);
        });
        DEBUG_PRINTLN("creating on alarm");
    }
    DEBUG_PRINT("alarm id: ");
    DEBUG_PRINTLN(id);
}

void ESmart::initLocalState(std::shared_ptr<StaticJsonDocument<1024>> doc) {
    JsonObject s = doc.get()->as<JsonObject>();

    for (auto kv : s) {
        EsmartFirebase esmartFirebase;

        esmartFirebase.init(kv.value());

        createButton(esmartFirebase.pin, esmartFirebase.buttonState, esmartFirebase.buttonPin, esmartFirebase.ledPin, esmartFirebase.id);

        setOutput(esmartFirebase.pin, esmartFirebase.state);
        setOutput(esmartFirebase.ledPin, esmartFirebase.state);

        if (esmartFirebase.defaultState == -1 && esmartFirebase.relayState != esmartFirebase.pin) {
            //job id 2
            FutureJob job = FutureJob(esmartFirebase.id, Job{esmartFirebase.pin, esmartFirebase.ledPin, esmartFirebase.state, false, 2});
            doWork(job, true);
        } else if (esmartFirebase.defaultState != -1) {
            FutureJob job = FutureJob(esmartFirebase.id, Job{esmartFirebase.pin, esmartFirebase.ledPin, esmartFirebase.defaultState, true, 2});
            doWork(job, true);
        }
        yield();
    }
    doc.reset();
}

void ESmart::changeRelayState(DynamicJsonDocument &doc) {
    if (!doc["id"].isNull()) {
        String eid = doc["id"];
        EsmartFirebase esmartFirebase;
        esmartFirebase.init(doc);
        DEBUG_PRINTLN(esmartFirebase);
        std::shared_ptr<DynamicJsonDocument> newData = std::make_shared<DynamicJsonDocument>(esmartFirebase.getJsonDoc());

        if (esmartFirebase.id != "null") {
            creatOnAlarm(esmartFirebase.pin, esmartFirebase.ledPin, esmartFirebase.startTime, esmartFirebase.id);
            creatOffAlarm(esmartFirebase.pin, esmartFirebase.ledPin, esmartFirebase.endTime, esmartFirebase.id);

            FutureJob job = FutureJob(esmartFirebase.id, Job{esmartFirebase.pin, esmartFirebase.ledPin, esmartFirebase.state, false});
            doWork(job);
        }

        getConfig().setLocalSingleData(esmartFirebase.id, newData);

    } else if (doc["relayState"].isNull()) {
        JsonObject obj = doc.as<JsonObject>();
        DynamicJsonDocument localDoc(1024);

        for (auto kv : obj) {
            EsmartFirebase esmartFirebase;

            esmartFirebase.init(kv.value());

            localDoc[esmartFirebase.id] = esmartFirebase.getJsonDoc();

            DEBUG_PRINTLN(esmartFirebase);

            if (esmartFirebase.defaultState == -1 && esmartFirebase.relayState != esmartFirebase.pin) {
                //job id 2
                FutureJob job = FutureJob(esmartFirebase.id, Job{esmartFirebase.pin, esmartFirebase.ledPin, esmartFirebase.state, false, 2});
                doWork(job);
            } else if (esmartFirebase.defaultState != -1) {
                FutureJob job = FutureJob(esmartFirebase.id, Job{esmartFirebase.pin, esmartFirebase.ledPin, esmartFirebase.defaultState, true, 2});
                doWork(job);
            }

            creatOnAlarm(esmartFirebase.pin, esmartFirebase.ledPin, esmartFirebase.startTime, esmartFirebase.id);
            creatOffAlarm(esmartFirebase.pin, esmartFirebase.ledPin, esmartFirebase.endTime, esmartFirebase.id);
            yield();
        }

        getConfig().setLocalData(std::make_shared<DynamicJsonDocument>(localDoc));
    }
}
