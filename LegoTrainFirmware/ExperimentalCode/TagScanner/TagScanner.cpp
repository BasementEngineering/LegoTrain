#include "TagScanner.h"

#include <Wire.h>
#define PN532_IRQ   (2)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield

TagScanner::TagScanner():nfc(PN532_IRQ, PN532_RESET){
  for(int i = 0; i < 20; i++){
      latestLocation[i] = '\0';
      }
  }

  void TagScanner::begin(){
    nfc.begin();
      uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);

  Serial.println("Waiting for an ISO14443A Card ...");
  }
  
  bool TagScanner::update(){
    if( (millis() - lastDetectionTime) < tagCooldownTime){
      return false;
    }
    uint8_t timeout = 100;
    uint8_t success;
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
    uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

    uint8_t messgeLength = 20+1+3;
    uint8_t pages = messgeLength/4; //6
    
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength,timeout);

    if (success) {
      // Display some basic information about the card
      Serial.println("Found an ISO14443A card");
      Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
      Serial.print("  UID Value: ");
      nfc.PrintHex(uid, uidLength);
      Serial.println("");

      if (uidLength == 7)
      {
        Serial.println("Seems to be an NTAG2xx tag (7 byte UID)");
        char stringBuffer[24];
        for (int i = 0; i < 24; i++) {
          stringBuffer[i] = '\0';
        }

        success = readCustomContent(stringBuffer);
        if(success){
          parseContent(stringBuffer);
          lastDetectionTime = millis();
        }
    }
    else
    {
      Serial.println("This doesn't seem to be an NTAG203 tag (UUID length != 7 bytes)!");
    }
  }
  return success;
  }


  void TagScanner::parseContent(char* stringBuffer){
   // Wait a bit before trying again
    for (int i = 0; i < 24; i++) {
      Serial.print(stringBuffer[i]);
      Serial.print(" ");
    }
    String message(stringBuffer);
    message.remove(0,1);
    message.remove(-1,1);

    int seperatorIdx = message.indexOf(",");
    String locationName = message.substring(0,seperatorIdx);
    int speedLimit = message.substring(seperatorIdx+1).toInt();
    
    Serial.println();
    Serial.println(stringBuffer);
        Serial.println();
    Serial.println(locationName);
        Serial.println();
    Serial.println(speedLimit);
}

bool TagScanner::readCustomContent(char* stringBuffer){
  int stringPosition = 0;
   for (uint8_t i = 6; i < 12; i++)
      {
        uint8_t tinyBuffer[4];
        bool success = nfc.ntag2xx_ReadPage(i, tinyBuffer);
        memcpy(stringBuffer + (4 * stringPosition), tinyBuffer, 4);
        stringPosition++;
 
        if (!success)
        {
          Serial.println("Unable to read the requested page!");
          return false;
        }
      }
      return true;
}
