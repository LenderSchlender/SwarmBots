#ifndef RotaryEncoder_h
#define RotaryEncoder_h
#include "Module.h"
#include "actors/Actors.h"

class RotaryEncoder : Sensor
{
    // for access to class members in the ISR
    static RotaryEncoder* instances[4];
    uint8_t pin;
    SingleMotorOutput *motor;
    volatile int32_t pulseAmount[4] = { 0 };
    volatile int32_t startTime[4] = { 0 };

public:
    /*
     * The `pin` is the pulse pin of the encoder.
     * It seems that Tumblers don't have a connection for the second pin of the encoder.
     * So we use the motor's direction controls to assume the direction of the wheel.
     * Providing a motor is optional, if none is provided, it'll assume that the wheel is only going forward.
     */
    RotaryEncoder(uint8_t pin, SingleMotorOutput *motor);
    void init();
    /*
     * Gets the latest rotational data.
     * The `index` should be a value from 0 to 3.
     * Since the rotary encoder's stored values reset after each read,
     * there are four "registers" with values.
     * This makes it possible to use the same encoder up to 4 times within the codebase.
     * Currently, these indices are reserved for operation in a modified Tumbller:
     * 0: Balancing the robots
     * 1: Sending the encoder data to the control server
     */
    RotationData get(uint8_t index);
};

struct RotationData
{
    // number of pulses [#]
    int pulses;
    // amount of time [ms]
    int time;
};

#endif