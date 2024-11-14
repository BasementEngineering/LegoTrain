#include "config.h"
#include "dataStructures.h"
#include "Motor.h"
#include "AnalogSensor.h"
#include "PropulsionSystem.h"
#include "Lights.h"
#include "SpeedSensor.h"

#include <Adafruit_NeoPixel.h>

#include <BluetoothSerial.h>
BluetoothSerial COMM_SERIAL;

#include <EEPROM.h>

#include <ArduinoJson.h>

Lights lights(LED_OUT);
Adafruit_NeoPixel ledStrip(14,LED_STRIP,NEO_GRB + NEO_KHZ800);

#define OUTPUT_COUNT 2
OutputDevice* outputDevices[OUTPUT_COUNT] = {&motor1,&lights};

uint32_t messageTime = 0;
uint32_t sensorUpdateInterval = 50; //how long between each sensor update - milliseconds

float battery_maxVoltage = 12.6;
float battery_minVoltage = 9.0;
bool errorDetected = false;

char receiveBuffer[RECEIVED_BUFFER_LENGTH];
unsigned int receivedChars = 0;
uint32_t lastHeartbeat = 0;

uint32_t lastLedUpdate = 0;

SpeedSensor mySpeedSensor;

//*** Function Prototypes ***//
void blinkLed();
void setupLedLights();
void updateLedLights();

//Sensor Functions
void updateSensors();
void setUpdateInterval(uint16_t newValue);

//Low Level Serial Functions
void checkHeartbeat();
void clearInputBuffer();
void checkInput();
void process_input(String indata);

// Higher Level Parsing
void sendJson(JsonDocument& document);
void parseInput(DynamicJsonDocument& input);

//*** CHANNELS JSON Object ***//
void parseChannels(DynamicJsonDocument& input);

//*** SENSOR JSON Object ***//
void sendSensorData();

//*** SETTINGS JSON Object ***//
void initSettings();
void parseSettings(DynamicJsonDocument& input);

//*** Setting Storage Functions ***//
bool loadSettings(DynamicJsonDocument& settings);
void saveSettings(DynamicJsonDocument& settings);

//Detailed Settings Functions calling setters and getters
bool setMotorSettings(JsonDocument& newSettings);
void setSensorSettings(JsonDocument& newSettings);
void setBatterySettings(JsonDocument& newSettings);
void setAccesorySettings(JsonDocument& newSettings);
void setSettings(JsonDocument& newSettings);
void getMotorSettings(JsonDocument& settingsBuffer);
void getSensorSettings(JsonDocument& settingsBuffer);
void getBatterySettings(JsonDocument& settingsBuffer);
void getAccesorySettings(JsonDocument& settingsBuffer);
void getSettings(JsonDocument& settingsBuffer);
bool checkSettingsValidity(JsonDocument& newSettings);

//Printing Functions for Debugging
#if DEBUG_REPORT
uint32_t receivedMessages = 0;
uint32_t sentMessages = 0;
uint32_t lastDebugUpdate = 0;
void printDebugReport();
#endif

//*** Main Program ***//
void setup() {
  #if DEBUG
  DEBUG_SERIAL.begin(115200);
  DEBUG_SERIAL.println("Starting Microcontroller");
  #endif

  pinMode(BUTTON_IN,INPUT);

  setupLedLights();
  
  initMotors();
  lights.setChannel(4);
  
  initSettings();

  pinMode(DEBUG_LED,OUTPUT);
  analogReadResolution(12);

  clearInputBuffer();

  mySpeedSensor.begin(SPEED_SENSOR_IN);
  
  COMM_SERIAL.begin("LEGO Train");
  
  #if DEBUG
  DEBUG_SERIAL.println("Setup Done");
  #endif
  delay(1000);
}

void loop() {
  //testJsonParsing();
  updateMotors();
  updateSensors();
  updateLedLights();
  
  unsigned long time_diff = millis() - messageTime;

  if (time_diff >= sensorUpdateInterval) {
    messageTime = millis();
    sendSensorData();
  }

  while(COMM_SERIAL.available()){
  checkInput();
  }
  checkHeartbeat();
  if(errorDetected){
    blinkLed();
  }
  
  #if DEBUG_REPORT
  printDebugReport();
  #endif
}

//*** Function Implementations ***//
void blinkLed(){
  if((millis() - lastLedUpdate) > 200){
    lastLedUpdate = millis();
    digitalWrite(DEBUG_LED,!digitalRead(DEBUG_LED));
  }
}

const int nrOfGlassLeds = 4;
int glassLeds[nrOfGlassLeds] = {1,4,8,11};
  
void setupLedLights(){
  ledStrip.begin();
  ledStrip.setBrightness(255);
  ledStrip.clear();
  
  ledStrip.setPixelColor(13, ledStrip.Color(255, 150, 50));
  ledStrip.setPixelColor(12, ledStrip.Color(100, 50, 10));

  for(int i = 0; i < 12;i++){
    ledStrip.setPixelColor(i, ledStrip.Color(0,0,0));
    for(int j = 0;j<nrOfGlassLeds; j++){
      if(glassLeds[j] == i){
        ledStrip.setPixelColor(i, ledStrip.Color(100, 50, 10));
      }
    }
  }
  ledStrip.show();
}

int effectCounter = 0;
unsigned long lastStripUpdate = 0;
bool ledsWaiting = true;
void updateLedLights(){ 
  if( (millis() - lastStripUpdate) < 20){
    return;
  }
  lastStripUpdate = millis();

  effectCounter++;
  int effectLimit =100*2;
  effectCounter%=effectLimit;

  float brightnessMultiplier = 1.0;
  if(motor1.getPower() == 0){
    if(effectCounter <= 100){
      brightnessMultiplier = (float)effectCounter/100.0;
    }
    else{
      brightnessMultiplier = 2.0 - (float)effectCounter/100.0 ;
    }
  }
  
  
  ledStrip.setPixelColor(13, ledStrip.Color(255, 150, 50));
  ledStrip.setPixelColor(12, ledStrip.Color(100, 50, 10));

  for(int i = 0; i < 12;i++){
    ledStrip.setPixelColor(i, ledStrip.Color(0,0,0));
    for(int j = 0;j<nrOfGlassLeds; j++){
      if(glassLeds[j] == i){
        ledStrip.setPixelColor(i, ledStrip.Color(brightnessMultiplier*100, brightnessMultiplier*50, brightnessMultiplier*10));
      }
    }
  }
  ledStrip.show();
}

//Sensor Functions
bool lastState = false;
unsigned long lastTouchUpdate = 0;

void updateSensors(){
  mySpeedSensor.update();
  
  if((millis()-lastTouchUpdate)>20){
    lastTouchUpdate = millis();
    bool state = digitalRead(BUTTON_IN);
    if(state && !lastState){
      Serial.println("Marker detected, stopping vehicle");
      outputDevices[0]->setPower(0);
    }
    lastState = state;
  }
}

void setUpdateInterval(uint16_t newValue){
   sensorUpdateInterval = (10 > newValue) ? 10 : newValue;
}


//Low Level Serial Functions //
void checkHeartbeat(){
  if((millis()-lastHeartbeat) > TIMEOUT){
    disarm();
    errorDetected = true;
  }
  else{
    errorDetected = false;
  }
}

int openedBrackets = 0;
 
void clearInputBuffer(){
  memset( receiveBuffer, '\0', sizeof(char)*RECEIVED_BUFFER_LENGTH );
  receivedChars = 0;
  openedBrackets = 0;
}

//Gets the input from the UART one char at a time and transfers it to a buffer.
//This method is non-blocking.
void checkInput(){
  if(COMM_SERIAL.available()){
    receiveBuffer[receivedChars] = COMM_SERIAL.read();
    if(receiveBuffer[receivedChars] == '\n'){
      #if DEBUG_SERIAL_PARSING
      DEBUG_SERIAL.println("received endline character ");
      #endif
      clearInputBuffer();
      return;
    }
    
    if(receiveBuffer[receivedChars] == '{'){
      openedBrackets++;
      #if DEBUG_SERIAL_PARSING
      DEBUG_SERIAL.print("openedBrackets: ");
      DEBUG_SERIAL.println(openedBrackets);
      DEBUG_SERIAL.print("receivedChars: ");
      DEBUG_SERIAL.println(receivedChars);
      #endif
    }
    else if(receiveBuffer[receivedChars] == '}'){
      openedBrackets--;
      #if DEBUG_SERIAL_PARSING
      DEBUG_SERIAL.println("openedBrackets: ");
      DEBUG_SERIAL.println(openedBrackets);
      DEBUG_SERIAL.print("receivedChars: ");
      DEBUG_SERIAL.println(receivedChars);
      #endif
      
      if(openedBrackets == 0){
        lastHeartbeat = millis();
        #if DEBUG_SERIAL_PARSING
          DEBUG_SERIAL.println("Received <-");
          DEBUG_SERIAL.println(receiveBuffer);
          DEBUG_SERIAL.println("Detected End of message");
        #endif
        #if DEBUG_REPORT
        receivedMessages++;
        #endif
        String input = receiveBuffer;
        process_input(input);
        clearInputBuffer();
        return;
      }
    }
    
    receivedChars++;
    if(receivedChars == RECEIVED_BUFFER_LENGTH){
    
    DEBUG_SERIAL.println("Buffer overflow");

    clearInputBuffer();
    }
  }
}

void process_input(String indata){
  #if DEBUG_PARSING
      DEBUG_SERIAL.println("Input: "+indata);
  #endif

  DynamicJsonDocument input(1024);
  deserializeJson(input, indata);
  parseInput(input);
}

// Higher Level Parsing
void sendJson(JsonDocument& document){
  serializeJson(document, COMM_SERIAL);
  COMM_SERIAL.println();
  
  #if DEBUG_RESPONSE
  COMM_SERIAL.println();
  DEBUG_SERIAL.println("Sent ->");
  serializeJson(document, DEBUG_SERIAL);
  DEBUG_SERIAL.println("");
  #endif
  #if DEBUG_REPORT
  sentMessages++; 
  #endif
}

void parseInput(DynamicJsonDocument& input){
  JsonVariant newSettings = (input)["outputSettings"];
  JsonVariant newChannels = (input)["channels"];
  
  if(!newSettings.isNull()){
    #if DEBUG_PARSING
    DEBUG_SERIAL.println("Detected Settings");
    #endif
    parseSettings(input);
  }
  if(!newChannels.isNull()){
    #if DEBUG_PARSING
    DEBUG_SERIAL.println("Detected Channels");
    #endif
    parseChannels(input); 
  }
}

//*** CHANNELS JSON Object ***//

// All channels can contain values from +100 to -100
//{"channels":[mot1,mot2,mot3/4,0,arm,led]}
//{"channels":[80,0,0,0,1,90]}
void parseChannels(DynamicJsonDocument& input){
  bool newArmed = input["channels"][armChannel].as<bool>();

  if(!newArmed && armed){
    disarm();
  }
  else if(newArmed && !armed){
    arm();
  }

    
  armed = newArmed;
  
  for(int i = 0; i < OUTPUT_COUNT; i++){
    int channel = outputDevices[i]->getChannel();
    int value = input["channels"][channel].as<int>(); 
    bool dangerous = outputDevices[i]->isDangerous();

    #if DEBUG_PARSING
      DEBUG_SERIAL.println("");
      DEBUG_SERIAL.print("OutputDevice ");
      DEBUG_SERIAL.println(i);
      DEBUG_SERIAL.print("channel ");
      DEBUG_SERIAL.println(channel);
      DEBUG_SERIAL.print("value ");
      DEBUG_SERIAL.println(value);
    #endif
      
    if( armed || (!dangerous)){
      #if DEBUG_PARSING
      DEBUG_SERIAL.println("setting output");
      #endif
      outputDevices[i]->setPower(value);
    }
    else{
      #if DEBUG_PARSING
      DEBUG_SERIAL.println("too dangerous !");
      #endif
      outputDevices[i]->setPower(0);
    } 
  }
}

//*** SENSOR JSON Object ***//
void sendSensorData(){
  StaticJsonDocument<256> sensors;
  float avgSpeed = mySpeedSensor.getSpeedCMS();
  float avgCurrent = 0.0;
  float avgVoltage = 0.0;
  #if DEBUG_SENSORS
  DEBUG_SERIAL.println("adding sensor values");
  DEBUG_SERIAL.println(avgCurrent);
  DEBUG_SERIAL.println(avgVoltage);
  #endif
  
  sensors["sensors"]["motorCurrent"] = (int)(avgCurrent * 100 + 0.5) / 100.0;
  sensors["sensors"]["batteryVoltage"] = (int)(avgVoltage * 100 + 0.5)/ 100.0;
  sensors["sensors"]["speed"] = (int)(avgSpeed * 100 + 0.5)/ 100.0;

  for(int i = 0; i < CHANNEL_COUNT; i++){
    sensors["sensors"]["outputs"][i] = 0; 
  }
  
  for(int i = 0; i < OUTPUT_COUNT; i++){
    int channel = outputDevices[i]->getChannel();
    if(channel >= 0){
      sensors["sensors"]["outputs"][channel] = outputDevices[i]->getPower();
    } 
  }
  
  sensors["sensors"]["outputs"][armChannel] = (int)armed;
  
  #if DEBUG_SENSORS
  DEBUG_SERIAL.println("new sensor JSON");
  serializeJsonPretty(sensors,DEBUG_SERIAL);
  #endif
  sendJson(sensors);
}

//*** SETTINGS JSON Object ***//
void initSettings(){
  #if DEBUG_SETTINGS
  delay(5000);
  DEBUG_SERIAL.println("Initializing Settings");
  #endif

  DynamicJsonDocument settings(RECEIVED_BUFFER_LENGTH);
  
  if(!loadSettings(settings)){
    #if DEBUG_SETTINGS
    DEBUG_SERIAL.println("did not find stored settings");
    DEBUG_SERIAL.println("loading default:");
    DEBUG_SERIAL.println(DEFAULT_SETTINGS);
    #endif
    deserializeJson(settings,DEFAULT_SETTINGS);
  }
  #if DEBUG_SETTINGS
  else{
    DEBUG_SERIAL.println("found stored settings");
  }
  #endif
  setSettings(settings);
}

void parseSettings(DynamicJsonDocument& input)
{
    #if DEBUG_SETTINGS
    DEBUG_SERIAL.println("New outputSettings available");
    serializeJsonPretty(input,DEBUG_SERIAL);
    DEBUG_SERIAL.println();
    #endif
      
    if( ! input["outputSettings"]["propulsionSystem"].isNull() ){

      if(checkSettingsValidity(input)){
        #if DEBUG_SETTINGS
        DEBUG_SERIAL.println("JSON object is valid");
        #endif
        setSettings(input);
      }
      else{
      #if DEBUG_SETTINGS
      DEBUG_SERIAL.println("JSON object is invalid");
      #endif
      }
       
    }

    if(! input["outputSettings"]["dafault"].isNull() ){
      #if DEBUG_SETTINGS
      DEBUG_SERIAL.println("default requested");
      DEBUG_SERIAL.println("restoring default");
      #endif
      DynamicJsonDocument defaultSettings(RECEIVED_BUFFER_LENGTH);
      deserializeJson(defaultSettings,DEFAULT_SETTINGS);
      setSettings(defaultSettings);
    }

    DynamicJsonDocument output(1024);
    getSettings(output);
    if(! input["outputSettings"]["save"].isNull() ){
      if( input["outputSettings"]["save"].as<bool>()){
        #if DEBUG_SETTINGS
        DEBUG_SERIAL.println("Saving current outputSettings");
        #endif
        saveSettings(output);
      }
    }
    sendJson(output);
}

//*** Setting Storage Functions ***//
bool loadSettings(DynamicJsonDocument& settings){
  EEPROM.begin(4096);
  if(EEPROM.read(0) == FLASH_INTEGRITY_NR){
    
    uint16_t length = 0;
    
    length = (EEPROM.read(1) << 8);
    length |= EEPROM.read(2);
  
    char dataString [length+1];
    
    for(int i = 0; i < length; i++){
      dataString[i] = EEPROM.read(i+3);
    }
    dataString[length] = '\0';

    #if DEBUG_SAVING
    DEBUG_SERIAL.println("found settings string in EEPROM");
    DEBUG_SERIAL.print("length: ");
    DEBUG_SERIAL.println(length);
    DEBUG_SERIAL.print("content: ");
    DEBUG_SERIAL.println(dataString);
    #endif

    deserializeJson(settings, String(dataString));
    return true;
  }
  return false;
}

void saveSettings(DynamicJsonDocument& settings){
  EEPROM.write(0,FLASH_INTEGRITY_NR);
  
  String settingsString;
  serializeJson(settings, settingsString);
  uint16_t length = settingsString.length();

  #if DEBUG_SAVING
  DEBUG_SERIAL.println("Saving settingsString");
  DEBUG_SERIAL.println(settingsString);
  DEBUG_SERIAL.println("length: ");
  DEBUG_SERIAL.println(length);
  #endif

  EEPROM.write(1, length >> 8);
  EEPROM.write(2, length);

  for(int i = 0; i < length; i++){
    EEPROM.write(i+3,settingsString[i]);
  }
  EEPROM.commit();
}

//Detailed Settings Functions calling setters and getters
bool setMotorSettings(JsonDocument& newSettings){
  #if DEBUG_SETTINGS
   DEBUG_SERIAL.print("setting motor settings ");
  #endif
  
  for(int i = 0; i < 1; i++){
    int channel = newSettings["outputSettings"]["propulsionSystem"]["motors"][i]["channel"].as<int>();
    bool polarity = newSettings["outputSettings"]["propulsionSystem"]["motors"][i]["polarity"].as<bool>();
    int minPower = newSettings["outputSettings"]["propulsionSystem"]["motors"][i]["minPower"].as<int>();
    int maxPower = newSettings["outputSettings"]["propulsionSystem"]["motors"][i]["maxPower"].as<int>();
    int responseTime = newSettings["outputSettings"]["propulsionSystem"]["motors"][i]["responseTime"].as<int>();

    motors[i]->setChannel(channel);
    motors[i]->setPolarity(polarity);
    motors[i]->setMinPower(minPower);
    motors[i]->setMaxPower(maxPower);
    motors[i]->setResponseTime(responseTime);
  }
  armChannel = newSettings["outputSettings"]["propulsionSystem"]["armChannel"];
  
  return true;
}

void setSensorSettings(JsonDocument& newSettings){
  float voltageFactor = newSettings["outputSettings"]["sensors"]["voltageFactor"].as<float>();
  float currentFactor = newSettings["outputSettings"]["sensors"]["currentFactor"].as<float>();
  int newUpdateInterval = newSettings["outputSettings"]["sensors"]["updateInterval"].as<int>();

  setUpdateInterval(newUpdateInterval);
}

void setBatterySettings(JsonDocument& newSettings){
  battery_maxVoltage = newSettings["outputSettings"]["battery"]["maxVoltage"];
  battery_minVoltage = newSettings["outputSettings"]["battery"]["minVoltage"];
}

void setAccesorySettings(JsonDocument& newSettings){
  int ledChannel = newSettings["outputSettings"]["accessories"]["ledChannel"].as<int>();
  int maxLedPower = newSettings["outputSettings"]["accessories"]["maxLedPower"].as<int>();

  lights.setMaxPower(maxLedPower);
  lights.setChannel(ledChannel);
}

void setSettings(JsonDocument& newSettings){
  #if DEBUG_SETTINGS
  DEBUG_SERIAL.println("setting settings");
  serializeJson(newSettings,DEBUG_SERIAL);
  DEBUG_SERIAL.println("");
  #endif
  setMotorSettings(newSettings);
  setBatterySettings(newSettings);
  setSensorSettings(newSettings);
  setAccesorySettings(newSettings);
}

void getMotorSettings(JsonDocument& settingsBuffer){
  for(int i = 0; i < 4; i++){
    settingsBuffer["outputSettings"]["propulsionSystem"]["motors"][i]["channel"] = motors[i]->getChannel();
    settingsBuffer["outputSettings"]["propulsionSystem"]["motors"][i]["polarity"] = motors[i]->getPolarity();
    settingsBuffer["outputSettings"]["propulsionSystem"]["motors"][i]["minPower"] = motors[i]->getMinPower();
    settingsBuffer["outputSettings"]["propulsionSystem"]["motors"][i]["maxPower"] = motors[i]->getMaxPower();
    settingsBuffer["outputSettings"]["propulsionSystem"]["motors"][i]["responseTime"] = motors[i]->getResponseTime();
  }
  settingsBuffer["outputSettings"]["propulsionSystem"]["armChannel"] = armChannel;
}

void getSensorSettings(JsonDocument& settingsBuffer){
  settingsBuffer["outputSettings"]["sensors"]["voltageFactor"] = 0.0;
  settingsBuffer["outputSettings"]["sensors"]["currentFactor"] = 0.0;
  settingsBuffer["outputSettings"]["sensors"]["updateInterval"] = sensorUpdateInterval;
}

void getBatterySettings(JsonDocument& settingsBuffer){
  settingsBuffer["outputSettings"]["battery"]["maxVoltage"] = (int)(battery_maxVoltage * 100 + 0.5) / 100.0;
  settingsBuffer["outputSettings"]["battery"]["minVoltage"] = (int)(battery_minVoltage * 100 + 0.5) / 100.0;
}

void getAccesorySettings(JsonDocument& settingsBuffer){
  settingsBuffer["outputSettings"]["accessories"]["ledChannel"] = lights.getChannel();
  settingsBuffer["outputSettings"]["accessories"]["maxLedPower"] = lights.getMaxPower();
}

void getSettings(JsonDocument& settingsBuffer){
  #if DEBUG_SETTINGS
  DEBUG_SERIAL.println("getting settings");
  #endif
  
  getMotorSettings(settingsBuffer);
  getSensorSettings(settingsBuffer);
  getBatterySettings(settingsBuffer);
  getAccesorySettings(settingsBuffer);
  
  #if DEBUG_SETTINGS
  serializeJson(settingsBuffer,DEBUG_SERIAL);
  DEBUG_SERIAL.println("");
  #endif
}

bool checkSettingsValidity(JsonDocument& newSettings){
  bool error = false;
  for(int i = 0; i < 4; i++){
    error = error || newSettings["outputSettings"]["propulsionSystem"]["motors"][i]["channel"].isNull();
    error = error || newSettings["outputSettings"]["propulsionSystem"]["motors"][i]["polarity"].isNull();
    error = error || newSettings["outputSettings"]["propulsionSystem"]["motors"][i]["minPower"].isNull();
    error = error || newSettings["outputSettings"]["propulsionSystem"]["motors"][i]["maxPower"].isNull();
    error = error || newSettings["outputSettings"]["propulsionSystem"]["motors"][i]["responseTime"].isNull();
  }
  error = error || newSettings["outputSettings"]["propulsionSystem"]["armChannel"].isNull();
  if(error){
    #if DEBUG_SETTINGS
    DEBUG_SERIAL.println("PropulsionSystem settings invalid");
    #endif
    return false;
  }
  
  error = error || newSettings["outputSettings"]["sensors"]["voltageFactor"].isNull();
  error = error || newSettings["outputSettings"]["sensors"]["currentFactor"].isNull();
  error = error || newSettings["outputSettings"]["sensors"]["updateInterval"].isNull();
  if(error){
    #if DEBUG_SETTINGS
    DEBUG_SERIAL.println("Sensor settings invalid");
    #endif
    return false;
  }

  error = error || newSettings["outputSettings"]["battery"]["maxVoltage"].isNull();
  error = error || newSettings["outputSettings"]["battery"]["minVoltage"].isNull();
  if(error){
    #if DEBUG_SETTINGS
    DEBUG_SERIAL.println("Battery settings invalid");
    #endif
    return false;
  }

  error = error || newSettings["outputSettings"]["accessories"]["ledChannel"].isNull();
  error = error || newSettings["outputSettings"]["accessories"]["maxLedPower"].isNull();
  if(error){
    #if DEBUG_SETTINGS
    DEBUG_SERIAL.println("Accesory settings invalid");
    #endif
    return false;
  }
  
  return true;
}

//Printing Functions for Debugging
#if DEBUG_REPORT
void printDebugReport(){
  if( (millis()-lastDebugUpdate) > 5000 ){
    lastDebugUpdate = millis();
    DEBUG_SERIAL.println("DEBUG REPORT");
    DEBUG_SERIAL.println("==============");
    DEBUG_SERIAL.print("Error     : ");DEBUG_SERIAL.println(errorDetected);
    DEBUG_SERIAL.print("Armed     : ");DEBUG_SERIAL.println(armed);
    DEBUG_SERIAL.println("Communication");
    DEBUG_SERIAL.println("----------");
    DEBUG_SERIAL.print("-Received : ");DEBUG_SERIAL.println(receivedMessages);
    DEBUG_SERIAL.print("-Sent     : ");DEBUG_SERIAL.println(sentMessages);
    DEBUG_SERIAL.println("Sensors");
    DEBUG_SERIAL.println("----------");
    DEBUG_SERIAL.print("-Voltage  : ");DEBUG_SERIAL.println(0.0);
    DEBUG_SERIAL.print("-Current  : ");DEBUG_SERIAL.println(0.0);
    DEBUG_SERIAL.println("Outputs");
    DEBUG_SERIAL.println("----------");
    DEBUG_SERIAL.print("-Motor1   : ");DEBUG_SERIAL.println(motor1.getPower());
    DEBUG_SERIAL.print("-Motor2   : ");DEBUG_SERIAL.println(motor2.getPower());
    DEBUG_SERIAL.print("-Motor3   : ");DEBUG_SERIAL.println(motor3.getPower());
    DEBUG_SERIAL.print("-Motor4   : ");DEBUG_SERIAL.println(motor4.getPower());
    DEBUG_SERIAL.print("-Lights   : ");DEBUG_SERIAL.println(lights.getPower());
    DEBUG_SERIAL.println("==============");
  }
}
#endif
