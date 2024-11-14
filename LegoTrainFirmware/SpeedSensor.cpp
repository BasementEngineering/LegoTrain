#include "SpeedSensor.h"
#include "Arduino.h"

#define LOW_SPEED_TIMEOUT 1000 //ms aka 0,05 m/s
#define REED_DEBOUNCE_TIME 5
#define WHEEL_CIRCUMFERENCE 0.050265F 

#define SCALE_FACTOR 40

//Defining the static variables
int SpeedSensor::numberOfInstances = 0;
unsigned long SpeedSensor::lastRoundSignal;
unsigned long SpeedSensor::durationWindow[BUFFER_SIZE];
int SpeedSensor::slotCounter;

SpeedSensor::SpeedSensor(){
    myInstanceNumber = numberOfInstances;
    numberOfInstances++;

    lastRoundSignal = 0;
    slotCounter = 0;
    resetBuffer();
  }
  
  void SpeedSensor::begin(int pin){
    myPin = pin;
    pinMode(myPin,INPUT_PULLUP);
    attachInterrupt(myPin, interruptRoutine, FALLING);
  }

  void SpeedSensor::interruptRoutine(){
    unsigned long currentTime = millis();
    if(currentTime - lastRoundSignal > REED_DEBOUNCE_TIME){
      durationWindow[slotCounter] = currentTime - lastRoundSignal;
      slotCounter++;
      slotCounter%=BUFFER_SIZE;
      lastRoundSignal = currentTime;
    }
  }

  void SpeedSensor::update(){
    unsigned long currentTime = millis();
    //too slow
    if( (currentTime - lastRoundSignal) > LOW_SPEED_TIMEOUT){
      //lastRoundSignal = currentTime;
      durationWindow[slotCounter] = 0;
      slotCounter++;
      slotCounter%=BUFFER_SIZE;
      resetBuffer();
    }
  }

  void SpeedSensor::resetBuffer(){
    for(int i = 0; i < BUFFER_SIZE; i++){
      durationWindow[i] = 0;
    }
  }
  
  unsigned long SpeedSensor::getAvgDuration(){
    unsigned long sum = 0;
    for(int i = 0; i < BUFFER_SIZE; i++){
      sum += durationWindow[i];
    }
    return sum/BUFFER_SIZE;
  }

  float SpeedSensor::getSpeedMS(){
    unsigned long avgDuration = getAvgDuration();
    if(avgDuration == 0){
      return 0.0;
    }
    return WHEEL_CIRCUMFERENCE/(avgDuration*0.001);
  }
  
  float SpeedSensor::getSpeedKMH(){
    return 3.6 * getSpeedMS();
  }
  
  float SpeedSensor::getScaleSpeed(){
    return SCALE_FACTOR * getSpeedKMH();
  }

  float SpeedSensor::getSpeedCMS(){
    return 100 * getSpeedMS();
  }
