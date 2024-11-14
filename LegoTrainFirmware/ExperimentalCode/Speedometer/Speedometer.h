#ifndef SPEEDOMETER_H
#define SPEEDOMETER_H

#include <Wire.h>
#include <AS5600.h>
#include <Arduino.h>

#define SENSE_INTERVAL 1 //Check sensor every ms to securely detect rollovers

#define COUNTER_LIMIT 100

static volatile int counter = 0;
static volatile int angle_sum = 0;
const int circumference_mm = 50;
const double stepToMm = 0.138888; // circumference_cm/360
const int timeMultiplier = 10; //Calculate speed every 10 ms

static volatile int currentSpeed = 0.0;
static volatile int newAngle = 0;
static volatile bool newReading = false;
static volatile int oldAngle = 0;
static volatile int revolutionDelta = 0;
static int analogSensePin;

#define JIGGLE_OFFSET 1

//Flats are not allowed in isr https://www.reddit.com/r/esp32/comments/lj2nkx/just_discovered_that_you_cant_use_floats_in_isr/?rdt=40811
void IRAM_ATTR getReading() {
      newAngle = analogRead(analogSensePin);
      newAngle = map(newAngle, 0, 4095, 0, 360);
      int newSpeed_mm_s = 0;

      if(newAngle > (oldAngle+JIGGLE_OFFSET) || newAngle < (oldAngle-JIGGLE_OFFSET)){
        int angleDelta = 0;

        if((oldAngle < 150) && (newAngle > 210)){ //Rollunder
            angleDelta = (360 - newAngle) + oldAngle;
            revolutionDelta --;
        }
        else if((oldAngle > 210) && (newAngle < 150)){ //Rollover
            angleDelta = newAngle + (360 - oldAngle);
            revolutionDelta ++;
        }
        else{
            angleDelta = newAngle - oldAngle;
        }

        angle_sum+=angleDelta;
      }

      oldAngle = newAngle;
      counter++;

      if(counter == COUNTER_LIMIT){
        counter = 0;
        currentSpeed = angle_sum * timeMultiplier * stepToMm;
        newReading = true;
        angle_sum = 0;
      }
    }

hw_timer_t *Timer0_Cfg = NULL;

void setupTimerInterrupt(){
  Timer0_Cfg = timerBegin(1000000);
  timerAttachInterrupt(Timer0_Cfg, &getReading);
  timerAlarm(Timer0_Cfg, SENSE_INTERVAL * 1000, true, 0);
}

class Speedometer {
  private:
    AMS_5600 ams5600;
    unsigned long lastUpdate = 0;

    bool inverted = false;  

  public:
    Speedometer(int analogPin) {
      analogSensePin = analogPin;
    }

    int getInterval() {
      return SENSE_INTERVAL;
    }

    void setupEncoder() {
      Wire.begin(); //SDA, SCL
      Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>> ");
      if (ams5600.detectMagnet() == 0 ) {
        while (1) {
          if (ams5600.detectMagnet() == 1 ) {
            Serial.print("Current Magnitude: ");
            Serial.println(ams5600.getMagnitude());
            ams5600.setOutPut(1);
            break;
          }
          else {
            Serial.println("Can not detect magnet");
          }
          delay(1000);
        }
      }
      pinMode(analogSensePin, INPUT);
    }

    float convertRawAngleToDegrees(word newAngle)
    {
      /* Raw data reports 0 - 4095 segments, which is 0.087890625 of a degree */
      float retVal = newAngle * 0.087890625;
      return retVal;
    }

    bool updateAvailable(){
      if(newReading){
        newReading = false;
        return true;
      }
      return false;
    }

    int getSpeed() {
      return currentSpeed;
    }

    float getDistance() {
      return revolutionDelta * circumference_mm;
    }
};

#endif
