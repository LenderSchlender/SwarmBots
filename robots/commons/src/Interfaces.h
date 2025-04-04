#include "Module.h"
#include "Protocol.h"
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

#ifndef Interfaces_h
#define Interfaces_h

// TODO wait until https://github.com/espressif/arduino-esp32/issues/7921 (wifi roaming) gets implemented
class WiFiConnection : public Interface {
  uint32_t lastConnectionAttempt;
  bool connecting;
  char hostname[64], ssid[64], password[64];
  void attemptConnection();

public:
  WiFiConnection(const char *hostname, const char *ssid, const char *password);
  void init();
  void tick();
};

// This class should only be instantiated once!
class WebSocketControls : public Interface {
  AsyncWebServer server;
  AsyncWebSocket ws;
  void (*moveCmdHandler)(MoveCmd cmd) = [](MoveCmd cmd) {Serial.println("WARN: Default moveCmdHandler!");};
  void (*ledCmdHandler)(LedCmd cmd) = [](LedCmd cmd) {Serial.println("WARN: Default ledCmdHandler!");};
  void _onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
  void _handleReceivedMessage(AsyncWebSocketClient *client, Wrapper msg);

public:
  WebSocketControls(/* TODO add options for port and path */);
  void init();
  void tick();
  void send(uint8_t *message, size_t len);
  void send(Wrapper *message);
  void setMoveCmdHandler(void (*handler)(MoveCmd cmd));
  void setLedCmdHandler(void (*handler)(LedCmd cmd));
  bool isConnected();
  bool availableForWrite();
  uint32_t seq();
  
  void sendEncoderData(
    int32_t pulses,
    uint32_t duration
  );
  void sendImuData(
    int32_t acceleration_x, int32_t acceleration_y, int32_t acceleration_z,
    int32_t rotation_x, int32_t rotation_y, int32_t rotation_z,
    int32_t temperature
  );
  //TODO sendLidarData maybe?
};

#endif