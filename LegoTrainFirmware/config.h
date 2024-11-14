#ifndef CONFIG_H
#define CONFIG_H

//*** Pin Definitions ***//
/*
 * ESP32 3C
 * Strapping Pins: 2,8,9
 * 
 * 
 */
#define DEBUG_LED 2
 
#define LED_OUT 15

#define LED_STRIP 14

//Sensors
#define BUTTON_IN 35
#define SPEED_SENSOR_IN 32

//Motor Pins
#define MOTOR_EN -1
#define MOTOR1_INA 25
#define MOTOR1_INB 26

#define SERIAL_TX 17 
#define SERIAL_RX 16 

#define MY_SDA 21
#define MY_SCL 22

//*** Communication Definitions ***//
#include <Arduino.h>
//#define COMM_SERIAL BluetoothSerial
#define DEBUG_SERIAL Serial

#define TIMEOUT 60000
#define RECEIVED_BUFFER_LENGTH 1024

//*** Ohter SETTINGS ***//
#define CHANNEL_COUNT 4
#define FLASH_INTEGRITY_NR 42 //Change this number to ignore previously stored data

//*** Debug Settings ***//
#define DEBUG 1

#define COMM_VIA_USB 0
#define DEBUG_REPORT 0

#define DEBUG_SERIAL_PARSING 0
#define DEBUG_PARSING 0
#define DEBUG_RESPONSE 0

#define DEBUG_SAVING 1
#define DEBUG_SETTINGS 1

#define DEBUG_SENSORS 0
#define DEBUG_MOTORS 1
#define DEBUG_LIGHTS 0

#endif
