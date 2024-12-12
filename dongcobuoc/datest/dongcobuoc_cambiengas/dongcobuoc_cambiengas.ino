// oke đã check

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Stepper.h>

// Network credentials
const char *SSID = "ESP8266-AP";
const char *PSWD = "";

// Gas Sensor
const int GAS_SENSOR_PIN = A0;
int gas_value;
char gas_buf[8];

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

  // Gas sensor
  pinMode(GAS_SENSOR_PIN, INPUT);

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
    img.sensor {
      width: 32px;
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
  <h1>Gas Sensor</h1>
  <p>
    <span class="sensor-labels">Gas Level</span>
    <span id="gas_value"></span>
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
    <input type="checkbox" name="CCW-checkbox" id="CCW-checkbox">
    <label for="CCW-checkbox">CCW</label>
    <input type="button" id="submit-btn" value="Rotate" class="button" onclick="rotateStepper()">
  </form>
  <script>
    setInterval(() => {
      fetch('/gas_value')
        .then(response => response.text())
        .then(gas_value => {
          document.getElementById("gas_value").textContent = gas_value;
        });
    }, 200);

    let options = document.querySelectorAll("select#rotate-degree option");
    let ccw_checkbox = document.getElementById("CCW-checkbox");
    function rotateStepper() {
      let n_options = options.length;
      for (let it = 0; it < n_options; it++) {
        if (options[it].selected) {
          if (ccw_checkbox.checked) {
            fetch("/rotate?degree=" + options[it].value + "&ccw=true");
          } else {
            fetch("/rotate?degree=" + options[it].value + "&ccw=false");
          }
          break;
        }
      }
    }
  </script>
</body>
</html>
    )rawliteral");
  });

  server.on("/gas_value", HTTP_GET, [](AsyncWebServerRequest *request) {
    dtostrf(gas_value, 6, 2, gas_buf);
    request->send(200, "text/plain", gas_buf);
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
  gas_value = analogRead(GAS_SENSOR_PIN);

  // Debug to Serial Monitor
  Serial.print("Gas Level: ");
  Serial.println(gas_value);

  delay(200);
  stepper.step(step_num * step_dir);
  delay(step_delay);
}
