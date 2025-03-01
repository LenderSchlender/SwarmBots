#ifndef MPU6050_h
#define MPU6050_h

#include "Module.h"
#include "Robota.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

class MPU6050 : public Sensor {
  Adafruit_MPU6050 mpu;

public:
  void init();
  void tick();
  void read(sensors_event_t *accel, sensors_event_t *gyro,
            sensors_event_t *temp);
};

#endif