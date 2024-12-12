#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>

// WiFi Credentials
const char* ssid = "ASPIRE";
const char* password = "12345678";

// DC Motor control pins
#define motorPin1 D6
#define motorPin2 D7
#define motorSpeedPin D8 // PWM pin for speed control

// Joystick pins
#define joystickXPin A0  // Analog pin for X-axis
#define joystickSWPin D5 // Digital pin for button SW

// Create Web Server
AsyncWebServer server(80);

// HTML interface
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>DC Motor and Joystick Control</title>
  <style>
    button, input { font-size: 20px; padding: 10px; margin: 10px; }
    #joystick-values { font-size: 18px; margin-top: 20px; }
  </style>
</head>
<body>
  <h1>DC Motor and Joystick Control</h1>
  <button onclick="sendData('forward')">Rotate Forward</button>
  <button onclick="sendData('reverse')">Rotate Reverse</button>
  <input type="range" min="0" max="255" value="0" id="speedSlider" oninput="updateSpeed(this.value)">
  <h3 id="status">Status: Waiting...</h3>
  
  <div id="joystick-values">
    <p><strong>Joystick X Value:</strong> <span id="joystickXValue">0</span></p>
    <p><strong>Joystick Button:</strong> <span id="joystickSWValue">Released</span></p>
  </div>

  <script>
    function sendData(action) {
      fetch(`/control?action=${action}`)
      .then(response => response.text())
      .then(data => {
        document.getElementById("status").innerHTML = "Status: " + data;
      });
    }

    function updateSpeed(speed) {
      fetch(`/speed?value=${speed}`)
      .then(response => response.text())
      .then(data => {
        document.getElementById("status").innerHTML = "Speed: " + data;
      });
    }

    // Function to get Joystick data
    function updateJoystickData() {
      fetch('/joystick_data')
      .then(response => response.json())
      .then(data => {
        document.getElementById("joystickXValue").innerText = data.joystickXValue;
        document.getElementById("joystickSWValue").innerText = data.joystickSWValue ? "Pressed" : "Released";
      });
    }

    setInterval(updateJoystickData, 200);  // Update joystick data every 0.2 seconds
  </script>
</body>
</html>
)rawliteral";

// Handle DC Motor Control
void handleMotorControl(AsyncWebServerRequest *request) {
  if (request->hasParam("action")) {
    String action = request->getParam("action")->value();
    
    if (action == "forward") {
      // Rotate motor forward
      digitalWrite(motorPin1, HIGH);
      digitalWrite(motorPin2, LOW);
      request->send(200, "text/plain", "Motor rotating forward");
    } else if (action == "reverse") {
      // Rotate motor reverse
      digitalWrite(motorPin1, LOW);
      digitalWrite(motorPin2, HIGH);
      request->send(200, "text/plain", "Motor rotating reverse");
    }
  } else {
    request->send(400, "text/plain", "Bad Request");
  }
}

// Handle Motor Speed Control
void handleMotorSpeed(AsyncWebServerRequest *request) {
  if (request->hasParam("value")) {
    int speed = request->getParam("value")->value().toInt();
    analogWrite(motorSpeedPin, speed);
    request->send(200, "text/plain", String(speed));
  } else {
    request->send(400, "text/plain", "Bad Request");
  }
}

// Handle sending Joystick data
void handleJoystickData(AsyncWebServerRequest *request) {
  int joystickXValue = analogRead(joystickXPin);       // Read X-axis value
  bool joystickSWValue = !digitalRead(joystickSWPin); // Read SW value (active LOW)

  // Send the joystick data as JSON
  String json = "{";
  json += "\"joystickXValue\":" + String(joystickXValue) + ",";
  json += "\"joystickSWValue\":" + String(joystickSWValue);
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

  // Initialize motor control pins
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorSpeedPin, OUTPUT);
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);  // Motor off initially

  // Initialize joystick pins
  pinMode(joystickSWPin, INPUT_PULLUP);

  // Define server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/control", HTTP_GET, handleMotorControl);

  server.on("/speed", HTTP_GET, handleMotorSpeed);

  server.on("/joystick_data", HTTP_GET, handleJoystickData);

  // Start server
  server.begin();
}

void loop() {
  // Nothing needed in loop as everything is handled via server
}
