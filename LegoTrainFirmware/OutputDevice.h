#ifndef OUTPUT_DEVICE
#define OUTPUT_DEVICE

#include "config.h"
#define MAX_PWM 255

//A generic output device class. 
//This "Interface" makes it easy to control different output devices through a common channel infrastructure.
//The channel can be switched to for example assign the output to another input channel from the UI.
//The channel architecture allows for different vehicel configurations.
//It also allows channel switching in software in case a device was connected to the wrong output.
class OutputDevice{
  public:
    // power input can be from -100% to 100%
    
    virtual int getPower();
    virtual void setPower(int newPower);
    virtual void resetValues();

    void setChannel(int newChannel){
      if((newChannel > -1) && (newChannel < CHANNEL_COUNT)){
        myChannel = newChannel;
      }
    }
    
    int getChannel(){return myChannel;}

    void setDangerous(bool _dangerous){dangerous = _dangerous;}
    bool isDangerous(){return dangerous;}

    void setMinPower(unsigned int newMin){
      unsigned int newPwm = powerToPwm(newMin);
      if(newPwm < maxPwm){
        minPwm = newPwm;
      }
      resetValues();
    }
    
    unsigned int getMinPower(){
      return pwmToPower(minPwm);
    }
    
    void setMaxPower(unsigned int newMax){
      unsigned int newPwm = powerToPwm(newMax);
      if(newPwm > minPwm){
        maxPwm = newPwm;
      }   
      resetValues();
    }
    
    unsigned int getMaxPower(){
      return pwmToPower(maxPwm);
    }

    unsigned int powerToPwm(unsigned int power){
      if(power == 0){return 0;}
      else if(power >= 100){return MAX_PWM;}
      return power*MAX_PWM/100;
    }
    
    int powerToPwm(int power,unsigned int lowerLimit,unsigned int upperLimit){
      int adjustableRange = upperLimit - lowerLimit;
      int addedPart = (power * adjustableRange)/100;
      if(power > 0){
        return minPwm+addedPart;
      }
      else if(power <0){
        return -minPwm+addedPart;
      }
      else{
        return 0;
      }
    }
    
    unsigned int pwmToPower(unsigned int pwm){
      if(pwm == 0){return 0;}
      else if(pwm == MAX_PWM){return 100;}
      return pwm*100/MAX_PWM;
    }

    int pwmToPower(int pwm,unsigned int lowerLimit,unsigned int upperLimit){
      int adjustableRange = upperLimit - lowerLimit;
  
      if(pwm > 0){
        int part = pwm - minPwm;
        return (100*part)/adjustableRange;
      }
      else if(pwm < 0){
      int part = pwm + minPwm;
      return (100*part)/adjustableRange;
      }
      return 0;
    }

    int myChannel = -1;
    bool dangerous = true;
  
    int minPwm = 0;
    int maxPwm = 255;
};

#endif
