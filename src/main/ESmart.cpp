#include "ESmart.hpp"

/* 
    Loading configs and local data
 */
void setup() {
    Serial.begin(115200);

    if (loadConfigs()) {
        connect();
        initTime();
        begin();
    }
}

/* 
    handling buttons and alarms
*/
void loop() {
    if (isInternetConnected()) Alarm.delay(0);

    for (size_t i = 0; i < buttons.size(); i++) {
        buttons[i].tick();
    }

    if (millis() - lastTimeUpdate >= UPDATE_TIME_INTERVAL) {
        lastTimeUpdate = millis();
        updateTime();
    }
}

/* 
    Connecting to WiFi
 */
void connect() {
    WiFi.begin(configs.wifiAp, configs.wifiPass);
    WiFi.setAutoConnect(true);

    SerialOut::info("Connecting to: %s\n\n", configs.wifiAp.c_str());

    int wifiTimout = 1;

    while (wifiTimout <= WIFI_TIMEOUT) {
        if (WiFi.status() == WL_CONNECTED) break;
        delay(200);
        wifiTimout++;
        SerialOut::info("Retrying WiFi connection: %d/%d\n\n", wifiTimout, WIFI_TIMEOUT);
    }

    SerialOut::info("Connected\n\n");
}

void initTime() {
    SerialOut::info("Start time syncing\n\n");

    timeClient.setUpdateInterval(55000);
    timeClient.begin();

    int ntpTimeOut = 1;

    while (ntpTimeOut <= NTP_TIMEOUT) {
        if (timeClient.update()) {
            isConnected = true;
            SerialOut::info("NTP client connected\n\n");
            break;
        } else {
            SerialOut::info("Retrying NTP connection: %d/%d\n\n", ntpTimeOut, NTP_TIMEOUT);
            isConnected = false;
        }
        ntpTimeOut++;
    }
}

/*
    Begin time syncing a initializing firebase and firebase stream
*/
void begin() {
    if (isNtpClientConnected() && !checkForNewVersion()) {
        setTime(timeClient.getEpochTime());
        SerialOut::info("Done syncing, current time: %ld\n\n", timeClient.getEpochTime());

        Firebase.begin(configs.firebaseUrl, configs.firebaseKey);

        Firebase.setMaxRetry(firebaseJobData, 2);
        Firebase.setMaxErrorQueue(firebaseJobData, 2);
        Firebase.setMaxRetry(firebaseStreamData, 5);
        Firebase.setMaxErrorQueue(firebaseStreamData, 10);

        firebaseJobData.setResponseSize(1024);
        firebaseJobData.setBSSLBufferSize(1024, 1024);

        firebaseStreamData.setResponseSize(2048);
        firebaseStreamData.setBSSLBufferSize(1024, 1024);

        Firebase.beginStream(firebaseStreamData, configs.getUserPath());
        Firebase.setStreamCallback(firebaseStreamData, streamCallback);

        checkForServerUpdate();

    } else {
        SerialOut::info("Couldn't connect to internet working in offline mode\n\n");
    }
}

void updateTime() {
    SerialOut::info("Updating time\n\n");

    if (timeClient.update()) {
        SerialOut::info("Time updated\n\n");

        if (!isConnected) {
            SerialOut::info("Wasn't connected, reinitializing firebase\n\n");
            isConnected = true;
            shouldSyncDataWithServer = true;
            begin();
        } else {
            shouldSyncDataWithServer = false;
            SerialOut::info("Already connected, updating time only\n\n");
        }

    } else {
        SerialOut::info("Couldn't connect to ntp server while updating time\n\n");
        if (!firebaseStreamData.httpConnected()) {
            SerialOut::info("Firebase not connected, reseting stream\n\n");

            Firebase.endStream(firebaseStreamData);
            Firebase.setStreamCallback(firebaseStreamData, nullptr);
        } else
            SerialOut::info("Firebase is connected while ntp is not, leaving firebase as is\n\n");

        isConnected = false;
    }
}

/* 
    Loading configs from file system using LittleFS and parsing json data using ArduinoJson
 */
bool loadConfigs() {
    SerialOut::info("Loading configs\n\n");

    if (!beginWrite()) return false;

    File configFile = LittleFS.open("/config.json", "r");
    File localDataFile = LittleFS.open("/data.json", "r");

    if (!configFile) {
        SerialOut::info("Couldn't open config file\n\n");
        return false;
    }

    if (!localDataFile) {
        SerialOut::info("Couldn't open data file\n\n");
    }

    DynamicJsonDocument configDoc(250);
    DynamicJsonDocument localData(2048);

    auto error1 = deserializeJson(configDoc, configFile);
    auto error2 = deserializeJson(localData, localDataFile);

    if (error1) {
        SerialOut::info("Failed to deserialize config file\n\n");
        return false;
    }

    if (error2) {
        SerialOut::info("Failed to deserialize data file\n\n");
    }

    configs = Configs(configDoc);
    initLocalData(localData);

    configFile.close();
    localDataFile.close();

    SerialOut::info("Config loaded successfuly\n\n");

    endWrite();

    configDoc.clear();
    configDoc.garbageCollect();

    localData.clear();
    localData.garbageCollect();

    return true;
}

/* 
    Handeling Firebase stream callback
 */
void streamCallback(StreamData data) {
    if (data.dataType() == JSON) {
        SerialOut::info("Data received: %s\n\n", data.jsonString().c_str());
        DynamicJsonDocument object(2048);
        deserializeJson(object, data.jsonString());
        handleReceivedData(object);

        object.clear();
        object.garbageCollect();
    }
}

void handleReceivedData(DynamicJsonDocument &document) {
    if (!document["id"].isNull()) {
        EsmartFirebase esmartFirebase;
        esmartFirebase.init(document);

        SerialOut::info("Handeling server data: %s\n\n", esmartFirebase.toString().c_str());

        writePin(esmartFirebase.pin, esmartFirebase.ledPin, esmartFirebase.state);

        esmartFirebase.relayState = readPin(esmartFirebase.pin);

        updateNode(esmartFirebase);
        createAlarms(esmartFirebase);

    } else if (document["relayState"].isNull()) {
        JsonObject obj = document.as<JsonObject>();
        for (auto kv : obj) {
            EsmartFirebase esmartFirebase;
            esmartFirebase.init(kv.value());

            SerialOut::info("Handeling initial server data: %s\n\n", esmartFirebase.toString().c_str());
            SerialOut::info("Pin state: %d\n\n", readPin(esmartFirebase.pin));

            if (esmartFirebase.defaultState == -1 && esmartFirebase.relayState != readPin(esmartFirebase.pin)) {
                if (!shouldSyncDataWithServer)
                    writePin(esmartFirebase.pin, esmartFirebase.ledPin, esmartFirebase.state);
                else
                    esmartFirebase.state = readPin(esmartFirebase.pin);

                esmartFirebase.relayState = readPin(esmartFirebase.pin);

                updateNode(esmartFirebase);
            } else if (esmartFirebase.defaultState != -1 && esmartFirebase.defaultState != esmartFirebase.relayState) {
                if (!shouldSyncDataWithServer) writePin(esmartFirebase.pin, esmartFirebase.ledPin, esmartFirebase.defaultState);

                esmartFirebase.relayState = readPin(esmartFirebase.pin);
                esmartFirebase.state = readPin(esmartFirebase.pin);

                updateNode(esmartFirebase);
            } else {
                setLocalData(esmartFirebase);
            }

            createAlarms(esmartFirebase);
        }
    }
}

void initLocalData(DynamicJsonDocument &document) {
    JsonObject obj = document.as<JsonObject>();
    for (auto kv : obj) {
        EsmartFirebase esmartFirebase;
        esmartFirebase.init(kv.value());

        SerialOut::info("Initiatinng initial data: %s\n\n", esmartFirebase.toString().c_str());

        if (esmartFirebase.defaultState == -1) {
            writePin(esmartFirebase.pin, esmartFirebase.ledPin, esmartFirebase.state);
            pinMode(esmartFirebase.pin, OUTPUT);
            pinMode(esmartFirebase.ledPin, OUTPUT);
        } else if (esmartFirebase.defaultState != -1) {
            writePin(esmartFirebase.pin, esmartFirebase.ledPin, esmartFirebase.defaultState);
            pinMode(esmartFirebase.pin, OUTPUT);
            pinMode(esmartFirebase.ledPin, OUTPUT);
        }

        createButton(esmartFirebase);
    }
}

void setLocalData(EsmartFirebase &esmart) {
    if (beginWrite()) {
        SerialOut::info("Setting local data: %s\n\n", esmart.toString().c_str());

        File localDataFile = LittleFS.open("/data.json", "r+");
        if (!localDataFile) SerialOut::info("Failed to open data file");

        DynamicJsonDocument document(2048);

        deserializeJson(document, localDataFile);
        localDataFile = LittleFS.open("/data.json", "w+");

        document[esmart.id] = esmart.getJsonDoc();

        serializeJson(document, localDataFile);

        localDataFile.close();
        document.clear();
        document.garbageCollect();

        endWrite();
    }
};

void updateNode(EsmartFirebase &esmart) {
    SerialOut::info("Updating node data: %s\n\n", esmart.toString().c_str());

    if (isInternetConnected()) {
        FirebaseJson json = esmart.getFirebaseJson();
        Firebase.updateNode(firebaseJobData, configs.getUserPath(esmart.id), json);
    }

    setLocalData(esmart);
}

void doWork(FutureJob &work) {
    SerialOut::info("Doing local work: %s\n\n", work.esmart.toString().c_str());

    writePin(work.esmart.pin, work.esmart.ledPin, work.esmart.state);

    updateNode(work.esmart);
}

void createButton(EsmartFirebase &esmart) {
    SerialOut::info("Creating button: %s\n\n", esmart.toString().c_str());

    OneButton button(esmart.buttonPin, esmart.buttonState, FutureJob(esmart));

    button.attachClick([&](FutureJob &work) {
        SerialOut::info("Triggering on click: %s\n\n", work.esmart.toString().c_str());

        work.esmart.state = !readPin(work.esmart.pin);
        work.esmart.relayState = work.esmart.state;

        doWork(work);
    });
    button.attachLongPressStop([&](FutureJob &work) {
        SerialOut::info("Triggering on long press stop: %s\n\n", work.esmart.toString().c_str());

        work.esmart.state = !readPin(work.esmart.pin);
        work.esmart.relayState = work.esmart.state;

        // doWork(work);

        longPressReset = 0;
    });
    button.attachDuringLongPress([&](FutureJob &work) {
        SerialOut::info("Triggering on long press: %s\n\n", work.esmart.toString().c_str());

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
            SerialOut::info("Updating off alarm: %s\n\n", esmart.toString().c_str());

            tmElements_t element;
            breakTime(offTime, element);
            Alarm.write(esmart.pin + 1, AlarmHMS(element.Hour, element.Minute, element.Second));
        } else {
            SerialOut::info("Deleting off alarm: %s\n\n", esmart.toString().c_str());

            Alarm.free(esmart.pin + 1);
        }
    } else if (offTime > 0) {
        tmElements_t element;
        breakTime(offTime, element);
        FutureJob work = FutureJob(esmart);
        SerialOut::info("Creating off alarm: %s\n\n", esmart.toString().c_str());

        Alarm.alarmRepeat(element.Hour, element.Minute, element.Second, esmart.pin + 1, work, [&](FutureJob work) {
            SerialOut::info("Triggering off alarm: %s\n\n", esmart.toString().c_str());

            work.esmart.state = !readPin(work.esmart.pin);
            work.esmart.relayState = !readPin(work.esmart.pin);
            doWork(work);
        });
    }
}

void createOnAlarm(EsmartFirebase &esmart) {
    time_t onTime = static_cast<time_t>(esmart.startTime);

    if (Alarm.isAllocated(esmart.pin)) {
        if (onTime != 0) {
            SerialOut::info("Updating on alarm: %s\n\n", esmart.toString().c_str());
            tmElements_t element;
            breakTime(onTime, element);
            Alarm.write(esmart.pin, AlarmHMS(element.Hour, element.Minute, element.Second));
        } else {
            SerialOut::info("Deleting on alarm: %s\n\n", esmart.toString().c_str());
            Alarm.free(esmart.pin);
        }
    } else if (onTime > 0) {
        SerialOut::info("Creating on alarm: %s\n\n", esmart.toString().c_str());

        tmElements_t element;
        breakTime(onTime, element);
        FutureJob work = FutureJob(esmart);

        Alarm.alarmRepeat(element.Hour, element.Minute, element.Second, esmart.pin, work, [&](FutureJob work) {
            SerialOut::info("Triggering on alarm: %s\n\n", esmart.toString().c_str());

            work.esmart.state = !readPin(work.esmart.pin);
            work.esmart.relayState = !readPin(work.esmart.pin);
            doWork(work);
        });
    }
}

int readPin(int pin) {
    int val = digitalRead(pin) ^ READ_OPERATOR;
    SerialOut::info("Reading pin %d value %d and new val %d\n\n", pin, digitalRead(pin), val);
    return val;
}

void writePin(int pin, int statusPin, int val) {
    int newVal = val ^ WRITE_OPERATOR;
    SerialOut::info("Writing to pin %d new value %d and val %d\n\n", pin, val, newVal);
    digitalWrite(pin, newVal);
    digitalWrite(statusPin, val);
}

void checkForServerUpdate() {
    Firebase.getJSON(firebaseJobData, FIRMWARE_PATH);
    handleUpdate(firebaseJobData.jsonString());
}

void handleUpdate(String jsonStr) {
    DynamicJsonDocument doc(250);
    deserializeJson(doc, jsonStr);
    SerialOut::info("Handeling update data %s\n\n", jsonStr.c_str());
    UpdateConfig config = UpdateConfig(doc);
    saveUpdates(doc);
}

bool checkForNewVersion() {
    SerialOut::info("Checking for new version\n\n");

    if (!beginWrite()) return false;

    File updateFile = LittleFS.open("/firmware.json", "r");
    DynamicJsonDocument doc(250);
    deserializeJson(doc, updateFile);
    UpdateConfig config = UpdateConfig(doc);

    if (VERSION < config.version) {
        SerialOut::info("New version found: old version %d new version %d\n\n", VERSION, config.version);
        startUpdate(config);
        return true;
    } else {
        SerialOut::info("No new version found\n");
        return false;
    }
}

void startUpdate(UpdateConfig &config) {
    SerialOut::info("Starting update\n\n");

    BearSSL::WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(5000);

    if (!client.connect(config.host, httpsPort)) {
        SerialOut::info("Could not connect to %s\n\n", config.getUpdateUrl().c_str());
        return;
    } else {
        SerialOut::info("Connected successfully to %s\n\n", config.getUpdateUrl().c_str());
    }

    auto ret = ESPhttpUpdate.update(client, config.getUpdateUrl());

    if (ret == HTTP_UPDATE_FAILED) {
        SerialOut::info("Update failed\n");
    } else if (ret == HTTP_UPDATE_OK) {
        SerialOut::info("Update success, rebooting\n\n");
    }
}

void saveUpdates(DynamicJsonDocument &doc) {
    if (!beginWrite()) return;
    File updateFile = LittleFS.open("/firmware.json", "w");
    serializeJson(doc, updateFile);
}