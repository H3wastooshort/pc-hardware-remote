#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
//#include <WiFiManager.h> //fucky with asyncwebserver
#include <ArduinoOTA.h>

#define WIFI_SSID "1234"
#define WIFI_PASS "1234"

#define POWER_LED_PIN D5
#define DISK_LED_PIN D6

#define POWER_SW_PIN D7
#define RESET_SW_PIN D8

AsyncWebServer server(80);
AsyncWebSocket ws("/websocket");

char devicename[14] = "";

uint64_t pwr_btn_on_millis = 0xFFFFFFFFFFFFFFFF;
uint64_t rst_btn_on_millis = 0xFFFFFFFFFFFFFFFF;

void websocketEvent(AsyncWebSocket *ws_server, AsyncWebSocketClient *ws_client, AwsEventType ws_type, void *ws_arg, uint8_t *ws_data, size_t ws_len) {
  digitalWrite(LED_BUILTIN, LOW);
  switch (ws_type) {
    case WS_EVT_CONNECT:
      Serial.printf("WS Client ID%u connected from %s\n", ws_client->id(), ws_client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WS Client ID%u disconnected\n", ws_client->id());
      break;
    case WS_EVT_DATA:
      websocketMessage(ws_arg, ws_data, ws_len);
      break;
    case WS_EVT_ERROR:
      Serial.printf("WS ERROR");
      break;
    case WS_EVT_PONG:
      break;
  }
  digitalWrite(LED_BUILTIN, HIGH);
}

void websocketMessage(void *ws_arg, uint8_t *ws_data, size_t ws_len) {
  AwsFrameInfo *ws_info = (AwsFrameInfo*)ws_arg;
  if (ws_info->final && ws_info->index == 0 && ws_info->len == ws_len && ws_info->opcode == WS_TEXT) {
    ws_data[ws_len] = 0;

    if (strcmp((char*)ws_data, "PWR0") == 0) {
      pwr_btn_on_millis = 0xFFFFFFFFFFFFFFFF;
      digitalWrite(POWER_SW_PIN, LOW);
      ws.textAll("pwr0");
    }

    if (strcmp((char*)ws_data, "PWR1") == 0) {
      pwr_btn_on_millis = millis();
      digitalWrite(POWER_SW_PIN, HIGH);
      ws.textAll("pwr1");
    }

    if (strcmp((char*)ws_data, "RST0") == 0) {
      rst_btn_on_millis = 0xFFFFFFFFFFFFFFFF;
      digitalWrite(RESET_SW_PIN, LOW);
      ws.textAll("rst0");
    }

    if (strcmp((char*)ws_data, "RST1") == 0) {
      rst_btn_on_millis = millis();
      digitalWrite(RESET_SW_PIN, HIGH);
      ws.textAll("rst1");
    }
  }
}

void handleTimeouts() {
  if (millis() - pwr_btn_on_millis > 30000 and pwr_btn_on_millis != 0xFFFFFFFFFFFFFFFF) {
    pwr_btn_on_millis = 0xFFFFFFFFFFFFFFFF;
    digitalWrite(POWER_SW_PIN, LOW);
    ws.textAll("pwr0");
  }

  if (millis() - rst_btn_on_millis > 30000 and rst_btn_on_millis != 0xFFFFFFFFFFFFFFFF) {
    pwr_btn_on_millis = 0xFFFFFFFFFFFFFFFF;
    digitalWrite(RESET_SW_PIN, LOW);
    ws.textAll("rst0");
  }
}

void IRAM_ATTR updateLEDs() {
  digitalWrite(LED_BUILTIN, LOW);
  ws.textAll(digitalRead(POWER_LED_PIN) ? "P1" : "P0");
  ws.textAll(digitalRead(DISK_LED_PIN) ? "H1" : "H0");
  digitalWrite(LED_BUILTIN, HIGH);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(POWER_LED_PIN, INPUT);
  pinMode(DISK_LED_PIN, INPUT);
  pinMode(POWER_SW_PIN, OUTPUT);
  pinMode(RESET_SW_PIN, OUTPUT);
  digitalWrite(POWER_SW_PIN, LOW);
  digitalWrite(RESET_SW_PIN, LOW);

  sprintf(devicename, "PC-Remote-%x", ESP.getChipId());

  WiFi.hostname(devicename);
  Serial.print(F("Connecting Wifi..."));
  /*WiFiManager wm;
    wm.setConfigPortalTimeout(180);
    WiFi.hostname(devicename);
    //wm.setAPCallback(wm_ap_c);
    if (!wm.autoConnect()) {
    ESP.restart();
    }*/
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(F("OK"));

  Serial.print(F("Hostname: "));
  Serial.println(devicename);
  Serial.print(F("IP: "));
  Serial.println(WiFi.localIP());

  Serial.print(F("OTA..."));
  ArduinoOTA.setHostname(devicename);
  ArduinoOTA.begin();
  Serial.println(F("OK"));


  Serial.print(F("webserver..."));
  SPIFFS.begin();
  ws.onEvent(websocketEvent);
  server.addHandler(&ws);
  server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");
  server.begin();
  Serial.println(F("OK"));

  //setup interrupts
  attachInterrupt(digitalPinToInterrupt(POWER_LED_PIN), updateLEDs, CHANGE);
  attachInterrupt(digitalPinToInterrupt(DISK_LED_PIN), updateLEDs, CHANGE);

  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  ArduinoOTA.handle();
  ws.cleanupClients();
  handleTimeouts();
}