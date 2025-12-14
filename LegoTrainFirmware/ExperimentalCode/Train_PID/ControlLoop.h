#ifndef CONTROLLOOP_H
#define CONTROLLOOP_H

#include <Arduino.h>
enum controlModes
{
    MANUAL = 0,
    TARGET_OUTPUT = 1,
    TARGET_SPEED = 2,
    FAST_TARGET_OUTPUT = 3,
    DOWNHILL_MODE = 4
};

class ControlLoop
{

private:
    int setPoint;
    int input;
    int output;
    int mode;
    bool startingFromStop = true;

public:
    ControlLoop()
    {
        Serial.println("Controller created");
    }

    void stop()
    {
        output = 0;
    }

    void reset()
    {
        setPoint = 0;
        input = 0;
        //output = 0;
        startingFromStop = true;
    }

    void setMode(int mode)
    {
        Serial.print("Changing mode to ");
        Serial.println(mode);
        this->mode = mode;
        reset();
        if(mode == DOWNHILL_MODE){
            Serial.println("Downhill Mode activated");
            output = 20; // Initial power for downhill
        }
    }

    int getMode()
    {
        return mode;
    }

    unsigned long startTime = 0;

    void setTarget(int setPoint)
    {
        int previousSetPoint = this->setPoint;
        this->setPoint = setPoint;
        if(previousSetPoint = 0 && setPoint != 0)
        {
            startingFromStop = true;
        }
    }

    void setInput(int input)
    {
        this->input = input;
    }

    void setOutput(int output)
    {
        this->output = output;
    }

    int getOutput()
    {
        return output;
    }

    int calculateOutput()
    {
        switch (mode)
        {
        case MANUAL:
            calculateManualMode();
            break;
        case TARGET_OUTPUT:
            calculateTargetOutputMode();
            break;
        case TARGET_SPEED:
            calculateTargetSpeedMode();
            break;
        case FAST_TARGET_OUTPUT:
            calculateFastTargetOutputMode();
            break;
        case DOWNHILL_MODE:
            calculateDownhillMode();
            break;
        }
        
        return output;
    }

    void calculateManualMode()
    {
        output = setPoint;
    }

    bool isMoving()
    {
        if (input >= 30 || input <= -30)
        {
            return true;
        }
    }

    void calculateTargetOutputMode()
    {
        input = output;
        if (input < setPoint)
        {
            output++;
        }
        else if (input > setPoint)
        {
            output--;
        }
        if (output > 100)
        {
            output = 100;
        }
        if (output < -100)
        {
            output = -100;
        }
    }

    void calculateFastTargetOutputMode()
    {
        input = output;
        const int rampRate = 2;
        const int targetThreshold = 5;
        if(input < setPoint){
            if(input < (setPoint - targetThreshold)){
                output += rampRate;
            }
            else{
                output += 1;
            }
        }
        else if(input > setPoint){
            if(input > (setPoint + targetThreshold)){
                output -= rampRate;
            }
            else{
                output -= 1;
            }
        }
        capOutput();
    }

    void slowRampControl()
    {
        // To be implemented
        if (input < setPoint)
        {
            output++;
        }
        else if (input > setPoint)
        {
            output--;
        }

        if (setPoint == 0)
        { // To ignore speed sensor noise
            if (input <= 30 && input >= -30)
            {
                output = 0;
            }
        }
    }

    void calculateDownhillMode()
    {
        int minSpeed = -50; // Minimum speed to maintain downhill movement
        int maxSpeed = -100; // Maximum speed to avoid overspeeding
        if(input <= maxSpeed)
        {
            output += 2; // Stop if we've reached or gone below the target speed
        }
        else if(input >= minSpeed)
        {
            output -= 1; // Speed up if we're below the minimum speed
        }
        capOutput();
    }

    void capOutput()
    {
        if (output > 100)
        {
            output = 100;
        }
        if (output < -100)
        {
            output = -100;
        }
    }

    void calculateTargetSpeedMode()
    {
        slowRampControl();
        capOutput();
    }

    void printStatus()
    {
        Serial.print("Setpoint:");
        Serial.print(setPoint);
        Serial.print(",");
        Serial.print("Input:");
        Serial.print(input);
        Serial.print(",");
        Serial.print("Output:");
        Serial.print(output);
        Serial.println("");
    }

    String getStatusJson()
    {
        String json = "{\"controlData\":{\"setpoint\":" + String(setPoint) + ",\"input\":" + String(input) + ",\"output\":" + String(output) + "}}";
        return json;
    }
};
#endif