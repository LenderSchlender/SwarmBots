#include "WiFiType.h"
#include "WiFi.h"
#include "Interfaces.h"
#include "Robota.h"

WiFiConnection::WiFiConnection(char *hostname, char *ssid, char *password) {
  this->hostname = hostname;
  this->ssid = ssid;
  this->password = password;
}

void WiFiConnection::attemptConnection() {
  lastConnectionAttempt = millis();
  int i_strongest = -1;
  int32_t rssi_strongest = -100;
  Serial.printf("Start scanning for SSID %s\r\n", ssid);

  int n = WiFi.scanNetworks();  // WiFi.scanNetworks will return the number of networks found
  Serial.println("Scan done.");

  if (n == 0) {
    Serial.println("No networks found!");
    return;
  } else {
    Serial.printf("%d networks found:", n);
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.printf("%d: BSSID: %s  %2ddBm, %3d%%  %9s  %s\r\n", i, WiFi.BSSIDstr(i).c_str(), WiFi.RSSI(i), constrain(2 * (WiFi.RSSI(i) + 100), 0, 100), (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "open" : "encrypted", WiFi.SSID(i).c_str());
      if ((String(ssid) == String(WiFi.SSID(i)) && (WiFi.RSSI(i)) > rssi_strongest)) {
        rssi_strongest = WiFi.RSSI(i);
        i_strongest = i;
      }
    }
  }

  if (i_strongest < 0) {
    Serial.printf("No network with SSID %s found!\r\n", ssid);
    return;
  }

  Serial.printf("SSID match found at %d. Connecting...\r\n", i_strongest);
  connecting = true;
  if (WiFi.status() != WL_DISCONNECTED) {
    WiFi.disconnect();
  }
  WiFi.begin(ssid, password, 0, WiFi.BSSID(i_strongest));
  WiFi.scanDelete();  // Free up memory space
}

void WiFiConnection::init() {
  WiFi.setHostname(hostname);
  WiFi.useStaticBuffers(true);
  WiFi.setAutoReconnect(false);
  attemptConnection();
}

// TODO maybe move this to event handlers
// as per https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/wifi.html#onevent-and-removeevent
void WiFiConnection::tick() {
  if (robota->getTicks() % 10000 != 0) {
    return;
  }

  if (connecting) {
    if (WiFi.status() == WL_CONNECTED) {
      connecting = false;
      Serial.printf("Connected! IP Address: %s", WiFi.localIP().toString());
    }
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    attemptConnection();
  }
}



void (*WebSocketControls::eventHandler)(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) = [](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {};

WebSocketControls::WebSocketControls()
  : server(80), ws("/ws") {}

void WebSocketControls::init() {
  ws.onEvent(_onWsEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Please connect to /ws for control");
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Not Found :(");
  });

  server.begin();
}

void WebSocketControls::tick() {
  if (robota->getTicks() % 10000 == 0) {  // Only do this every now and then
    ws.cleanupClients();
  }
}

void WebSocketControls::setEventHandler(void (*eventHandler)(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)) {
  WebSocketControls::eventHandler = eventHandler;
}

void WebSocketControls::send(uint8_t *message, size_t len) {
  ws.binaryAll(message, len);
}

void WebSocketControls::sendText(const char *message) {
  ws.textAll(message);
}

void WebSocketControls::sendText(char *message) {
  ws.textAll(message);
}

void WebSocketControls::_onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    if (server->count() >= 2) {
      client->close(1013, "Someone is already controlling this Robot!");
    }
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
  } else if (type == WS_EVT_ERROR) {
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
  } else if (type == WS_EVT_PONG) {
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
    return;  // Do not forward pong events to the event handler
  } else if (type == WS_EVT_DATA) {
    // do nothing (for now?)
  }
  eventHandler(server, client, type, arg, data, len);
}

bool WebSocketControls::isConnected() {
  return ws.count() != 0;
}

bool WebSocketControls::availableForWrite() {
  return ws.availableForWriteAll();
}