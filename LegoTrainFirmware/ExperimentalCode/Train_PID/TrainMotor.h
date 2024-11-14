#ifndef TRAIN_MOTOR_H
#define TRAIN_MOTOR_H

#include <Arduino.h>
#include <ESP32Servo.h>

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
    //Serial.println(mircorsenconds);
    motorDriver.writeMicroseconds(mircorsenconds);
  }

  private:
  int driverPin;
  Servo motorDriver;
};

#endif