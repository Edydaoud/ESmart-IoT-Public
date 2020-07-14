#ifndef OneButton_h
#define OneButton_h

#include "Arduino.h"
#include "FutureJob.hpp"

extern "C"
{
  using callbackFunction = std::function<void(FutureJob)>;
}

class OneButton
{
public:
  OneButton();
  OneButton(int buttonPin, FutureJob work, boolean activeLow = true, bool pullupActive = true);

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
  bool isLongPressed();
  int getPressedTicks();
  void reset(void);

private:
  int _pin;                         // hardware pin number.
  FutureJob _work;

  unsigned int _debounceTicks = 50; // number of ticks for debounce times.
  unsigned int _clickTicks = 600;   // number of ticks that have to pass by
                                    // before a click is detected.
  unsigned int _pressTicks = 1000;  // number of ticks that have to pass by
                                    // before a long button press is detected
  int _buttonPressed;

  bool _isLongPressed = false;

  callbackFunction _clickFunc = NULL;
  void *_clickFuncParam = NULL;

  callbackFunction _doubleClickFunc = NULL;
  void *_doubleClickFuncParam = NULL;

  callbackFunction _pressFunc = NULL;
  callbackFunction _pressStartFunc = NULL;

  callbackFunction _longPressStartFunc = NULL;
  void *_longPressStartFuncParam = NULL;

  callbackFunction _longPressStopFunc = NULL;
  void *_longPressStopFuncParam;

  callbackFunction _duringLongPressFunc = NULL;
  void *_duringLongPressFuncParam = NULL;

  int _state = 0;
  unsigned long _startTime; // will be set in state 1
  unsigned long _stopTime;  // will be set in state 2
};

#endif