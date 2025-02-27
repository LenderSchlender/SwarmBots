#ifndef BalanceController_h
#define BalanceController_h
#include "Module.h"
#include "actors/Actors.h"
#include "sensors/MPU6050.h"
#include "sensors/RotaryEncoder.h"
#include "utils/KalmanFilter.h"

// For balancing on one axis
class BalanceController : public Module {
    SingleMotorOutput *left, *right;
    RotaryEncoder *leftEnc, *rightEnc;
    MPU6050 *mpu;

    KalmanFilter kalman;
    // Calculated angle of the robot using the Kalman filter
    // positive value = falling forwards
    float angle;
    // Read data from IMU and calculate current robot angle
    void updateAngle();

public:
    BalanceController(SingleMotorOutput *left, SingleMotorOutput *right,
                      RotaryEncoder *leftEnc, RotaryEncoder *rightEnc, MPU6050 *mpu);
    void init();
    void tick();
};

#endif