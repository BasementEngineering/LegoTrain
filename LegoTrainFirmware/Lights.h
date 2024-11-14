#ifndef LIGHTS_H
#define LIGHTS_H

#include "OutputDevice.h"

class Lights:public OutputDevice{
  public: 
  Lights(int _pin);
  
  void setPower(int newPower);
  int getPower();
  void resetValues(){}
  
  private:
    int pin;
    int currentPwm = 0;
};

#endif
