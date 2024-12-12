// ĐỘNG CƠ SERVO 
// dã test oke
// Chọn esp8266 => NodeMCU 1.0 (ESP-12E Module)
#include <Servo.h>
Servo myservo;                 

// Wifi Nên cắm ở cổng bên trái của máy tính (cổng bên trái và phía trên gần với màn hình) thì nó mới nhận 
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
// Servo dây cam nối với D4 của wifi , dây đỏ nối 3V3 của wifi, dây đà nối GND của wifi
ESP8266WebServer server(80);            
// Đối với wifi khi setup cổng thì cần thêm A với D ví dụ A0, D4 

// Joystick 
int Joystick_x = A0; // chân x của joystick      (chân A0) (A0 nối với A0 của wifi, VCC nối 3V3 của wifi, GND nối GND của wifi)   
// int Joystick_y = A3; // chân y của joystick   (chân A3) // Không dùng chân A3 vì không có 
int Joystick_sw = D3; // chân sw của joystick     (nối với chân D3 của wifi)                 
// Joystick_sw Để bình thường là 1 , bấm xuống là 0 
// Không dùng Joystick_y vẫn chạy bình thường , nó chỉ là không đọc giá trị y thôi 

void setup() {
  Serial.begin(9600); 
  delay(1000);
  WiFi.begin("GT3", "88886666"); //SSID && Pasword
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.print(".");
  }
  Serial.print("IP: ");
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
//                "<label>Value Gas Sensor</label><br>"
//                "<textarea id='serialMonitor' style='font-family: monospace; width: 100%; height: 300px; white-space: pre; '></textarea>"
//                "<label>Degrees : </label>"
//                "<input type='text' name='degrees' id = 'degrees'>"
//                "<input type='button' value='Send' onclick = sendValue()>"
  // myservo.attach(D4); // D4 của wifi 
  myservo.attach(D4, 500, 2400); // fix không quay đủ 180 độ 
  pinMode(Joystick_x, INPUT); // A0 Setup chân cho x  
  pinMode(Joystick_sw, INPUT); // D0 Setup chân cho sw 
  setupRoutes();
  server.begin();  // Bắt đầu web server
}

// Gửi giá trị đọc được từ cảm biến lên web 
void handleReadSerial() {
  int x,y,sw; //khai báo các biến 
  x=analogRead(Joystick_x); //đọc giá trị từ chân Analog x // A0 
  // y=analogRead(Joystick_y); //đọc giá trị từ chân Analog y
  sw=digitalRead(Joystick_sw); //đọc tín hiệu từ chân digital sw // D3
  server.send(200, "text/plain", "x : " + String(x) + " , sw : " + String(sw) + "\n");

  Serial.print(x);
  Serial.print("\t");
  Serial.print(y);
  Serial.print("\t");
  Serial.print(sw);
  Serial.print("\n");
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
  delay(200);  // Đợi để hệ thống xử lý các yêu cầu khác
  if (true) {
    //quay servo thuận theo góc INPUT
    myservo.write(receivedValue);
    Serial.println("Received Value: " + String(receivedValue));
    delay(500);
    //chỉnh servo về vị trí 0 độ
    myservo.write(0);
    delay(500);
  }
}
