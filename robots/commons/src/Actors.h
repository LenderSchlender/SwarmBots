#include <cstdint>
#include "Arduino.h"
#include "Module.h"

#ifndef Actors_h
#define Actors_h

class GenericDigitalOutput : public Actor {
  uint8_t pin;
public:
  GenericDigitalOutput(uint8_t pin);
  uint16_t getType();
  void init();

  uint8_t getState();
  void write(uint8_t state);
  void enable();
  void disable();
  void toggle();
};

class PwmOutput : public Actor {
  // set bits for used channels - since there are only 16 channels we can just use uint16_t
  //TODO clear usedChannels bit when destroying
  static uint16_t usedChannels;

  uint8_t pin;
  uint32_t frequency;
  uint8_t channel;
  uint8_t resolution;
public:
  static uint8_t nextBestChannel(uint32_t frequency);
  static uint8_t maxResolution(uint32_t frequency);
  static PwmOutput servo(uint8_t pin);
  PwmOutput(uint8_t pin);
  // if the resolution is 0, it will default to the maximum possible resolution
  PwmOutput(uint8_t pin, uint32_t frequency, uint8_t resolution = 0);
  uint16_t getType();
  void init();
  uint32_t getFrequency();

  uint16_t getTrueDutyCycle();
  uint16_t getDutyCycle();
  uint16_t getPulseWidth();
  int8_t getServoAngle();

  void writeTrueDutyCycle(uint16_t dutyCycle); // uses whatever actual resolution the pwm output has available
  void writeDutyCycle(uint16_t dutyCycle); // always use 16 bits of resolution
  void writePulseWidth(uint16_t microSeconds);  // 1ms = -90° ; 2ms = 90°
  void writeServoAngle(int8_t angle);           // -90° to 90°

  void terminate();
};

class SingleMotorLN298NOutput : public MotorOutput {
  static const uint16_t SIGN_BIT_MASK = 0x8000;
  PwmOutput *enable;
  GenericDigitalOutput *forward, *backward;
public:
  SingleMotorLN298NOutput(PwmOutput* enable, GenericDigitalOutput* forward, GenericDigitalOutput* backward);
  uint16_t getType();
  void init();
  //TODO maybe implement this one day
  //static SingleMotorLN298NOutput create(Robota robota, uint8_t pinEnable, uint8_t pinForward, uint8_t pinBackward);
  void setSpeed(int16_t speed);
  int16_t getSpeed();
};
#endif