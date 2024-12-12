#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Stepper.h>
// oke

// WiFi Credentials
const char* ssid = "GT3";
const char* password = "88886666";

// Create Web Server
AsyncWebServer server(80);

// Stepper Motor Pins (using 4-wire stepper motor)
#define stepPin1 D1
#define stepPin2 D2
#define stepPin3 D3
#define stepPin4 D4
#define stepsPerRevolution 2048  // Adjust this depending on your stepper motor

// Create Stepper Motor
Stepper stepper(stepsPerRevolution, stepPin1, stepPin2, stepPin3, stepPin4);

// Joystick Pin (A0 for reading analog value)
#define analogPin A0

// HTML interface
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Stepper Control</title>
  <style>
    button, select { font-size: 20px; padding: 10px; margin: 10px; }
    #analog-values { font-size: 18px; margin-top: 20px; }
  </style>
</head>
<body>
  <h1>Stepper Motor Control</h1>
  <h2>Rotate to Specific Angles</h2>
  <label for="angle">Choose an angle:</label>
  <select id="angle">
    <option value="45">45</option>
    <option value="90">90</option>
    <option value="135">135</option>
    <option value="180">180</option>
  </select>
  <button onclick="rotateToSelectedAngle()">Rotate</button>

  <h3 id="status">Status: Waiting...</h3>

  <div id="analog-values">
    <p><strong>Analog Value (A0):</strong> <span id="analog-value">0</span></p>
  </div>

  <script>
    function rotateToSelectedAngle() {
      const angle = document.getElementById("angle").value;
      fetch(`/rotate?angle=${angle}`)
      .then(response => response.text())
      .then(data => {
        document.getElementById("status").innerHTML = "Status: Rotated to " + angle;
      });
    }

    function updateAnalogValue() {
      fetch('/analog_value')
      .then(response => response.json())
      .then(data => {
        document.getElementById("analog-value").innerText = data.value;
      });
    }

    setInterval(updateAnalogValue, 500);  // Update analog value every 500ms
  </script>
</body>
</html>
)rawliteral";

// Handle rotating to specific angles
void handleRotateToAngle(AsyncWebServerRequest *request) {
  if (request->hasParam("angle")) {
    int angle = request->getParam("angle")->value().toInt();
    int steps = (stepsPerRevolution * angle) / 360;  // Calculate steps for given angle

    stepper.setSpeed(15);  // Set speed in RPM
    stepper.step(steps);
    request->send(200, "text/plain", "Rotated to " + String(angle));
  } else {
    request->send(400, "text/plain", "Bad Request");
  }
}

// Handle sending analog value from A0
void handleAnalogValue(AsyncWebServerRequest *request) {
  int analogValue = analogRead(analogPin);  // Read the analog value from A0

  // Send the analog value as JSON
  String json = "{";
  json += "\"value\":" + String(analogValue);
  json += "}";

  request->send(200, "application/json", json);
}

void setup() {
  // Serial for debugging
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected!");
  Serial.println(WiFi.localIP());  

  // Initialize Stepper Motor
  stepper.setSpeed(15);  // Set the speed of the motor to 60 RPM

  // Web Interface
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  // Handle rotation to specific angles
  server.on("/rotate", HTTP_GET, handleRotateToAngle);

  // Endpoint to send analog value from A0
  server.on("/analog_value", HTTP_GET, handleAnalogValue);

  // Start the web server
  server.begin();
}

void loop() {
  // Nothing needs to be in the loop as everything is handled in the server
}