#ifndef MAGNETSENSORS_H
#define MAGNETSENSORS_H

#include <Arduino.h>

class MagnetSensors
{
    private:
    int bottomPin;
    int sidePin;
    unsigned long lastSensorUpdate = 0;
    bool lastBottomState = true;
    bool lastSideState = true;

    bool bottomTriggered = false;
    bool sideTriggered = false;

    public:
    MagnetSensors(int bottomPin, int sidePin){
        this->bottomPin = bottomPin;
        this->sidePin = sidePin;
    }

    void setup(){
        pinMode(bottomPin, INPUT_PULLUP);
        pinMode(sidePin, INPUT_PULLUP);
    }

    void update(){
        if((millis() - lastSensorUpdate) > 20){
            lastSensorUpdate = millis();
            
            bool bottomState = digitalRead(bottomPin);
            bool sideState = digitalRead(sidePin);

            if(lastBottomState && !bottomState){
                bottomTriggered = true;
                Serial.println("Bottom Triggered");
            }
            if(lastSideState && !sideState){
                sideTriggered = true;
                Serial.println("Side Triggered");
            }

            lastBottomState = bottomState;
            lastSideState = sideState;            
        }
    }

        bool bottomWasTriggered(){
        if(bottomTriggered){
            bottomTriggered = false;
            return true;
        }
        return false;
    }

    bool sideWasTriggered(){
        if(sideTriggered){
            sideTriggered = false;
            return true;
        }
        return false;
    }
};
#endif