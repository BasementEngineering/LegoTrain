#define PUMP_ENABLE_PIN 15
#define PUMP_IN_A 13
#define PUMP_IN_B 12

#define LED_PIN 2 //D4
#define POTI_PIN A0
#define TEST_BUTTON_PIN 16

int fillInterval = 1000;

bool filling = false;

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "index.h"

#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel strip(1, LED_PIN, NEO_GRB + NEO_KHZ800);

ESP8266WebServer server(80);
unsigned long fillingStart;

void handleFillPost() {
  if (!filling) {
    filling = true;
    fillingStart = millis();
    activatePump();
    server.send(200, "text/plain", "Pump activated");
  } else {
    server.send(200, "text/plain", "Pump is already running");
  }
}

void handleFillGet() {
  if (filling) {
    server.send(200, "text/plain", "Pump is running");
  } else {
    server.send(200, "text/plain", "Pump is not running");
  }
}

void handleRoot() {
  server.send(200, "text/html", index_html);
}

#include "Credentials.h"

void setupWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void setupServer() {
  server.on("/fill", HTTP_POST, handleFillPost);
  server.on("/fill", HTTP_GET, handleFillGet);
  server.on("/test",HTTP_GET,handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void activatePump(){
  Serial.println("Enabling Pump");
  digitalWrite(PUMP_ENABLE_PIN, HIGH);
  digitalWrite(PUMP_IN_A, HIGH);
  digitalWrite(PUMP_IN_B, LOW);
  strip.setPixelColor(0, 0, 255, 0);
  strip.show();
}

void deactivatePump(){
  Serial.println("Disabling Pump");
  digitalWrite(PUMP_ENABLE_PIN, LOW);
  digitalWrite(PUMP_IN_A, LOW);
  digitalWrite(PUMP_IN_B, LOW);
  strip.setPixelColor(0, 255, 0, 0);
  strip.show();
}

void setupPump(){
  Serial.println("Setting up Pump");
  pinMode(PUMP_ENABLE_PIN, OUTPUT);
  pinMode(PUMP_IN_A, OUTPUT);
  pinMode(PUMP_IN_B, OUTPUT);
  deactivatePump();
}

unsigned long lastTimingUpdate = 0;

void updateTiming(){
  if((millis() - lastTimingUpdate) > 500){
    lastTimingUpdate = millis();
    int potiValue = analogRead(POTI_PIN);
    
    int newFillInterval = map(potiValue, 0, 1023, 1000, 5000);
    if(newFillInterval != fillInterval){
      fillInterval = newFillInterval;
      Serial.println("New Fill Interval: " + String(fillInterval));
    }
  }
}

bool previousButtonState = LOW;
unsigned long lastButtonCheck = 0;

void checkInput() {
  if((millis() - lastButtonCheck) > 100){
    lastButtonCheck = millis();

    if (!previousButtonState && digitalRead(TEST_BUTTON_PIN)) {
      activatePump();
      filling = true;
      fillingStart = millis();
    }

    previousButtonState = digitalRead(TEST_BUTTON_PIN);
  }
}

void setup() {
  Serial.begin(115200);
  setupWiFi();
  setupPump();
  setupServer();
  pinMode(POTI_PIN, INPUT);
  pinMode(TEST_BUTTON_PIN, INPUT);
  strip.begin();
  strip.show();
  strip.setPixelColor(0, 255, 0, 0);
  strip.show();
}

void loop(){
  server.handleClient();
  if (filling && ((millis() - fillingStart) > fillInterval) ) {
    filling = false;
    deactivatePump();
  }

  updateTiming();
  checkInput();
}