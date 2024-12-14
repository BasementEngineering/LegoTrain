#ifndef AUTOPILOT_H
#define AUTOPILOT_H

#include <Arduino.h>
#include "MagnetSensors.h"
#include "ActionButton.h"
#include "ControlLoop.h"

#define SLOW_APPROACH_SPEED 50
#define FW_TRAVEL_SPEED 200
#define RW_TRAVEL_SPEED 200

enum AutopilotState
{
    UNKNOWN,
    CALIBRATING,
    ZERO_REACHED,
    WAITING_AT_PICKUP,
    FILLING_UP,
    MOVING_TO_NEXT_SHOT,
    ON_ROUTE,
    ALMOST_THERE,
    WAITING_AT_DESTINATION,
    RETURNING_TO_PICKUP,
    ALMOST_AT_PICKUP,
    EMERGENCY_STOPPED
};

class Autopilot
{
private:
    int currentState = 0;
    unsigned long lastStateChange = 0;
    int* setpoint = nullptr;
    MagnetSensors* magnetSensors = nullptr;
    ActionButton* actionButton = nullptr;
    ControlLoop* controlLoop = nullptr;
    void (*stopCallback)();
    void (*fillCallback)();
    bool fillTriggered = false;
    int shotCount = 0;
    int bottomMagnetCount = 0;

public:
    Autopilot(int* setpoint, ActionButton* actionButton, MagnetSensors* magnetSensors, ControlLoop* controlLoop, void (*stopCallback)(), void (*fillCallback)()){
        this->actionButton = actionButton;
        this->stopCallback = stopCallback;
        this->setpoint = setpoint;
        this->magnetSensors = magnetSensors;
        this->fillCallback = fillCallback;
        this->controlLoop = controlLoop;
        currentState = UNKNOWN;
    }

    String getState(){
        const String statusStrings[] = {
        "UNKNOWN",
        "CALIBRATING",
        "ZERO_REACHED",
        "WAITING_AT_PICKUP",
        "FILLING_UP",
        "MOVING_TO_NEXT_SHOT",
        "ON_ROUTE",
        "ALMOST_THERE",
        "WAITING_AT_DESTINATION",
        "RETURNING_TO_PICKUP",
        "ALMOST_AT_PICKUP",
        "EMERGENCY_STOPPED"
        };

        return statusStrings[currentState];
    }

    void switchState(AutopilotState newState){
        Serial.print("Switching from state: ");
        Serial.print(currentState);
        Serial.print(" to: ");
        Serial.println(newState);
        currentState = newState;
        lastStateChange = millis();
    }

    void triggerFill(){
        fillTriggered = true;
    }

    void reset(){
        currentState = UNKNOWN;
    }

    void runStateMachine(){
        if(actionButton->stopTriggered()){
            stopCallback();
            actionButton->setStart();
            switchState(EMERGENCY_STOPPED);
        }

        switch (currentState)
        {
        case EMERGENCY_STOPPED:
            if(actionButton->startTriggered()){
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
            switchState(CALIBRATING);
            break;
        case CALIBRATING:	
            if(magnetSensors->bottomWasTriggered()){
                Serial.println("Got Bottom Signal");
            }
            else if(magnetSensors->sideWasTriggered()){
                Serial.println("Got Side Signal");
                stopCallback();
                Serial.println("Calibration done");
                Serial.println("Waiting at pickup");
                actionButton->setStart();
                switchState(ZERO_REACHED);
            }
            break;
        case ZERO_REACHED:
            if(actionButton->startTriggered()){
                *setpoint = SLOW_APPROACH_SPEED;
                Serial.println("Waiting at pickup");
                shotCount = 0;
                actionButton->setStop();
                switchState(MOVING_TO_NEXT_SHOT);
            }
            break;
        case MOVING_TO_NEXT_SHOT:
            if(magnetSensors->bottomWasTriggered()){
                stopCallback();
                fillCallback();
                Serial.println("Filling up");
                actionButton->setLoading();
                switchState(FILLING_UP);
            }
            break;
        case FILLING_UP:
            if(millis() - lastStateChange > 8000){
                shotCount++;
                if(shotCount > 2){
                    Serial.println("Waiting at pickup");
                    actionButton->setStart();
                    switchState(WAITING_AT_PICKUP);
                }
                else{
                    *setpoint = SLOW_APPROACH_SPEED;
                    Serial.println("Moving to next shot");
                    actionButton->setStop();
                    switchState(MOVING_TO_NEXT_SHOT);  
                }
            }
            break;
        case WAITING_AT_PICKUP:
            //if(actionButton->startTriggered()){//fillTriggered){
                Serial.println("Goin en route");
                *setpoint = FW_TRAVEL_SPEED;
                actionButton->setStop();
                switchState(ON_ROUTE);
            //}
            break;
        case ON_ROUTE:
            if( (millis() - lastStateChange > 3000) && magnetSensors->bottomWasTriggered()){
                *setpoint = SLOW_APPROACH_SPEED;
                Serial.println("Almost there");
                switchState(ALMOST_THERE);
            }
            break;
        case ALMOST_THERE:
            if(magnetSensors->sideWasTriggered()){
                stopCallback();
                Serial.println("Waiting at destination");
                actionButton->setStart();
                switchState(WAITING_AT_DESTINATION);
            }
            break;
        case WAITING_AT_DESTINATION:
            if(actionButton->startTriggered()){
                shotCount = 0;
                *setpoint = -RW_TRAVEL_SPEED;
                Serial.println("Returning to pickup");
                actionButton->setStop();
                switchState(RETURNING_TO_PICKUP);
            }
            break;
        case RETURNING_TO_PICKUP:
            if(magnetSensors->sideWasTriggered()){
              Serial.println("Side was triggered");
            }
            if(magnetSensors->bottomWasTriggered()){
                bottomMagnetCount++;
                if(bottomMagnetCount > 2){
                    *setpoint = -SLOW_APPROACH_SPEED;
                    Serial.println("Almost at pickup");
                    switchState(ALMOST_AT_PICKUP);
                }
            }
            break;
        case ALMOST_AT_PICKUP:
            if(magnetSensors->bottomWasTriggered()){
                Serial.println("Bottom triggered");
            }
            if(magnetSensors->sideWasTriggered()){
                *setpoint = 0;
                stopCallback();
                bottomMagnetCount = 0;
                Serial.println("Waiting at pickup");
                actionButton->setStart();
                switchState(ZERO_REACHED);
            }
            break;
        default:
            break;
        }
    }
};


#endif