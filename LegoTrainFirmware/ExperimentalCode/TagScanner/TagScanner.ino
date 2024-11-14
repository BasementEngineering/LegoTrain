#include "TagScanner.h"

TagScanner myScanner;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello World");
  myScanner.begin();
}

void loop() {
  if(myScanner.update()){
    
  }
}
