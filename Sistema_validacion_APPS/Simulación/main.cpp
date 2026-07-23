const int pinSensor1 = A3; 
const int pinSensor2 = A2;
const int pinAlerta  = 7;

const int MIN_ADC_S1 = 64,   MAX_ADC_S1 = 642;
const int MIN_ADC_S2 = 93,   MAX_ADC_S2 = 516;


const float UMBRAL_DISCREPANCIA = 10.0;
const unsigned long VENTANA_FALLA_MS = 100;

unsigned long tiempoFalla = 0;
bool fallaActiva = false;
bool motorBloqueado = false;

bool sensorFueraDeRango(int lectura, int minEsperado, int maxEsperado) {
  return (lectura < minEsperado - 15) || (lectura > maxEsperado + 15);
}

void setup() {
  Serial.begin(115200);
  pinMode(pinAlerta, OUTPUT);
  digitalWrite(pinAlerta, LOW);
}

void loop() {
  if (motorBloqueado) {
    Serial.println("MOTOR BLOQUEADO. Reinicio manual requerido.");
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
  bool condicionFalla = (discrepancia > UMBRAL_DISCREPANCIA) || falloSensor1 || falloSensor2;

  if (condicionFalla) {
    if (!fallaActiva) {
      fallaActiva = true;
      tiempoFalla = millis();
    } else if (millis() - tiempoFalla > VENTANA_FALLA_MS) {
      motorBloqueado = true;
      digitalWrite(pinAlerta, HIGH);
      Serial.println("Discrepancia sostenida. Cortando motor.");
    }
  } else {
    fallaActiva = false;
  }

  Serial.print("RAW1: "); Serial.print(voltajeCrudo1);
  Serial.print(" | RAW2: "); Serial.print(voltajeCrudo2);
  Serial.print(" | Canal1: "); Serial.print(recorrido1); Serial.print("%");
  Serial.print(" | Canal2: "); Serial.print(recorrido2); Serial.print("%");
  Serial.print(" | Dif: "); Serial.print(discrepancia); Serial.println("%");

  delay(20);
}
