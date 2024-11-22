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
  SpeedMonitor():display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET){
    this->lastUpdate = 0;
  }

  void setSpeed(float speed){
    this->speed = speed;
    //Serial.println(this->speed);
  }

  void setDistance(float distance){
    this->distance = distance;
    //Serial.println(this->distance);
  }

  void update(){
    Serial.println("Updating speed monitor");
    if((millis() - this->lastUpdate) > 1000){
      Serial.println("About to draw");
      draw();
      Serial.println("Drew");
      this->lastUpdate = millis();
    }
  }

  void draw(){
    Serial.println("Clearing display");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    Serial.println("Starting to print");
    display.print("Speed: ");
    display.setTextSize(2);
    Serial.println("Printing speed");
    display.print(String(this->speed));
    display.setTextSize(1);
    display.println(" m/s");
    display.setCursor(0, 16);
    Serial.println("Printing distance");
    display.print("Distance: ");
    display.setTextSize(2);
    display.print(String(this->distance));
    display.setTextSize(1);
    display.println(" m");
    Serial.println("Attempting to display");
    display.display();
  }

  void begin(){
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    display.display();
    delay(500);
    display.clearDisplay();
    display.setRotation(2);
    draw();
  }

  private:
    float speed;
    float distance;
    unsigned long lastUpdate;
    Adafruit_SSD1306 display;
};

#endif