#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

const char* ssid = "Nicolai";  
const char* password = "**********";  


const int stepPin = 18; 
const int dirPin = 35; 
const int sleepPin = 33;
const int buttonPin = 16;
 
const int speed = 500;

const float revsToBottom = 15.0;
const float revsToSteps = 200.0;

float curtainPos = 0.0;
float move = 0.0;

int val = 0;

const char* hostname = "Curtain1";

String prevValue = "";

WebServer server(80);

const int servoPin = 12;

void connectWifi(){
  Serial.println("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  if (!MDNS.begin(hostname)) {
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");
  Serial.println("Hostname: ");
  Serial.print(hostname);


  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

  server.on("/a", handle_input); 
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(stepPin,OUTPUT); 
  pinMode(dirPin,OUTPUT);
  pinMode(sleepPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  connectWifi();
  while(true){
    val = digitalRead(buttonPin);
    if(val == 0){
      break;
    }
    digitalWrite(stepPin,HIGH);
    delayMicroseconds(5000);
    digitalWrite(stepPin,LOW);
    delayMicroseconds(5000);
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }
  else{
    server.handleClient();
  }
}

void handle_input() {
  if (server.hasArg("v")) {
    int sensorValue = server.arg("v").toInt();
    Serial.print(sensorValue*100.0/255);
    Serial.print("  |  ");
    Serial.println(sensorValue);
    if(prevValue != ""){
      prevValue = prevValue + ", " + sensorValue;
    }
    else{
      prevValue = sensorValue;
    }
    float value = revsToBottom*revsToSteps/255*sensorValue;
    String response = "<html><body><h1>Sensor Value: " + String(prevValue) + "</h1></body></html>";
    server.send(200, "text/html", response);
    if(value != curtainPos){
      if(value > curtainPos){
        digitalWrite(dirPin,HIGH);
        move = value - curtainPos;
        curtainPos = value;
      }
      else if(value < curtainPos){
        digitalWrite(dirPin,LOW);
        move = curtainPos - value;
        curtainPos = value;
      }
      for(int x = 0; x <= move; x++) {
        digitalWrite(stepPin,HIGH); 
        delayMicroseconds(speed); 
        digitalWrite(stepPin,LOW); 
        delayMicroseconds(speed); 
      }
    }
  } 
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}


