#include "ESmarts.hpp"

void setup() {
    Serial.begin(115200);
    if (loadConfigs()) {
        connect();
        begin();
    }
}

void loop() {
    for (size_t i = 0; i < buttons.size(); i++) {
        buttons[i].tick();
    }
    Alarm.delay(0);
}

void begin() {
    INFOM("Start time syncing");
    delay(500);

    if (waitForSync(5)) {
        setTime(UTC.now());
        INFO("Done syncing, current time: %ld\n\n", UTC.now());

        Firebase.begin(configs.firebaseUrl, configs.firebaseToken);
        Firebase.reconnectWiFi(true);

        Firebase.setMaxRetry(firebaseJobData, 5);
        Firebase.setMaxErrorQueue(firebaseJobData, 10);
        Firebase.setMaxRetry(firebaseStreamData, 5);
        Firebase.setMaxErrorQueue(firebaseStreamData, 10);

        firebaseJobData.setResponseSize(1024);
        firebaseJobData.setBSSLBufferSize(1024, 1024);

        firebaseStreamData.setResponseSize(2048);
        firebaseStreamData.setBSSLBufferSize(1024, 1024);
        Firebase.beginStream(firebaseStreamData, configs.getUserPath());
        Firebase.setStreamCallback(firebaseStreamData, streamCallback);
    } else {
        INFOM("Couldn't connect to internet working in offline mode");
    }
}

void connect() {
    WiFi.begin(configs.wifiAp, configs.wifiPass);
    INFO("Connecting to: %s\n", configs.wifiAp.c_str());
    while (maxRetry >= 0) {
        if (WiFi.status() == WL_CONNECTED) break;
        delay(200);
        maxRetry--;
        INFO("Retrying to connect: %d\n\n", maxRetry);
    }
    INFOM("Connected");
}

bool loadConfigs() {
    INFOM("Loading configs");
    if (!beginWrite()) return false;

    File configFile = LittleFS.open("/config.json", "r");
    File localDataFile = LittleFS.open("/data.json", "r");

    if (!configFile) {
        INFOM("Couldn't open config file");
        return false;
    }

    StaticJsonDocument<1024> configDoc;

    auto error1 = deserializeJson(configDoc, configFile);
    deserializeJson(localData, localDataFile);

    if (error1) {
        INFOM("Failed to deserialize config file");
        return false;
    }

    configs = Configs(configDoc["firebaseUrl"],
                      configDoc["firebaseKey"],
                      configDoc["userId"],
                      configDoc["wifiAp"],
                      configDoc["wifiPass"]);

    configFile.close();
    localDataFile.close();
    configDoc.garbageCollect();

    INFOM("Config loaded successfuly");

    endWrite();
    initLocalData();

    return true;
}

void streamCallback(StreamData data) {
    if (data.dataType() == JSON) {
        INFO("Data received: %s\n\n", data.jsonString().c_str());
        DynamicJsonDocument object(2048);
        deserializeJson(object, data.jsonString());
        handleReceivedData(object);
    }
}

void handleReceivedData(DynamicJsonDocument &document) {
    if (!document["id"].isNull()) {
        EsmartFirebase esmartFirebase;
        esmartFirebase.init(document);

        INFO("Handeling server data: %s\n\n", esmartFirebase.toString().c_str());

        digitalWrite(esmartFirebase.pin, esmartFirebase.state);
        esmartFirebase.relayState = digitalRead(esmartFirebase.pin);
        updateNode(esmartFirebase);

        createAlarms(esmartFirebase);
    } else if (document["relayState"].isNull()) {
        JsonObject obj = document.as<JsonObject>();
        for (auto kv : obj) {
            EsmartFirebase esmartFirebase;
            esmartFirebase.init(kv.value());

            INFO("Handeling initial server data: %s\n\n", esmartFirebase.toString().c_str());
            INFO("Pin state: %d\n\n", digitalRead(esmartFirebase.pin));

            if (esmartFirebase.defaultState == -1 && esmartFirebase.relayState != digitalRead(esmartFirebase.pin)) {
                digitalWrite(esmartFirebase.pin, esmartFirebase.state);
                esmartFirebase.relayState = digitalRead(esmartFirebase.pin);
                updateNode(esmartFirebase);
            } else if (esmartFirebase.defaultState != -1 && esmartFirebase.defaultState != esmartFirebase.relayState) {
                digitalWrite(esmartFirebase.pin, esmartFirebase.defaultState);
                esmartFirebase.relayState = digitalRead(esmartFirebase.pin);
                esmartFirebase.state = digitalRead(esmartFirebase.pin);
                updateNode(esmartFirebase);
            }

            createAlarms(esmartFirebase);
        }
    }
}

void initLocalData() {
    JsonObject obj = localData.as<JsonObject>();
    for (auto kv : obj) {
        EsmartFirebase esmartFirebase;
        esmartFirebase.init(kv.value());

        INFO("Initiatinng initial data: %s\n\n", esmartFirebase.toString().c_str());

        if (esmartFirebase.defaultState == -1) {
            pinMode(esmartFirebase.pin, OUTPUT);
            digitalWrite(esmartFirebase.pin, esmartFirebase.state);
        } else if (esmartFirebase.defaultState != -1) {
            pinMode(esmartFirebase.pin, OUTPUT);
            digitalWrite(esmartFirebase.pin, esmartFirebase.defaultState);
        }
        createButton(esmartFirebase);
    }
}

void setLocalData(EsmartFirebase &esmart) {
    if (beginWrite()) {
        INFO("Setting local data: %s\n\n", esmart.toString().c_str());

        File localDataFile = LittleFS.open("/data.json", "r+");
        if (!localDataFile) INFOM("Failed to open data file");
        deserializeJson(localData, localDataFile);
        localData[esmart.id] = esmart.getJsonDoc();
        localDataFile = LittleFS.open("/data.json", "w+");
        serializeJson(localData, localDataFile);
        localDataFile.close();
        endWrite();
    }
};

void updateNode(EsmartFirebase &esmart) {
    INFO("Updating node data: %s\n\n", esmart.toString().c_str());

    FirebaseJson json = esmart.getFirebaseJson();
    delay(250);
    Firebase.updateNode(firebaseJobData, configs.getUserPath(esmart.id), json);
    delay(250);
    setLocalData(esmart);
}

void doWork(FutureJob &work) {
    INFO("Doing local work: %s\n\n", work.esmart.toString().c_str());

    digitalWrite(work.esmart.pin, work.esmart.state);
    updateNode(work.esmart);
}

void createButton(EsmartFirebase &esmart) {
    INFO("Creating button: %s\n\n", esmart.toString().c_str());

    OneButton button(esmart.buttonPin, esmart.buttonState, FutureJob(esmart), true, false);
    button.attachClick([&](FutureJob work) {
        INFO("Triggering on click: %s\n\n", esmart.toString().c_str());

        work.esmart.state = !digitalRead(work.esmart.pin);
        work.esmart.relayState = !digitalRead(work.esmart.pin);
        doWork(work);
    });
    button.attachLongPressStop([&](FutureJob work) {
        INFO("Triggering on long press stop: %s\n\n", esmart.toString().c_str());

        work.esmart.state = !digitalRead(work.esmart.pin);
        work.esmart.relayState = !digitalRead(work.esmart.pin);
        doWork(work);
        longPressReset = 0;
    });
    button.attachDuringLongPress([&](FutureJob work) {
        INFO("Triggering on long press: %s\n\n", esmart.toString().c_str());

        if (longPressReset == 0) {
            longPressReset = millis();
        } else if (millis() - longPressReset > 5000) {
            ESP.reset();
        }
    });
    buttons.push_back(button);
}

void createAlarms(EsmartFirebase &esmart) {
    createOffAlarm(esmart);
    createOnAlarm(esmart);
}

void createOffAlarm(EsmartFirebase &esmart) {
    time_t offTime = static_cast<time_t>(esmart.endTime);

    if (Alarm.isAllocated(esmart.pin + 1)) {
        if (offTime != 0) {
            INFO("Updating off alarm: %s\n\n", esmart.toString().c_str());

            tmElements_t element;
            breakTime(offTime, element);
            Alarm.write(esmart.pin + 1, AlarmHMS(element.Hour, element.Minute, element.Second));
        } else {
            INFO("Deleting off alarm: %s\n\n", esmart.toString().c_str());

            Alarm.free(esmart.pin + 1);
        }
    } else if (offTime > 0) {
        tmElements_t element;
        breakTime(offTime, element);
        FutureJob work = FutureJob(esmart);
        INFO("Creating off alarm: %s\n\n", esmart.toString().c_str());

        Alarm.alarmRepeat(element.Hour, element.Minute, element.Second, esmart.pin + 1, work, [&](FutureJob work) {
            INFO("Triggering off alarm: %s\n\n", esmart.toString().c_str());

            work.esmart.state = !digitalRead(work.esmart.pin);
            work.esmart.relayState = !digitalRead(work.esmart.pin);
            doWork(work);
        });
    }
}

void createOnAlarm(EsmartFirebase &esmart) {
    time_t onTime = static_cast<time_t>(esmart.startTime);

    if (Alarm.isAllocated(esmart.pin)) {
        if (onTime != 0) {
            INFO("Updating on alarm: %s\n\n", esmart.toString().c_str());
            tmElements_t element;
            breakTime(onTime, element);
            Alarm.write(esmart.pin, AlarmHMS(element.Hour, element.Minute, element.Second));
        } else {
            INFO("Deleting on alarm: %s\n\n", esmart.toString().c_str());
            Alarm.free(esmart.pin);
        }
    } else if (onTime > 0) {
        INFO("Creating on alarm: %s\n\n", esmart.toString().c_str());

        tmElements_t element;
        breakTime(onTime, element);
        FutureJob work = FutureJob(esmart);

        Alarm.alarmRepeat(element.Hour, element.Minute, element.Second, esmart.pin, work, [&](FutureJob work) {
            INFO("Triggering on alarm: %s\n\n", esmart.toString().c_str());

            work.esmart.state = !digitalRead(work.esmart.pin);
            work.esmart.relayState = !digitalRead(work.esmart.pin);
            doWork(work);
        });
    }
}
