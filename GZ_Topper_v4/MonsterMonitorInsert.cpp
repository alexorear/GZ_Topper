#include "MonsterMonitorInsert.h"
#include "Arduino.h"
#include "Mux.h"
using namespace admux;

MonsterMonitorInsert::MonsterMonitorInsert(byte insertIn, byte topperOut) {
  _insertIn = insertIn;
  _topperOut = topperOut;
  _lastTimeInsertOff = millis();
  _lastTimeInsertOn = 0;
  _currInsertState = insertState::OFF;

  pinMode(topperOut, OUTPUT);
}

void MonsterMonitorInsert::update(Mux mux) {
  switch (_currInsertState) {
    case insertState::OFF:
      // initial check to see if the insert is lit
      if (getVoltageFromAnalogValue(mux.read(_insertIn)) > 4) {
        _currInsertState = insertState::PENDING;
        _lastTimeInsertOn = millis();
      }
      break;
    
    case insertState::PENDING:
      // insert has been lit previously, we wanted to wait and check the insert status again in 600ms to
      // ensure the statis is actually active vs the insert being lit due to one of
      // many various in game light shows
      if (millis() - _lastTimeInsertOn > 600) {
        if(getVoltageFromAnalogValue(mux.read(_insertIn)) > 4) {
          digitalWrite(_topperOut, HIGH);
          _currInsertState = insertState::ON;
          _lastTimeInsertOff = millis();
        } else {
          // insert wasn't lit on recheck so lets set the state to OFF
          _currInsertState = insertState::OFF;
        }

      }
      break;

    case insertState::ON:
      // when the insert is the active state on the playfield, the insert actually pulses
      // vs being a solid light, this logic helps us determine if the insert is off or just
      // pulsing
      if (mux.read(_insertIn) < 800) {
        if (millis() - _lastTimeInsertOff > 200) {
          digitalWrite(_topperOut, LOW);
          _currInsertState = insertState::OFF;
        }
      } else if (mux.read(_insertIn) > 800) {
        _lastTimeInsertOff = millis();
      }
      break;
    
    default:
      // do nothing
      Serial.println("Default state, something is broken");
  }
}

void MonsterMonitorInsert::setOffState() {
  digitalWrite(_topperOut, LOW);
  _currInsertState = insertState::OFF;
}

// Private method for this class
float MonsterMonitorInsert::getVoltageFromAnalogValue(int inputValue) {
  float voltage = inputValue * (5.0 / 1023.0);
  return voltage;
}

insertState MonsterMonitorInsert::getState() {
  return _currInsertState;
}
