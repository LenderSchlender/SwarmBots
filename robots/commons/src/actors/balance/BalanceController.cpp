#include "BalanceController.h"

BalanceController::BalanceController(SingleMotorOutput *left, SingleMotorOutput *right,
                                     RotaryEncoder *leftEnc, RotaryEncoder *rightEnc, MPU6050 *mpu) {
    this->left = left;
    this->right = right;
    this->leftEnc = leftEnc;
    this->rightEnc = rightEnc;
    this->mpu = mpu;
}

void BalanceController::init() {
    kalman.setAngle(0);
}

void BalanceController::tick() {
  updateAngle();
  Serial.printf("Angle: %f\n", angle);
  // TODO: balance
}

// Heavily inspired by https://github.com/TKJElectronics/KalmanFilter/blob/master/examples/MPU6050/MPU6050.ino
void BalanceController::updateAngle() {
    static uint32_t last = 0;
    sensors_event_t a, g, t;

    mpu->read(&a, &g, &t);
    float dt = (micros() - last) / 1000000.0;
    last = micros();

    float gyroX = g.gyro.x * RAD_TO_DEG;
    float roll = atan2(a.acceleration.y, a.acceleration.z) * RAD_TO_DEG;

    // This fixes the transition problem when the accelerometer angle jumps between -180 and 180 degrees
    if ((roll < -90 && angle > 90) || (roll > 90 && angle < -90)) {
        kalman.setAngle(roll);
        angle = roll;
    } else {
        angle = kalman.getAngle(roll, gyroX, dt); // Calculate the angle using a Kalman filter
    }
}