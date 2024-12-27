#ifndef AUTOPILOT_H
#define AUTOPILOT_H

#include <Arduino.h>
#include "Pinout.h"
#include "MagnetSensors.h"
#include "ActionButton.h"
#include "ControlLoop.h"
#include "TrainMotor.h"

#define SLOW_APPROACH_SPEED 50
#define FW_TRAVEL_SPEED 200
#define RW_TRAVEL_SPEED 100

#define STOP_DURATION 5000
#define BRAKE_DURATION 1000

#define DRINK_STATION_TIMEOUT  8000

#define MAX_SHOTS 6

enum AutopilotState
{
    UNKNOWN,
    CALIBRATING,
    ZERO_REACHED,
    WAITING_AT_PICKUP,
    FILLING_UP,
    MOVING_TO_NEXT_SHOT,
    BRAKING,
    CORRECTING,
    STOPPED,
    ON_ROUTE,
    ALMOST_THERE,
    WAITING_AT_DESTINATION,
    RETURNING_TO_PICKUP,
    ALMOST_AT_PICKUP,
    EMERGENCY_STOPPED
};

enum directions
{
    FORWARD,
    BACKWARD,
    NONE
};

class Autopilot
{
private:
    int currentState = UNKNOWN;
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

    int shotCount = 0;
    int bottomMagnetCount = 0;
    bool middleCleared = false;

    int misses = 0;
    unsigned long randomWaitInterval = 60000;

public:
    Autopilot(int *setpoint, ActionButton *actionButton, TrainMotor *trainMotor, MagnetSensors *magnetSensors, ControlLoop *controlLoop, void (*stopCallback)(), void (*fillCallback)(), void (*setStopIntterrupt)(bool))
    {
        this->actionButton = actionButton;
        this->stopCallback = stopCallback;
        this->setpoint = setpoint;
        this->magnetSensors = magnetSensors;
        this->fillCallback = fillCallback;
        this->controlLoop = controlLoop;
        this->motor = trainMotor;
        this->setStopIntterrupt = setStopIntterrupt;
    
        currentState = UNKNOWN;
        
        pinMode(INTERRUPT_CHECK_PIN, OUTPUT);
        digitalWrite(INTERRUPT_CHECK_PIN, LOW);
    }

    String getState()
    {
        const String statusStrings[] = {
            "UNKNOWN",
            "CALIBRATING",
            "ZERO_REACHED",
            "WAITING_AT_PICKUP",
            "FILLING_UP",
            "MOVING_TO_NEXT_SHOT",
            "BRAKING",
            "CORRECTING",
            "STOPPED",
            "ON_ROUTE",
            "ALMOST_THERE",
            "WAITING_AT_DESTINATION",
            "RETURNING_TO_PICKUP",
            "ALMOST_AT_PICKUP",
            "EMERGENCY_STOPPED"};

        return statusStrings[currentState];
    }

    void switchState(AutopilotState newState)
    {
        Serial.print("Switching from state: ");
        Serial.print(getState());
        currentState = newState;
        Serial.print(" to: ");
        Serial.print(getState());
        lastStateChange = millis();
        //lastStateChange = millis();
    }

    void reset()
    {
        currentState = UNKNOWN;
        setStopIntterrupt(false);
    }

    void updateRandomInterval()
    {
        randomWaitInterval = random(10000, 30000);
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
        *setpoint = speed;
        actionButton->setStop();
        setStopIntterrupt(true);
    }



    void runStateMachine()
    {
        if (actionButton->stopTriggered())
        {
            stopCallback();
            actionButton->setStart();
            setStopIntterrupt(false);
            switchState(EMERGENCY_STOPPED);
        }

        switch (currentState)
        {
        case EMERGENCY_STOPPED:
            if (actionButton->startTriggered())
            {
                *setpoint = -SLOW_APPROACH_SPEED;
                Serial.println("Calibrating");
                actionButton->setStop();
                switchState(CALIBRATING);
            }
            break;
        case UNKNOWN:
            *setpoint = -SLOW_APPROACH_SPEED;
            Serial.println("Calibrating");
            actionButton->setStop();
            shotCount = 0;
            currentDirection = BACKWARD;
            switchState(CALIBRATING);
            break;
        case CALIBRATING:
            if (magnetSensors->bottomWasTriggered())
            {
                Serial.println("Got Bottom Signal");
            }
            else if (magnetSensors->sideWasTriggered())
            {
                Serial.println("Got Side Signal");
                stopCallback();
                Serial.println("Calibration done");
                Serial.println("Waiting at pickup");
                actionButton->setStart();
                currentDirection = NONE;
                switchState(ZERO_REACHED);
            }
            break;
        case ZERO_REACHED:
            if (actionButton->startTriggered() || 
            ((millis() - lastStateChange) > randomWaitInterval)) //Or random time has passed
            {
                shotCount = 0;
                actionButton->setStop();
                updateRandomInterval();
                currentDirection = FORWARD;
                creepToStop(SLOW_APPROACH_SPEED);
                switchState(MOVING_TO_NEXT_SHOT);
            }
            break;
        case MOVING_TO_NEXT_SHOT:
            if (magnetSensors->bottomWasTriggered())
            {
                stopCallback();
                Serial.println("Filling up");
                actionButton->setLoading();
                switchState(FILLING_UP);
                fillCallback();
            }

            if(millis() - lastStateChange > DRINK_STATION_TIMEOUT){
                Serial.println("Timed Out at Drink Station");
                stopCallback();
                setStopIntterrupt(false);
                switchState(WAITING_AT_PICKUP);
            }
            break;
        case BRAKING:
            if (millis() - lastStateChange > BRAKE_DURATION)
            {
                switchState(STOPPED);
            }
            break;
        case STOPPED:
            if (!magnetSensors->bottomMagnetPresent())
                {
                    Serial.println("Missed Magnet");
                    misses++;

                    if (currentDirection == FORWARD)
                    {
                        creepToStop(-SLOW_APPROACH_SPEED);
                        switchState(CORRECTING);
                    }
                    else
                    {
                        creepToStop(SLOW_APPROACH_SPEED);
                        currentDirection = FORWARD;
                        switchState(MOVING_TO_NEXT_SHOT);
                    }
                }
                else
                {
                    Serial.println("Stopped On Point");
                    currentDirection = NONE;
                    if(shotCount < MAX_SHOTS){
                        fillCallback();
                        switchState(FILLING_UP);
                        actionButton->setLoading();
                    }
                    switchState(FILLING_UP);
                }
            break;
        case CORRECTING:
            break;
        case FILLING_UP:
            if (millis() - lastStateChange > 8000)
            {
                shotCount++;
                if(shotCount < MAX_SHOTS){
                    creepToStop(SLOW_APPROACH_SPEED);
                    switchState(MOVING_TO_NEXT_SHOT);
                }
                else{
                    Serial.println("All Shots Fired");
                    actionButton->setStop();
                    switchState(WAITING_AT_PICKUP);
                }
            }
            break;
        case WAITING_AT_PICKUP:
            // if(actionButton->startTriggered()){//fillTriggered){
            Serial.println("Goin en route");
            *setpoint = FW_TRAVEL_SPEED;
            currentDirection = FORWARD;
            actionButton->setStop();
            switchState(ON_ROUTE);
            //}
            break;
        case ON_ROUTE:
            if ((millis() - lastStateChange > 3000) && magnetSensors->bottomWasTriggered())
            {
                *setpoint = SLOW_APPROACH_SPEED;
                Serial.println("Almost there");
                switchState(ALMOST_THERE);
            }
            break;
        case ALMOST_THERE:
            if (magnetSensors->sideWasTriggered())
            {
                stopCallback();
                Serial.println("Waiting at destination");
                actionButton->setStart();
                currentDirection = NONE;
                switchState(WAITING_AT_DESTINATION);
            }
            break;
        case WAITING_AT_DESTINATION:
            if (actionButton->startTriggered())
            {
                shotCount = 0;
                *setpoint = -RW_TRAVEL_SPEED;
                currentDirection = BACKWARD;
                Serial.println("Returning to pickup");
                actionButton->setStop();
                switchState(RETURNING_TO_PICKUP);
            }
            break;
        case RETURNING_TO_PICKUP:
            if (magnetSensors->sideWasTriggered())
            {
                Serial.println("Side was triggered");
            }
            if (magnetSensors->bottomWasTriggered())
            {
                bottomMagnetCount++;
                if (bottomMagnetCount > 2)
                {
                    *setpoint = -SLOW_APPROACH_SPEED;
                    Serial.println("Almost at pickup");
                    switchState(ALMOST_AT_PICKUP);
                }
            }
            break;
        case ALMOST_AT_PICKUP:
            if (magnetSensors->bottomWasTriggered())
            {
                Serial.println("Bottom triggered");
            }
            if (magnetSensors->sideWasTriggered())
            {
                *setpoint = 0;
                stopCallback();
                bottomMagnetCount = 0;
                Serial.println("Waiting at pickup");
                actionButton->setStart();
                currentDirection = NONE;
                switchState(ZERO_REACHED);
            }
            break;
        default:
            break;
        }
    }
};

#endif