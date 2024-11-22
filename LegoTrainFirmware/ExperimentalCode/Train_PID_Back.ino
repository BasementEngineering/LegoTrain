#include "Speedometer.h"
#include "SpeedMonitor.h"
#include "TrainMotor.h"

#define MOTOR_PIN 26
#define ANALOG_SENSOR_PIN 32
#define SPEED_POTI_PIN 34

SpeedMonitor speedMonitor;
Speedometer speedometer(ANALOG_SENSOR_PIN);
TrainMotor motor(MOTOR_PIN);

unsigned long lastMotorSpeedUpdate = 0;

#include <PID_v1.h>
double Setpoint, Input, Output;
//Specify the links and initial tuning parameters
double Kp=0.15, Ki=0.0, Kd=0.01;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

#define SPEED_TIMEOUT 10000
unsigned long speedTimeoutStart = 0;

void updateMotor(){
  if( (millis() - lastMotorSpeedUpdate) > 100){
    int speed = map(analogRead(SPEED_POTI_PIN), 0, 4095, -400, 400);
  if(speed < 20 && speed > -20){
    speed = 0;
    Output = 0;
  }
  Setpoint = speed;
  Input = speedometer.getSpeed();
  //myPID.Compute();

/*
  int delta = Input - Setpoint;
  if(delta == 0){
    speedTimeoutStart = millis();
  }

  if((millis() - speedTimeoutStart) > SPEED_TIMEOUT){
    Output = 0;
  }*/

  if(Input < Setpoint){
    Output++;
    if(Output > 100){
      Output = 100;
    }
  }
  else if(Input > Setpoint){
    Output--;
    if(Output < -100){
      Output = -100;
    }
  }
  motor.setSpeed(Output);
  
  Serial.print("Speed:");
  Serial.print(Input);
  Serial.print(",");
  Serial.print("Setpoint:");
  Serial.print(Setpoint);
  Serial.print(",");
  Serial.print("Output:");
  Serial.print(Output);
  Serial.println("");


  lastMotorSpeedUpdate = millis();
  }
  
}

void setup() {
  Serial.begin(9600);
  myPID.SetMode(AUTOMATIC);
  delay(500);
  Serial.println("Starting setup");
  delay(500);
  Serial.println("Setting up speed monitor");
  speedMonitor.begin();
  
  speedMonitor.setSpeed(40);
  speedMonitor.update();
  delay(500);
  Serial.println("Setting up encoder");
  speedometer.setupEncoder();
  Serial.println("Setting up motor");
  motor.begin();
  Serial.println("Activating HW Timer");
  setupTimerInterrupt();
  Serial.println("Ready to go");
  pinMode(SPEED_POTI_PIN, INPUT);
}

void loop() {
  delay(10);
  speedMonitor.setSpeed(speedometer.getSpeed());
  speedMonitor.setDistance(speedometer.getDistance());
  speedMonitor.update();
  updateMotor();
}
