#include "CityInsert.h"
#include "Arduino.h"
#include <Adafruit_NeoPixel.h>
#include "Mux.h"
using namespace admux;

CityInsert::CityInsert(byte insertIn, int pending, int timeOffOffset = 200) {
    _insertIn = insertIn;
    _cityInsertStatus = cityInsertStatus::OFF;
    _lastTimeInsertOff = 0;
    _pendingDelay = pending;
    pendingDelay = pending;
    lastFireMillis = 0;
    fireLightsOff = true; //read and modified by main loop code
    _animationPos = 0;
    _timeOffOffset = timeOffOffset;
    isAnimating = false;
}

cityInsertStatus CityInsert::getStatus(Mux mux) {
  switch (_cityInsertStatus) {
    case cityInsertStatus::OFF:
      _animationPos = 0;
      if (getVoltageFromAnalogValue(mux.read(_insertIn)) > 4) {
        _cityInsertStatus = cityInsertStatus::PENDING;
        _lastTimeInsertOn = millis();
      }
      break;

    case cityInsertStatus::PENDING:
      if (millis() - _lastTimeInsertOn > _pendingDelay) {
        if(getVoltageFromAnalogValue(mux.read(_insertIn)) > 4) {
          // Serial.println("light on");
          _cityInsertStatus = cityInsertStatus::ON;
          _lastTimeInsertOff = millis();
        } else {
          _cityInsertStatus = cityInsertStatus::OFF;
        }
      }
      break;

    case cityInsertStatus::ON:
      if (mux.read(_insertIn) < 800) {
          if (_animationPos <= 0 || _animationPos == 64 || _animationPos >= 128) {
            if (millis() - _lastTimeInsertOff > _timeOffOffset) {
              _animationPos = 0;
              _cityInsertStatus = cityInsertStatus::OFF;
            }
          }
        } else if (mux.read(_insertIn) > 800) {
          _lastTimeInsertOff = millis();
        }
      break;

    default:
    // do nothing
    Serial.println('busted');
  }
  return _cityInsertStatus;
}

float CityInsert::getVoltageFromAnalogValue(int inputValue) {
  float voltage = inputValue * (5.0 / 1023.0);
  return voltage;
}

void CityInsert::updateAnimationPos(int pos) {
  _animationPos = pos;
}

int CityInsert::getAnimationPos() {
  return _animationPos;
}
