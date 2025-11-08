#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <mcp_can.h>
#include <SPI.h>

// ---------- WiFi Configuration ----------
//const char* ssid = ""; //Add WIFI Network Name
//const char* password = ""; //Add WIFI Network Password
const char* authUser = "admin";
const char* authPass = "1234";

// ---------- Lock Control ----------
#define LOCK_PIN 4

// ---------- CAN Configuration ----------
#define CAN0_INT 5
#define CAN0_CS  2
MCP_CAN CAN0(CAN0_CS);

const unsigned long CAN_SEND_INTERVAL = 1000; // periodic send (ms)
volatile unsigned long lastCanSend = 0;
bool lockStatus = 1; // 1 = locked, 0 = unlocked

// ---------- Web Server ----------
AsyncWebServer server(80);

// ---------- Dashboard HTML (unchanged) ----------
String getDashboardHTML() {
  return R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Lock Control</title>
    <style>
      body { font-family: Arial; text-align:center; background:#f2f2f2; }
      button { padding:15px 30px; margin:10px; font-size:20px; border:none; border-radius:10px; cursor:pointer; }
      .lock { background-color:#e74c3c; color:white; }
      .unlock { background-color:#2ecc71; color:white; }
    </style>
  </head>
  <body>
    <h2>ESP32 Lock Dashboard</h2>
    <button class="lock" onclick="sendCommand('lock')">Lock</button>
    <button class="unlock" onclick="sendCommand('unlock')">Unlock</button>
    <br>
    <button onclick="window.location.href='/logout'">Logout</button>
    <p id="status"></p>
    <script>
      function sendCommand(cmd){
        fetch('/'+cmd)
          .then(r => r.text())
          .then(t => document.getElementById('status').innerText = t)
          .catch(e => console.error(e));
      }
    </script>
  </body>
  </html>
  )rawliteral";
}

// ---------- CAN Send Function ----------
void sendCANMessage() {
  // 8-byte data array, first byte = lock status
  byte data[1] = {lockStatus};

  byte sndStat = CAN0.sendMsgBuf(0x200, 0, 1, data);
  if (sndStat == CAN_OK) {
    Serial.print("CAN Sent | Lock status: ");
    Serial.println(lockStatus ? "LOCKED" : "UNLOCKED");
  } else {
    Serial.println("Error Sending CAN Message");
  }
}

// ---------- FreeRTOS Task for Periodic CAN ----------
void canTask(void *pvParameters) {
  (void) pvParameters;
  while (true) {
    unsigned long now = millis();
    if (now - lastCanSend >= CAN_SEND_INTERVAL) {
      sendCANMessage();
      lastCanSend = now;
    }
    vTaskDelay(pdMS_TO_TICKS(50)); // yield
  }
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  pinMode(LOCK_PIN, OUTPUT);
  digitalWrite(LOCK_PIN, LOW); // start locked

  // ---- CAN Setup ----
  SPI.begin();
  Serial.println("Initializing MCP2515...");
  if (CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");
  CAN0.setMode(MCP_NORMAL);

  // ---- Start CAN Task ----
  xTaskCreatePinnedToCore(canTask, "CAN_Task", 4096, NULL, 1, NULL, 1);

  // ---- WiFi Setup ----
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());

  // ---- Web Endpoints ----
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!request->authenticate(authUser, authPass))
      return request->requestAuthentication();
    request->send(200, "text/html", getDashboardHTML());
  });

  server.on("/lock", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!request->authenticate(authUser, authPass))
      return request->requestAuthentication();

    lockStatus = 1; // update global state
    sendCANMessage(); // send immediate update
    request->send(200, "text/plain", "🔒 Locked");
  });

  server.on("/unlock", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!request->authenticate(authUser, authPass))
      return request->requestAuthentication();

    lockStatus = 0; // update global state
    sendCANMessage(); // send immediate update
    request->send(200, "text/plain", "🔓 Unlocked");
  });

  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(401, "text/plain", "Logged out");
  });

  server.begin();
}

void loop() {
  // nothing here; handled by AsyncWebServer and CAN task
}