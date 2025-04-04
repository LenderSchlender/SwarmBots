#ifndef RotaryEncoder_h
#define RotaryEncoder_h
#include "Module.h"
#include "actors/Actors.h"

struct RotationData {
  // number of pulses [#]
  int pulses;
  // amount of time [ms]
  int time;
};

class RotaryEncoder : public Sensor {
  // How many "output channels" the encoder should have.
  static const uint8_t N = 4;

  uint8_t pin;
  SingleMotorOutput *motor;
  volatile int32_t pulseAmount[N] = {0};
  volatile int32_t startTime[N] = {0};

public:
  /*
   * The `pin` is the pulse pin of the encoder.
   * It seems that Tumblers don't have a connection for the second pin of the encoder.
   * So we use the motor's direction controls to assume the direction of the wheel.
   * Providing a motor is optional, if none is provided, it'll assume that the wheel is only going forward.
   */
  RotaryEncoder(uint8_t pin, SingleMotorOutput *motor);
  /*
   * This should be called with a pointer to a method that
   * only calls the pulse() function.
   */
  void attachISR(void (*intRoutine)());
  void init();
  void terminate();
  /*
   * Gets the latest rotational data.
   * The `index` should be a value from 0 to N-1.
   * Since the rotary encoder's stored values reset after each read,
   * there are four "registers" with values.
   * This makes it possible to use the same encoder up to 4 times within the codebase.
   * Currently, these indices are reserved for operation in a modified Tumbller:
   * 0: Balancing the robots
   * 1: Sending the encoder data to the control server
   */
  RotationData get(uint8_t index);

  /*
   * To be called in the attached ISR
   * Processes the encoder "event" by incrementing all of the related counters.
   */
  void pulse();

  // 330 pulses per rotation
  static float pulsesToRotations(uint32_t pulses);
};

#endif