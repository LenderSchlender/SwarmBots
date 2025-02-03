#include "RotaryEncoder.h"

RotaryEncoder::RotaryEncoder(uint8_t pin, SingleMotorOutput *motor) {

}

void RotaryEncoder::attachISR(void (*intRoutine)()) {
    attachInterrupt(pin, intRoutine, RISING);
}

void RotaryEncoder::init() {
    
}

void RotaryEncoder::terminate() {
    detachInterrupt(pin);
}

RotationData RotaryEncoder::get(uint8_t index) {
    RotationData data;
    data.pulses = pulseAmount[index];
    data.time = millis() - startTime[index];

    pulseAmount[index] = 0;
    startTime[index] = millis();

    return data;
}