#ifndef AUTOPILOT_H
#define AUTOPILOT_H

#include <Arduino.h>
#include "MagnetSensors.h"

#define SLOW_APPROACH_SPEED 50
#define FW_TRAVEL_SPEED 200
#define RW_TRAVEL_SPEED 200

enum AutopilotState
{
    UNKNOWN,
    CALIBRATING,
    WAITING_AT_PICKUP,
    FILLING_UP,
    MOVING_TO_NEXT_SHOT,
    ON_ROUTE,
    ALMOST_THERE,
    WAITING_AT_DESTINATION,
    RETURNING_TO_PICKUP,
    ALMOST_AT_PICKUP
};

class Autopilot
{
private:
    int currentState = 0;
    unsigned long lastStateChange = 0;
    int* setpoint = nullptr;
    MagnetSensors* magnetSensors = nullptr;
    void (*stopCallback)();
    void (*fillCallback)();
    bool fillTriggered = false;
    int shotCount = 0;
    int bottomMagnetCount = 0;

public:
    Autopilot(int* setpoint, MagnetSensors* magnetSensors, void (*stopCallback)(), void (*fillCallback)()){
        this->stopCallback = stopCallback;
        this->setpoint = setpoint;
        this->magnetSensors = magnetSensors;
        this->fillCallback = fillCallback;
        currentState = UNKNOWN;
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

    void runStateMachine(){
        switch (currentState)
        {
        case UNKNOWN:
            *setpoint = -SLOW_APPROACH_SPEED;
            Serial.println("Calibrating");
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
                switchState(WAITING_AT_PICKUP);
            }
            break;
        case WAITING_AT_PICKUP:
            if(true){//fillTriggered){
                Serial.println("Moving to next shot");
                *setpoint = SLOW_APPROACH_SPEED;
                switchState(MOVING_TO_NEXT_SHOT);
            }
            break;
        case FILLING_UP:
            if(millis() - lastStateChange > 10000){
                *setpoint = SLOW_APPROACH_SPEED;
                shotCount++;
                if(shotCount > 2){
                    *setpoint = FW_TRAVEL_SPEED;
                    Serial.println("On route");
                    switchState(ON_ROUTE);
                }
                else{
                    Serial.println("Moving to next shot");
                    switchState(MOVING_TO_NEXT_SHOT);  
                }
            }
            break;
        case MOVING_TO_NEXT_SHOT:
            if(magnetSensors->bottomWasTriggered()){
                stopCallback();
                fillCallback();
                Serial.println("Filling up");
                switchState(FILLING_UP);
            }
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
                switchState(WAITING_AT_DESTINATION);
            }
            break;
        case WAITING_AT_DESTINATION:
            if(millis() - lastStateChange > 5000){
                shotCount = 0;
                *setpoint = -RW_TRAVEL_SPEED;
                Serial.println("Returning to pickup");
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
                switchState(WAITING_AT_PICKUP);
            }
            break;
        default:
            break;
        }
    }
};


#endif