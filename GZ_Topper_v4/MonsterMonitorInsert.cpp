#include "MonsterMonitorInsert.h"
#include "Arduino.h"
#include "Mux.h"
using namespace admux;

MonsterMonitorInsert::MonsterMonitorInsert(byte insertIn, byte topperOut) {
  // Anything you need when instantiating your object goes here
  _insertIn = insertIn;
  _topperOut = topperOut;
  _blinkWait = 200;
  _lastTimeInsertOff = millis();
  _lastTimeInsertOn = 0;
  _currInsertState = insertState::OFF;

  pinMode(topperOut, OUTPUT);
}

// Pretend this is one or more complex and involved functions you have written
void MonsterMonitorInsert::update(Mux mux) {
  switch (_currInsertState) {
    case insertState::OFF:
      if (getVoltageFromAnalogValue(mux.read(_insertIn)) > 4) {
        // Serial.println("light pending");
        _currInsertState = insertState::PENDING;
        _lastTimeInsertOn = millis();
      }
      break;
    
    case insertState::PENDING:
      // known working time 1600
      if (millis() - _lastTimeInsertOn > 600) {
        if(getVoltageFromAnalogValue(mux.read(_insertIn)) > 4) {
          digitalWrite(_topperOut, HIGH);
          // Serial.println("light on");
          _currInsertState = insertState::ON;
          _lastTimeInsertOff = millis();
        } else {
          _currInsertState = insertState::OFF;
        }

      }
      break;

    case insertState::ON:
      if (mux.read(_insertIn) < 800) {
        if (millis() - _lastTimeInsertOff > 200) {
          // Serial.println("light off");
          digitalWrite(_topperOut, LOW);
          _currInsertState = insertState::OFF;
        }
      } else if (mux.read(_insertIn) > 800) {
        // Serial.println("restart timer");
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
