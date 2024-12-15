#include "TrainMotor.h"
#include "ControlLoop.h"
#include "Speedometer.h"
#include "MagnetSensors.h"

#include <Pinout.h>

int newSetpoint = 0;

TrainMotor motor(MOTOR_PIN);
ControlLoop controlLoop;
Speedometer speedometer(ANALOG_SENSOR_PIN);
MagnetSensors magnetSensors(MAGNET_BOTTOM_PIN, MAGNET_SIDE_PIN);

unsigned long lastMotorSpeedUpdate = 0;
unsigned long lastEventUpdate = 0;

#define STOP_DURATION 3000

enum modes {
    AT_START_POSITION,
    GOING_FORWARD,
    STOPPED_FW,
    STOPPED_RW,
    CORRECTING,
    GOING_BACKWARD
};

int currentState = AT_START_POSITION;
#define TRAVEL_SPEED 50

void handleStopInterrupt(){
  detachInterrupt(MAGNET_BOTTOM_PIN); 
    controlLoop.reset();
    newSetpoint = 0;
    motor.brake(); //Only works when moving forwards
    //motor.setSpeed(0);
    if(currentState == GOING_FORWARD){
        currentState = STOPPED_FW;
    }
    else if(currentState == CORRECTING){
        currentState = STOPPED_RW;
    }

    digitalWrite(INTERRUPT_CHECK_PIN, !digitalRead(INTERRUPT_CHECK_PIN));
    lastEventUpdate = millis();
}

void setStopIntterrupt(){
    attachInterrupt(MAGNET_BOTTOM_PIN, handleStopInterrupt, FALLING);
}

bool middleCleared = false;

void runTest(){
    switch (currentState)
    {
    case AT_START_POSITION:
        if(millis() - lastEventUpdate > STOP_DURATION){
            lastEventUpdate = millis();
            newSetpoint = TRAVEL_SPEED;
            setStopIntterrupt();
            Serial.println("Starting at start position");
            Serial.println("Going forward");
            currentState = GOING_FORWARD;
        }
        break;
    case STOPPED_FW:
        if(millis() - lastEventUpdate > 1000){
            if(!magnetSensors.bottomMagnetPresent()){
                newSetpoint = -TRAVEL_SPEED;
                setStopIntterrupt();
                Serial.println("Stopped FW Correcting");
                Serial.println("Missed Magnet");
                Serial.println("Correcting");
                currentState = CORRECTING;
                lastEventUpdate = millis();
            }
        }
        if(millis() - lastEventUpdate > STOP_DURATION){
            lastEventUpdate = millis();
            newSetpoint = TRAVEL_SPEED;
            setStopIntterrupt();
            Serial.println("Stopped On Point");
            Serial.println("Going forward");
            currentState = GOING_FORWARD;
        }
        break;
    case STOPPED_RW:
        if(millis() - lastEventUpdate > 1000){
            if(!magnetSensors.bottomMagnetPresent()){
                Serial.println("Stopped RW Correcting");
                Serial.println("Missed Magnet");
                setStopIntterrupt();
                newSetpoint = TRAVEL_SPEED;
                currentState = GOING_FORWARD;
                lastEventUpdate = millis();
            }
        }
        if(millis() - lastEventUpdate > STOP_DURATION){
            lastEventUpdate = millis();
            setStopIntterrupt();
            Serial.println("GOING FORWARD");
            newSetpoint = TRAVEL_SPEED;
            currentState = GOING_FORWARD;
        }
        break;
    case CORRECTING:
        break;
    case GOING_FORWARD:
      if(magnetSensors.bottomWasTriggered()){
            middleCleared = true;
        }
        if(magnetSensors.sideWasTriggered() && middleCleared){
            lastEventUpdate = millis();
            handleStopInterrupt();
            newSetpoint = -TRAVEL_SPEED;
            Serial.println("Going backward");
            currentState = GOING_BACKWARD;
            middleCleared = false;
        }
        break;
    case GOING_BACKWARD:
    if(magnetSensors.bottomWasTriggered()){
            middleCleared = true;
        }
        if(magnetSensors.sideWasTriggered() && middleCleared){
            lastEventUpdate = millis();
            handleStopInterrupt();
            Serial.println("Stopping");
            currentState = AT_START_POSITION;
            middleCleared = false;
        }
        break;
    default:
        break;
    }
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Starting setup");
    Serial.println("Setting up encoder");
    speedometer.setupEncoder();
    speedometer.setInverted(true);
    Serial.println("Activating HW Timer");
    setupTimerInterrupt();

    magnetSensors.setup();
    motor.begin();
    pinMode(INTERRUPT_CHECK_PIN, OUTPUT);
    digitalWrite(INTERRUPT_CHECK_PIN, LOW);

    controlLoop.setMode(2);

    Serial.println("Ready to go");
}

void loop()
{
    updateMotor();

    magnetSensors.update();

    runTest();
}

unsigned long lastSpeedPrint = 0;

void updateMotor()
{
    if ((millis() - lastMotorSpeedUpdate) > 300)
    {

      if(millis() - lastSpeedPrint > 1000){

        Serial.print("Speed: ");
        Serial.println(speedometer.getSpeed());
        lastSpeedPrint = millis();
      }
        
        controlLoop.setInput(speedometer.getSpeed());
        controlLoop.setTarget(newSetpoint);
        motor.setSpeed(controlLoop.calculateOutput());

        lastMotorSpeedUpdate = millis();
    }
}