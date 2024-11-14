#include "SpeedMonitor.h"

SpeedMonitor speedMonitor;

unsigned long lastRandomUpdate = 0;
float currentSpeed = 0.0;
float distance = 0.0;

void setup() {
  Serial.begin(9600);
  delay(500);
  Serial.println("Starting setup");
  delay(500);
  Serial.println("Setting up speed monitor");
  speedMonitor.begin();
  
  speedMonitor.setSpeed(40);
  speedMonitor.update();
  delay(500);
}

void loop() {
  delay(10);
  speedMonitor.update();
  if(millis()-lastRandomUpdate > 1000){
    lastRandomUpdate = millis();
    distance += currentSpeed;
    currentSpeed = random(0,10)*0.1;

    speedMonitor.setSpeed(currentSpeed);
    speedMonitor.setDistance(distance);
  }
}
