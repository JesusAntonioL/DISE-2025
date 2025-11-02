// Merged: SketchPhoneUnlock + ESP32 CAN sender (MCP2515)
// Make sure MCP_CAN library (MCP_CAN.h) is installed.

#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#include <mcp_can.h>
#include <SPI.h>

// ---------- WiFi / Web server (from SketchPhoneUnlock) ----------
const char* ssid = ""; //Add WIFI Network Name
const char* password = ""; //Add WIFI Network Password
const char* authUser = "admin";
const char* authPass = "1234";

#define LOCK_PIN 4

AsyncWebServer server(80);

String getDashboardHTML() {
  return R"rawliteral(
  <!DOCTYPE html>
  <html>
    <head><meta name="viewport" content="width=device-width, initial-scale=1"></head>
    <body>
      <h1>Unlock</h1>
      <button onclick="fetch('/unlock')">Unlock</button>
    </body>
  </html>
  )rawliteral";
}

// ---------- CAN (from ESP32_CAN_Test_Code.ino) ----------
#define CAN0_INT 5   // interrupt pin from MCP2515 to ESP32
#define CAN0_CS  2   // CS pin for MCP2515 (change if needed)

MCP_CAN CAN0(CAN0_CS);  // Set CS pin

// CAN send interval (ms)
const unsigned long CAN_SEND_INTERVAL = 1000;

// ---------- Globals for CAN task ----------
volatile unsigned long lastCanSend = 0;

// ---------- CAN sending function ----------
void sendCANMessage() {
  byte data[8] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
  byte sndStat = CAN0.sendMsgBuf(0x200, 0, 8, data); // 0x200 ID, 0 = standard frame
  if (sndStat == CAN_OK) {
    Serial.println("CAN Message Sent Successfully!");
  } else {
    Serial.print("CAN Send Error: ");
    Serial.println(sndStat);
  }
}

// ---------- FreeRTOS task for CAN sending ----------
void canTask(void *pvParameters) {
  (void) pvParameters;
  while (true) {
    unsigned long now = millis();
    if (now - lastCanSend >= CAN_SEND_INTERVAL) {
      sendCANMessage();
      lastCanSend = now;
    }
    // small delay so the task yields CPU
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);

  // pins
  pinMode(LOCK_PIN, OUTPUT);
  digitalWrite(LOCK_PIN, LOW); // locked default

  // ---- Initialize SPI and MCP2515 ----
  SPI.begin(); // can also pass SCLK, MISO, MOSI pins if you reassign
  Serial.println("Initializing CAN...");
  if (CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
    Serial.println("MCP2515 Initialized Successfully!");
  } else {
    Serial.println("Error Initializing MCP2515...");
    // keep going — you may want to handle recover or halt here
  }
  CAN0.setMode(MCP_NORMAL); // normal operation

  // Optional: attach interrupt for receive (not used in this example)
  // pinMode(CAN0_INT, INPUT);

  // ---- Start the CAN task ----
  // Create task pinned to core 1 or 0 as you prefer
  xTaskCreatePinnedToCore(
    canTask,            // task function
    "CAN_Task",         // name
    4096,               // stack size (bytes)
    NULL,               // param
    1,                  // priority
    NULL,               // handle
    1                   // core 1
  );

  // ---- Start WiFi and server (original unlock server) ----
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 20000) {
    delay(200);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected! IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi not connected (timeout).");
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", getDashboardHTML());
  });

  // Unlock endpoint remains the same
  server.on("/unlock", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!request->authenticate(authUser, authPass))
      return request->requestAuthentication();

    Serial.println("Xdd");
    request->send(200, "text/plain", "🔓 Unlocked");
    // optionally reset the lock after a short time:
    // delay is blocking - avoid here. Could schedule a timer or set millis-based logic.
  });

  server.begin();
}

// main loop must remain non-blocking for AsyncWebServer
void loop() {
  // keep empty or use it for very short non-blocking tasks
}