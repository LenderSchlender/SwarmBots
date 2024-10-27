#include <Arduino.h>
#include <Robota.h>
#include <sensors/Sensors.h>
#include <actors/Actors.h>
#include <Interfaces.h>
#include <Interfaces.cpp>

// Following two macros are just placeholders, in reality they're externally defined in the .env file
#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif

#ifndef WIFI_PASS
#define WIFI_PASS ""
#endif

Robota robota;

LD20Sensor lidar(&Serial2);

// Left Motor controls
PwmOutput leftMotorEnable(14, 2000);
GenericDigitalOutput leftMotorForward(26);
GenericDigitalOutput leftMotorBackward(27);
SingleMotorLN298NOutput leftMotor(&leftMotorEnable, &leftMotorForward, &leftMotorBackward);
// Right Motor controls TODO set pins
PwmOutput rightMotorEnable(32, 2000);
GenericDigitalOutput rightMotorForward(25);
GenericDigitalOutput rightMotorBackward(33);
SingleMotorLN298NOutput rightMotor(&rightMotorEnable, &rightMotorForward, &rightMotorBackward);

WiFiConnection wifi("sb-guide", WIFI_SSID, WIFI_PASS);
// Remote Control Interface
WebSocketControls controls;

void lidarMeasurementHandler(SingleLiDARMeasurement measurement);
void moveCommandHandler(MoveCmd cmd);
void ledCommandHandler(LedCmd cmd);

void setup() {
  Serial.begin(115200);
  lidar.setCallback(lidarMeasurementHandler);
  robota.addModule(&lidar);

  // Left Motor
  robota.addModule(&leftMotorEnable);
  robota.addModule(&leftMotorForward);
  robota.addModule(&leftMotorBackward);
  robota.addModule(&leftMotor);
  // Right motor
  robota.addModule(&rightMotorEnable);
  robota.addModule(&rightMotorForward);
  robota.addModule(&rightMotorBackward);
  robota.addModule(&rightMotor);

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

void websocketEventHandler(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
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
  //TODO new lidar measurement handler system
  // featuring batching of measurements and sending them as protobufs
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
  //TODO move
}

void ledCommandHandler(LedCmd cmd) {
  //TODO change led colors (low priority)
}

void loop() {
  static uint32_t longestTickTime = 0;
  uint32_t start = micros();
  robota.tick();  // This iterates through all modules and executes their tick() function
  if (micros() - start > longestTickTime) {
    longestTickTime = micros() - start;
    Serial.printf("NEW LONGEST TICK: %d\n", longestTickTime);
  }

  delay(1000);
}