#include "Interfaces.h"
#include "Robota.h"
#include "WiFi.h"
#include "WiFiType.h"

// uncomment this to see websocket messages:
//#define DEBUG_WEBSOCKET

WiFiConnection::WiFiConnection(const char *hostname, const char *ssid, const char *password) {
  strncpy(this->hostname, hostname, sizeof(this->hostname));
  strncpy(this->ssid, ssid, sizeof(this->ssid));
  strncpy(this->password, password, sizeof(this->password));
}

void WiFiConnection::attemptConnection() {
  lastConnectionAttempt = millis();
  int i_strongest = -1;
  int32_t rssi_strongest = -100;
  Serial.printf("Start scanning for SSID %s\r\n", ssid);

  int n = WiFi.scanNetworks(); // WiFi.scanNetworks will return the number of networks found
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
  WiFi.scanDelete(); // Free up memory space
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
      Serial.printf("Connected! IP Address: %s\n", WiFi.localIP().toString());
      return;
    }
    Serial.print(WiFi.status());
    Serial.print('.');
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    attemptConnection();
  }
}

WebSocketControls::WebSocketControls()
    : server(80), ws("/ws") {}

void WebSocketControls::init() {
  // Listen to ws events
  ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client,
                    AwsEventType type, void *arg, uint8_t *data, size_t len) {
    _onWsEvent(server, client, type, arg, data, len);
  });

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
  if (robota->getTicks() % 1000000 == 0) { // Only do this every now and then
    ws.cleanupClients();
  }
}

void WebSocketControls::setMoveCmdHandler(void (*handler)(MoveCmd cmd)) {
  this->moveCmdHandler = handler;
}

void WebSocketControls::setLedCmdHandler(void (*handler)(LedCmd cmd)) {
  this->ledCmdHandler = handler;
}

void WebSocketControls::send(uint8_t *message, size_t len) {
  ws.binaryAll(message, len);
}

void WebSocketControls::send(Wrapper *message) {
  uint8_t buffer[2048];
  // prepare the stream for writing to the ws send buffer
  pb_ostream_t writeStream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  // Write the message to the buffer
  pb_encode(&writeStream, at_htlw10_swarmbots_Wrapper_fields, message);
  // Send the buffer to all connected clients
#ifdef DEBUG_WEBSOCKET
  Serial.print("Tx:");
  for (int i = 0; i<writeStream.bytes_written; i++) {
    Serial.printf("%02x", buffer[i]);
  }
  Serial.println();
#endif
  
  AsyncWebSocketMessageBuffer *wsBuffer = ws.makeBuffer(writeStream.bytes_written);

  if (!wsBuffer) {
    Serial.println("ERROR: Couldn't create websocket send buffer!");
    return;
  }

  memcpy(wsBuffer->get(), buffer, writeStream.bytes_written);
  ws.binaryAll(wsBuffer);
}

void WebSocketControls::_onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  // Prepare an empty message wrapper
  Wrapper msg = Wrapper_init_zero;
  // Prepare an input stream from the provided data
  pb_istream_t stream = pb_istream_from_buffer(data, len);
  switch (type) {
  case WS_EVT_CONNECT:
    if (server->count() >= 2) {
      client->close(1013, "Someone is already controlling this Robot!");
      return;
    }
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
    break;
  case WS_EVT_ERROR:
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(),
                  *((uint16_t *)arg), (char *)data);
    break;
  case WS_EVT_PONG:
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(),
                  len, (len) ? (char *)data : "");
    break;
  case WS_EVT_DATA:
    // Decode the stream into the prepared wrapper object
    if (pb_decode(&stream, at_htlw10_swarmbots_Wrapper_fields, &msg)) {
#ifdef DEBUG_WEBSOCKET
      Serial.print("Rx: ");
      for (int i = 0; i < len; i++) {
        Serial.printf("%02x", data[i]);
      }
      Serial.println();
#endif
      _handleReceivedMessage(client, msg);
    } else {
      Serial.println("ERROR: Received invalid data!");
    }
    break;
  default:
    Serial.printf("Unrecognised WS event type %d!", type);
    return; // Unrecognised event type, should never happen anyways
  }
}

// This gets called for websocket events of type WS_EVT_DATA with a valid message wrapper inside
void WebSocketControls::_handleReceivedMessage(AsyncWebSocketClient *client, Wrapper msg) {
  static uint32_t latestSeq = 0;
  if (msg.seq <= latestSeq) {
    Serial.println("WARN: Messages are out of order!");
    return;
  }

  // ping pong response
  Wrapper response = Wrapper_init_zero;

  switch (msg.which_message) {
  case at_htlw10_swarmbots_Wrapper_ping_pong_tag:
    response.which_message = at_htlw10_swarmbots_Wrapper_ping_pong_tag;
    response.message.ping_pong = at_htlw10_swarmbots_Wrapper_PingPong_PONG;
    send(&response);
    break;
  case at_htlw10_swarmbots_Wrapper_move_cmd_tag:
    if (moveCmdHandler) {
      moveCmdHandler(msg.message.move_cmd);
    } else {
      Serial.println("WARN: No moveCmdHandler!");
    }
    break;
  case at_htlw10_swarmbots_Wrapper_led_cmd_tag:
    if (ledCmdHandler) {
      ledCmdHandler(msg.message.led_cmd);
    } else {
      Serial.println("WARN: No ledCmdHandler!");
    }
    break;
  default:
    Serial.print("ERROR: Invalid Protobuf message type: ");
    Serial.println(msg.which_message);
    return;
  }
}

bool WebSocketControls::isConnected() {
  return ws.count() != 0;
}

bool WebSocketControls::availableForWrite() {
  return ws.availableForWriteAll();
}

uint32_t WebSocketControls::seq() {
  static uint32_t s = 0;
  return s++;
}

void WebSocketControls::sendEncoderData(
  int32_t pulses,
  uint32_t duration
) {
  Wrapper msg = Wrapper_init_zero;
  msg.seq = seq();
  msg.which_message = at_htlw10_swarmbots_Wrapper_encoder_data_tag;
  msg.message.encoder_data.pulses = pulses;
  msg.message.encoder_data.duration = duration;
  // Send
  send(&msg);
}

void WebSocketControls::sendImuData(
  int32_t acceleration_x, int32_t acceleration_y, int32_t acceleration_z,
  int32_t rotation_x, int32_t rotation_y, int32_t rotation_z,
  int32_t temperature
) {
  Wrapper msg = Wrapper_init_zero;
  msg.seq = seq();
  msg.which_message = at_htlw10_swarmbots_Wrapper_imu_data_tag;
  // Set acceleration (accelerometer)
  msg.message.imu_data.acceleration_x = acceleration_x;
  msg.message.imu_data.acceleration_y = acceleration_y;
  msg.message.imu_data.acceleration_z = acceleration_z;
  // Set rotation (gyroscope)
  msg.message.imu_data.rotation_x = rotation_x;
  msg.message.imu_data.rotation_y = rotation_y;
  msg.message.imu_data.rotation_z = rotation_z;
  // Set temperature
  msg.message.imu_data.temperature = temperature;
  // Send
  send(&msg);
}