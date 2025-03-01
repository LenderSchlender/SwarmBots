#include "Actors.h"
#include "Arduino.h"
#include "driver/ledc.h"
#include "esp32-hal-ledc.h"
#include "hal/ledc_types.h"
#include "math.h"
#include <cmath>
#include <cstdint>

GenericDigitalOutput::GenericDigitalOutput(uint8_t pin) {
  this->pin = pin;
}

uint16_t GenericDigitalOutput::getType() {
  return ACTOR_GENERIC_DIGITAL_OUTPUT;
}

void GenericDigitalOutput::init() {
  pinMode(pin, OUTPUT);
}

uint8_t GenericDigitalOutput::getState() {
  uint8_t bit = digitalPinToBitMask(pin);
  int8_t port = digitalPinToPort(pin);
  if (port == NOT_A_PIN)
    return LOW;

  return (*portOutputRegister(port) & bit) ? HIGH : LOW;
}

void GenericDigitalOutput::write(uint8_t state) {
  digitalWrite(pin, state);
}

void GenericDigitalOutput::enable() {
  digitalWrite(pin, HIGH);
}

void GenericDigitalOutput::disable() {
  digitalWrite(pin, LOW);
}

void GenericDigitalOutput::toggle() {
  uint8_t bit = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  if (port == NOT_A_PIN)
    return;

  *portOutputRegister(port) ^= bit;
}

uint16_t PwmOutput::getType() {
  return ACTOR_PWM_OUTPUT;
}

uint16_t PwmOutput::usedChannels = 0x0;

uint8_t PwmOutput::nextBestChannel(uint32_t frequency) {
  // Assume this is the only place where pwm channels are chosen
  for (int chan = 0; chan < 16; chan++) {
    // Check if this channel is in use
    if ((usedChannels >> chan) & 1 != 0) {
      continue;
    }

    // Check the other channel using the same timer if it is using the same frequency
    if (chan % 2 == 0) {
      // if the next channel is in use
      if ((usedChannels >> chan + 1) & 1 == 1) {
        // ...and if the next channel is using the same frequency
        if (ledcReadFreq(chan + 1) != frequency) {
          // ...search for another channel
          continue;
        }
      }
    } else {
      // if the previous channel is in use
      if ((usedChannels >> chan - 1) & 1 == 1) {
        // ...and if previous channel is not using the same frequency
        if (ledcReadFreq(chan - 1) != frequency) {
          // ...search for another channel
          continue;
        }
      }
    }

    return chan;
  }
  return 0xFF;
}

uint8_t PwmOutput::maxResolution(uint32_t frequency) {
  // res_max = log2(clk_speed / f)
  return min((uint8_t)(log(getCpuFrequencyMhz() * 1000000 / frequency) / log(2)), (uint8_t)16);
}

PwmOutput::PwmOutput(uint8_t pin) {
  this->pin = pin;
  this->frequency = 2000;
  this->channel = 0xFF; // placeholder value
  this->resolution = maxResolution(frequency);
}

PwmOutput::PwmOutput(uint8_t pin, uint32_t frequency, uint8_t resolution) {
  this->pin = pin;
  this->frequency = frequency;
  this->channel = 0xFF; // placeholder value
  if (resolution == 0) {
    this->resolution = maxResolution(frequency);
  } else {
    this->resolution = resolution;
  }
}

PwmOutput PwmOutput::servo(uint8_t pin) {
  return PwmOutput(pin, 50);
}

void PwmOutput::init() {
  // Select best possible channel
  channel = nextBestChannel(frequency);

  // Mark channel as "in use"
  usedChannels |= (1 << channel);

  ledcSetup(channel, frequency, resolution);
  ledcAttachPin(pin, channel);
}

uint16_t PwmOutput::getTrueDutyCycle() {
  return (uint16_t)ledcRead(channel);
}

uint16_t PwmOutput::getDutyCycle() {
  return ((uint16_t)ledcRead(channel)) << resolution;
}

uint16_t PwmOutput::getPulseWidth() {
  // PW = DC / (2^res) * period_µs
  return getDutyCycle() / pow(2.0, resolution) * (1000000.0 / frequency);
}

int8_t PwmOutput::getServoAngle() {
  return map(getPulseWidth(), 1000, 2000, -90, 90);
}

void PwmOutput::writeTrueDutyCycle(uint16_t dutyCycle) {
  ledcWrite(channel, dutyCycle);
}

void PwmOutput::writeDutyCycle(uint16_t dutyCycle) {
  ledcWrite(channel, dutyCycle >> (16 - resolution));
}

void PwmOutput::writePulseWidth(uint16_t microSeconds) {
  // DC = (2^res) * PW * period_µs
  writeTrueDutyCycle(pow(2.0, resolution) * microSeconds / (1000000.0 / frequency));
}

void PwmOutput::writeServoAngle(int8_t angle) {
  writePulseWidth(map(angle, -90, 90, 1000, 2000));
}

void PwmOutput::terminate() {
  ledcDetachPin(pin);
  // TODO stop timer
  usedChannels &= ~(1 << channel);
}

SingleMotorOutput::SingleMotorOutput(PwmOutput *enable, GenericDigitalOutput *forward, GenericDigitalOutput *backward) {
  this->enable = enable;
  this->forward = forward;
  this->backward = backward;
}

uint16_t SingleMotorOutput::getType() {
  return ACTOR_SINGLE_MOTOR_LN298N_OUTPUT;
}

void SingleMotorOutput::init() {
  setSpeed(0);
}

void SingleMotorOutput::setSpeed(int16_t speed) {
  enable->writeDutyCycle(((uint16_t)abs(speed)) << 1); // Take absolute value and shift left by one (since it's a signed int we can only use 15 bits, but the pwm function takes 16)
  if (speed == 0) {
    if (forward != nullptr)
      forward->disable();
    if (backward != nullptr)
      backward->disable();
  } else {
    // We only care about the sign bit here
    if (forward != nullptr)
      forward->write(~((uint16_t)speed) >> 15 & 1);
    if (backward != nullptr)
      backward->write(((uint16_t)speed) >> 15 & 1);
  }
}

int16_t SingleMotorOutput::getSpeed() {
  if (forward != nullptr && backward != nullptr) {
    if (forward->getState() == backward->getState()) {
      return 0;
    } else {
      return forward->getState() == HIGH ? enable->getDutyCycle() >> 1 : -(enable->getDutyCycle() >> 1);
    }
  } else if (forward != nullptr) {
    return forward->getState() == HIGH ? enable->getDutyCycle() >> 1 : -(enable->getDutyCycle() >> 1);
  } else if (backward != nullptr) {
    return backward->getState() == LOW ? enable->getDutyCycle() >> 1 : -(enable->getDutyCycle() >> 1);
  }

  return 0;
}