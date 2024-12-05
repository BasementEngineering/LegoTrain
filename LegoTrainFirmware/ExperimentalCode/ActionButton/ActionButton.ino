#include "ActionButton.h"

#include "Pinout.h"

ActionButton actionButton(BUTTON_PIN, LED_PIN);

void setup() {
    Serial.begin(9600);
    actionButton.begin();
    actionButton.setStop();
}

void loop() {
    actionButton.update();
    if(actionButton.stopTriggered()){
        Serial.println("Stop triggered");
        actionButton.setStart();
    }
    else if(actionButton.startTriggered()){
        Serial.println("Start triggered");
        actionButton.setLoading();
    }
}