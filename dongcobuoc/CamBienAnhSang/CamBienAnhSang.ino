#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Stepper.h>

// Network credentials
const char *SSID = "ESP8266-AP";
const char *PSWD = "";

// Step Motor
#define STEPS 2048
#define IN1 D1
#define IN2 D2
#define IN3 D3
#define IN4 D4
Stepper stepper(STEPS, IN1, IN3, IN2, IN4);
const int step_speed = 15;
int step_num = STEPS / 12;
int step_dir = 1;

// Light Sensor
const int LIGHT_SENSOR_PIN = A0; // Pin cảm biến ánh sáng
int light_sensor_value;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

  // Step motor
  stepper.setSpeed(step_speed);

  // Light sensor
  pinMode(LIGHT_SENSOR_PIN, INPUT);

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
    .units {
      font-size: 1.2rem;
    }
  </style>
</head>
<body>
  <h1>ESP8266 Web Server</h1>
  <h1>Light Sensor</h1>
  <p>Light Sensor Value: <span id="light_value"></span></p>
  <h1>Step Motor</h1>
  <form action="">
    <label for="rotate-degree">Degree</label>
    <select name="rotate-degree" id="rotate-degree">
      <option value="45" selected>45</option>
      <option value="90">90</option>
      <option value="135">135</option>
      <option value="180">180</option>
    </select>
    <label for="rotate-input">Custom Degree</label>
    <input type="number" id="rotate-input" min="1" max="360" step="1">
    <input type="checkbox" name="CCW-checkbox" id="CCW-checkbox">
    <label for="CCW-checkbox">CCW</label>
    <input type="button" id="submit-btn" value="Rotate" class="button" onclick="rotateStepper()">
  </form>
  <script>
    setInterval(() => {
      fetch('/light_value')
        .then(response => response.text())
        .then(value => {
          document.getElementById("light_value").textContent = value;
        });
    }, 200);

    let options = document.querySelectorAll("select#rotate-degree option");
    let ccw_checkbox = document.getElementById("CCW-checkbox");
    let rotate_input = document.getElementById("rotate-input");

    function rotateStepper() {
      let degree = rotate_input.value || Array.from(options).find(opt => opt.selected).value;
      let ccw = ccw_checkbox.checked ? "true" : "false";
      fetch(`/rotate?degree=${degree}&ccw=${ccw}`);
    }
  </script>
</body>
</html>
    )rawliteral");
  });

  server.on("/light_value", HTTP_GET, [](AsyncWebServerRequest *request) {
    light_sensor_value = analogRead(LIGHT_SENSOR_PIN);
    Serial.print("Light Sensor Value: ");
    Serial.println(light_sensor_value);
    request->send(200, "text/plain", String(light_sensor_value));
  });

  server.on("/rotate", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("degree") && request->hasParam("ccw")) {
      int degree = request->getParam("degree")->value().toInt();
      step_num = degree / 360.0 * STEPS;
      String ccw = request->getParam("ccw")->value();
      step_dir = (ccw == "true") ? -1 : 1;
      Serial.print("Rotating: ");
      Serial.print(degree);
      Serial.print(" degrees ");
      Serial.println(step_dir == -1 ? "CCW" : "CW");
      request->send(200, "text/plain", "Rotation successful");
    } else {
      request->send(400, "text/plain", "Missing degree or ccw parameter");
    }
  });

  server.begin();
}

void loop() {
  // Xuất giá trị cảm biến ánh sáng ra Serial Monitor
  light_sensor_value = analogRead(LIGHT_SENSOR_PIN);
  Serial.print("Light Sensor Value (Loop): ");
  Serial.println(light_sensor_value);

  delay(500); // Thời gian trễ 500ms để giảm tốc độ xuất
}
