#include "TrainMotor.h"
#include "Credentials.h"
#include "Frontend.h"
#include "ControlLoop.h"

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

#include <Pinout.h>

TrainMotor motor(MOTOR_PIN);
ControlLoop controlLoop;

unsigned long lastMotorSpeedUpdate = 0;
//int currentSpeed = 0;
//int setpoint = 0;
int speed = 0;

unsigned long lastInputSignal = 0;
#define INPUT_TIMEOUT 1000

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void handleRoot() {
    server.send(200, "text/html", htmlPage);
}

void handleMode(){
    if(server.method() == HTTP_POST){
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, server.arg("plain"));
        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.c_str());
            return;
        }
        controlLoop.setMode(doc["mode"]);
        server.send(200, "application/json", "{\"mode\":" + String(controlLoop.getMode()) + "}");
    }else if(server.method() == HTTP_GET){
        server.send(200, "application/json", "{\"mode\":" + String(controlLoop.getMode()) + "}");
    }
}

void handleStop(){
    lastInputSignal = millis();
    controlLoop.reset();
    speed = 0;
    motor.setSpeed(0);
    server.send(200, "application/json", "{\"mode\":" + String(controlLoop.getMode()) + "}");
}

void parsePayload(uint8_t * payload, size_t length) {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }
    if (doc.containsKey("setpoint")) {
        speed = doc["setpoint"];
        lastInputSignal = millis();
    }
}

uint8_t clientNum = 0;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            clientNum = num;
            Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
            //webSocket.sendTXT(num, "Connected");
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
    server.on("/mode", handleMode);
    server.on("/stop", handleStop);
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
  if( (millis() - lastInputSignal) > INPUT_TIMEOUT){
    Serial.println("Input Timeout");
    speed = 0;
    currentSpeed = 0;
    motor.setSpeed(0);
    return;
  }

  if( (millis() - lastMotorSpeedUpdate) > 100){
  controlLoop.setTarget(speed);
  motor.setSpeed(controlLoop.calculateOutput());
  //controlLoop.printStatus();
  
  String statusJson = controlLoop.getStatusJson();
  webSocket.sendTXT(clientNum , statusJson);
  //webSocket.sendTXT(5, "Hello");

  lastMotorSpeedUpdate = millis();
  }
  
}