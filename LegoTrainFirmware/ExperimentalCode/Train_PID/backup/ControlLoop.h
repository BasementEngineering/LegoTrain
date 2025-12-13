#ifndef CONTROLLOOP_H
#define CONTROLLOOP_H

#include <Arduino.h>

class ControlLoop {
    
    private: 
    int setPoint;
    int input;
    int output;
    int mode;
    
    // PID controller state variables
    float integral;
    int lastError;
    int lastSetPoint;
    unsigned long lastTime;
    
    // Soft start variables
    bool softStartActive;
    unsigned long softStartTime;
    int softStartMaxOutput;
    
    public:
    ControlLoop(){
        Serial.println("Controller created");
    }

    void reset(){
        setPoint = 0;
        input = 0;
        output = 0;
        
        // Reset PID state
        integral = 0;
        lastError = 0;
        lastSetPoint = 0;
        lastTime = millis();
        
        // Reset soft start
        softStartActive = false;
        softStartTime = 0;
        softStartMaxOutput = 0;
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

    bool isMoving(){
        if( input >= 30 || input <= -30){
            return true;
        }
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

    void slowRampControl(){
        //To be implemented
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

    void pidControl(){
        unsigned long now = millis();
        float dt = (now - lastTime) / 1000.0; // Convert to seconds
        
        // Skip if time hasn't advanced
        if (dt <= 0) return;
        
        // Check if we need to start soft start
        if (abs(setPoint) > 0 && !isMoving() && !softStartActive) {
            softStartActive = true;
            softStartTime = now;
            softStartMaxOutput = 15; // Start with low power
        }
        
        // Check if soft start should end
        if (softStartActive && (isMoving() || (now - softStartTime > 3000))) {
            softStartActive = false;
        }
        
        // Reset soft start if setpoint becomes 0
        if (setPoint == 0) {
            softStartActive = false;
            if (abs(input) <= 30) { // Handle noise tolerance
                output = 0;
                integral = 0; // Reset integral when stopped
                lastError = 0;
                lastTime = now;
                return;
            }
        }
        
        int error = setPoint - input;
        
        // PID gains - tune these for your system
        float kP = 0.8;    // Proportional gain
        float kI = 0.2;    // Integral gain  
        float kD = 0.1;    // Derivative gain
        
        // Calculate integral with windup protection
        integral += error * dt;
        
        // Integral windup protection - clamp integral term
        float integralMax = 50.0;  // Maximum integral contribution
        if (integral > integralMax) integral = integralMax;
        if (integral < -integralMax) integral = -integralMax;
        
        // Calculate derivative
        float derivative = (error - lastError) / dt;
        
        // PID output calculation
        float pidOutput = (kP * error) + (kI * integral) + (kD * derivative);
        
        // Convert to integer
        output = (int)pidOutput;
        
        // Apply soft start limiting
        if (softStartActive) {
            // Gradually increase max power during soft start
            unsigned long elapsed = now - softStartTime;
            softStartMaxOutput = 15 + (elapsed / 100); // Increase by 1 every 100ms
            if (softStartMaxOutput > 60) softStartMaxOutput = 60; // Cap at 60% during soft start
            
            // Limit output during soft start
            if (output > softStartMaxOutput) output = softStartMaxOutput;
            if (output < -softStartMaxOutput) output = -softStartMaxOutput;
        } else {
            // Normal operation - clamp to motor limits
            if (output > 100) output = 100;
            if (output < -100) output = -100;
        }
        
        // Reset integral if setpoint changes significantly
        if (abs(setPoint - lastSetPoint) > 15) {
            integral = 0;
            softStartActive = false; // Restart soft start on big changes
        }
        
        // Additional windup protection
        if ((output >= 100 && integral > 0) || (output <= -100 && integral < 0)) {
            integral *= 0.9;
        }
        
        // Update state variables
        lastError = error;
        lastTime = now;
        lastSetPoint = setPoint;
    }

    void calculateTargetSpeedMode(){
        pidControl(); // PID now handles both soft start and normal operation
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