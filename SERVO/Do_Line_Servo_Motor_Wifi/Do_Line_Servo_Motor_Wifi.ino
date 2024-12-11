// ĐỘNG CƠ SERVO 
// Chọn esp8266 => NodeMCU 1.0 (ESP-12E Module)
#include <Servo.h>
Servo myservo;                 

// Wifi Nên cắm ở cổng bên trái của máy tính (cổng bên trái và phía trên gần với màn hình) thì nó mới nhận 
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
// Servo dây cam nối với D4 của wifi , dây đỏ nối 3V3 của wifi, dây nâu nối GND của wifi
ESP8266WebServer server(80);    

// Đối với wifi khi setup cổng thì cần thêm A với D ví dụ A0, D4 
// Cảm biến dò Line 
int SensorLine = D3; // D0 nối với chân số D3 của wifi (1 là đen , 0 là trắng) , VCC nối 3V3 của wifi, GND nối GND của wifi
// int SensorLine = A0; // A0 Chân đọc giá trị của cảm biến dò line (1024 là đen , 0 là trắng)
// Đổi chân ở đây thì đổi ở dưới handleReadSerial nữa 
int LineSensorValue = 0;          
// + A0 
//     + Đen là 1 
//     + Trắng là 0 
// + D0 
//     + Đen là 1024 
//     + Trắng là 0 

void setup() {
  Serial.begin(9600); 
  delay(1000);
  WiFi.begin("Airport", "truongbk"); //SSID && Pasword
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connecting...");
  }
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());
  server.on("/", [] {
    server.send(200, "text/html",
                "<!DOCTYPE html>"
                "<html>"
                "<head>"
                "<title>Serial Monitor</title>"
                "<script>"
                "window.onload = function() {"
                "readSerial();"
                "};"
                "function readSerial() {"
                "var serialMonitor = document.getElementById('serialMonitor');"
                "var xhttp = new XMLHttpRequest();"
                "xhttp.onreadystatechange = function() {"
                "if (this.readyState == 4 && this.status == 200) {"
                "serialMonitor.value = this.responseText;"
                "serialMonitor.scrollTop = serialMonitor.scrollHeight;"
                "setTimeout(readSerial, 500);"
                "}"
                "};"
                "xhttp.open('GET', 'readSerial', true);"
                "xhttp.send();"
                "}"
                "function sendValue(){"
                "var degreesInput = document.getElementById('degrees');"
                "var xhttp2 =  new XMLHttpRequest();"
                "var degrees = degreesInput.value;"
                "xhttp2.open('POST', 'sendValue', true);"
                "xhttp2.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');"
                "xhttp2.send('degrees=' + degrees);"
                "}"
                "</script>"
                "</head>"
                "<body>"
                
                "<label>Value Sensor</label><br>"
                "<input type='text' id='serialMonitor'>"
                "<br><br>"
                "<label>Degrees : </label>"
                "<input type='text' name='degrees' id = 'degrees'>"
                "<input type='button' value='Send' onclick = sendValue()>"
                "</body>"
                
                "</html>");
  });


//  
//                "<label>Value Line Sensor</label><br>"
//                "<textarea id='serialMonitor' style='font-family: monospace; width: 100%; height: 300px; white-space: pre; '></textarea>"
//                "<label>Degrees : </label>"
//                "<input type='text' name='degrees' id = 'degrees'>"
//                "<input type='button' value='Send' onclick = sendValue()>"
  // myservo.attach(D4); // D4 của wifi 
  myservo.attach(D4, 500, 2400); // fix không quay đủ 180 độ 
  pinMode(SensorLine, INPUT); // D0 hay A0 thì cũng để như thế này 
  // D0 Thêm setup nếu dùng D0 => D3 , còn nếu dùng A0 thì không cần 
  // A0 Chân analog cũng có thể setup pinMode (có mấy cái không cần setup cũng được) 
  setupRoutes();
  server.begin();  // Bắt đầu web server
}

// Gửi giá trị đọc được từ cảm biến lên web 
void handleReadSerial() {
  // LineSensorValue = digitalRead(SensorLine); // D0 nối với D3 wifi 
  LineSensorValue = analogRead(SensorLine); // A0 nối với A0 wifi 
  server.send(200, "text/plain", String(LineSensorValue) + "\n");
}

// Nhận giá trị độ từ web để điều khiển servo 
int receivedValue = 0;
void handleSendValue() {
  if (server.hasArg("degrees")) {
    receivedValue = server.arg("degrees").toInt();
  }
}

void setupRoutes() {
  server.on("/readSerial", handleReadSerial);
  server.on("/sendValue", handleSendValue);
}
void loop() {
  // Xử lý các yêu cầu từ client
  server.handleClient();
  delay(500);  // Đợi để hệ thống xử lý các yêu cầu khác
  if (true) {
    //quay servo thuận theo góc INPUT
    myservo.write(receivedValue);
    Serial.println(receivedValue);
    delay(1000);
    //chỉnh servo về vị trí 0 độ
    myservo.write(0);
    delay(1000);
  }
}
