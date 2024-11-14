#include "Motor.h"
#include "config.h"

Motor::Motor(int _in1, int _in2,int _en){
  en = _en;
  in1 = _in1;
  in2 = _in2;
}

void Motor::initPins(){
  if(en >= 0){
  pinMode(en,OUTPUT);
  }
  pinMode(in1,OUTPUT);
  pinMode(in2,OUTPUT);

  stop();
}

void Motor::enable(){
  if(en >= 0){
  digitalWrite(en,HIGH);
  }
}

void Motor::disable(){
  if(en >= 0){
  digitalWrite(en,LOW);
  }
}

void Motor::update(){
  if((millis() - lastUpdate) > updateInterval){
    lastUpdate = millis();
    float deltaPwm = (float)(maxPwm*updateInterval)/responseTime;
    float nextStep = 0;
    if(targetPwm > (currentPwm+deltaPwm) ){
      //Serial.println("Adding");
      nextStep = currentPwm + deltaPwm;
    }
    else if(targetPwm < (currentPwm-deltaPwm) ){
      //Serial.println("Subtracting");
      nextStep = currentPwm - deltaPwm;
    }
    else{
      nextStep = targetPwm;
    }
    
    if(currentPwm != targetPwm){
      moveMotor((int)nextStep);
      #if DEBUG_MOTORS
      Serial.print("nextStep");
      Serial.println((int)nextStep);
      Serial.print("delta PWM");
      Serial.println(deltaPwm);
      Serial.print("Current PWM");
      Serial.println(currentPwm);
      Serial.print("Target PWM");
      Serial.println(targetPwm);
      #endif
    }
    
    currentPwm = nextStep;
  }
}

bool Motor::targetReached(){
  return currentPwm == targetPwm;
}

void Motor::stop(){
  currentPwm = 0;
  targetPwm = 0;
  digitalWrite(in1,LOW);
  digitalWrite(in2,LOW);
}

void Motor::setPower(int newPower){
  if(!polarity){
    newPower = -newPower;
  }
  
  if((newPower >= -100) && (newPower <= 100)){
    targetPwm = powerToPwm(newPower,minPwm,maxPwm);
  }
  else{
  targetPwm = 0;
  }

  #if DEBUG_MOTOR
      DEBUG_SERIAL.println("Motor::setPower()");
      DEBUG_SERIAL.print("newPower: ");
      DEBUG_SERIAL.println(newPower);
      DEBUG_SERIAL.print("targetPwm ");
      DEBUG_SERIAL.println(targetPwm);
  #endif
}

int Motor::getPower(){
  return pwmToPower(currentPwm,minPwm,maxPwm);
}

int Motor::getTargetPower(){
  return pwmToPower(targetPwm,minPwm,maxPwm);
}

void Motor::moveMotor(int pwm){
  //digitalWrite(en,HIGH);
  Serial.print("Move Motor: ");
  Serial.println(pwm);

  if(pwm > 255){
    pwm = 255;
  }
  else if(pwm < -255){
    pwm = -255;  
  }
  
  if(pwm > 0){
    digitalWrite(in1,LOW);
    analogWrite(in2,pwm);
  }
  else if(pwm < 0){
    analogWrite(in1,-pwm);
    digitalWrite(in2,LOW);
  }
  else{
    analogWrite(in1,0);
    analogWrite(in2,0);
    digitalWrite(in1,LOW);
    digitalWrite(in2,LOW);
  }
}
