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

// Infrared Motion Sensor
const int PIR_PIN = D5; // Chân tín hiệu của cảm biến hồng ngoại
int motionDetected = 0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

  // Step motor
  stepper.setSpeed(step_speed);

  // Infrared Motion Sensor
  pinMode(PIR_PIN, INPUT);

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
  <h1>Infrared Motion Sensor</h1>
  <p>Motion Detected: <span id="motion_status"></span></p>
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
      fetch('/motion_status')
        .then(response => response.text())
        .then(value => {
          document.getElementById("motion_status").textContent = value;
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

  server.on("/motion_status", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Đọc trạng thái từ cảm biến hồng ngoại
    motionDetected = digitalRead(PIR_PIN);

    String status = motionDetected == HIGH ? "Yes" : "No";
    Serial.print("Motion Detected: ");
    Serial.println(status);

    request->send(200, "text/plain", status);
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
  // Đọc trạng thái từ cảm biến hồng ngoại và xuất ra Serial Monitor
  motionDetected = digitalRead(PIR_PIN);
  String status = motionDetected == HIGH ? "Yes" : "No";
  Serial.print("Motion Detected (Loop): ");
  Serial.println(status);

  delay(500); // Thời gian trễ 500ms để giảm tốc độ xuất
}
