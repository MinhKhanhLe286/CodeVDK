#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Stepper.h>

//Buoc: 5V - GND arduino, 12v - 5v arduino, 4 chan D3456
// CB sieu am: Vcc - 3.3V cua esp...
// Network credentials
const char *SSID = "ESP8266-Trang";
const char *PSWD = "";

// Ultrasonic Sensor
const int TRIG_PIN = D1; // Trigger pin
const int ECHO_PIN = D2; // Echo pin
float distance;
char distance_buf[8];

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

  // Ultrasonic Sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

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
  <h1>Ultrasonic Sensor</h1>
  <p>
    <span class="sensor-labels">Distance</span>
    <span id="distance"></span>
    <sup class="units">cm</sup>
  </p>
  <h1>Step Motor</h1>
  <form action="">
    <label for="rotate-degree">Enter Degree</label>
    <input type="number" id="rotate-degree" name="rotate-degree" min="1" max="360" placeholder="Enter angle (1-360)">
    <br>
    <input type="checkbox" name="CCW-checkbox" id="CCW-checkbox">
    <label for="CCW-checkbox">CCW</label>
    <br>
    <input type="button" id="submit-btn" value="Rotate" class="button" onclick="rotateStepper()">
  </form>
  <script>
    setInterval(() => {
      fetch('/distance')
        .then(response => response.text())
        .then(distance => {
          document.getElementById("distance").textContent = distance;
        });
    }, 200);

    function rotateStepper() {
      let degree = document.getElementById("rotate-degree").value;
      let ccw_checkbox = document.getElementById("CCW-checkbox").checked;
      if (degree && degree > 0 && degree <= 360) {
        let ccw = ccw_checkbox ? "true" : "false";
        fetch(`/rotate?degree=${degree}&ccw=${ccw}`)
          .then(response => response.text())
          .then(message => {
            alert(message);
          });
      } else {
        alert("Please enter a valid degree (1-360).");
      }
    }
  </script>
</body>
</html>
    )rawliteral");
  });

  server.on("/distance", HTTP_GET, [](AsyncWebServerRequest *request) {
    dtostrf(distance, 6, 2, distance_buf);
    request->send(200, "text/plain", distance_buf);
  });

  server.on("/rotate", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("degree") && request->hasParam("ccw")) {
      int degree = request->getParam("degree")->value().toInt();
      if (degree > 0 && degree <= 360) { // Kiểm tra độ hợp lệ
        step_num = degree / 360.0 * STEPS;
        String ccw = request->getParam("ccw")->value();
        step_dir = (ccw == "true") ? -1 : 1;
        request->send(200, "text/plain", "Rotation successful");
      } else {
        request->send(400, "text/plain", "Invalid degree value. Must be between 1 and 360.");
      }
    } else {
      request->send(400, "text/plain", "Missing degree or ccw parameter.");
    }
  });

  server.begin();
}

void loop() {
  // Measure distance using ultrasonic sensor
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  distance = (duration * 0.034) / 2.0;

  // Debug to Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  delay(200);
  stepper.step(step_num * step_dir);
  delay(step_delay);
}
