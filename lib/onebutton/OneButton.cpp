#include "OneButton.h"

OneButton::OneButton() {
    _pin = -1;
}

OneButton::OneButton(int buttonPin, int buttonState, FutureJob work, boolean activeLow, bool pullupActive) {
    _work = work;
    _pin = buttonPin;
    _buttonPressed = buttonState;
    pinMode(buttonPin, INPUT_PULLUP);
}

void OneButton::setDebounceTicks(int ticks) {
    _debounceTicks = ticks;
}
void OneButton::setClickTicks(int ticks) {
    _clickTicks = ticks;
}  // setClickTicks
void OneButton::setPressTicks(int ticks) {
    _pressTicks = ticks;
}  // setPressTicks
void OneButton::attachClick(callbackFunction newFunction) {
    _clickFunc = newFunction;
}  // attachClick
void OneButton::attachDoubleClick(callbackFunction newFunction) {
    _doubleClickFunc = newFunction;
}  // attachDoubleClick
void OneButton::attachPress(callbackFunction newFunction) {
    _pressFunc = newFunction;
}  // attachPress
void OneButton::attachPressStart(callbackFunction newFunction) {
    _pressStartFunc = newFunction;
}  // attachPressStart
void OneButton::attachLongPressStart(callbackFunction newFunction) {
    _longPressStartFunc = newFunction;
}  // attachLongPressStart
void OneButton::attachLongPressStop(callbackFunction newFunction) {
    _longPressStopFunc = newFunction;
}  // attachLongPressStop
void OneButton::attachDuringLongPress(callbackFunction newFunction) {
    _duringLongPressFunc = newFunction;
}  // attachDuringLongPress
bool OneButton::isLongPressed() {
    return _isLongPressed;
}

int OneButton::getPressedTicks() {
    return _stopTime - _startTime;
}

void OneButton::reset(void) {
    _state = 0;  // restart.
    _startTime = 0;
    _stopTime = 0;
    _isLongPressed = false;
}

void OneButton::tick(void) {
    if (_pin >= 0) tick(GPIP(_pin) == _buttonPressed);
}

void OneButton::tick(bool activeLevel) {
    unsigned long now = millis();  // current (relative) time in msecs.
    if (_state == 0) {             // waiting for menu pin being pressed.
        if (activeLevel) {
            _state = 1;        // step to state 1
            _startTime = now;  // remember starting time
        }                      // if
    } else if (_state == 1) {  // waiting for menu pin being released.

        if ((!activeLevel) &&
            ((unsigned long)(now - _startTime) < _debounceTicks)) {
            _state = 0;
        } else if (!activeLevel) {
            _state = 2;       // step to state 2
            _stopTime = now;  // remember stopping time
        } else if ((activeLevel) &&
                   ((unsigned long)(now - _startTime) > _pressTicks)) {
            _stopTime = now;        // remember stopping time
            _isLongPressed = true;  // Keep track of long press state
            if (_pressFunc) {
                _pressFunc(_work);
            }
            if (_longPressStartFunc)
                _longPressStartFunc(_work);
            if (_duringLongPressFunc)
                _duringLongPressFunc(_work);
            _state = 6;  // step to state 6
        } else {
            if (_pressStartFunc)
                _pressStartFunc(_work);
        }  // if
    } else if (_state == 2) {
        if ((_doubleClickFunc == NULL) ||
            (unsigned long)(now - _startTime) > _clickTicks) {
            if (_clickFunc) {
                _clickFunc(_work);
            }
            _state = 0;  // restart.
        } else if ((activeLevel) &&
                   ((unsigned long)(now - _stopTime) > _debounceTicks)) {
            _state = 3;        // step to state 3
            _startTime = now;  // remember starting time
        }                      // if
    } else if (_state == 3) {
        if ((!activeLevel) &&
            ((unsigned long)(now - _startTime) > _debounceTicks)) {
            _stopTime = now;  // remember stopping time
            if (_doubleClickFunc)
                _doubleClickFunc(_work);
            _state = 0;  // restart.
        }                // if
    } else if (_state == 6) {
        if (!activeLevel) {
            _isLongPressed = false;  // Keep track of long press state
            _stopTime = now;         // remember stopping time
            if (_longPressStopFunc) {
                _longPressStopFunc(_work);
            }
            _state = 0;  // restart.
        } else {
            _stopTime = now;        // remember stopping time
            _isLongPressed = true;  // Keep track of long press state
            if (_duringLongPressFunc)
                _duringLongPressFunc(_work);
        }  // if

    }  // if
}  // OneButton.tick()