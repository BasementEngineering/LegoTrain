#ifndef PINOUT
#define PINOUT

//#define SPEED_POTI_PIN 34
#define LED_PIN 33
#define BUTTON_PIN 25


#define ANALOG_SENSOR_PIN 32
#define E_STOP_PIN 23
//25
#define MOTOR_PIN 26
// RX and TX have to be changed cause 34/35 can only be inputs

#define SOFT_TX 17 // -> goes to RX
#define SOFT_RX 16 //-> goes to TX
//SoftwareSerial softSerial(/*rx =*/SOFT_RX, /*tx =*/SOFT_TX);


/*#include <HardwareSerial.h>
HardwareSerial MySerial(2);
MySerial.begin(9600, SERIAL_8N1, SOFT_RX, SOFT_TX);
*/


#define MAGNET_BOTTOM_PIN 19
#define MAGNET_SIDE_PIN 18

#endif