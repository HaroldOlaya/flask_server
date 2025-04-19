#include <WiFi.h>
#include <WebServer.h>
#include "esp_camera.h"
#include <HTTPClient.h>  // Librería para cliente HTTP

// Datos de la red Wi-Fi
const char* ssid = "Ospina_Beltran";
const char* password = "D4NN418_D4V1D13_0SC4R85";

// Servidor web en el puerto 80
WebServer server(80);

// Ruta para recibir la IP de otro servidor y mostrarla por consola
void handleReceiveIp() {
  if (server.hasArg("ip")) {
    String ipReceived = server.arg("ip");
    Serial.print("IP recibida de otro servidor: ");
    Serial.println(ipReceived);
    server.send(200, "text/plain", "IP recibida correctamente");
    sendMessageToServer(ipReceived);
  } else {
    server.send(400, "text/plain", "No se recibió IP");
  }
}

// Función para enviar imágenes continuamente al servidor recibido
void sendMessageToServer(String ip) {
  String serverUrl = "http://" + ip + ":5000/upload";

  Serial.print("Enviando imágenes a: ");
  Serial.println(serverUrl);

  while (true) {
    Serial.println("Capturando imagen...");
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Error al capturar la imagen");
      delay(1000);
      continue;  // Cambiado de return a continue para que siga intentando
    }

    Serial.println("Imagen capturada, preparando envío...");

    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "image/jpeg");

    Serial.print("Enviando imagen, tamaño: ");
    Serial.println(fb->len);

    int httpResponseCode = http.POST(fb->buf, fb->len);

    if (httpResponseCode > 0) {
      Serial.print("✅ Imagen enviada, código de respuesta: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("❌ Error al enviar la imagen: ");
      Serial.println(http.errorToString(httpResponseCode).c_str());
    }

    http.end();
    esp_camera_fb_return(fb);

    Serial.println("Esperando 100 ms antes de capturar otra imagen...\n");
    delay(10);  // Esperar antes de enviar otra imagen
  }
}

// Ruta para enviar la IP de la ESP32 CAM al servidor Flask
void sendIpToFlask(String ip) {
  HTTPClient http;
  String url = "https://haroldolaya99.pythonanywhere.com/set_ip?set_ip=" + ip;

  http.begin(url);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    Serial.print("IP enviada al servidor Flask, código de respuesta: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Error al enviar la IP: ");
    Serial.println(http.errorToString(httpResponseCode).c_str());
  }

  http.end();
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");

  Serial.print("IP de la cámara: ");
  Serial.println(WiFi.localIP());

  // Configurar la ruta del servidor web
  server.on("/receive_ip", HTTP_POST, handleReceiveIp);

  // Configuración de la cámara
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  esp_camera_init(&config);

  server.begin();
  Serial.println("Servidor HTTP iniciado");

  // Enviar IP local al servidor Flask
  sendIpToFlask(WiFi.localIP().toString());
}

void loop() {
  server.handleClient();
}
