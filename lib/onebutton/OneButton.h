#ifndef OneButton_h
#define OneButton_h

#include "Arduino.h"
#include "FutureJob.hpp"

extern "C" {
using callbackFunction = std::function<void(FutureJob&)>;
}

class OneButton {
   public:
    OneButton();
    OneButton(int buttonPin, int buttonState, FutureJob work);

    void setDebounceTicks(int ticks);
    void setClickTicks(int ticks);
    void setPressTicks(int ticks);
    void attachClick(callbackFunction newFunction);
    void attachDoubleClick(callbackFunction newFunction);
    void attachPress(callbackFunction newFunction);
    void attachPressStart(callbackFunction newFunction);
    void attachLongPressStart(callbackFunction newFunction);
    void attachLongPressStop(callbackFunction newFunction);
    void attachDuringLongPress(callbackFunction newFunction);
    void tick(void);
    void tick(bool level);
    void reset(void);
    
    int getPressedTicks();
    bool isLongPressed();

   private:
    int _pin;  // hardware pin number.
    FutureJob _work;

    unsigned int _debounceTicks = 50;  // number of ticks for debounce times.
    unsigned int _clickTicks = 400;    // number of ticks that have to pass by
                                       // before a click is detected.
    unsigned int _pressTicks = 800;    // number of ticks that have to pass by
                                       // before a long button press is detected
    int _buttonPressed;

    bool _isLongPressed = false;

    callbackFunction _clickFunc = NULL;
    callbackFunction _doubleClickFunc = NULL;
    callbackFunction _pressFunc = NULL;
    callbackFunction _pressStartFunc = NULL;
    callbackFunction _longPressStartFunc = NULL;
    callbackFunction _longPressStopFunc = NULL;
    callbackFunction _duringLongPressFunc = NULL;

    int _state = 0;
    unsigned long _startTime;  // will be set in state 1
    unsigned long _stopTime;   // will be set in state 2
};

#endif