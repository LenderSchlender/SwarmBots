#ifndef TumbllerConstants_h
#define TumbllerConstants_h

#include <inttypes.h>

/*
 * Everything defined within this namespace has
 * been assigned as per the official Tumbller documentation
 * for the unmodified tumbller.
 * Robot-specific modifications should be defined in their
 * respective projects.
 */
namespace TumbllerConstants {
  uint8_t
      // Serial received
      PIN_SERIAL_RX = D0,
      // Serial transmit
      PIN_SERIAL_TX = D1,
      // Left motor encoder pulse input
      PIN_LM_ENC = D2,
      // RGB LED control
      PIN_RGB_LED = D3,
      // Right motor encoder pulse input
      PIN_RM_ENC = D4,
      // Left motor PWM speed control
      PIN_LM_PWM = D5,
      // Right motor PWM speed control
      PIN_RM_PWM = D6,
      // Right motor direction control
      PIN_RM_DIR = D7,
      // Motor enable
      PIN_MOTOR_EN = D8,
      // Infrared distance sensor transmit
      PIN_IR_TX = D9,
      // Mode Button (low-active)
      PIN_MODE = D10,
      // Ultrasonic trigger
      PIN_US_TRIG = D11,
      // Left motor direction control
      PIN_LM_DIR = D12,
      // Left infrared receiver input
      PIN_IR_L_RX = A0,
      // Right infrared receiver input
      PIN_IR_R_RX = A1,
      // Battery voltage monitor
      PIN_BATT_MONI = A2,
      // Ultrasonic sensor echo input
      PIN_US_ECHO = A3,
      // I2C interface: SDA
      PIN_I2C_SDA = A4,
      // I2C interface: SDA
      PIN_I2C_SCL = A5;
}

#endif