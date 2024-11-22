#ifndef AUTOPILOT_H
#define AUTOPILOT_H

#include <Arduino.h>
#include "MagnetSensors.h"

enum AutopilotState
{
    UNKNOWN,
    CALIBRATING,
    WAITING_AT_PICKUP,
    ON_ROUTE,
    ALMOST_THERE,
    WAITING_AT_DESTINATION
};

class Autopilot
{
private:
    int currentState = 0;
    unsigned long lastStateChange = 0;
    int* setpoint = nullptr;
    MagnetSensors* magnetSensors = nullptr;
    void (*stopCallback)();

public:
    Autopilot(int* setpoint, MagnetSensors* magnetSensors, void (*stopCallback)()){
        this->stopCallback = stopCallback;
        this->setpoint = setpoint;
        this->magnetSensors = magnetSensors;
        currentState = UNKNOWN;
    }

    void runStateMachine(){
        switch (currentState)
        {
        case UNKNOWN:
            currentState = CALIBRATING;
            *setpoint = -200;
            break;
        case WAITING_AT_PICKUP:
            // Check if the train is ready to leave
            // If so, change state to ON_ROUTE
            break;
        case ON_ROUTE:
            // Check if the train is almost at the destination
            // If so, change state to ALMOST_THERE
            break;
        case ALMOST_THERE:
            // Check if the train is at the destination
            // If so, change state to WAITING_AT_DESTINATION
            break;
        case WAITING_AT_DESTINATION:
            // Check if the train is ready to leave
            // If so, change state to ON_ROUTE
            break;
        default:
            break;
        }
    }
};


#endif