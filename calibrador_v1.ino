#include <TM1637Display.h>

const int CLK = 2;
const int DIO = 4;
TM1637Display display(CLK, DIO);

const int analogInPin = A1;     // Potenciómetro
const int analogOutPin = 3;     // PWM Voltaje
const int mAOutPin = 5;         // PWM Corriente
const int buttonPin = 7;        // Pulsador

// Nuevos pines para los LEDs indicadores
const int voltageLedPin = 8;    // LED que enciende en modo Voltaje
const int currentLedPin = 9;    // LED que enciende en modo Corriente

int sensorValue = 0;          
int lastFilteredValue = 0;
int outputValue = 0;          
int mAOut = 0;

bool showVoltage = true;        // true = Voltaje (0-10V), false = Corriente (4-20mA)
bool lastButtonState = HIGH;  

void setup() {
  pinMode(analogOutPin, OUTPUT);
  pinMode(mAOutPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);    // Botón con resistencia pull-up interna
  
  // Pines de los LEDs como salidas
  pinMode(voltageLedPin, OUTPUT);
  pinMode(currentLedPin, OUTPUT);
  
  display.setBrightness(0x0f);      
  Serial.begin(9600);
}

void loop() {
  // 1. Lectura del botón
  bool currentButtonState = digitalRead(buttonPin);
  
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    showVoltage = !showVoltage; // Alterna el modo
    
    Serial.print(">>> CAMBIO DE MODO: ");
    Serial.println(showVoltage ? "VOLTAJE (0-10V)" : "CORRIENTE (0-24mA)");
    
    delay(150); // Anti-rebote
  }
  lastButtonState = currentButtonState;

  // 2. Control físico de los LEDs según el modo activo
  if (showVoltage) {
    digitalWrite(voltageLedPin, HIGH); // Enciende LED de voltaje
    digitalWrite(currentLedPin, LOW);  // Apaga LED de corriente
  } else {
    digitalWrite(voltageLedPin, LOW);  // Apaga LED de voltaje
    digitalWrite(currentLedPin, HIGH); // Enciende LED de corriente
  }

  // 3. Lectura del potenciómetro con filtro de estabilidad
  int rawValue = analogRead(analogInPin);
  if (abs(rawValue - lastFilteredValue) > 3) {
    lastFilteredValue = rawValue;
    sensorValue = rawValue;
    
    outputValue = map(sensorValue, 0, 1023, 0, 255);
    mAOut = map(sensorValue, 0, 1023, 0, 255);
    
    analogWrite(analogOutPin, outputValue);
    analogWrite(mAOutPin, mAOut);
  }

  // 4. Cálculos para el display
  int voltageVal = map(sensorValue, 0, 1023, 0, 1000);   // 0.00 a 10.00 V
  int currentVal = map(sensorValue, 0, 1023, 0, 2400); // 0.00 a 24.00 mA

  // 5. Mostrar en el TM1637 según el modo activo
  if (showVoltage) {
    display.showNumberDecEx(voltageVal, 0b01000000, true, 4, 0);
  } else {
    display.showNumberDecEx(currentVal, 0b01000000, true, 4, 0);
  }

  delay(50);
}
