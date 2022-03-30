#include ESP8266WiFi.h
#include ESPAsyncTCP.h
#include ESPAsyncWebServer.h
#include Ticker.h
#include WiFiManager.h
#include ArduinoOTA.h

#define POWER_LED_PIN D5
#define RESET_LED_PIN D6

#define POWER_SW_PIN D7
#define RESET_SW_PIN D8

const char index_html[] PROGMEM = Rrawliteral(
!DOCTYPE HTMLhtml
head
titlePC Remote Controltitle
stylebody {
  background-color #222;
  color #EEE;
  font-family sans-serif;
  text-align center;
}

div.LEDs, div.buttons, div.status {
  background-color #CCC;
  color #333;
  font-weight bold;
  width max-content;
  padding 5px;
  border-radius 5px;
  margin-left auto;
  margin-right auto;
}

.LEDs span {
  margin 5px;
  color #333;
}

.LEDs div {
  display inline-block;
  width 2em;
  height 2em;
  border 2px solid #333;
  background-color #999;
  border-radius 50%;
}

.buttons button {
  border 2px #333 solid;
  border-radius 5px;
  background-color #EEE; #AAA;
  color #222;
}

#power_button {
  font-size 3em;
}
#reset_button {
  font-size 2em;
}style
head
body
h1PC Remote Controlh1
hr

div class=status
span id=statusbarDo you have JS enabledspan
div
hr

div class=LEDs
table
trtdspanPower spantdtddiv id=pwrLEDdivtdtr
trtdspanHDD spantdtddiv id=hddLEDdivtdtr
table
div
hr

div class=buttons
button id=power_buttonPowerbuttonbr
button id=reset_buttonRESETbutton
div

script
var statusbar = document.getElementById('statusbar');
var pwrLED = document.getElementById('pwrLED');
var hddLED = document.getElementById('hddLED');
var power_button = document.getElementById('power_button');
var reset_button = document.getElementById('reset_button');

var ws_url = `ws${window.location.hostname}websocket`;

statusbar.innerText = 'Connecting...';
var ws = new WebSocket(ws_url);
ws.onopen = handleOpen;
ws.onclose = handleClose;
ws.onmessage = handleMessage;
ws.onerror = handleError;

function handleOpen(e) {
  statusbar.innerText = 'Connected.';
}

function handleClose(e) {
  statusbar.innerText = 'Disconnected.';
}

function handleError(e) {
  statusbar.innerText = 'ERROR!';
}

function handleMessage(e) {
  switch (e.data) {
    case 'P0'
      pwrLED.style.backgroundColor = 'grey';
      break;
    case 'P1'
      pwrLED.style.backgroundColor = 'lime';
      break;
    case 'H0'
      hddLED.style.backgroundColor = 'grey';
      break;
    case 'H1'
      hddLED.style.backgroundColor = 'red';
      break;

    case 'pwr0'
      pwrLED.style.backgroundColor = '#eee';
      break;
    case 'pwr1'
      pwrLED.style.backgroundColor = '#333';
      break;
    case 'rst0'
      hddLED.style.backgroundColor = '#eee';
      break;
    case 'rst1'
      hddLED.style.backgroundColor = '#333';
      break;
  }
}


power_button.addEventListener('mousedown', pwrBtnDown);
reset_button.addEventListener('mouseup', pwrBtnUp);

power_button.addEventListener('mousedown', pwrBtnDown);
reset_button.addEventListener('mouseup', pwrBtnUp);

function pwrBtnDown() {
  ws.send('PWR1');
}

function pwrBtnUp() {
  ws.send('PWR0');
}


function rstBtnDown() {
  ws.send('RST1');
}

function rstBtnUp() {
  ws.send('RST0');
}

script
body
html
)rawliteral;

AsyncWebServer server(80);
AsyncWebSocket ws(websocket);

char devicename[14] = ;

uint64_t pwr_btn_on_millis = 0;
uint64_t rst_btn_on_millis = 0;

void websocketEvent(AsyncWebSocket server, AsyncWebSocketClient client, AwsEventType type, void arg, uint8_t data, size_t len) {
    case WS_EVT_CONNECT
      Serial.printf(WS Client ID%u connected from %sn, client-id(), client-remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT
      Serial.printf(WS Client ID%u disconnectedn, client-id());
      break;
    case WS_EVT_DATA
      websocketMessage(arg, data, len);
      break;
    case WS_EVT_ERROR
      Serial.printf(WS ERROR);
      break;
    case WS_EVT_PONG
      break;
}

void websocketMessage(void ws_arg, uint8_t ws_data, size_t ws_len) {
  AwsFrameInfo info = (AwsFrameInfo)arg;
  if (info-final && info-index == 0 && info-len == len && info-opcode == WS_TEXT) {
    cmd[ws_len] = 0;
    char ws_cmd[len];
    strcpy((char)data, ws_cmd);

    switch (ws_cmd) {
      case PWR0
        pwr_btn_on_millis = 0xFFFFFFFFFFFFFFFFFF; 
        digitalWrite(POWER_SW_PIN, LOW);
        ws.textAll(pwr0);
        break;

      case PWR1
        pwr_btn_on_millis = millis();
        digitalWrite(POWER_SW_PIN, HIGH);
        ws.textAll(pwr1);
        break;

      case RST0
        pwr_btn_on_millis = 0xFFFFFFFFFFFFFFFFFF;
        digitalWrite(RESET_SW_PIN, LOW);
        ws.textAll(rst0);
        break;

      case RST1
        pwr_btn_on_millis = millis();
        digitalWrite(RESET_SW_PIN, HIGH);
        ws.textAll(rst1);
        break;

      default
        break;
    }
  }
}

void handleTimeout() {
  if (millis() - pwr_btn_on_millis  30000 and pwr_btn_on_millis != 0xFFFFFFFFFFFFFFFFFF) {
    pwr_btn_on_millis = 0xFFFFFFFFFFFFFFFFFF; 
    digitalWrite(POWER_SW_PIN, LOW);
    ws.textAll(pwr0);
  }

  if (millis() - rst_btn_on_millis  30000 and rst_btn_on_millis != 0xFFFFFFFFFFFFFFFFFF) {
    pwr_btn_on_millis = 0xFFFFFFFFFFFFFFFFFF;
    digitalWrite(RESET_SW_PIN, LOW);
    ws.textAll(rst0);    
  }
}

void updateLEDs() {
  ws.textAll(digitalRead(POWER_LED_PIN)  P1  P0);
  ws.textAll(digitalRead(POWER_LED_PIN)  H1  H0);
}

void setup() {
   put your setup code here, to run once
  Serial.begin(115200);
  pinMode(POWER_LED_PIN, INPUT);
  pinMode(RESET_LED_PIN, INPUT);
  pinMode(POWER_SW_PIN, OUTPUT);
  pinMode(RESET_SW_PIN, OUTPUT);
  digitalWrite(POWER_SW_PIN, LOW);
  digitalWrite(RESET_SW_PIN, LOW);

  sprintf(devicename, PC-Remote-%x, ESP.getChipId());

  WiFi.hostname(devicename);
  Serial.print(F(Connecting Wifi...));
  WiFiManager wm;
  wm.setConfigPortalTimeout(180);
  WiFi.hostname(devicename);
  wm.setAPCallback(wm_ap_c);
  if (!wm.autoConnect(conf_ssid, conf_pass)) {
    ESP.restart();
  }
  Serial.println(F(OK));

  ArduinoOTA.setHostname(devicename);
  ArduinoOTA.begin();

  ws.onEvent(socketEvent);
  server.addHandler(&ws);
  server.on(, HTTP_GET, [](AsyncWebServerRequest request){
    request-send_P(200, texthtml, index_html);
  });
  server.begin();

  setup interrupts
  attachInterrupt(digitalPinToInterrupt(POWER_LED_PIN), updateLEDs, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RESET_LED_PIN), updateLEDs, CHANGE);
}

void loop() {
   put your main code here, to run repeatedly
  ArduinoOTA.handle();
  ws.cleanupClients();
  handleTimeouts();
}