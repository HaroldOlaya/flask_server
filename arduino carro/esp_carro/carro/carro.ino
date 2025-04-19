#include <WiFi.h>
#include <WebServer.h>
#include <TM1637Display.h>

const char* ssid = "Harold Olaya";
const char* password = "haroldolaya";
WebServer server(80);

const int CLK = 22;
const int DIO = 21;
TM1637Display display(CLK, DIO);

const int redPin = 19;
const int greenPin = 5;
const int bluePin = 18;

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
    Serial.println("LED Rojo encendido");
}

void ledVerde() {
    setRGBColor(HIGH, LOW, HIGH); delay(500); setRGBColor(HIGH, HIGH, HIGH);
    Serial.println("LED Verde encendido");
}

void ledAzul() {
    setRGBColor(HIGH, HIGH, LOW); delay(500); setRGBColor(HIGH, HIGH, HIGH);
    Serial.println("LED Azul encendido");
}

void motorEncender(){
    setRGBColor(HIGH, HIGH, LOW);
    Serial.println("Motor encendido");
}
void motorDetener(){
    setRGBColor(HIGH, HIGH, HIGH);
    Serial.println("Motor apagado");
}

void IRAM_ATTR displayInterrupt() {
    static int counter = 0;
    counter++;
    display.showNumberDec(counter);
    Serial.println("Display actualizado con: " + String(counter));
}

void definirFuncion(char dispositivo, char func) {
    switch (dispositivo) {
        case '1':
            Serial.println("Dispositivo: 1");
            if (func == 'A') {
                Serial.println("Motor A");
            } else if (func == 'B') {
                Serial.println("Motor B");
            } else {
                Serial.println("Función no reconocida para dispositivo 1");
            }
            break;
        case '7':
            Serial.println("Dispositivo: 7");
            if (func == 'L') {
                Serial.println("Función L");
                ledRojo();
            } else if (func == 'M') {
                Serial.println("Función M");
            } else {
                Serial.println("Función no reconocida para dispositivo 7");
            }
            break;
        case '8':
            Serial.println("Dispositivo: 8");
            if (func == 'P') {
                Serial.println("Función P");
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
                Serial.println("Función N");
                motorEncender();
            } else if (func == 'O') {
                Serial.println("Función O");
                motorDetener();
            } else {
                Serial.println("Función no reconocida para dispositivo 9");
            }
            break;
        case ';':
            Serial.println("Dispositivo: ;");
            break;
        case ':':
            Serial.println("Dispositivo: :");
            break;
        default:
            Serial.println("Dispositivo no reconocido.");
            break;
    }
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
            Serial.println("Error: Formato de mensaje incorrecto.");
            server.send(400, "text/plain", "Error: Formato incorrecto, debe tener 3 caracteres y comenzar con /");
        }
    } else {
        Serial.println("Error: No se recibió el mensaje");
        server.send(400, "text/plain", "Error: No se recibió el mensaje");
    }
}

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    setupRGB();
    pinMode(2, INPUT_PULLUP);  // Configurar pin 2 como entrada con resistencia interna
    attachInterrupt(digitalPinToInterrupt(2), displayInterrupt, FALLING);  // Interrupción en el pin 2
    display.setBrightness(0x0f);
    display.showNumberDec(1234);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConectado a la red WiFi");
    Serial.print("Dirección IP: ");
    Serial.println(WiFi.localIP());

    server.on("/message", HTTP_GET, handleMessage);
    server.begin();
    Serial.println("Servidor web iniciado.");
    ledAzul();
}

void loop() {
    server.handleClient();
}
