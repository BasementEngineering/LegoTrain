#ifndef TRACKPILOT_H
#define TRACKPILOT_H

#include <Arduino.h>
#include "Pinout.h"
#include "MagnetSensors.h"
#include "ActionButton.h"
#include "ControlLoop.h"
#include "TrainMotor.h"

#define SLOW_APPROACH_SPEED 50
#define FW_TRAVEL_SPEED 200
#define RW_TRAVEL_SPEED 100

#define CRUISE_POWER 40
#define CLIMB_POWER 80
#define SLOW_POWER 20

#define STOP_DURATION 5000
#define BRAKE_DURATION 1000

#define DRINK_STATION_TIMEOUT 8000

#define MAX_SHOTS 6

enum TrackpilotState
{
    AT_START_STATION,
    CRUISING,
    CLIMBING,
    ARRIVING_UPPER_STATION,
    STOPPING,
    AT_END_STATION,
    EMERGENCY_STOPPED
};

const String statusStrings[] = {
    "AT_START_STATION",
    "CRUISING",
    "CLIMBING",
    "ARRIVING_UPPER_STATION",
    "STOPPING",
    "AT_END_STATION",
    "EMERGENCY_STOPPED"
};

enum directions
{
    FORWARD,
    BACKWARD,
    NONE
};

class Trackpilot
{
private:
    int currentState = AT_START_STATION;
    int currentDirection = NONE;

    unsigned long lastStateChange = 0;
    int *setpoint = nullptr;

    MagnetSensors *magnetSensors = nullptr;
    ActionButton *actionButton = nullptr;
    ControlLoop *controlLoop = nullptr;
    TrainMotor *motor = nullptr;

    void (*stopCallback)();
    void (*fillCallback)();
    void (*setStopIntterrupt)(bool);
    void (*anounceArrival)();

    int bottomMagnetCount = 0;
    bool middleCleared = false;

    unsigned long randomWaitInterval = 3600000;

public:
    Trackpilot(int *setpoint, ActionButton *actionButton, TrainMotor *trainMotor, MagnetSensors *magnetSensors, ControlLoop *controlLoop, void (*stopCallback)(), void (*fillCallback)(),void (*announceCallback)(), void (*setStopIntterrupt)(bool))
    {
        this->actionButton = actionButton;
        this->stopCallback = stopCallback;
        this->setpoint = setpoint;
        this->magnetSensors = magnetSensors;
        this->fillCallback = fillCallback;
        this->anounceArrival = announceCallback;
        this->controlLoop = controlLoop;
        this->motor = trainMotor;
        this->setStopIntterrupt = setStopIntterrupt;

        currentState = AT_START_STATION;

        pinMode(INTERRUPT_CHECK_PIN, OUTPUT);
        digitalWrite(INTERRUPT_CHECK_PIN, LOW);
    }

    int getState(){
      return currentState;
    }

    String getStateString(int stateNumber)
    {
        return statusStrings[stateNumber];
    }

    void switchState(TrackpilotState newState)
    {
        Serial.println("_____________________");
        Serial.print("Switching from state: ");
        Serial.print(getStateString(currentState));
        Serial.print(" to: ");
        Serial.println(getStateString(newState));
        Serial.println("_____________________");
        onEntry(newState);
        currentState = newState;
        lastStateChange = millis();
    }

    void enter()
    {
        Serial.println("Entering Autopilot Mode");
        switchState(AT_START_STATION);
    }

    void reset()
    {
        setStopIntterrupt(false);
    }

    void updateRandomInterval()
    {
        randomWaitInterval = random(600000, 3600000);
        Serial.print("New Random Interval: ");
        Serial.println(randomWaitInterval);
    }

    void creepToStop(int speed)
    {
        if (speed > 0)
        {
            currentDirection = FORWARD;
        }
        else
        {
            currentDirection = BACKWARD;
        }
        Serial.print("Creep To Stop wit speed: ");
        Serial.println(speed);
        *setpoint = speed;
        actionButton->setStop();
        setStopIntterrupt(true);
    }

    void onEntry(int newState)
    {
        switch (newState)
        {
        case EMERGENCY_STOPPED:
            stopCallback();
            setStopIntterrupt(false);
            //actionButton->setStart();
            break;
        case AT_START_STATION:
            Serial.println("At Start Station"); 
            controlLoop->setMode(TARGET_OUTPUT); // Target Output Mode
            *setpoint = 0;
            actionButton->setStart();
            break;
        case CRUISING:
            *setpoint = CRUISE_POWER;
            //controlLoop->setMode(1); // Target Output Mode
            actionButton->setStop();
            break;
        case CLIMBING:
            *setpoint = CLIMB_POWER;
            //controlLoop->setMode(1); // Target Output Mode
            actionButton->setStop();
            break;
        case ARRIVING_UPPER_STATION:
            controlLoop->setMode(FAST_TARGET_OUTPUT); // Fast Target Output Mode
            *setpoint = CRUISE_POWER;
            //controlLoop->setMode(1); // Target Output Mode
            actionButton->setStop();
            anounceArrival();
            break;
        case STOPPING:
            controlLoop->setMode(TARGET_OUTPUT); // Target Output Mode
            *setpoint = 0;
            //controlLoop->setMode(1); // Target Output Mode
            actionButton->setStart();
            break;
        
        default:
            break;
        }
    }

    void runStateMachine()
    {
        if (actionButton->stopTriggered())
        {
            switchState(EMERGENCY_STOPPED);
        }

        switch (currentState)
        {
        case EMERGENCY_STOPPED:
            if (actionButton->startTriggered())
            {
                switchState(AT_START_STATION);
            }
            break;
        case AT_START_STATION:
            if(actionButton->startTriggered()){
                currentDirection = FORWARD;
                switchState(CRUISING);
            }
            break;
        case CRUISING:
            if(magnetSensors->bottomWasTriggered()){
                switchState(CLIMBING);
            }
            break;
        case CLIMBING:
            if(magnetSensors->bottomWasTriggered()){
                switchState(ARRIVING_UPPER_STATION);
            }
            break;
        case ARRIVING_UPPER_STATION:
            if(magnetSensors->bottomWasTriggered()){
                switchState(STOPPING);
            }
            break;
        case STOPPING:
            if(magnetSensors->bottomWasTriggered()){
                switchState(AT_END_STATION);
            }
        default:
            break;
        }
    }
};

#endif