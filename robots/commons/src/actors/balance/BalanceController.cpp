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
  pidLeft.setWeights(2,0,0);
  targetSpeed = 1 / 3.75 * INT16_MAX;
}

void BalanceController::tick() {
  updateAngle();
  // max speed = 3.8 rps
  if (robota->getTicks() % 10 == 0) {
    RotationData rotL = leftEnc->get(0);
    // left speed [rps]
    float rpsL = RotaryEncoder::pulsesToRotations(rotL.pulses) / (rotL.time / 1000.0);
    float targetRps = targetSpeed / (float) INT16_MAX * 3.75;
    double lOut = pidLeft.pid(targetRps - rpsL, rotL.time / 1000.0);
    Serial.printf("%f rps; lOut: %f", rpsL, lOut);
    lOut = min(1.0, max(-1.0, lOut));
    left->setSpeed(-(INT16_MAX));
    Serial.printf("; ss: %f\n", lOut * (INT16_MAX-1));
  }
  //pidLeft.pid(0,0);
  //Serial.printf("Angle: %f\n", angle);
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

void BalanceController::setTarget(int32_t speed, int32_t steer) {
  targetSpeed = speed;
  targetSteer = steer;
}