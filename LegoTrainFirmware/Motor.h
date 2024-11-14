#ifndef MOTOR_H
#define MOTOR_H

#include "OutputDevice.h"

#include <Arduino.h>

class Motor:public OutputDevice{
  public:
  Motor(int _in1, int _in2, int _en=-1 );
  void initPins();
  void update();
  bool targetReached();

  void enable();
  void disable();
  
  void setPower(int newPower); //Sets target
  int getPower(); //Delivers actual current output power
  void resetValues(){stop();}
  
  int getTargetPower();
  
  void stop();
/*
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
*/
  void setResponseTime(unsigned int timeMs){
    int maxTime = 5000;
    int minTime = 1;
    timeMs = (timeMs > minTime) ? timeMs : minTime;
    timeMs = (timeMs < maxTime) ? timeMs : minTime; 
    responseTime = timeMs;
    }
  unsigned int getResponseTime(){return responseTime;}
  
  void setPolarity(bool newPolarity){polarity = newPolarity;}
  bool getPolarity(){return polarity;}
   
  private:
    int en;
    int in1;
    int in2;
    
    bool polarity = false;
    unsigned int updateInterval = 20;

    int currentDirection = 0; //1 = forward, 0 = stop, -1 = backward
    float currentPwm = 0.0; //Percentage
    int targetPwm = 0;

    unsigned int responseTime = 1000;
    unsigned long lastUpdate = 0;

    void moveMotor(int pwm);
};

#endif
