#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <math.h>

// Configuración WiFi
const char* ssid = "IZZI-CE85";
const char* password = "50A5DC2DCE85";

// Web server en el puerto 80
WebServer server(80);

// Sensores
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x29);
Adafruit_BNO055 bno1 = Adafruit_BNO055(55, 0x28);

float relDeg = 0.0;
unsigned long id = 1;
unsigned long StartTime;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Conectar a WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());

  // Configurar servidor

server.on("/", HTTP_GET, []() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <meta charset="UTF-8">
      <title>Ángulo Relativo BNO055</title>
      <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
      <style>
        body { font-family: Arial; text-align: center; margin-top: 30px; }
        h1 { font-size: 28px; }
        #angle { font-size: 36px; color: #007bff; margin-bottom: 20px; }
        canvas { max-width: 90%; height: auto; }
      </style>
    </head>
    <body>
      <h1>Ángulo relativo entre sensores</h1>
      <div id="angle">Cargando...</div>
      <canvas id="angleChart"></canvas>

      <script>
        const ctx = document.getElementById('angleChart').getContext('2d');
        const chart = new Chart(ctx, {
          type: 'line',
          data: {
            labels: [],
            datasets: [{
              label: 'Ángulo (°)',
              data: [],
              borderColor: 'rgba(0, 123, 255, 1)',
              backgroundColor: 'rgba(0, 123, 255, 0.1)',
              tension: 0.1
            }]
          },
          options: {
            responsive: true,
            scales: {
              x: {
                title: { display: true, text: 'Tiempo (ms)' }
              },
              y: {
                title: { display: true, text: 'Ángulo (°)' },
                min: 0,
                max: 180
              }
            }
          }
        });

        function fetchAngle() {
          fetch('/data')
            .then(response => response.json())
            .then(data => {
              document.getElementById('angle').innerHTML = data.angle.toFixed(2) + "°";

              // Agregar nuevo punto al gráfico
              chart.data.labels.push(data.id);  // puedes usar data.time si lo prefieres
              chart.data.datasets[0].data.push(data.angle);

              // Limitar puntos para no saturar
              if (chart.data.labels.length > 100) {
                chart.data.labels.shift();
                chart.data.datasets[0].data.shift();
              }

              chart.update();
            });
        }

        setInterval(fetchAngle, 1000);
        fetchAngle();
      </script>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html);
});

  server.on("/data", HTTP_GET, []() {
    server.send(200, "application/json", "{\"id\":" + String(id) + ",\"angle\":" + String(relDeg) + "}");
  });

  server.begin();

  // Inicializar sensores
  if (!bno.begin()) {
    Serial.println("Sensor BNO055 (0x29) no detectado.");
    while (1);
  }

  if (!bno1.begin()) {
    Serial.println("Sensor BNO055 (0x28) no detectado.");
    while (1);
  }

  // Usar modo IMUPLUS para evitar interferencias del magnetómetro
  bno.setMode(OPERATION_MODE_IMUPLUS);
  bno1.setMode(OPERATION_MODE_IMUPLUS);
  bno.setExtCrystalUse(true);
  bno1.setExtCrystalUse(true);

  // Esperar calibración del giroscopio
  uint8_t gyro = 0;
  while (gyro != 3) {
    uint8_t sys, accel, mag;
    bno.getCalibration(&sys, &gyro, &accel, &mag);
    Serial.print("Calibrando giroscopio... Nivel: "); Serial.println(gyro);
    delay(500);
  }

  Serial.println("✅ Giroscopios calibrados. Iniciando medición...");
  StartTime = millis();
}

void loop() {
  server.handleClient(); // Escuchar peticiones HTTP

  // Leer cuaterniones
  imu::Quaternion q1 = bno.getQuat();
  imu::Quaternion q2 = bno1.getQuat();
  q1.normalize();
  q2.normalize();

  // Calcular ángulo relativo
  double dot = q1.w() * q2.w() + q1.x() * q2.x() + q1.y() * q2.y() + q1.z() * q2.z();
  dot = constrain(dot, -1.0, 1.0);
  relDeg = 57.2958 * 2 * acos(abs(dot));

  // Imprimir en serial también
  unsigned long now = millis() - StartTime;
  Serial.print("ID: "); Serial.print(id);
  Serial.print(" | Tiempo: "); Serial.print(now);
  Serial.print(" ms | Ángulo: "); Serial.print(relDeg); Serial.println("°");

  id++;
  delay(100);
}
