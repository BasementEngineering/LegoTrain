#ifndef SPEED_SENSOR
#define SPEED_SENSOR

#define BUFFER_SIZE 1

//This class also encapsulates the interrupt service routine for the sensor.
//Interrupts only work with static functions and static fundtions can only work on static members.
//For this reason this class incorperates some staticy magic.

//!!! The class currently only works with a single rpm sensor, as all instances use the same ISR.
class SpeedSensor{
  private:
  int myPin;
  
  static unsigned long lastRoundSignal;
  static unsigned long durationWindow[BUFFER_SIZE];
  static int slotCounter;

  static void interruptRoutine();

  //Staticy stuff
  static int numberOfInstances;
  const int maxInstances = 2;

  int myInstanceNumber;
  
  public:
  SpeedSensor();  
  void begin(int pin);
  void update();
  void resetBuffer();
  unsigned long getAvgDuration();
  float getScaleSpeed();
  float getSpeedKMH();
};
#endif
