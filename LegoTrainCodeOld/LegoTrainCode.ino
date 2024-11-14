/*
 * ! The Pin Markings on the WEMOS D1 Mini Board don't match the GPIO numbers !
 * Do not use GPIO 0 aka. D3 as it is used for flashing programs.
 * 
 * Motor Pins:
 * EN  D1 = GPIO 05
 * IN1 D8 = GPIO 15
 * In2 D7 = GPIO 13
 * In3 D6 = GPIO 12
 * IN4 D5 = GPIO 14
 * 
 * LED Pin:
 * D4 = GPIO 2
 * 
 * ADC (Voltmeter) Pin:
 * A0 ADC0
 */

#define MOTOR_EN 5
#define MOTOR_IN1 16
#define MOTOR_IN2 14
#define TREE_PIN_1 2 //D4
#define TREE_PIN_2 4 //D2
#define WHITE_PIN 5 //D1
 
#define MODULE_RX 13 // TX
#define MODULE_TX 15 // RX

#include "PropulsionSystem.h"
PropulsionSystem propulsionSystem(MOTOR_EN,MOTOR_IN1,MOTOR_IN2);

#include "Lights.h"
Lights lights(TREE_PIN_1, TREE_PIN_2, WHITE_PIN);

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "backend.h"

#define APSSID "DampfmaschineOnSpeed"
#define APPSK  "ImFastAsFuckB0i"
//#define AP_MODE

const char *ssid = APSSID;
const char *password = APPSK;

//helper function decalarations (full implementations are further down)
bool wifiOnline = false;
void setupWifiAp(); // to open a wifi access point

void setup(){
  Serial.begin(9600);
  Serial.println("Starting Setup");
  propulsionSystem.initPins();
  lights.initPins();

  Serial.println("Starting Backend");
  setupBackend(&propulsionSystem,&lights);
  Serial.println("Ready");
}

void loop(){
  if(wifiOnline){
    updateBackend();
  } else{
    setupWifiAp();
  }
  lights.update();
  yield();
}

unsigned long lastWifiUpdate = 0;
int nextWaitInterval = 1000;
int attemptCounter = 0;

void setupWifiAp(){
  if( (millis() - lastWifiUpdate) > nextWaitInterval){
    lastWifiUpdate = millis();
  #ifdef AP_MODE
    WiFi.softAP(ssid, password);
    #ifdef DEBUG
      Serial.print("Access Point \"");
      Serial.print(ssid);
      Serial.println("\" started");
      Serial.print("IP address:\t");
      Serial.println(WiFi.softAPIP());         // Send the IP address of the ESP8266 to the computer
    #endif
    wifiOnline = true;
  #endif
  #ifndef AP_MODE
    if(attemptCounter == 0){
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
      attemptCounter++;
    }
    if(attemptCounter > 0){
      if(WiFi.waitForConnectResult() == WL_CONNECTED){
        wifiOnline = true;
      }
      else{
        ESP.restart();
      }
    }
  #endif
  }
  
}
