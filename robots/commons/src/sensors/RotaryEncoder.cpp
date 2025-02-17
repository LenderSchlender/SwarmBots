#include "RotaryEncoder.h"

RotaryEncoder::RotaryEncoder(uint8_t pin, SingleMotorOutput *motor) {
    this->pin = pin;
    this->motor = motor;
}

void RotaryEncoder::attachISR(void (*intRoutine)()) {
    attachInterrupt(pin, intRoutine, RISING);
}

void RotaryEncoder::init() {
    pinMode(pin, INPUT);
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

void IRAM_ATTR RotaryEncoder::pulse() {
    for (int i= 0; i < N; i++) {
        pulseAmount[i]++;
    }
}

float RotaryEncoder::pulsesToRotations(uint32_t pulses) {
    return pulses / 330.0;
}