#ifndef CONTROLLOOP_H
#define CONTROLLOOP_H

#include <Arduino.h>
enum controlModes
{
    MANUAL = 0,
    TARGET_OUTPUT = 1,
    TARGET_SPEED = 2,
    FAST_TARGET_OUTPUT = 3
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

    void reset()
    {
        setPoint = 0;
        input = 0;
        output = 0;
        startingFromStop = true;
    }

    void setMode(int mode)
    {
        Serial.print("Changing mode to ");
        Serial.println(mode);
        this->mode = mode;
        reset();
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

    /*int previousInputs[10] = {0};
    int inputIndex = 0;

    void updateInputHistory(int newInput) {
        previousInputs[inputIndex] = newInput;
        inputIndex = (inputIndex + 1) % 10;
    }*/

    // If we are going downhill, the input speed will suddenly start to increase
    // while the output is still pushing forward (>0).
    // If we are going uphill in reverse, the input speed will suddenly start to decrease (more negative)
    // while the output is still pushing backward (<0).
    // This function detects both scenarios and reduces output to compensate
    /*bool detectUnwantedInputIncrease()
    {
        if (output == 0)
        {
            return false; // No correction needed when output is zero
        }

        int unwantedChanges = 0;

        for (int i = 0; i < 9; i++)
        {
            if (output > 0)
            {
                // When going forward, detect unwanted speed increases
                if (previousInputs[i + 1] > previousInputs[i])
                {
                    unwantedChanges++;
                }
            }
            else
            {
                // When going backward, detect unwanted speed decreases (more negative = faster backward)
                if (previousInputs[i + 1] < previousInputs[i])
                {
                    unwantedChanges++;
                }
            }
        }

        if (unwantedChanges >= 7)
        { // If 7 out of last 9 readings show unwanted change
            // Take corrective action
            return true;
        }
        return false;
    }
*/
    void fasterController()
    {
        // max speed is -200mm/s to 200mm/s
        //  therefore error range is -400 to 400
        int error = setPoint - input;
        int normalizedError = map(error, -400, 400, -100, 100);
        int adjustment = normalizedError * 0.05; // Scale down adjustment

        // To be implemented
        if (input < setPoint)
        {
            output += 1;
            output += adjustment;
        }
        else if (input > setPoint)
        {
            output -= 1;
            output += adjustment;
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

    void hillCompensation()
    {
        int delta = setPoint - input;

        // Moving Forward downhill
        if (input > 0 && output > 0 && delta < -50)
        {
            Serial.println("Unwanted forward speed increase detected, reducing output");
            output -= 5; // Reduce forward output to compensate for downhill
        }
        else if (input < 0 && output < 0 && delta > 50)
        {
            Serial.println("Unwanted backward speed increase detected, reducing output magnitude");
            output += 5; // Reduce backward output magnitude to compensate for uphill in reverse
        }
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