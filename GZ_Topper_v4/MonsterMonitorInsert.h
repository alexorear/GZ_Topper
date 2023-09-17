#ifndef mmi
#define mmi

#include "Arduino.h"
#include "Mux.h"
using namespace admux;

enum class insertState : uint8_t {
  OFF,
  ON,
  PENDING
};

class MonsterMonitorInsert  {
  public:
    // Constructor 
    MonsterMonitorInsert(byte insertIn, byte topperOut);

    // Methods
    void update(Mux mux);
    void setOffState();
    insertState getState();
    

  private:
    // Methods
    float getVoltageFromAnalogValue(int inputValue);

    // variables
    unsigned long _lastTimeInsertOff;
    unsigned long _lastTimeInsertOn; 
    insertState _currInsertState;
    byte _insertIn;
    byte _topperOut;
    int _blinkWait;
};
#endif