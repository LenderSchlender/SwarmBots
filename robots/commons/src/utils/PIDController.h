#ifndef PIDController_h
#define PIDController_h

class PIDController {
  float kp, ki, kd;

public:
  // Adjusts the PID Weights/Constants
  void setWeights(float kp, float ki, float kd);

  /*
   * Calculates the adjustment value based on the error, and time difference.
   * The PID Weights can be set using #setWeights
   * Arguments:
   *  float error: The error
   *  float dt: Time difference [s]
   */
  float pid(float error, float dt);
};

#endif