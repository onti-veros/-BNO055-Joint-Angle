#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

// Dirección I2C de los sensores
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x29);  // Sensor 1
Adafruit_BNO055 bno1 = Adafruit_BNO055(55, 0x28); // Sensor 2

// Pin del LED indicador
const int ledPin = 2;  // Puedes cambiar este pin según tu ESP32

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Apagar LED al inicio

  // Inicializar sensor 1
  if (!bno.begin()) {
    Serial.println("Sensor BNO055 (0x29) no detectado.");
    while (1);
  }

  // Inicializar sensor 2
  if (!bno1.begin()) {
    Serial.println("Sensor BNO055 (0x28) no detectado.");
    while (1);
  }

  // Usar cristal externo para mayor estabilidad
  bno.setExtCrystalUse(true);
  bno1.setExtCrystalUse(true);

  Serial.println("Sensores listos. Esperando calibración completa (3/3/3/3)...");
}

void loop() {
  uint8_t sys1, gyro1, accel1, mag1;
  uint8_t sys2, gyro2, accel2, mag2;

  bno.getCalibration(&sys1, &gyro1, &accel1, &mag1);
  bno1.getCalibration(&sys2, &gyro2, &accel2, &mag2);

  Serial.println("=== Sensor 1 (0x29) ===");
  Serial.print("SYS: "); Serial.print(sys1);
  Serial.print(" | GYRO: "); Serial.print(gyro1);
  Serial.print(" | ACCEL: "); Serial.print(accel1);
  Serial.print(" | MAG: "); Serial.println(mag1);

  Serial.println("=== Sensor 2 (0x28) ===");
  Serial.print("SYS: "); Serial.print(sys2);
  Serial.print(" | GYRO: "); Serial.print(gyro2);
  Serial.print(" | ACCEL: "); Serial.print(accel2);
  Serial.print(" | MAG: "); Serial.println(mag2);

  // Verificar si ambos sensores están completamente calibrados
  bool sensor1Calibrado = (sys1 == 3 && gyro1 == 3 && accel1 == 3 && mag1 == 3);
  bool sensor2Calibrado = (sys2 == 3 && gyro2 == 3 && accel2 == 3 && mag2 == 3);

  if (sensor1Calibrado && sensor2Calibrado) {
    Serial.println("✅ Ambos sensores están totalmente calibrados.");
    digitalWrite(ledPin, HIGH); // Enciende LED
  } else {
    digitalWrite(ledPin, LOW); // Apaga LED
  }

  Serial.println();
  delay(1000); // Espera 1 segundo
}
