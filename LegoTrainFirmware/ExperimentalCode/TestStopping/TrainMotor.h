#ifndef TRAIN_MOTOR_H
#define TRAIN_MOTOR_H

#include <Arduino.h>
#include <ESP32Servo.h>

#define ESC_BRAKE_ENABLED

#define BRAKE_DEAD_TIME 500
#define ESC_ZERO_COOLDOWN_TIME 100

enum escStates{
  ESC_IDLE,
  ESC_FORWARD,
  ESC_REVERSE,
  ESC_BRAKING,
  ESC_ZERO_COOLDOWN
};

class TrainMotor{
  public:
  TrainMotor(int driverPin):motorDriver(){
    this->driverPin = driverPin;
  }

  void begin(){
    Serial.println("Attachig motor driver");
    Serial.println(driverPin);
    ESP32PWM::allocateTimer(0);
	  //ESP32PWM::allocateTimer(1);
	  //ESP32PWM::allocateTimer(2);
	  //ESP32PWM::allocateTimer(3);
	  motorDriver.setPeriodHertz(50);
    motorDriver.attach(driverPin, 1000, 2000);
    motorDriver.writeMicroseconds(1500);
    delay(15);
  }

  void setSpeed(int speed){
    int mircorsenconds = map(speed, -100, 100, 1000, 2000);

    switch (state)
    {
    case ESC_IDLE:
      if(speed > 0){
        Serial.println("Moving forward");
        state = ESC_FORWARD;
        motorDriver.writeMicroseconds(mircorsenconds);
      }
      else if(speed < 0){
        Serial.println("Moving backward");
        state = ESC_REVERSE;
        motorDriver.writeMicroseconds(mircorsenconds);
      }
      break;
    case ESC_FORWARD:
      if(speed < 0){
        brake();
        Serial.println("Braking");
        state = ESC_BRAKING;
      }
      else{
        motorDriver.writeMicroseconds(mircorsenconds);
      }
      break;
    case ESC_BRAKING:
      if((millis() - brakeStart) > BRAKE_DEAD_TIME){
        Serial.println("Brake dead time over Cooldown started");
        state = ESC_ZERO_COOLDOWN;
        motorDriver.writeMicroseconds(1500);
        brakeStart = 0;
        brakeStart = millis();
      }
      break;
    case ESC_ZERO_COOLDOWN:
      if((millis() - brakeStart) > ESC_ZERO_COOLDOWN_TIME){
        Serial.println("Cooldown over");
        brakeStart = 0;
        state = ESC_IDLE;
      }
      break;
    case ESC_REVERSE:
      if(speed > 0){
        Serial.println("Moving forward");
        state = ESC_IDLE;
      }
      else{
        motorDriver.writeMicroseconds(mircorsenconds);
      }
      break;
    default:
      break;
    }
  }

  void brake(){
    if(state == ESC_FORWARD){
      //Serial.println("Sending Brake Signal");
      motorDriver.writeMicroseconds(1200);
      brakeStart = millis();
      state = ESC_BRAKING;
    }
  }

  private:
  int driverPin;
  Servo motorDriver;
  
  unsigned long brakeStart = 0;
  int state = ESC_IDLE;

};

#endif