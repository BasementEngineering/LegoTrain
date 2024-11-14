#include "AnalogSensor.h"
#include <Arduino.h>
#include "config.h"

 AnalogSensor::AnalogSensor(int analogPin, float factor,int windowSize,int updateInterval,int zeroOffset, int decimalPlaces){
  _analogPin = analogPin;
  _factor = factor;
  _windowSize = windowSize;
  _updateInterval = updateInterval;
  _zeroOffset = zeroOffset;
  _decimalPlaces = decimalPlaces;

  lastUpdate = 0;
  position = 0;

  dataBuffer = new unsigned int[windowSize];

  pinMode(_analogPin,INPUT);
 }

 AnalogSensor::~AnalogSensor(){
  if(dataBuffer!=NULL){
    delete[] dataBuffer;
  }
 }
 
 void AnalogSensor::update(){
  if((millis() - lastUpdate) > _updateInterval){
    lastUpdate = millis();
    addValue(getValue());
  }
 }

 float AnalogSensor::getValue(){
  unsigned int rawValue = analogRead(_analogPin);
  if(rawValue < _zeroOffset){
    return 0.00;
  }
  else{
    return (float)(rawValue-_zeroOffset) * _factor;
  }
 }

 void AnalogSensor::addValue(float newValue){
    int storeValue = newValue * pow(10,_decimalPlaces);
    dataBuffer[position] = storeValue;
    position++;
    position%=_windowSize;
  }
 
 float AnalogSensor::getAvg(){
    unsigned long sum = 0;
    #if DEBUG_SENSORS
    DEBUG_SERIAL.print("[");
    #endif
    for(int i = 0;i<_windowSize;i++){
     sum+=dataBuffer[i];
     #if DEBUG_SENSORS
     DEBUG_SERIAL.print(dataBuffer[i]);
     DEBUG_SERIAL.print(",");
     #endif
    }
    #if DEBUG_SENSORS
    DEBUG_SERIAL.print("]");
    #endif
    unsigned int avg = sum/_windowSize;
    #if DEBUG_SENSORS
    DEBUG_SERIAL.println("getAvg()");
    DEBUG_SERIAL.println((float)(avg/pow(10,_decimalPlaces)));
    #endif
    return (float)(avg/pow(10,_decimalPlaces));
 }
