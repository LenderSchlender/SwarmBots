#include "PIDController.h"

void PIDController::setWeights(float kp, float ki, float kd) {
  this->kp = kp;
  this->ki = ki;
  this->kd = kd;
}

float PIDController::pid(float error, float dt) {
  static float integral, previous;

  // P
  float proportional = error;
  // I
  integral += error * dt;
  // D
  float derivative = (error - previous) / dt;
  previous = error;

  return (kp * proportional) +
         (ki * integral) +
         (kd * derivative);
}