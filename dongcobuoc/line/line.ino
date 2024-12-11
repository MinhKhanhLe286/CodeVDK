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

// Line Sensor
const int LINE_SENSOR_PIN = D5;
int line_sensor_state;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

  // Step motor
  stepper.setSpeed(step_speed);

  // Line sensor
  pinMode(LINE_SENSOR_PIN, INPUT);

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
  <h1>Line Sensor</h1>
  <p>Line Sensor State: <span id="line_state"></span></p>
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
      fetch('/line_state')
        .then(response => response.text())
        .then(state => {
          document.getElementById("line_state").textContent = state;
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

  server.on("/line_state", HTTP_GET, [](AsyncWebServerRequest *request) {
    line_sensor_state = digitalRead(LINE_SENSOR_PIN);
    Serial.print("Line Sensor State: ");
    Serial.println(line_sensor_state);
    request->send(200, "text/plain", String(line_sensor_state));
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
  // Xuất giá trị cảm biến dò line ra Serial Monitor
  line_sensor_state = digitalRead(LINE_SENSOR_PIN);
  Serial.print("Line Sensor State (Loop): ");
  Serial.println(line_sensor_state);

  delay(500); // Thời gian trễ 500ms để giảm tốc độ xuất
}

