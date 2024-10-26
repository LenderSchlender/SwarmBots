#include <Arduino.h>
#include "Module.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "Protocol.h"

#ifndef Interfaces_h
#define Interfaces_h

//TODO wait until https://github.com/espressif/arduino-esp32/issues/7921 (wifi roaming) gets implemented
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

// This class can only be instantiated  one time!
class WebSocketControls : public Interface {
  AsyncWebServer server;
  AsyncWebSocket ws;
  static void (*eventHandler)(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
  static void _onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
  static void _handleReceivedData(AsyncWebSocketClient *client, uint8_t *data, size_t len);
public:
  WebSocketControls(/* TODO add options for port and path */);
  void init();
  void tick();
  void send(uint8_t *message, size_t len);
  void sendText(const char *message);const 
  void sendText(char *message);
  //TODO implement
  void setMoveCmdHandler(void (*handler)(int32_t seq, MoveCmd cmd));
  //TODO implement
  void setLedCmdHandler(void (*handler)(int32_t seq, LedCmd cmd));
  // deprecated // TODO remove soon
  void setEventHandler(void (*eventHandler)(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len));
  bool isConnected();
  bool availableForWrite();
};

#endif