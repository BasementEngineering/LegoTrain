#define PUMP_ENABLE_PIN 15
#define PUMP_IN_A 13
#define PUMP_IN_B 12

#define FILL_TIME 1000

bool filling = false;

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "index.h"

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
}

void deactivatePump(){
  Serial.println("Disabling Pump");
  digitalWrite(PUMP_ENABLE_PIN, LOW);
  digitalWrite(PUMP_IN_A, LOW);
  digitalWrite(PUMP_IN_B, LOW);
}

void setupPump(){
  Serial.println("Setting up Pump");
  pinMode(PUMP_ENABLE_PIN, OUTPUT);
  pinMode(PUMP_IN_A, OUTPUT);
  pinMode(PUMP_IN_B, OUTPUT);
  deactivatePump();
}

void setup() {
  Serial.begin(115200);
  setupWiFi();
  setupPump();
  setupServer();
}

void loop(){
  server.handleClient();
  if (filling && ((millis() - fillingStart) > FILL_TIME) ) {
    filling = false;
    deactivatePump();
  }
}