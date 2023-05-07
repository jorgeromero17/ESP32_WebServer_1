#include "globals.h"

// Función para conectarse a la red Wi-Fi especificada
void connectToWifi(){ //Esta funcion esta basada en el ejemplo: https://todomaker.com/blog/conexion-de-esp32-devkit-v1-a-internet/
  // Inicializar la comunicación serial a 9600 baudios
  Serial.begin(115200);
  // Esperar 
  delay(200);
   // Iniciar la conexión a la red Wi-Fi usando las credenciales especificadas
  WiFi.begin(ssid, password);
  Serial.print("\nConnecting to Wi-Fi.");
  // Esperar y verificar el estado de la conexión Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Mostrar mensaje de conexión exitosa y la dirección IP asignada
  Serial.println("\nWi-Fi connection established");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void ledInitialization() {
  // Configura el canal de LED para utilizar la frecuencia y resolución dadas.
  ledcSetup(ledChannel, freq, resolution);
  // Asocia el pin del LED con el canal de LED.
  ledcAttachPin(ledPin, ledChannel);
  // Escribe el valor inicial 0 en el canal de LED para apagar el LED.
  ledcWrite(ledChannel, 0);
}

String processor(const String& var){ 
  if(var == "VALUE"){
    return String(ledValue);
  }
  return String();
}

void notifyClients(bool isLed) {
  StaticJsonDocument<128> doc;
  if(isLed){ //envia un clave valor, dependiendo del booleano
    doc["ledValue"] = ledValue;
  } else {
    doc["potValue"] = potValue;
  }
  // Convertir el objeto JSON a una cadena de texto
  String jsonStr;
  serializeJson(doc, jsonStr);
  //Enviar json
  ws.textAll(jsonStr);
}

void readPot() {
  // Leer el valor del potenciómetro
  int potValueRaw = analogRead(potPin);
  // Mapear el valor del potenciómetro a un rango de 0 a 100
  potValue = round((potValueRaw * 100) / 4095);
  // Si el valor del potenciómetro ha cambiado, llamar a la función
  if (abs(potValue - lastPotValue) > 2 || lastPotValue == -1) {
    lastPotValue = potValue;
    notifyClients(false);
  }
}
// Basada en el proyecto https://RandomNerdTutorials.com/esp32-websocket-server-arduino/
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    ledValue = atoi((char*)data);
    notifyClients(true);
  }
}

// Basada en el proyecto https://RandomNerdTutorials.com/esp32-websocket-server-arduino/
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u 
      connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

