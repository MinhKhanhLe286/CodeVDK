// Dã check
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>

// Network credentials
const char *SSID = "ESP8266-NHOM66";
const char *PSWD = "";

// Sensor
#define DHTPIN D5 // Pin connected to the DHT sensor
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);
float temperature;
float humidity;

// DC Motor using H-Bridge
const int MOTOR_IN1 = D2;
const int MOTOR_IN2 = D3;
const int MOTOR_EN = D4;
int motor_speed = 255; // Default speed

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

  // Initialize DHT sensor
  dht.begin();

  // Motor setup
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_EN, OUTPUT);

  // Initialize motor to stopped
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  analogWrite(MOTOR_EN, 0);

  // WiFi softAP
  WiFi.softAP(SSID, PSWD);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("http://");
  Serial.print(myIP);
  Serial.println("/");

  // Web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP8266 Web Server</title>
  <style>
    html {
      font-family: Arial;
      display: inline-block;
      margin: 0px auto;
      text-align: center;
    }
    h1 {
      color: #0F3376;
      padding: 2vh;
    }
    p {
      font-size: 1.5rem;
    }
    .button {
      display: inline-block;
      background-color: #008CBA;
      border: none;
      border-radius: 4px;
      color: white;
      padding: 16px 40px;
      font-size: 30px;
      margin: 2px;
      cursor: pointer;
    }
    .button2 {
      background-color: #f44336;
    }
  </style>
</head>
<body>
  <h1>ESP8266 Web Server</h1>
  <p>Temperature: <strong id="temperature_value">0</strong> &deg;C</p>
  <p>Humidity: <strong id="humidity_value">0</strong> %</p>
  <h1>Motor Control</h1>
  <label for="motor_speed">Motor Speed (0-255):</label>
  <input type="number" id="motor_speed" min="0" max="255" value="255">
  <button onclick="setMotorSpeed()" class="button">Set Speed</button>
  <br>
  <button onclick="motorForward()" class="button">Forward</button>
  <button onclick="motorBackward()" class="button">Backward</button>
  <button onclick="motorStop()" class="button button2">Stop</button>
  <script>
    function updateSensorData() {
      fetch('/temperature')
        .then(response => response.text())
        .then(value => {
          document.getElementById("temperature_value").textContent = value;
        });
      fetch('/humidity')
        .then(response => response.text())
        .then(value => {
          document.getElementById("humidity_value").textContent = value;
        });
    }
    function motorForward() {
      fetch("/motor_forward");
    }
    function motorBackward() {
      fetch("/motor_backward");
    }
    function motorStop() {
      fetch("/motor_stop");
    }
    function setMotorSpeed() {
      const speed = document.getElementById("motor_speed").value;
      fetch(`/set_motor_speed?speed=${speed}`);
    }
    setInterval(updateSensorData, 200);
  </script>
</body>
</html>
    )rawliteral");
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(temperature));
  });

  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(humidity));
  });

  server.on("/motor_forward", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(MOTOR_IN1, HIGH);
    digitalWrite(MOTOR_IN2, LOW);
    analogWrite(MOTOR_EN, motor_speed);
    request->send(200, "text/plain", "Motor moving forward");
  });

  server.on("/motor_backward", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, HIGH);
    analogWrite(MOTOR_EN, motor_speed);
    request->send(200, "text/plain", "Motor moving backward");
  });

  server.on("/motor_stop", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, LOW);
    analogWrite(MOTOR_EN, 0);
    request->send(200, "text/plain", "Motor stopped");
  });

  server.on("/set_motor_speed", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("speed")) {
      String speedParam = request->getParam("speed")->value();
      motor_speed = speedParam.toInt();
      motor_speed = constrain(motor_speed, 0, 255); // Ensure valid range
      request->send(200, "text/plain", "Motor speed set to " + String(motor_speed));
    } else {
      request->send(400, "text/plain", "Speed parameter missing");
    }
  });

  server.begin();
}

void loop() {
  // Read DHT sensor values
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // Output to Serial Monitor
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    // Serial.print("Temperature: ");
    // Serial.print(temperature);
    // Serial.println(" °C");

    // Serial.print("Humidity: ");
    // Serial.print(humidity);
    // Serial.println(" %");
  }

  delay(200);
}
