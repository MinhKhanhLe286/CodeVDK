#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);                           
Servo myservo; 
//**
int trig = D6;
int echo = D7;
unsigned long duration = 0;
int distance = 0; 
//**                

void setup() {
  //**
  // put your setup code here, to run once:
  pinMode(trig, OUTPUT); 
  pinMode(echo, INPUT); 
  //**
  Serial.begin(9600); 
  delay(1000);
  WiFi.begin("Redmi", "06042002"); //SSID && Pasword
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println(".");
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
                "<label>Value sieu am : </label><br>"
                "<input type='text' id='serialMonitor'  style='width:100%; font-family: monospace; display:block; ' />"
                "<label>Degrees : </label>"
                "<input type='text' name='degrees' id = 'degrees'>"
                "<input type='button' value='Send' onclick = sendValue()>"
                "</body>"
                "</html>");
  });
  myservo.attach(D4 , 500 , 2400);
  setupRoutes();
  server.begin();  // Báº¯t Ä‘áº§u web server
}
void handleReadSerial() {
//  //phat tin hieu
  digitalWrite(trig, 0);
  delayMicroseconds(2);
  digitalWrite(trig, 1);
  delayMicroseconds(10);
  digitalWrite(trig, 0); 
  duration = pulseIn(echo, 1); 
  distance = int(duration*0.034/2); 
//  if (Serial.available()>0) {
//    int distance = Serial.parseInt();
//    distance = analogRead(A0); 
    Serial.print(distance);
    Serial.println("cm");
    //**
    server.send(200, "text/plain", "distance : " + String(distance) + "cm\n");
  

}
int receivedValue = 0;
void handleSendValue() {
  if (server.hasArg("degrees")) {
    receivedValue = server.arg("degrees").toInt();
  }
  else receivedValue =0;
}

void setupRoutes() {
  server.on("/readSerial", handleReadSerial);
  server.on("/sendValue", handleSendValue);
}
void loop() {
  server.handleClient();
  delay(500); 
  if (true) {
    //quay servo thuáº­n theo gÃ³c INPUT
    myservo.write(receivedValue);
    delay(1000);
    //chá»‰nh servo vá» vá»‹ trÃ­ 0 Ä‘á»™
    myservo.write(0);
    delay(1000);
  }
}
