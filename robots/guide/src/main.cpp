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

// how many LiDAR measurements to send in one 
// it's best if this is a multiple of POINT_PER_PACK (12)
// but it can be any value
const int LIDAR_BATCH_SIZE = 12*4;
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
  delay(1500);
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

// encodes an array of LIDAR_BATCH_SIZE measurements
bool encode_lidar_batch(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {    
    for (int i = 0; i < LIDAR_BATCH_SIZE; i++) {
      if (!pb_encode_tag_for_field(stream, field))
          return false;

      if (!pb_encode_submessage(stream, at_htlw10_swarmbots_LidarData_LidarMeasurement_fields,
        *arg + (i * sizeof(LidarData_LidarMeasurement))))
          return false;
    }
    return true;
}

void lidarMeasurementHandler(SingleLiDARMeasurement measurement) {
  // TODO new lidar measurement handler system
  //  featuring batching of measurements and sending them as protobufs
  // Tx buffer
  static LidarData_LidarMeasurement buffer[LIDAR_BATCH_SIZE];
  static uint8_t count = 0;
  // fill measurement data
  buffer[count] = LidarData_LidarMeasurement();
  buffer[count].angle = measurement.angle;
  buffer[count].distance = measurement.distance;
  buffer[count].intensity = measurement.intensity;
  // increase counter and rollover once it reaches LIDAR_BATCH_SIZE
  count = (count + 1) % LIDAR_BATCH_SIZE;

  if (!controls.isConnected())
    return;

  if (count == 0 && controls.availableForWrite()) {
    Wrapper msg = Wrapper_init_zero;
    msg.seq = controls.seq();
    msg.which_message = at_htlw10_swarmbots_Wrapper_lidar_data_tag;
    msg.message.lidar_data.measurements.funcs.encode = &encode_lidar_batch;
    msg.message.lidar_data.measurements.arg = buffer;
    // Send
    controls.send(&msg);
  }
}

void moveCommandHandler(MoveCmd cmd) {
  Serial.printf("Move for max. %d!\n", cmd.duration);
  Serial.printf("Speed: %d, Steer: %d\n", cmd.speed, cmd.steer);
  //balancer.setTarget(cmd.speed, cmd.steer);
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
}