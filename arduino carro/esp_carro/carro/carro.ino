#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <TM1637Display.h>

//const char* ssid = "Harold Olaya";
//const char* password = "haroldolaya";
const char* ssid = "Ospina_Beltran";
const char* password = "D4NN418_D4V1D13_0SC4R85";
WebServer server(80);

// Display 7 segmentos
const int CLK = 22;
const int DIO = 21;
TM1637Display display(CLK, DIO);

//Pines motores
#define MOTOR_DER_ADELANTE 13
#define MOTOR_DER_ATRAS    12
#define MOTOR_IZQ_ADELANTE 27
#define MOTOR_IZQ_ATRAS    14
// Pines RGB
const int redPin = 19;
const int greenPin = 5;
const int bluePin = 18;
// Pines seguidor de linea
const int sensorDerecho = 26;
const int sensorCentral = 25;
const int sensorIzquierdo = 23;
// Pulsador
int lastState = HIGH;
int counter = 0;
// camera
const int ledCamara = 15;
const int baseCamera = 4;
// Sensor Hall externo
const int hallPin = 35;
int minHall = 1800;
int actHall;
int maxHall = 2100; // Ajusta según tus pruebas
bool lastHallDetected = false;
bool Hallactivated =false;
bool ledRojoActivo = false;
bool ledVerdeActivo = false;
bool ledAzulActivo = false;
bool ledAmarilloActivo = false;
bool linterna = false;
bool camara = false;
bool derecha = false;
bool izquierda = false;
bool centro = false;
bool seguidor = false;

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
void setupLinea(){
  pinMode(sensorDerecho, INPUT);
  pinMode(sensorCentral, INPUT);
  pinMode(sensorIzquierdo, INPUT);
}

void ledRojo() {
    setRGBColor(HIGH, LOW, LOW); delay(500); setRGBColor(LOW, LOW, LOW);
    ledRojoActivo = true;
}

void ledVerde() {
    setRGBColor(LOW, HIGH, LOW); delay(500); setRGBColor(LOW, LOW, LOW);
    ledVerdeActivo = true;
}

void ledAzul() {
    setRGBColor(LOW, LOW, HIGH); delay(500); setRGBColor(LOW, LOW, LOW);
    ledAzulActivo = true;
}

void ledAmarillo(){
    setRGBColor(HIGH, HIGH, LOW); delay(500); setRGBColor(LOW, LOW, LOW);
    ledAmarilloActivo = true;
}
// Función corregida para mover la derecha hacia atrás
void derechaAtras() {
  digitalWrite(MOTOR_DER_ADELANTE, HIGH);
  digitalWrite(MOTOR_DER_ATRAS, LOW);
}

// Función corregida para mover la derecha hacia adelante
void derechaAdelante() {
  digitalWrite(MOTOR_DER_ADELANTE, LOW);
  digitalWrite(MOTOR_DER_ATRAS, HIGH);
}

// Función corregida para mover la izquierda hacia atrás
void izquierdaAtras() {
  digitalWrite(MOTOR_IZQ_ADELANTE, HIGH);
  digitalWrite(MOTOR_IZQ_ATRAS, LOW);
}

// Función corregida para mover la izquierda hacia adelante
void izquierdaAdelante() {
  digitalWrite(MOTOR_IZQ_ADELANTE, LOW);
  digitalWrite(MOTOR_IZQ_ATRAS, HIGH);
}

void girarDerecha() {
  digitalWrite(MOTOR_DER_ADELANTE, LOW);
  digitalWrite(MOTOR_DER_ATRAS, LOW);
  izquierdaAdelante();
}

void girarIzquierda() {
  digitalWrite(MOTOR_IZQ_ADELANTE, LOW);
  digitalWrite(MOTOR_IZQ_ATRAS, LOW);
  derechaAdelante();
}

void detener() {
  digitalWrite(MOTOR_DER_ADELANTE, LOW);
  digitalWrite(MOTOR_DER_ATRAS, LOW);
  digitalWrite(MOTOR_IZQ_ADELANTE, LOW);
  digitalWrite(MOTOR_IZQ_ATRAS, LOW);
}
void Oncamara(){
    setRGBColor(LOW, HIGH, LOW); delay(500); setRGBColor(LOW, LOW, LOW);
    digitalWrite(baseCamera,HIGH);
    camara = true;

}
void Offcamara(){
    setRGBColor(HIGH, LOW, LOW); delay(500); setRGBColor(LOW, LOW, LOW);
    digitalWrite(baseCamera,LOW);
    camara = false;
}
void onlinterna(){
    digitalWrite(ledCamara,HIGH);
    linterna = true;
}
void offlinterna(){
    digitalWrite(ledCamara,LOW);
    linterna = false;
}
// ------------------------------------
// DECODIFICACIÓN DE MENSAJE
// ------------------------------------
void definirFuncion(char dispositivo, char func) {
    switch (dispositivo) {
        case '1':
            if (func == 'A') {
                Hallactivated = true;
            } else if (func == 'B') {
                Hallactivated = false;
            }
            break;
        case '2':
            if (func == 'C') {
                ledRojo();
            } else if (func == 'D') {
                ledAzul();
            }
            else if (func == 'E') {
                ledVerde();
            }
            else if (func == 'F') {
                ledAmarillo();
            }
            break;
        case '3':
            if (func == 'G'){
                Oncamara();
            }
            else if (func == 'H'){
                Offcamara();
            }
            break;
        case '4':
            if (func == 'I'){ //funcion para avanzar
                derechaAdelante();
                izquierdaAdelante();
            }
            else if(func == 'J'){ // funcion para devolver
                derechaAtras();
                izquierdaAtras();
            }
            else if(func == 'K'){ // funcion para girar derecha
                girarDerecha();
            }
            else if(func == 'L'){ // funcion para girar izquierda
                girarIzquierda();
            }else if(func == 'M'){ // funcion para detener carro
                detener();
            }break;
        case '5':
            if (func == 'N'){
                onlinterna();
            }else if(func == 'O'){
                offlinterna();
            }
            break;
        case '6':
            if (func == 'P'){
                seguidor = true;
            }else if(func == 'Q'){
                seguidor = false;
                detener();
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
    json += "\"led_amarillo\":" + String(ledAmarilloActivo ? "true" : "false") + ",";
    json += "\"derecha\":" + String(derecha ? "true" : "false") + ",";
    json += "\"centro\":" + String(centro ? "true" : "false") + ",";
    json += "\"izquierda\":" + String(izquierda ? "true" : "false") + ",";
    json += "\"led_azul\":" + String(ledAzulActivo ? "true" : "false") + ",";
    json += "\"actHall\":" + String(actHall) + ",";
    json += "\"linterna\":" + String(linterna ? "true" : "false") + ",";
    json += "\"camara\":" + String(camara ? "true" : "false");
    json += "}";

    server.send(200, "application/json", json);
}

void handleMessage() {
    if (server.hasArg("message")) {
        String message = server.arg("message");
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
    setRGBColor(LOW, HIGH, LOW); delay(500); setRGBColor(LOW, LOW, LOW);
    int hallValue = analogRead(hallPin);
    actHall = hallValue;
    Serial.println("Valor del sensor Hall: " + String(hallValue));

    // Si el valor está fuera del rango aceptado (hay campo magnético fuerte)
    if ((hallValue < minHall || hallValue > maxHall) && !lastHallDetected) {
        counter++;
        display.showNumberDec(counter, true);
        lastHallDetected = true;
        //delay(1000); // Esperar 1 segundo para evitar rebotes o múltiples detecciones
    } 
    else if (hallValue >= minHall && hallValue <= maxHall) {
        // Restablecer el estado solo si volvemos al rango sin campo
        lastHallDetected = false;
    }
}
void setupMotores(){
  pinMode(MOTOR_DER_ADELANTE, OUTPUT);
  pinMode(MOTOR_DER_ATRAS, OUTPUT);
  pinMode(MOTOR_IZQ_ADELANTE, OUTPUT);
  pinMode(MOTOR_IZQ_ATRAS, OUTPUT);
  digitalWrite(MOTOR_DER_ADELANTE, LOW);
  digitalWrite(MOTOR_DER_ATRAS, LOW);
  digitalWrite(MOTOR_IZQ_ADELANTE, LOW);
  digitalWrite(MOTOR_IZQ_ATRAS, LOW);
}
void seguidorDeLinea(){
    setRGBColor(LOW, HIGH, LOW); delay(500); setRGBColor(LOW, LOW, LOW);
    int derecha = digitalRead(sensorDerecho);
    int centro  = digitalRead(sensorCentral);
    int izquierda = digitalRead(sensorIzquierdo);

    Serial.print("Izq: "); Serial.print(izquierda);
    Serial.print(" | Cen: "); Serial.print(centro);
    Serial.print(" | Der: "); Serial.println(derecha);

    if (centro == LOW && izquierda == HIGH && derecha == HIGH) {
        derechaAdelante();
        izquierdaAdelante();
        centro = true;
    } else if ((izquierda == LOW) || (centro == LOW && izquierda == LOW && derecha == HIGH)) {
        girarIzquierda();
        izquierda = true;
    } else if ((derecha == LOW) || (centro == LOW && derecha == LOW && izquierda == HIGH)) {
        girarDerecha();
        derecha = true;
    } else {
        detener();
        centro = false;
        izquierda = false;
        derecha = false;
    }
}

// ------------------------------------
// SETUP
// ------------------------------------
void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    setupMotores();
    setupRGB();
    setupLinea();
    pinMode(baseCamera, OUTPUT); // on off camara
    pinMode(ledCamara, OUTPUT);
    pinMode(hallPin, INPUT);           // Sensor hall externo
    display.setBrightness(5);
    display.showNumberDec(counter); // Mostrar valor inicial

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    sendIpToFlask(WiFi.localIP().toString());
    server.on("/estado", HTTP_GET, handleEstado);
    server.on("/message", HTTP_GET, handleMessage);
    server.begin();
    ledAzul();
    Oncamara();
    offlinterna();
    ledAzulActivo = false;
    ledRojoActivo = false;
    ledVerdeActivo =false;
    ledAmarilloActivo = false;
}

// ------------------------------------
// LOOP PRINCIPAL
// ------------------------------------
void loop() {
    server.handleClient();
    if (Hallactivated){
        display.showNumberDec(counter, true);
        detectarCampoMagneticoYContar();
    }
    if(seguidor){
        seguidorDeLinea();
    }
}
