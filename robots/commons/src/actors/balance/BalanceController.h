#ifndef BalanceController_h
#define BalanceController_h
#include "Module.h"
#include "actors/Actors.h"
#include "sensors/RotaryEncoder.h"

// For balancing on one axis
class BalanceController
{
    SingleMotorOutput *left, *right;
    RotaryEncoder *leftEnc, *rightEnc;

public:
    BalanceController(SingleMotorOutput *left, SingleMotorOutput *right,
                      RotaryEncoder *leftEnc, RotaryEncoder *rightEnc);
    void init();
};

#endif