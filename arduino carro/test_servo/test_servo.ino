const int pinBoton = 2;
const int pin4 = 13;
const int pin5 = 5;

bool presionado = false;

void setup() {
  pinMode(pinBoton, INPUT);
  pinMode(pin4, OUTPUT);
  pinMode(pin5, OUTPUT);

  digitalWrite(pin4, LOW);
  digitalWrite(pin5, LOW);
}

void loop() {
  int lectura = digitalRead(pinBoton);
      // Colocar ambos pines en alto
    digitalWrite(pin4, HIGH);
    digitalWrite(pin5, HIGH);
    delay(500);
}
