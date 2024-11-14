#include "Lights.h"
#include "config.h"

#include <Arduino.h>

Lights::Lights(int _pin){
  setDangerous(false);
  pin = _pin;
  pinMode(pin,OUTPUT);
}

void Lights::setPower(int newPower){
  if( (newPower >= 100) ){
    currentPwm = maxPwm;
  }
  else if( newPower <= 0){
    currentPwm = 0;
  }
  else{
    currentPwm = powerToPwm(newPower,minPwm,maxPwm);
  }
  analogWrite(pin,currentPwm);
}

int Lights::getPower(){
  #if DEBUG_LIGHTS
  DEBUG_SERIAL.println("Getting power lights");
  DEBUG_SERIAL.print("currentPwm: ");DEBUG_SERIAL.println(currentPwm);
  DEBUG_SERIAL.print("power: ");DEBUG_SERIAL.println(pwmToPower(currentPwm,minPwm,maxPwm));
  #endif
  return pwmToPower(currentPwm,minPwm,maxPwm);
}
