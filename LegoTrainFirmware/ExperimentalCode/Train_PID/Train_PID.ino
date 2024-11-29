#include "TrainMotor.h"
#include "Credentials.h"
#include "Frontend.h"
#include "ControlLoop.h"
#include "SpeedMonitor.h"
#include "Speedometer.h"
#include "MagnetSensors.h"
#include "Autopilot.h"

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

#include <Pinout.h>

#include <HTTPClient.h>

int newSetpoint = 0;

TrainMotor motor(MOTOR_PIN);
ControlLoop controlLoop;
SpeedMonitor speedMonitor;
Speedometer speedometer(ANALOG_SENSOR_PIN);
MagnetSensors magnetSensors(MAGNET_BOTTOM_PIN, MAGNET_SIDE_PIN);

unsigned long lastMotorSpeedUpdate = 0;

unsigned long lastInputSignal = 0;
#define INPUT_TIMEOUT 1000

void startPump(){
    Serial.println("Starting pump");
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    //http.begin("https://www.howsmyssl.com/a/check", ca); //HTTPS
    http.begin("192.168.178.78/fill");  //HTTP
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST("{\"filling\":true}");
     
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
        
    http.end();
}

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void handleRoot()
{
    server.send(200, "text/html", htmlPage);
}

void handleMode()
{
    if (server.method() == HTTP_POST)
    {
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, server.arg("plain"));
        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.c_str());
            return;
        }
        controlLoop.setMode(doc["mode"]);
        server.send(200, "application/json", "{\"mode\":" + String(controlLoop.getMode()) + "}");
    }
    else if (server.method() == HTTP_GET)
    {
        server.send(200, "application/json", "{\"mode\":" + String(controlLoop.getMode()) + "}");
    }
}

void handleStop()
{
    controlLoop.reset();
    newSetpoint = 0;
    motor.setSpeed(0);
    server.send(200, "application/json", "{\"mode\":" + String(controlLoop.getMode()) + "}");
}

Autopilot autopilot(&newSetpoint,&magnetSensors,handleStop,startPump);

void parsePayload(uint8_t *payload, size_t length)
{
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }
    if (doc.containsKey("setpoint"))
    {
        if(controlLoop.getMode() < 3){
            newSetpoint = doc["setpoint"];
            lastInputSignal = millis();
        }
    }
}

uint8_t clientNum = 0;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        Serial.printf("[%u] Disconnected!\n", num);
        break;
    case WStype_CONNECTED:
    {
        IPAddress ip = webSocket.remoteIP(num);
        clientNum = num;
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        // webSocket.sendTXT(num, "Connected");
    }
    break;
    case WStype_TEXT:
        // Serial.printf("[%u] get Text: %s\n", num, payload);
        parsePayload(payload, length);
        break;
    }
}

void setupWiFi()
{
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
}

void setupWebServer()
{
    server.on("/", handleRoot);
    server.on("/mode", handleMode);
    server.on("/stop", handleStop);
    server.begin();
    Serial.println("HTTP server started");
}

void setupWebSocket()
{
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("WebSocket server started");
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Starting setup");
    delay(500);
    Serial.println("Setting up speed monitor");
    speedMonitor.begin();
    speedMonitor.setSpeed(40);
    speedMonitor.update();
    delay(500);
    Serial.println("Setting up encoder");
    speedometer.setupEncoder();
    speedometer.setInverted(true);
    Serial.println("Activating HW Timer");
    setupTimerInterrupt();

    setupWiFi();
    setupWebServer();
    setupWebSocket();

    magnetSensors.setup();
    motor.begin();
    Serial.println("Ready to go");
}

void loop()
{
    delay(10);
    updateMotor();
    server.handleClient();
    webSocket.loop();
    speedMonitor.setSpeed(speedometer.getSpeed());
    speedMonitor.setDistance(speedometer.getDistance());
    speedMonitor.update();
    magnetSensors.update();

    if(controlLoop.getMode() == 3){
        autopilot.runStateMachine();
    }
    else{
      if(magnetSensors.sideWasTriggered()){
        handleStop();
      }
    }
}

void updateMotor()
{
  
  if(controlLoop.getMode() != 3){
    if ((millis() - lastInputSignal) > INPUT_TIMEOUT)
        {
            Serial.println("Input Timeout");
            newSetpoint = 0;
            motor.setSpeed(0);
            return;
        }
    }

    if ((millis() - lastMotorSpeedUpdate) > 100)
    {
        controlLoop.setInput(speedometer.getSpeed());
        controlLoop.setTarget(newSetpoint);
        motor.setSpeed(controlLoop.calculateOutput());

        String statusJson = controlLoop.getStatusJson();
        webSocket.sendTXT(clientNum, statusJson);

        lastMotorSpeedUpdate = millis();
    }
}