#ifndef TAG_SCANNER
#define TAG_SCANNER

#include <Adafruit_PN532.h>

class TagScanner{
  private:
  unsigned long lastDetectionTime= 0;
  int tagCooldownTime = 2000;
  char latestLocation[20];
  int latestSpeedLimit;
  Adafruit_PN532 nfc;

  bool readCustomContent(char* stringBuffer);
  void parseContent(char* stringBuffer);

  public:
  TagScanner();
  void begin();
  bool update();
  char* getLatestLocation(){
    return latestLocation;
  }
  int getLatestSpeedLimit(){
    return latestSpeedLimit;
  }
};

#endif
