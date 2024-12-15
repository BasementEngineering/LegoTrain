#ifndef SPEEDMONITOR_H	
#define SPEEDMONITOR_H

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class SpeedMonitor{

  public:

  enum screens{
    SPEED,
    AUTOPILOT,
    STOP_TEST
  };

  SpeedMonitor():display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET){
    this->lastUpdate = 0;
    currentScreen = STOP_TEST;
  }


  void setTestValues(int values[]){
    for(int i = 0; i < 4; i++){
      this->values[i] = values[i];
    }
  }

  void setSpeed(float speed){
    this->speed = speed;
    //Serial.println(this->speed);
  }

  void setDistance(float distance){
    this->distance = distance;
    //Serial.println(this->distance);
  }

  void setTotalDistance(int distance){
    this->totalDistance = distance;
  }

  void setAutopilotState(String state){
    this->autopilotState = state;
  }

  void update(){
    //Serial.println("Updating speed monitor");
    if((millis() - this->lastUpdate) > 1000){
      updateCounter++;
      if(updateCounter == 5){
        updateCounter = 0;
        currentScreen++;
        currentScreen%=3;
      }

      switch (currentScreen)
      {
      case SPEED: 
        drawSpeedScreen();
        break;
      case AUTOPILOT:
        drawAutopilotScreen();
        break;
      case STOP_TEST:
        drawStopTestScreen();
        break;
      
      default:
        break;
      }

      this->lastUpdate = millis();
    }
  }

  void drawStopTestScreen(){
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print("Rounds: ");
    display.println(this->values[0]);
    display.print("Incomplete: ");
    display.println(this->values[1]);
    display.print("Misses: ");
    display.println(this->values[2]);
    display.print("Shots: ");
    display.println(this->values[3]);
    display.display();
  }

  void drawAutopilotScreen(){
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Autopilot: ");
    display.setTextSize(1);
    display.print(this->autopilotState);
    display.display();
  }

  void drawSpeedScreen(){
    //Serial.println("Clearing display");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    //Serial.println("Starting to print");
    display.print("Speed: ");
    display.setTextSize(2);
    //Serial.println("Printing speed");
    display.print(String(this->speed));
    display.setTextSize(1);
    display.println(" m/s");
    display.setCursor(0, 16);
    //Serial.println("Printing distance");
    display.print("Distance: ");
    display.print(String(this->distance));
    display.println(" m");
    display.print("Total: ");
    display.println(String(this->totalDistance));
    //Serial.println("Attempting to display");
    display.display();
  }

  void begin(){
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    display.display();
    delay(500);
    display.clearDisplay();
    display.setRotation(2);
  }

  private:
    float speed;
    float distance;
    int totalDistance;
    String autopilotState;

    int values[4] = {0, 0, 0, 0};

    unsigned long lastUpdate;
    int updateCounter = 0;

    int currentScreen = STOP_TEST;

    Adafruit_SSD1306 display;
};

#endif