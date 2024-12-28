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

#define DRINK_STATION_TIMEOUT 8000

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
    unsigned long randomWaitInterval = 3600000;

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

    int getState(){
      return currentState;
    }

    String getStateString(int stateNumber)
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

        return statusStrings[stateNumber];
    }

    void switchState(AutopilotState newState)
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

    void reset()
    {
        currentState = UNKNOWN;
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
            actionButton->setStart();
            break;
        case CALIBRATING:
            actionButton->setStop();
            shotCount = 0;
            *setpoint = -SLOW_APPROACH_SPEED;
            currentDirection = BACKWARD;
            break;
        case ZERO_REACHED:
            stopCallback();
            actionButton->setStart();
            bottomMagnetCount = 0;
            shotCount = 0;
            *setpoint = 0;
            currentDirection = NONE;
            break;
        case MOVING_TO_NEXT_SHOT:
            magnetSensors ->bottomWasTriggered(); //clear signal
            shotCount = 0;
            actionButton->setStop();
            updateRandomInterval();
            currentDirection = FORWARD;
            creepToStop(SLOW_APPROACH_SPEED);
            break;
        case FILLING_UP:
            stopCallback();
            fillCallback();
            actionButton->setLoading();
        case WAITING_AT_PICKUP:
            stopCallback();
            setStopIntterrupt(false);
            actionButton->setStop();
            break;
        case BRAKING:
            break;
        case STOPPED:
            break;
        case CORRECTING:
            magnetSensors ->bottomWasTriggered(); //clear signal
            currentDirection = BACKWARD;
            creepToStop(-SLOW_APPROACH_SPEED);
            break;
        case ON_ROUTE:
            Serial.println("Goin en route");
            *setpoint = FW_TRAVEL_SPEED;
            currentDirection = FORWARD;
            actionButton->setStop();
            break;
        case ALMOST_THERE:
            *setpoint = SLOW_APPROACH_SPEED;
            break;
        case WAITING_AT_DESTINATION:
            stopCallback();
            actionButton->setStart();
            currentDirection = NONE;
            break;
        case RETURNING_TO_PICKUP:
            shotCount = 0;
            *setpoint = -RW_TRAVEL_SPEED;
            currentDirection = BACKWARD;
            actionButton->setStop();
            break;
        case ALMOST_AT_PICKUP:
            *setpoint = -SLOW_APPROACH_SPEED;
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
                switchState(CALIBRATING);
            }
            break;
        case UNKNOWN:
            switchState(CALIBRATING);
            break;
        case CALIBRATING:
            if (magnetSensors->sideWasTriggered())
            {
                switchState(ZERO_REACHED);
            }
            break;
        case ZERO_REACHED:
            if (actionButton->startTriggered() ||
                ((millis() - lastStateChange) > randomWaitInterval))
            { // Or random time has passed
                switchState(MOVING_TO_NEXT_SHOT);
            }
            break;
        case MOVING_TO_NEXT_SHOT:
            if (magnetSensors->bottomWasTriggered())
            {
                switchState(BRAKING);
            }
            if (millis() - lastStateChange > DRINK_STATION_TIMEOUT)
            {
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
                    switchState(CORRECTING);
                }
                else
                {
                    switchState(MOVING_TO_NEXT_SHOT);
                }
            }
            else
            {
                Serial.println("Stopped on Point");
                switchState(FILLING_UP);
            }
            break;
        case CORRECTING:
            if (magnetSensors->bottomWasTriggered())
            {
                switchState(BRAKING);
            }
            break;
        case FILLING_UP:
            if (millis() - lastStateChange > 8000)
            {
                shotCount++;
                if (shotCount < MAX_SHOTS)
                {
                    switchState(MOVING_TO_NEXT_SHOT);
                }
                else
                {
                    switchState(WAITING_AT_PICKUP);
                }
            }
            break;
        case WAITING_AT_PICKUP:
            switchState(ON_ROUTE);
            break;
        case ON_ROUTE:
            if (magnetSensors->sideWasTriggered())
            {
              Serial.println("Side was triggered in on route ahead of bottom");
                switchState(WAITING_AT_DESTINATION);
            }
            if ((millis() - lastStateChange > 3000) && magnetSensors->bottomWasTriggered())
            {
                switchState(ALMOST_THERE);
            }
            break;
        case ALMOST_THERE:
            if (magnetSensors->sideWasTriggered())
            {
                switchState(WAITING_AT_DESTINATION);
            }
            break;
        case WAITING_AT_DESTINATION:
            if (actionButton->startTriggered())
            {
                switchState(RETURNING_TO_PICKUP);
            }
            break;
        case RETURNING_TO_PICKUP:
            if (magnetSensors->bottomWasTriggered())
            {
                bottomMagnetCount++;
                if (bottomMagnetCount > 2)
                {
                    switchState(ALMOST_AT_PICKUP);
                }
            }
            break;
        case ALMOST_AT_PICKUP:
            if (magnetSensors->sideWasTriggered())
            {
                switchState(ZERO_REACHED);
            }
            break;
        default:
            break;
        }
    }
};

#endif