#include "PropulsionSystem.h"
#include <Arduino.h>

//#define DEBUG_PROPULSION

PropulsionSystem::PropulsionSystem(int _en, int _in1, int _in2){
  en = _en;
  in1 = _in1;
  in2 = _in2;
}

void PropulsionSystem::initPins(){
  pinMode(en,OUTPUT);
  
  pinMode(in1,OUTPUT);
  pinMode(in2,OUTPUT);

  stop();
}

void PropulsionSystem::moveForward(){
    currentDirection = 1;
  moveMotor();
}

void PropulsionSystem::moveBackward(){
  currentDirection = -1;
  moveMotor();
}

void PropulsionSystem::setDirection(int value){
  if(value > 0){
    moveForward();
  }
  else if(value < 0){
    moveBackward();
  }
  else{
    stop();
  }
  
}

void PropulsionSystem::moveMotor(){
  digitalWrite(en,HIGH);

  uint8_t pwmValue = (currentSpeed/100) * 255;
  
  if(currentDirection == 1){
    digitalWrite(in1,LOW);
    analogWrite(in2,pwmValue);
  }
  else if(currentDirection == -1){
    digitalWrite(in2,LOW);
    analogWrite(in1,pwmValue);
  }
}

void PropulsionSystem::stop(){
  currentDirection == 0;
  digitalWrite(en,LOW);
  
  digitalWrite(in1,LOW);
  digitalWrite(in2,LOW);
}

void PropulsionSystem::setSpeed(int newSpeed){
  currentSpeed = newSpeed;
  moveMotor();
}
