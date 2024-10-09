#include <Arduino.h>
#include "Module.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#ifndef Interfaces_h
#define Interfaces_h

//TODO wait until https://github.com/espressif/arduino-esp32/issues/7921 (wifi roaming) gets implemented
class WiFiConnection : public Interface {
  uint32_t lastConnectionAttempt;
  bool connecting;
  char *hostname, *ssid, *password;
  void attemptConnection();
public:
  WiFiConnection(char *hostname, char *ssid, char *password);
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
  void sendText(const char *message);
  void sendText(char *message);
  void setEventHandler(void (*eventHandler)(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len));
  bool isConnected();
  bool availableForWrite();
};

#endif