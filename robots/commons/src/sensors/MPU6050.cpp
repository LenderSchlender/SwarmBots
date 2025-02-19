#include "MPU6050.h"

void MPU6050::init() {
    if (!mpu.begin()) {
        Serial.println("CRIT: Failed to initialise MPU!");
        while (1);
    }
    Serial.println("MPU initialised!");

    // set accelerometer range to +-8G
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

    // set gyro range to +- 500 deg/s
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);

    // set filter bandwidth to 21 Hz
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

void MPU6050::tick() {

}

void MPU6050::read(sensors_event_t *accel, sensors_event_t *gyro,
    sensors_event_t *temp) {
    static uint32_t lastRead = 0;
    static sensors_event_t a, g, t;
    
    // this prevents wasting time by reading the sensor multiple times per tick
    if (lastRead != robota->getTicks()) {
        mpu.getEvent(&a, &g, &t);
        lastRead = robota->getTicks();
    }

    memcpy(accel, &a, sizeof(sensors_event_t));
    memcpy(gyro, &g, sizeof(sensors_event_t));
    memcpy(temp, &t, sizeof(sensors_event_t));
}