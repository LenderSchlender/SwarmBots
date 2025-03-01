#include "Arduino.h"
#include "LD20Sensor.h"
#include "Module.h"

#ifndef Sensors_h
#define Sensors_h

class GenericDigitalInput : public Sensor {
  uint8_t pin;
  bool pullup;

public:
  GenericDigitalInput(uint8_t pin);
  GenericDigitalInput(uint8_t pin, bool pullup);
  uint16_t getType();
  void init();
  uint8_t read();
};

#endif
