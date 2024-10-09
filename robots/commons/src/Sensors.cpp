#include "Sensors.h"

GenericDigitalInput::GenericDigitalInput(uint8_t pin) {
  this->pin = pin;
  this->pullup = false;
}

GenericDigitalInput::GenericDigitalInput(uint8_t pin, bool pullup) {
  this->pin = pin;
  this->pullup = pullup;
}

uint16_t GenericDigitalInput::getType() {
  return SENSOR_GENERIC_DIGITAL_INPUT;
}

void GenericDigitalInput::init() {
  if (pullup) {
    pinMode(pin, INPUT_PULLUP);
  } else {
    pinMode(pin, INPUT);
  }
}

uint8_t GenericDigitalInput::read() {
  return digitalRead(pin);
}