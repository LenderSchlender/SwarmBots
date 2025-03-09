#include "Arduino.h"

#ifndef Module_h
#define Module_h

#define MODULE_TYPE_UNKNOWN 0x0         // may also be used for custom modules
#define MODULE_TYPE_SENSOR 0x1 << 13    // sensor modules
#define MODULE_TYPE_ACTOR 0x2 << 13     // actor modules
#define MODULE_TYPE_INTERFACE 0x3 << 13 // interface modules

#define SENSOR_GENERIC_DIGITAL_INPUT MODULE_TYPE_SENSOR | 1
#define SENSOR_LD20_LIDAR MODULE_TYPE_SENSOR | 2

#define ACTOR_GENERIC_DIGITAL_OUTPUT MODULE_TYPE_ACTOR | 1
#define ACTOR_PWM_OUTPUT MODULE_TYPE_ACTOR | 2
#define ACTOR_SINGLE_MOTOR_LN298N_OUTPUT MODULE_TYPE_ACTOR | 3
#define ACTOR_DIFFERENTIAL_STEERING_MOTOR_CONTROLLER MODULE_TYPE_ACTOR | 4

// forward declare Robota so we can use it in the Module class
class Robota;

class Module {
  friend class Robota;

protected:
  // used to provide access to the relevant Robota object
  // set by the Robota before init() is called
  Robota *robota;

public:
  /* Module type format:
       Bits 0-13: "fine" type (e.g. GenericDigitalInput)
       Bits 14-15: "coarse" type (Interface/Sensor/Actor)
    */
  virtual uint16_t getType() {
    return MODULE_TYPE_UNKNOWN;
  }
  virtual void init() {}
  virtual void tick() {}
  virtual void terminate() {}
};

class Sensor : public Module {
};

class Actor : public Module {
};

class Interface : public Module {
};
#endif
