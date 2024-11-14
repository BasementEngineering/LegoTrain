#ifndef PROPULSIONSYSTEM_H
#define PROPULSIONSYSTEM_H

class PropulsionSystem{
  public: 
  PropulsionSystem(int _en, int _in1, int _in2);
  void initPins();

  void stop();

  void setSpeed(int newSpeed);
  void moveForward();
  void moveBackward();
  void setDirection(int value);
  
  
private:

  private:
    int en;
    int in1;
    int in2;

    int currentDirection = 0; //1 = forward, 0 = stop, -1 = backward
    int currentSpeed = 0; //Percentage

    void moveMotor();
};

#endif
