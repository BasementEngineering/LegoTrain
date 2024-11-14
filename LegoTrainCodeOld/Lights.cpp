 #include "Lights.h"
 #include <Arduino.h>
  
  Lights::Lights(int _treePin1, int _treePin2, int _whitePin){
    treeLed1 = _treePin1;
    treeLed2 = _treePin2;
    whiteLed = _whitePin;
  }
  
  void Lights::initPins(){
    pinMode(treeLed1, OUTPUT);
    pinMode(treeLed2, OUTPUT);
    pinMode(whiteLed, OUTPUT);
  
  }
  
  void Lights::update(){
    if(isOn){
      blinkTree();
      breatheWhite();
    }
  }
  
  void Lights::turnOff(){
    isOn = false;
    digitalWrite(treeLed1,LOW);
    digitalWrite(treeLed2,LOW);
    digitalWrite(whiteLed,LOW);
  }
  
  void Lights::turnOn(){
    isOn = true;
  }

  void Lights::blinkTree(){
  if(millis() - lastTreeUpdate > 1000){
    digitalWrite(treeLed1,treeState);
    digitalWrite(treeLed2,!treeState);

    treeState = !treeState;
    lastTreeUpdate = millis();
  }
}

void Lights::breatheWhite(){
  if(millis() - lastWhiteUpdate > 10){
    if(whiteLedState >= 0){
      analogWrite(whiteLed, whiteLedState);
    }
    else{
      analogWrite(whiteLed, -whiteLedState);
    }

    whiteLedState++;
    if(whiteLedState > 255){
      whiteLedState = -255;
    }
    lastWhiteUpdate = millis();
  }
}
