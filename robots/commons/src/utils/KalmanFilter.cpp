#include "KalmanFilter.h"

KalmanFilter::KalmanFilter() {
  /* We will set the variables like so, these can also be tuned by the user */
  Q_angle = 0.001f;
  Q_bias = 0.003f;
  R_measure = 0.03f;

  angle = 0.0f; // Reset the angle
  bias = 0.0f;  // Reset bias

  P[0][0] = 0.0f; // Since we assume that the bias is 0 and we know the starting angle (use setAngle), the error covariance matrix is set like so - see: http://en.wikipedia.org/wiki/Kalman_filter#Example_application.2C_technical
  P[0][1] = 0.0f;
  P[1][0] = 0.0f;
  P[1][1] = 0.0f;
};

// The angle should be in degrees and the rate should be in degrees per second and the delta time in seconds
float KalmanFilter::getAngle(float newAngle, float newRate, float dt) {
  // KasBot V2  -  Kalman filter module - http://www.x-firm.com/?page_id=145
  // Modified by Kristian Lauszus
  // See my blog post for more information: http://blog.tkjelectronics.dk/2012/09/a-practical-approach-to-kalman-filter-and-how-to-implement-it

  // Discrete Kalman filter time update equations - Time Update ("Predict")
  // Update xhat - Project the state ahead
  /* Step 1 */
  rate = newRate - bias;
  angle += dt * rate;

  // Update estimation error covariance - Project the error covariance ahead
  /* Step 2 */
  P[0][0] += dt * (dt * P[1][1] - P[0][1] - P[1][0] + Q_angle);
  P[0][1] -= dt * P[1][1];
  P[1][0] -= dt * P[1][1];
  P[1][1] += Q_bias * dt;

  // Discrete Kalman filter measurement update equations - Measurement Update ("Correct")
  // Calculate Kalman gain - Compute the Kalman gain
  /* Step 4 */
  float S = P[0][0] + R_measure; // Estimate error
  /* Step 5 */
  float K[2]; // Kalman gain - This is a 2x1 vector
  K[0] = P[0][0] / S;
  K[1] = P[1][0] / S;

  // Calculate angle and bias - Update estimate with measurement zk (newAngle)
  /* Step 3 */
  float y = newAngle - angle; // Angle difference
  /* Step 6 */
  angle += K[0] * y;
  bias += K[1] * y;

  // Calculate estimation error covariance - Update the error covariance
  /* Step 7 */
  float P00_temp = P[0][0];
  float P01_temp = P[0][1];

  P[0][0] -= K[0] * P00_temp;
  P[0][1] -= K[0] * P01_temp;
  P[1][0] -= K[1] * P00_temp;
  P[1][1] -= K[1] * P01_temp;

  return angle;
};

void KalmanFilter::setAngle(float angle) { this->angle = angle; }; // Used to set angle, this should be set as the starting angle
float KalmanFilter::getRate() { return this->rate; };              // Return the unbiased rate

/* These are used to tune the Kalman filter */
void KalmanFilter::setQangle(float Q_angle) { this->Q_angle = Q_angle; };
void KalmanFilter::setQbias(float Q_bias) { this->Q_bias = Q_bias; };
void KalmanFilter::setRmeasure(float R_measure) { this->R_measure = R_measure; };

float KalmanFilter::getQangle() { return this->Q_angle; };
float KalmanFilter::getQbias() { return this->Q_bias; };
float KalmanFilter::getRmeasure() { return this->R_measure; };