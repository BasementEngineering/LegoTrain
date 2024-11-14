#ifndef LIGHTS_H
#define LIGHTS_H

class Lights{
  public: 
  Lights(int _treePin1, int _treePin2, int _whitePin);
  void initPins();
  void update();
  void turnOff();
  void turnOn();
  
  
private:

  private:
    int treeLed1;
    int treeLed2;
    int whiteLed;

    bool isOn = true;

    unsigned long lastTreeUpdate = 0;
    unsigned long lastWhiteUpdate = 0;

    int whiteLedState = 0;
    int treeState = 0;

    void blinkTree();
    void breatheWhite();
};


#endif
