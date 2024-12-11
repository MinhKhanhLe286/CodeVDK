#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Network credentials
const char *SSID = "ESP8266-AP";
const char *PSWD = "";

// Sensor
const int LINE_SENSOR_PIN = D1;
int line_sensor_value;

// DC Motor using H-Bridge
const int MOTOR_IN1 = D2;
const int MOTOR_IN2 = D3;
const int MOTOR_EN = D7;

// Motor speed and direction
int motor_speed = 0; // 0-1023
String motor_direction = "stop"; // forward, backward, stop

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

  // Line sensor
  pinMode(LINE_SENSOR_PIN, INPUT);

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
  <title>ESP8266 Motor Control</title>
  <style>
    html {
      font-family: Arial;
      text-align: center;
      margin: 0 auto;
    }
    h1 {
      color: #0F3376;
    }
    input, button {
      font-size: 1.2rem;
      margin: 10px;
    }
    .button {
      background-color: #008CBA;
      color: white;
      border: none;
      padding: 10px 20px;
      cursor: pointer;
    }
  </style>
</head>
<body>
  <h1>ESP8266 Motor Control</h1>
  <p>Line Sensor Value: <strong id="line_sensor_value">0</strong></p>
  <form action="/control_motor" method="get">
    <label for="speed">Speed (0-100):</label>
    <input type="number" id="speed" name="speed" min="0" max="100" required>
    <br>
    <label for="direction">Direction:</label>
    <select id="direction" name="direction">
      <option value="forward">Forward</option>
      <option value="backward">Backward</option>
      <option value="stop">Stop</option>
    </select>
    <br>
    <button type="submit" class="button">Submit</button>
  </form>
  <script>
    function updateLineSensor() {
      fetch('/line_sensor')
        .then(response => response.text())
        .then(value => {
          document.getElementById("line_sensor_value").textContent = value;
        });
    }
    setInterval(updateLineSensor, 200);
  </script>
</body>
</html>
    )rawliteral");
  });

  server.on("/line_sensor", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(line_sensor_value));
  });

  server.on("/control_motor", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("speed") && request->hasParam("direction")) {
      String speedValue = request->getParam("speed")->value();
      String directionValue = request->getParam("direction")->value();
      motor_speed = map(speedValue.toInt(), 0, 100, 0, 1023); // Map 0-100% to 0-1023
      motor_direction = directionValue;

      if (motor_direction == "forward") {
        digitalWrite(MOTOR_IN1, HIGH);
        digitalWrite(MOTOR_IN2, LOW);
        analogWrite(MOTOR_EN, motor_speed);
      } else if (motor_direction == "backward") {
        digitalWrite(MOTOR_IN1, LOW);
        digitalWrite(MOTOR_IN2, HIGH);
        analogWrite(MOTOR_EN, motor_speed);
      } else if (motor_direction == "stop") {
        digitalWrite(MOTOR_IN1, LOW);
        digitalWrite(MOTOR_IN2, LOW);
        analogWrite(MOTOR_EN, 0);
      }

      request->send(200, "text/plain", "Motor set to " + motor_direction + " at speed " + speedValue + "%");
    } else {
      request->send(400, "text/plain", "Missing parameters");
    }
  });

  server.begin();
}

void loop() {
  // Read line sensor value
  line_sensor_value = digitalRead(LINE_SENSOR_PIN);

  // Output to Serial Monitor
  Serial.print("Line Sensor Value: ");
  Serial.println(line_sensor_value);

  delay(200);
}

