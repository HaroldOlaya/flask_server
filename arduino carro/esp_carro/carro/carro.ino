#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <TM1637Display.h>

const char* ssid = "Ospina_Beltran";
const char* password = "D4NN418_D4V1D13_0SC4R85";
WebServer server(80);

// Display 7 segmentos
const int CLK = 22;
const int DIO = 21;
TM1637Display display(CLK, DIO);

// Pines RGB
const int redPin = 19;
const int greenPin = 5;
const int bluePin = 18;

// Pulsador
const int buttonPin = 2;
int lastState = HIGH;
int counter = 0;

// Sensor Hall externo
const int hallPin = 35;
int minHall = 2300;
int maxHall = 3100; // Ajusta según tus pruebas
bool lastHallDetected = false;
bool Hallactivated =false;
bool ledRojoActivo = false;
bool ledVerdeActivo = false;
bool ledAzulActivo = false;

// ------------------------------------
// FUNCIONES RGB
// ------------------------------------
void setupRGB() {
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
}

void setRGBColor(int r, int g, int b) {
    digitalWrite(redPin, r);
    digitalWrite(greenPin, g);
    digitalWrite(bluePin, b);
}

void ledRojo() {
    setRGBColor(LOW, HIGH, HIGH); delay(500); setRGBColor(HIGH, HIGH, HIGH);
    ledRojoActivo = true;
    Serial.println("LED Rojo encendido");
}

void ledVerde() {
    setRGBColor(HIGH, LOW, HIGH); delay(500); setRGBColor(HIGH, HIGH, HIGH);
    ledVerdeActivo = true;
    Serial.println("LED Verde encendido");
}

void ledAzul() {
    setRGBColor(HIGH, HIGH, LOW); delay(500); setRGBColor(HIGH, HIGH, HIGH);
    ledAzulActivo = true;
    Serial.println("LED Azul encendido");
}

void motorEncender() {
    setRGBColor(HIGH, HIGH, LOW);
    Serial.println("Motor encendido");
}

void motorDetener() {
    setRGBColor(HIGH, HIGH, HIGH);
    Serial.println("Motor apagado");
}

// ------------------------------------
// DECODIFICACIÓN DE MENSAJE
// ------------------------------------
void definirFuncion(char dispositivo, char func) {
    switch (dispositivo) {
        case '1':
            Serial.println("Dispositivo: 1");
            if (func == 'A') {
                Serial.println("hal encendido");
                Hallactivated = true;
            } else if (func == 'B') {
                Serial.println("hall apagado");
                Hallactivated = false;
            } else {
                Serial.println("Función no reconocida para dispositivo 1");
            }
            break;
        case '7':
            Serial.println("Dispositivo: 7");
            if (func == 'L') {
                ledRojo();
            } else if (func == 'M') {
                ledAzul();
            } else {
                Serial.println("Función no reconocida para dispositivo 7");
            }
            break;
        case '8':
            Serial.println("Dispositivo: 8");
            if (func == 'P') {
                ledVerde();
            } else if (func == 'Q') {
                Serial.println("Función Q");
            } else {
                Serial.println("Función no reconocida para dispositivo 8");
            }
            break;
        case '9':
            Serial.println("Dispositivo: 9");
            if (func == 'N') {
                motorEncender();
            } else if (func == 'O') {
                motorDetener();
            } else {
                Serial.println("Función no reconocida para dispositivo 9");
            }
            break;
        default:
            Serial.println("Dispositivo no reconocido.");
            break;
    }
}
void handleEstado() {
    String json = "{";
    json += "\"contador\":" + String(counter) + ",";
    json += "\"led_rojo\":" + String(ledRojoActivo ? "true" : "false") + ",";
    json += "\"led_verde\":" + String(ledVerdeActivo ? "true" : "false") + ",";
    json += "\"led_azul\":" + String(ledAzulActivo ? "true" : "false");
    json += "}";

    server.send(200, "application/json", json);
}

void handleMessage() {
    if (server.hasArg("message")) {
        String message = server.arg("message");
        Serial.print("Mensaje recibido: ");
        Serial.println(message);

        if (message.length() == 3 && message[0] == '/') {
            char dispositivo = message[1];
            char func = message[2];
            definirFuncion(dispositivo, func);
            server.send(200, "text/plain", "Mensaje procesado correctamente");
        } else {
            server.send(400, "text/plain", "Error: Formato incorrecto, debe tener 3 caracteres y comenzar con /");
        }
    } else {
        server.send(400, "text/plain", "Error: No se recibió el mensaje");
    }
}

// ------------------------------------
// ENVIAR IP A FLASK
// ------------------------------------
void sendIpToFlask(String ip) {
    HTTPClient http;
    String url = "https://haroldolaya99.pythonanywhere.com/set_ip_motor?set_ip_motor=" + ip;

    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
        Serial.print("IP enviada al servidor Flask. Código: ");
        Serial.println(httpResponseCode);
    } else {
        Serial.print("Error al enviar la IP: ");
        Serial.println(http.errorToString(httpResponseCode).c_str());
    }

    http.end();
}

// ------------------------------------
// FUNCIONES DEL SENSOR HALL Y CONTADOR
// ------------------------------------
void detectarCampoMagneticoYContar() {
    int hallValue = analogRead(hallPin);
    Serial.println("Valor del sensor Hall: " + String(hallValue));

    // Si el valor está fuera del rango aceptado (hay campo magnético fuerte)
    if ((hallValue < minHall || hallValue > maxHall) && !lastHallDetected) {
        counter++;
        display.showNumberDec(counter, true);
        Serial.println("¡Campo magnético detectado! Contador: " + String(counter));
        lastHallDetected = true;

        delay(1000); // Esperar 1 segundo para evitar rebotes o múltiples detecciones
    } 
    else if (hallValue >= 2500 && hallValue <= 2700) {
        // Restablecer el estado solo si volvemos al rango sin campo
        lastHallDetected = false;
    }
}

// ------------------------------------
// SETUP
// ------------------------------------
void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    setupRGB();
    pinMode(buttonPin, INPUT_PULLUP);  // Pulsador con resistencia pull-up
    pinMode(hallPin, INPUT);           // Sensor hall externo
    display.setBrightness(5);
    display.showNumberDec(counter); // Mostrar valor inicial

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConectado a la red WiFi");
    Serial.print("Dirección IP: ");
    Serial.println(WiFi.localIP());

    sendIpToFlask(WiFi.localIP().toString());
    server.on("/estado", HTTP_GET, handleEstado);
    server.on("/message", HTTP_GET, handleMessage);
    server.begin();
    Serial.println("Servidor web iniciado.");
    ledAzul();
    ledAzulActivo = false;
    ledRojoActivo = false;
    ledVerdeActivo =false;
}

// ------------------------------------
// LOOP PRINCIPAL
// ------------------------------------
void loop() {
    server.handleClient();
    if (Hallactivated){
        display.showNumberDec(counter, true);
    // Sensor hall
        detectarCampoMagneticoYContar();
    }
    
 // Pausa ligera para estabilidad
}
