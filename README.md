# ESmart-IoT [![Build Status](https://travis-ci.org/Edydaoud/ESmart-IoT-Public.svg?branch=sonoff_basic)](https://travis-ci.org/Edydaoud/ESmart-IoT-Public)

This project aims to let anyone with basic Arduino knowledge to make a smarter home using [Firebase](http://console.firebase.google.com/), [Platformio IDE](https://platformio.org/platformio-ide) and any supported [ESP8266](https://www.espressif.com/en/products/socs/esp8266/overview) board.

## Firebase Configurations

Create a Firebase project.

Enable Realtime Database and create a json file under `/users/{user id}/{board mac adress}/`

```javascript
{
  "uRwPtyKzTM": {
    "buttonPin": 0, // (optional) hardware button to manually control your device
    "buttonState": 0, // (required if buttonnPin is set) to check whether the button is in HIGH or LOW state
    "defaultState": 1, // (required) for default boot mode
    "endTime": 0, // (Optional) Automation end time
    "id": "uRwPtyKzTM", //(Requered) Device ID
    "ledPin": 13, // (Optional) Status pin if available
    "pin": 12, // (Required) The device relay pin
    "relayState": 1, // (Required) Relay callback for making sure the device is same as state
    "startTime": 0, // (Optional) Automation start time
    "state": 1 // (Required) This will change the state of the relay 0 is ON, 1 is OFF
  }
}
```

## Project Configurations

Go to `data/config.json` and edit it

```json
{
  "wifiAp": "your wifi name",
  "wifiPass": "your wifi password",
  "userId": "firebase user id",
  "firebaseUrl": "firbase project url",
  "firebaseKey": "firebase database secret"
}
```

Save it and upload it to the your Arduino filesystem by pressing on Upload File System image

## Build

You can build either in Cli or in IDE by running `platformio run -e {env name}` or check original doc [here](https://docs.platformio.org/en/latest/core/userguide/cmd_run.html)

**ESmart-IoT** was tested on the following platforms:

* Wemos D1 Mini
* Sonoff Basic
