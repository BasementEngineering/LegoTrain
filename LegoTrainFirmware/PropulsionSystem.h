#ifndef PROPULSION_SYSTEM_H
#define PROPULSION_SYSTEM_H

bool armed = false;
int armChannel = 0;

Motor motor1(MOTOR1_INA,MOTOR1_INB,MOTOR_EN);

Motor* motors[1] = {&motor1};

void initMotors(){
  motor1.initPins();
}

void updateMotors(){
  motor1.update();
}

void disarm(){
  motor1.stop();
  motor1.disable(); //disables common enable pin
  armed = false;
}

void arm(){
  motor1.enable(); //disables common enable pin
  armed = true;
}

#endif
