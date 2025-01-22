#include "RotaryEncoder.h"

RotaryEncoder::RotaryEncoder(uint8_t pin, SingleMotorOutput *motor) {

}

void RotaryEncoder::init() {

}

RotationData RotaryEncoder::get(uint8_t index) {
    RotationData data;
    data.pulses = pulseAmount[index];
    data.time = millis() - startTime[index];

    pulseAmount[index] = 0;
    startTime[index] = millis();

    return data;
}