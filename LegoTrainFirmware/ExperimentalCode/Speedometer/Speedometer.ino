#include "Speedometer.h"

#define ANALOG_SENSOR_PIN 32

Speedometer speedometer(ANALOG_SENSOR_PIN);

#define WINDOW_SIZE 1

float values[WINDOW_SIZE];
int position = 0;

float getAverage(){
  float sum = 0.0;
  for(int i = 0; i < WINDOW_SIZE; i++){
    sum+=values[i];
  }
  return sum/WINDOW_SIZE;
}

void setup() {
  Serial.begin(9600);
  delay(500);
  Serial.println("Starting setup");
  Serial.println("Setting up encoder");
  speedometer.setupEncoder();
  Serial.println("Activating HW Timer");
  setupTimerInterrupt();
  Serial.println("Ready to go");
}

void loop() {
    if(speedometer.updateAvailable()){
        values[position++]=speedometer.getSpeed();
        position %= WINDOW_SIZE;
        Serial.print(getAverage());
        Serial.println();
      }
}
