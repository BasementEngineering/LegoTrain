#ifndef CONTROLLOOP_H
#define CONTROLLOOP_H

#include <Arduino.h>

class ControlLoop {
    
    private: 
    int setPoint;
    int input;
    int output;
    int mode;
    
    public:
    ControlLoop(){
        Serial.println("Controller created");
    }

    void reset(){
        setPoint = 0;
        input = 0;
        output = 0;
    }

    void setMode(int mode){
      Serial.print("Changing mode to ");
      Serial.println(mode);
        this->mode = mode;
        reset();
    }

    int getMode(){
      return mode;
    }

    void setTarget(int setPoint){
        this->setPoint = setPoint;
    }

    void setInput(int input){
        this->input = input;
    }

    void setOutput(int output){
        this->output = output;
    }

    int getOutput(){
        return output;
    }

    int calculateOutput(){
        switch(mode){
            case 0:
                calculateManualMode();
                break;
            case 1:
                calculateTargetOutputMode();
                break;
            case 2:
                calculateTargetSpeedMode();
                break;
            case 3:
                calculateTargetSpeedMode();
                break;
        }
        return output;
    }

    void calculateManualMode(){
        output = setPoint;
    }

    void calculateTargetOutputMode(){
        input = output;
        if(input < setPoint){
            output++;
        }
        else if(input > setPoint){
            output--;
        }
        if(output > 100){
            output = 100;
        }
        if(output < -100){
            output = -100;
        }
    }

    void calculateTargetSpeedMode(){
        if(input < setPoint){
            output++;
        }
        else if(input > setPoint){
            output--;
        }
        if(output > 100){
            output = 100;
        }
        if(output < -100){
            output = -100;
        }

        if(setPoint == 0){ //To ignore speed sensor noise
            if(input <= 30 && input >= -30){
                output = 0;
            }
        }
    }

    void printStatus(){
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

    String getStatusJson(){
        String json = "{\"controlData\":{\"setpoint\":" + String(setPoint) + ",\"input\":" + String(input) + ",\"output\":" + String(output) + "}}";
        return json;
    }

};
#endif