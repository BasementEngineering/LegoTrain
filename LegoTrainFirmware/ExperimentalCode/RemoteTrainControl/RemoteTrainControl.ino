#include "TrainMotor.h"
#include "Credentials.h"
#include "Frontend.h"

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

#define MOTOR_PIN 26
#define ANALOG_SENSOR_PIN 32
#define SPEED_POTI_PIN 34

TrainMotor motor(MOTOR_PIN);

unsigned long lastMotorSpeedUpdate = 0;
int currentSpeed = 0;
int setpoint = 0;
int speed = 0;

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void handleRoot() {
    server.send(200, "text/html", htmlPage);
}

void parsePayload(uint8_t * payload, size_t length) {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }
    if (doc.containsKey("speed")) {
        speed = doc["speed"];
    }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
            webSocket.sendTXT(num, "Connected");
        }
            break;
        case WStype_TEXT:
            //Serial.printf("[%u] get Text: %s\n", num, payload);
            parsePayload(payload, length);
            break;
    }
}

void setupWiFi() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
}

void setupWebServer() {
    server.on("/", handleRoot);
    server.begin();
    Serial.println("HTTP server started");
}

void setupWebSocket() {
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("WebSocket server started");
}

void setup() {
    Serial.begin(9600);
    Serial.println("Starting setup");
    delay(500);
    motor.begin();
    Serial.println("Ready to go");

    setupWiFi();
    setupWebServer();
    setupWebSocket();
}

void loop() {
    delay(10);
    updateMotor();
    server.handleClient();
    webSocket.loop();
}

void updateMotor(){
  if( (millis() - lastMotorSpeedUpdate) > 100){
  if(speed < 10 && speed > -10){
    speed = 0;
    currentSpeed = 0;
  }
  setpoint = speed;
  currentSpeed = speed;

  /*if(currentSpeed < setpoint){
    currentSpeed++;
    if(currentSpeed > 100){
      currentSpeed = 100;
    }
  }
  else if(currentSpeed > setpoint){
    currentSpeed--;
    if(currentSpeed < -100){
      currentSpeed = -100;
    }
  }*/
  motor.setSpeed(currentSpeed);
  
  Serial.print("Setpoint:");
  Serial.print(setpoint);
  Serial.print(",");
  Serial.print("Output:");
  Serial.print(currentSpeed);
  Serial.println("");


  lastMotorSpeedUpdate = millis();
  }
  
}