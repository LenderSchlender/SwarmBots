#ifndef BalanceController_h
#define BalanceController_h
#include "Module.h"
#include "actors/Actors.h"
#include "sensors/RotaryEncoder.h"
#include "sensors/MPU6050.h"

// For balancing on one axis
class BalanceController : public Module
{
    SingleMotorOutput *left, *right;
    RotaryEncoder *leftEnc, *rightEnc;
    MPU6050 *mpu;

public:
    BalanceController(SingleMotorOutput *left, SingleMotorOutput *right,
                      RotaryEncoder *leftEnc, RotaryEncoder *rightEnc, MPU6050 *mpu);
    void init();
    void tick();
};

#endif