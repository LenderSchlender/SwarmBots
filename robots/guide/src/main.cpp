#include <Arduino.h>
#include <Interfaces.cpp>
#include <Interfaces.h>
#include <Robota.h>
#include <TumbllerConstants.h>
#include <actors/Actors.h>
#include <actors/balance/BalanceController.h>
#include <sensors/MPU6050.h>
#include <sensors/RotaryEncoder.h>
#include <sensors/Sensors.h>
#include <Protocol.h>

// Following two macros are just placeholders, in reality they're externally defined in the .env file
#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif

#ifndef WIFI_PASS
#define WIFI_PASS ""
#endif

// Global function definitions
void leftEncoderISR();
void rightEncoderISR();
void websocketEventHandler(AsyncWebSocket *server, AsyncWebSocketClient *client,
                           AwsEventType type, void *arg, uint8_t *data, size_t len);
void lidarMeasurementHandler(SingleLiDARMeasurement measurement);
void moveCommandHandler(MoveCmd cmd);
void ledCommandHandler(LedCmd cmd);

using namespace TumbllerConstants;

Robota robota;

LD20Sensor lidar(&Serial2);

// Left Motor controls
PwmOutput leftMotorEnable(PIN_LM_PWM, 2000, 14);
GenericDigitalOutput leftMotorForward(PIN_LM_DIR);
SingleMotorOutput leftMotor(&leftMotorEnable, &leftMotorForward);
// Right Motor controls TODO set pins
PwmOutput rightMotorEnable(PIN_RM_PWM, 2000, 14);
GenericDigitalOutput rightMotorForward(PIN_RM_DIR);
SingleMotorOutput rightMotor(&rightMotorEnable, &rightMotorForward);

// Motor speed measuring
RotaryEncoder leftEncoder(PIN_LM_ENC, &leftMotor);
RotaryEncoder rightEncoder(PIN_RM_ENC, &rightMotor);

MPU6050 mpu;

// Balancing
BalanceController balancer(&leftMotor, &rightMotor, &leftEncoder, &rightEncoder, &mpu);

WiFiConnection wifi("sb-guide", WIFI_SSID, WIFI_PASS);
// Remote Control Interface
WebSocketControls controls;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");
  lidar.setCallback(lidarMeasurementHandler);
  robota.addModule(&lidar);
  pinMode(PIN_MOTOR_EN, OUTPUT);
  digitalWrite(PIN_MOTOR_EN, HIGH);

  // Left Motor
  robota.addModule(&leftMotorEnable);
  robota.addModule(&leftMotorForward);
  robota.addModule(&leftMotor);
  // Right motor
  robota.addModule(&rightMotorEnable);
  robota.addModule(&rightMotorForward);
  robota.addModule(&rightMotor);

  robota.addModule(&leftEncoder);
  robota.addModule(&rightEncoder);

  // Attach encoder ISRs
  leftEncoder.attachISR(leftEncoderISR);
  rightEncoder.attachISR(rightEncoderISR);

  robota.addModule(&mpu);

  // Balancing
  robota.addModule(&balancer);

  // WiFi Connection
  robota.addModule(&wifi);

  // Remote Controls
  controls.setMoveCmdHandler(moveCommandHandler);
  controls.setLedCmdHandler(ledCommandHandler);
  robota.addModule(&controls);

  // Init
  robota.init();
  Serial.println("Setup done!");
}

void leftEncoderISR() {
  leftEncoder.pulse();
}

void rightEncoderISR() {
  rightEncoder.pulse();
}

void websocketEventHandler(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  Serial.printf("I got a message! Type: %d; Client: %s", type, client->client()->remoteIP());
  if (type == WS_EVT_DISCONNECT || type == WS_EVT_ERROR) {
    leftMotor.setSpeed(0);
    rightMotor.setSpeed(0);
  }
  if (type != WS_EVT_DATA) {
    return;
  }

  switch (data[0]) {
  case 0x00:
    leftMotor.setSpeed((data[1] << 8) | data[2]);
    rightMotor.setSpeed((data[3] << 8) | data[4]);
    break;
  default:
    client->printf("I don't know what message type 0x%x means :(", data[0]);
  }
}

void lidarMeasurementHandler(SingleLiDARMeasurement measurement) {
  // TODO new lidar measurement handler system
  //  featuring batching of measurements and sending them as protobufs
  /*if (!controls.isConnected())
    return;
  static const uint8_t BUFFER_SIZE = 100;
  static SingleLiDARMeasurement buffer[BUFFER_SIZE];
  static uint8_t count = 0;
  count = count++ % BUFFER_SIZE;
  buffer[count] = measurement;
  if (count == 0 && controls.availableForWrite()) {
    //Serial.printf("SENDING %d BYTES!\n", sizeof(buffer));
    controls.send((uint8_t *)&buffer, sizeof(buffer)); //TODO apparently this gets interpreted as a ping by the python program
  }*/
}

void moveCommandHandler(MoveCmd cmd) {
  Serial.printf("MOVE for max. %d!\n", cmd.duration);
  Serial.printf("Speed: %d, Steer: %d\n", cmd.speed, cmd.steer);
  balancer.setTarget(cmd.speed, cmd.steer);
  // TODO move
}

void ledCommandHandler(LedCmd cmd) {
  // TODO change led colors (low priority)
}

void loop() {
  static uint32_t longestTickTime = 0;
  uint32_t start = micros();
  robota.tick(); // This iterates through all modules and executes their tick() function
  if (micros() - start > longestTickTime) {
    longestTickTime = micros() - start;
    Serial.printf("NEW LONGEST TICK: %d\n", longestTickTime);
  }

  // Send telemetry
  if (controls.isConnected() && robota.getTicks() % 10000 == 0) {
    //TODO
  }


  //if (controls.isConnected()) {
  //  controls.send(&msg);
  //}
}