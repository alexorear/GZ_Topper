#ifndef ci
#define ci
#include "Arduino.h"
#include <Adafruit_NeoPixel.h>
#include "Mux.h"
using namespace admux;

enum class cityInsertStatus : uint8_t {
  ON,
  OFF,
  PENDING
};

class CityInsert {
  public:
    // Constructor 
    CityInsert(byte insertIn, int pending, int timeOffOffset = 200);

    // Methods
    cityInsertStatus getStatus(Mux mux);
    void updateAnimationPos(int pos);
    void setIsAnimation(bool isAnimating);
    int getAnimationPos();
    bool topperLightsActive();
    void setTopperLightsActive(bool active);
    
    // variables
    unsigned long lastFireMillis;

  private:
    // Methods
    float getVoltageFromAnalogValue(int inputValue);
  
    // variables
    byte _insertIn;
    cityInsertStatus _cityInsertStatus;
    unsigned long _lastTimeInsertOff;
    unsigned long _lastTimeInsertOn; 
    int _pendingDelay;
    int _timeOffOffset;
    int _animationPos;
    bool _topperLightsActive;
};
#endif