#include "webServerFunctions.h"

void setup(){
  connectToWifi();
  ledInitialization();
  initWebSocket();
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", html, processor);
  });
  server.begin();
}

void loop() {
  ws.cleanupClients();
  ledcWrite(ledChannel, ledValue);
  readPot();
}

