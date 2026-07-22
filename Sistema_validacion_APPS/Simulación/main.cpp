#include <Arduino.h>
const int pinSensor1 = 34; 
const int pinSensor2 = 35; 
const int pinAlerta = 2;   

const int MIN_ADC_S1 = 300,  MAX_ADC_S1 = 3800;
const int MIN_ADC_S2 = 1500, MAX_ADC_S2 = 3200;

unsigned long tiempoFalla = 0;
bool fallaActiva = false;
bool motorBloqueado = false;

bool sensorFueraDeRango(int lectura, int minEsperado, int maxEsperado) {
  return (lectura < minEsperado - 50) || (lectura > maxEsperado + 50);
}

void setup() {
  Serial.begin(115200);
  pinMode(pinAlerta, OUTPUT);
  digitalWrite(pinAlerta, LOW);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
}

void loop() {
  if (motorBloqueado) {
    Serial.println("MOTOR BLOQUEADO - Fallo de seguridad. Reinicio manual requerido.");
    delay(500);
    return;
  }

  int voltajeCrudo1 = analogRead(pinSensor1);
  int voltajeCrudo2 = analogRead(pinSensor2);

  bool falloSensor1 = sensorFueraDeRango(voltajeCrudo1, MIN_ADC_S1, MAX_ADC_S1);
  bool falloSensor2 = sensorFueraDeRango(voltajeCrudo2, MIN_ADC_S2, MAX_ADC_S2);

  float recorrido1 = constrain(map(voltajeCrudo1, MIN_ADC_S1, MAX_ADC_S1, 0, 1000), 0, 1000) / 10.0;
  float recorrido2 = constrain(map(voltajeCrudo2, MIN_ADC_S2, MAX_ADC_S2, 0, 1000), 0, 1000) / 10.0;

  float discrepancia = abs(recorrido1 - recorrido2);

  if (discrepancia > 10.0 || falloSensor1 || falloSensor2) {
    if (!fallaActiva) {
      fallaActiva = true;
      tiempoFalla = millis(); 
    } else {
      if (millis() - tiempoFalla > 100) {
        motorBloqueado = true;
        digitalWrite(pinAlerta, HIGH); 
        Serial.println("¡ALERTA T.4.2! Discrepancia > 100ms. Cortando motor.");
      }
    }
  } else {
    fallaActiva = false; 
  }

  Serial.print("Canal 1: "); Serial.print(recorrido1); Serial.print("% | ");
  Serial.print("Canal 2: "); Serial.print(recorrido2); Serial.print("% | ");
  Serial.print("Diferencia: "); Serial.print(discrepancia); Serial.println("%");

  delay(20); 
}
