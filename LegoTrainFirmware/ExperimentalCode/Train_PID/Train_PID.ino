#include "TrainMotor.h"
#include "Credentials.h"
#include "Frontend.h"
#include "ControlLoop.h"
#include "SpeedMonitor.h"
#include "Speedometer.h"
#include "MagnetSensors.h"
#include "Autopilot.h"
#include "ActionButton.h"

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
ActionButton actionButton(BUTTON_PIN, LED_PIN);

enum controlModes
{
    MANUAL = 0,
    TARGET_OUTPUT = 1,
    TARGET_SPEED = 2,
    AUTONOMOUS = 3
};

unsigned long lastMotorSpeedUpdate = 0;

unsigned long lastInputSignal = 0;
#define INPUT_TIMEOUT 1000

void startPump(){
    Serial.println("Starting pump");
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    //http.begin("https://www.howsmyssl.com/a/check", ca); //HTTPS
    http.begin("http://192.168.178.95/fill");  //HTTP
    http.addHeader("Content-Type", "text/plain");
    int httpResponseCode = http.POST("");
     
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
        
    http.end();
    
}

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void stopCallback(){
    controlLoop.reset();
    newSetpoint = 0;
    motor.brake(); //Only works when moving forwards
    motor.setSpeed(0);
}

void handleStop()
{
    stopCallback();
    server.send(200, "application/json", "{\"mode\":" + String(controlLoop.getMode()) + "}");
}

void setStopIntterrupt();

Autopilot autopilot(&newSetpoint,&actionButton, &motor,&magnetSensors, &controlLoop,stopCallback,startPump,setStopIntterrupt);

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
        if(controlLoop.getMode() < 3){
            autopilot.reset();
        }
        server.send(200, "application/json", "{\"mode\":" + String(controlLoop.getMode()) + "}");
    }
    else if (server.method() == HTTP_GET)
    {
        server.send(200, "application/json", "{\"mode\":" + String(controlLoop.getMode()) + "}");
    }
}

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

bool stopInterruptTriggered = false;
void handleStopInterrupt()
    {
        detachInterrupt(MAGNET_BOTTOM_PIN);
        controlLoop.reset();
        newSetpoint = 0;
        motor.brake(); // Only works when moving forwards
        // motor.setSpeed(0);

        digitalWrite(INTERRUPT_CHECK_PIN, !digitalRead(INTERRUPT_CHECK_PIN));
        stopInterruptTriggered = true;
    }

    void updateOnStop(){
        if(stopInterruptTriggered){
            autopilot.switchState(BRAKING);
            stopInterruptTriggered = false;
        }
    }

void setStopIntterrupt(bool enabled)
{
  if(enabled){
    attachInterrupt(MAGNET_BOTTOM_PIN, handleStopInterrupt, FALLING);
  }
  else{
    detachInterrupt(MAGNET_BOTTOM_PIN);
  }
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Starting setup");
    actionButton.begin();
    actionButton.setStop();
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
    pinMode(INTERRUPT_CHECK_PIN, OUTPUT);
    digitalWrite(INTERRUPT_CHECK_PIN, LOW);
    Serial.println("Ready to go");
}

void loop()
{
    updateOnStop();
    updateMotor();
    server.handleClient();
    webSocket.loop();

    speedMonitor.setSpeed(speedometer.getSpeed());
    speedMonitor.setDistance(speedometer.getDistance());
    speedMonitor.setAutopilotState(autopilot.getState());
    speedMonitor.update();

    magnetSensors.update();
    actionButton.update();

    if(controlLoop.getMode() == AUTONOMOUS){
        autopilot.runStateMachine();
    }
    else{
      if(magnetSensors.sideWasTriggered()){
        stopCallback();
      }
    }
}

void updateMotor()
{
  
  if(controlLoop.getMode() != AUTONOMOUS){
    if ((millis() - lastInputSignal) > INPUT_TIMEOUT)
        {
            //Serial.println("Input Timeout");
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