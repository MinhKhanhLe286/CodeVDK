#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>
#include <Stepper.h>
// ok
// Network credentials
const char *SSID = "ESP8266-Nhom666";
const char *PSWD = "";

// DHT Sensor
#define DHTPIN D2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float temperature;
float humidity;

// Step Motor
#define STEPS 2048
#define IN1 D3
#define IN2 D4
#define IN3 D5
#define IN4 D6
Stepper stepper(STEPS, IN1, IN3, IN2, IN4);
const int step_speed = 15;
int step_num = STEPS / 12;
int step_delay = 200;
int step_dir = 1;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

  // Initialize DHT sensor
  dht.begin();

  // Step motor
  stepper.setSpeed(step_speed);

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
    .units {
      font-size: 1.2rem;
    }
    .sensor-labels {
      font-size: 1.5rem;
      vertical-align: middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h1>ESP8266 Web Server</h1>
  <h1>Temperature and Humidity</h1>
  <p>
    <span class="sensor-labels">Temperature:</span>
    <span id="temperature"></span> °C
  </p>
  <p>
    <span class="sensor-labels">Humidity:</span>
    <span id="humidity"></span> %
  </p>
  <h1>Step Motor</h1>
  <form action="">
    <label for="rotate-degree">Degree</label>
    <select name="rotate-degree" id="rotate-degree">
      <option value="45" selected>45</option>
      <option value="90">90</option>
      <option value="135">135</option>
      <option value="180">180</option>
    </select>
    <input type="number" id="input-degree" placeholder="Enter degree">
    <input type="checkbox" name="CCW-checkbox" id="CCW-checkbox">
    <label for="CCW-checkbox">CCW</label>
    <input type="button" id="submit-btn" value="Rotate" class="button" onclick="rotateStepper()">
  </form>
  <script>
    setInterval(() => {
      fetch('/temperature')
        .then(response => response.text())
        .then(temp => {
          document.getElementById("temperature").textContent = temp;
        });
      fetch('/humidity')
        .then(response => response.text())
        .then(hum => {
          document.getElementById("humidity").textContent = hum;
        });
    }, 2000);

    let options = document.querySelectorAll("select#rotate-degree option");
    let ccw_checkbox = document.getElementById("CCW-checkbox");
    function rotateStepper() {
      let inputDegree = document.getElementById("input-degree").value;
      let degree = inputDegree ? inputDegree : document.querySelector("#rotate-degree").value;
      let ccw = ccw_checkbox.checked ? "true" : "false";
      fetch(`/rotate?degree=${degree}&ccw=${ccw}`);
    }
  </script>
</body>
</html>
    )rawliteral");
  });

  // Other routes
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(temperature));
  });

  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(humidity));
  });

  server.on("/rotate", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("degree") && request->hasParam("ccw")) {
      int degree = request->getParam("degree")->value().toInt();
      step_num = degree / 360.0 * STEPS;
      String ccw = request->getParam("ccw")->value();
      step_dir = (ccw == "true") ? -1 : 1;
      request->send(200, "text/plain", "Rotation successful");
    } else {
      request->send(400, "text/plain", "Missing degree or ccw parameter");
    }
  });

  server.begin();
}

void loop() {
  // Read temperature and humidity
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // Debug to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  delay(2000);
  stepper.step(step_num * step_dir);
  delay(step_delay);
}

