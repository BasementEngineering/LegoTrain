#ifndef ACTION_BUTTON_H
#define ACTION_BUTTON_H

#include <Adafruit_NeoPixel.h>

enum event_t {
    START,
    STOP,
    LOADING
};

class ActionButton {
public:
    ActionButton(uint8_t buttonPin, uint8_t ledPin, uint8_t numLeds=8): 
    _ledRing(numLeds, ledPin, NEO_GRB + NEO_KHZ800) {
        _buttonPin = buttonPin;
        _numLeds = numLeds;
        _lastButtonState = LOW;
        lastButtonUpdate = 0;
        _ledRing = Adafruit_NeoPixel(numLeds, ledPin, NEO_GRB + NEO_KHZ800);
        nextEvent = STOP;
    }
    void begin(){
        pinMode(_buttonPin, INPUT);
        _ledRing.setBrightness(150);	
        _ledRing.clear();
        _ledRing.begin();
        _ledRing.show();
    }

    void update(){
        int reading = digitalRead(_buttonPin);
        if ((millis() - lastButtonUpdate) > _debounceDelay) {
            if(nextEvent == LOADING){
                updateLoading();
            }

            lastButtonUpdate = millis();
            if(reading && !_lastButtonState){ //Rising Edge
                    eventTriggered = true;
            }
            _lastButtonState = reading;
        }
    }

    void updateLoading(){
      int scaleFactor = 5;
      int adjustedCounter = animationCounter/scaleFactor;
      //Serial.println(adjustedCounter);
        for(int i = 0; i < _numLeds; i++){
            int distance = adjustedCounter - i;
            if(distance < 0){
              distance += _numLeds;
            }
            int brightness = 255 - distance*50;
            if(brightness < 0){
              brightness = 0;
            }
           // Serial.print(brightness);Serial.print(" ");
            _ledRing.setPixelColor(i, _ledRing.Color(0, 0, brightness));
        }
        //Serial.println();
        _ledRing.show();
        animationCounter++;
        animationCounter %= (_numLeds*scaleFactor);
    }

    bool stopTriggered(){
        if(eventTriggered && nextEvent == STOP){
            eventTriggered = false;
            return true;
        }
        return false;
    }

    bool startTriggered(){
        if(eventTriggered && nextEvent == START){
            eventTriggered = false;
            return true;
        }
        return false;
    }

    void setLoading(){
        _ledRing.fill(_ledRing.Color(0, 0, 255), 0, _numLeds);
        _ledRing.show();
        nextEvent = LOADING;
        eventTriggered = false;
    }

    void setStop(){
        _ledRing.fill(_ledRing.Color(255, 0, 0), 0, _numLeds);
        _ledRing.show();
        nextEvent = STOP;
        eventTriggered = false;
    }

    void setStart(){
        _ledRing.fill(_ledRing.Color(0, 255, 0), 0, _numLeds);
        _ledRing.show();
        nextEvent = START;
        eventTriggered = false;
    }

private:
    uint8_t _buttonPin;
    uint8_t _numLeds;
    bool _lastButtonState;
    unsigned long lastButtonUpdate;
    const unsigned long _debounceDelay = 20; // Debounce delay in milliseconds

    int animationCounter = 0;

    event_t nextEvent;
    bool eventTriggered = false;

    Adafruit_NeoPixel _ledRing;
};

#endif // ACTION_BUTTON_H