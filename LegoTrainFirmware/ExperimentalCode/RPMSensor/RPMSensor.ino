#define REED_PIN 32
#include "SpeedSensor.h"

SpeedSensor mySpeedSensor;

void setup() {
  Serial.begin(115200);
  Serial.println("Hello World");
  mySpeedSensor.begin(REED_PIN);
}

unsigned long lastPrintUpdate = 0;

void loop() {
  mySpeedSensor.update();
  if(millis() - lastPrintUpdate > 100){
    lastPrintUpdate = millis();
    Serial.println(mySpeedSensor.getScaleSpeed());
  }
}
