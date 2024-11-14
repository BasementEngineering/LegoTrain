#ifndef ANALOG_SENSOR_H
#define ANALOG_SENSOR_H

class AnalogSensor{
  public:
  AnalogSensor(int analogPin, float factor = 1.0, int windowSize=10, int updateInterval=10,int zeroOffset = 0,int decimalPlaces = 2);
  ~AnalogSensor();
  
  void update();
  float getAvg();
  void setCorrectionFactor(float newFactor){
    _factor = newFactor;
  }

  float getCorrectionFactor(){
    return _factor;
  }
  
  private: 
  int _analogPin;
  float _factor;
  int _windowSize;
  int _zeroOffset;
  int _decimalPlaces;

  int _updateInterval;

  unsigned long lastUpdate;
  unsigned int* dataBuffer;
  int position;

  float getValue();
  void addValue(float newValue);
};
#endif
