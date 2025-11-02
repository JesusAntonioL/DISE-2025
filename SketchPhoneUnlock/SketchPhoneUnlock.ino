#include <WiFi.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "Nintendo Switch de Chuy";
const char* password = "tacos2x1";

const char* authUser = "admin";
const char* authPass = "1234";

#define LOCK_PIN 4

AsyncWebServer server(80);

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

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  pinMode(LOCK_PIN, OUTPUT);
  digitalWrite(LOCK_PIN, LOW);  // default locked

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  // Dashboard
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!request->authenticate(authUser, authPass))
      return request->requestAuthentication();
    request->send(200, "text/html", getDashboardHTML());
  });

  // Lock
  server.on("/lock", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!request->authenticate(authUser, authPass))
      return request->requestAuthentication();

    digitalWrite(LOCK_PIN, LOW);
    request->send(200, "text/plain", "🔒 Locked");
  });

  // Unlock
  server.on("/unlock", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!request->authenticate(authUser, authPass))
      return request->requestAuthentication();

    digitalWrite(LOCK_PIN, HIGH);
    request->send(200, "text/plain", "🔓 Unlocked");
  });


  server.begin();
}

void loop() {}
