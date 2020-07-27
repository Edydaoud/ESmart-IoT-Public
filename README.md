# ESmart-IoT ![Badge](https://github.com/Edydaoud/ESmart-IoT-Public/workflows/PlatformIO%20CI/badge.svg)

This project aims to let anyone with basic Arduino knowledge to make a smarter home using [Firebase](http://console.firebase.google.com/), [Platformio IDE](https://platformio.org/platformio-ide), and any supported [ESP8266](https://www.espressif.com/en/products/socs/esp8266/overview) board.

## Prerequisites

- Basic Arduino knowledge.
- Platformio [CLI](https://docs.platformio.org/en/latest/core/) or [IDE](https://platformio.org/platformio-ide),
- Arduino board with integrated ESP8266 chip.
- Firebase project.

## Dependencies

- ESP8266 Core SDK version 2.4.0 and above.
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson).
- [Firebase-ESP8266](https://github.com/mobizt/Firebase-ESP8266).
- [NTPClient](https://github.com/arduino-libraries/NTPClient).
- [Time](https://github.com/PaulStoffregen/Time).

## Instructions - Firebase

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

## Instructions - Project

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

You can build it either in IDE or CLI by running `platformio run -e {env name}` in terminal [&#9432;](https://docs.platformio.org/en/latest/core/userguide/cmd_run.html)

## Tested Devices

- Wemos D1 Mini
- Sonoff Basic
