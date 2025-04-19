#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>   // Librería para servidor web

// Datos de la red Wi-Fi
const char* ssid = "Harold Olaya";
const char* password = "haroldolaya";

// Servidor web en el puerto 80
WebServer server(80);

// Ruta para recibir la IP de otro servidor y mostrarla por consola
void handleReceiveIp() {
  if (server.hasArg("ip")) {  // Verificar si la IP fue recibida en el parámetro
    String ipReceived = server.arg("ip");  // Obtener la IP enviada
    Serial.print("IP recibida de otro servidor: ");
    Serial.println(ipReceived);  // Mostrar la IP por consola
    server.send(200, "text/plain", "IP recibida correctamente");
  } else {
    server.send(400, "text/plain", "No se recibió IP");
  }
}

// Ruta para enviar la IP de la ESP32 CAM al servidor Flask
void sendIpToFlask(String ip) {
  HTTPClient http;
  String url = "https://haroldolaya99.pythonanywhere.com/set_ip?set_ip=" + ip;  // URL del servidor Flask

  http.begin(url);  // Iniciar la solicitud HTTP con la URL

  int httpResponseCode = http.GET();  // Hacer la solicitud GET

  if (httpResponseCode > 0) {
    Serial.print("IP enviada al servidor Flask, código de respuesta: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Error al enviar la IP: ");
    Serial.println(http.errorToString(httpResponseCode).c_str());
  }

  http.end();  // Finalizar la conexión HTTP
}

void setup() {
  Serial.begin(115200);

  // Conectar a la red Wi-Fi
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);  // Desactivar el modo de bajo consumo

  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");

  // Imprimir la IP local de la ESP32 CAM
  Serial.print("IP de la cámara: ");
  Serial.println(WiFi.localIP());

  // Configurar las rutas para el servidor web
  server.on("/receive_ip", HTTP_POST, handleReceiveIp);

  // Iniciar el servidor web
  server.begin();
  Serial.println("Servidor HTTP iniciado");

  // Enviar la IP local al servidor Flask
  sendIpToFlask(WiFi.localIP().toString());
}

void loop() {
  server.handleClient();  // Gestionar las solicitudes entrantes
}
