#include "TrainMotor.h"
#include "ControlLoop.h"
#include "Speedometer.h"
#include "MagnetSensors.h"

#include "SpeedMonitor.h"

#include <Pinout.h>

// Metrics
int roundCounter = 0;
int incompleteRuns = 0;
int misses = 0;
int shotCount = 0;

int newSetpoint = 0;

int startDistance = 0;
int endDistance = 0;
int avgDistance = -1;

TrainMotor motor(MOTOR_PIN);
ControlLoop controlLoop;
Speedometer speedometer(ANALOG_SENSOR_PIN);
MagnetSensors magnetSensors(MAGNET_BOTTOM_PIN, MAGNET_SIDE_PIN);

SpeedMonitor speedMonitor;

unsigned long lastMotorSpeedUpdate = 0;
unsigned long lastEventUpdate = 0;

unsigned long lastStopTime = 0;
int longestTimeBetweenStops = 0;

#define STOP_DURATION 5000

#define BRAKE_DURATION 1000

enum modes
{
    AT_START_POSITION,
    GOING_FORWARD,
    BRAKING,
    STOPPED,
    CORRECTING,
    GOING_BACKWARD
};

enum directions
{
    FORWARD,
    BACKWARD,
    NONE
};

int currentDirection = NONE;

int currentState = AT_START_POSITION;
#define TRAVEL_SPEED 50

bool middleCleared = false;
int extraStops = 0;

void handleStopInterrupt()
{
    detachInterrupt(MAGNET_BOTTOM_PIN);
    controlLoop.reset();
    newSetpoint = 0;
    motor.brake(); // Only works when moving forwards
    // motor.setSpeed(0);
    

    digitalWrite(INTERRUPT_CHECK_PIN, !digitalRead(INTERRUPT_CHECK_PIN));
    lastEventUpdate = millis();
    middleCleared = true;
    currentState = BRAKING;
}

void setStopIntterrupt()
{
    attachInterrupt(MAGNET_BOTTOM_PIN, handleStopInterrupt, FALLING);
}

void checkInterStopTimer()
{
    if (shotCount > 1)
    {
        int timeDelta = millis() - lastStopTime - STOP_DURATION;
        if (timeDelta > longestTimeBetweenStops)
        {
            longestTimeBetweenStops = timeDelta;
            speedMonitor.setDistance((float)timeDelta * 0.001);
        }

        Serial.println();
        Serial.print("Longest Time Between Stops: ");
        Serial.println(longestTimeBetweenStops);
        Serial.print("Time Delta: ");
        Serial.println(timeDelta);
        Serial.println();
    }
}

void printMetrics()
{
    speedMonitor.setTotalDistance(avgDistance);

    Serial.println("---------------------");
    Serial.print("Round: ");
    Serial.println(roundCounter);
    Serial.print("Misses: ");
    Serial.println(misses);
    Serial.print("Incomplete Runs: ");
    Serial.println(incompleteRuns);
    Serial.print("Avg Distance: ");
    Serial.println(avgDistance);
    Serial.println("---------------------");
}

void onEndReached()
{
    if (shotCount < 6)
    {
        incompleteRuns++;
        Serial.println("Incomplete Run");
        shotCount = 0;
    }
    endDistance = speedometer.getDistance();

    int distance = endDistance - startDistance;
    if (avgDistance == -1)
    {
        avgDistance = distance;
    }
    else
    {
        avgDistance = (avgDistance + distance) / 2;
    }
    
    Serial.print("Distance: ");
    Serial.println(distance);

    lastStopTime = 0;
    roundCounter++;
    printMetrics();
}

void switchState(int newState){
    String stateString[] = {"AT_START_POSITION", "GOING_FORWARD", "BRAKING", "STOPPED", "CORRECTING", "GOING_BACKWARD"};

    Serial.println("Switching State");	
    Serial.print("From: ");
    Serial.println(stateString[currentState]);
    Serial.print("To: ");
    Serial.println(stateString[newState]);
    
    currentState = newState;
    lastEventUpdate = millis();
}

void creepToStop(int speed)
{
    if(speed > 0){
        currentDirection = FORWARD;
    }
    else{
        currentDirection = BACKWARD;
    }
    newSetpoint = speed;
    lastStopTime = 0;
    setStopIntterrupt();
}
// Have to add timeout, while driving forward. Start it after the first shot spot was reached.
// And if no more is reached after a couple of mm or seconds just go to target.
void runTest()
{
    switch (currentState)
    {
    case AT_START_POSITION:
        if (millis() - lastEventUpdate > STOP_DURATION)
        {
            Serial.println("Starting at start position");
            Serial.println("Going forward");
            shotCount = 0;

            startDistance = speedometer.getDistance();
            
            creepToStop(TRAVEL_SPEED);
            switchState(GOING_FORWARD);
        }
        break;
    case BRAKING:
        if (millis() - lastEventUpdate > BRAKE_DURATION)
        {
            if (!magnetSensors.bottomMagnetPresent())
            {
                Serial.println("Missed Magnet");
                misses++;

                if(currentDirection == FORWARD){
                    creepToStop(-TRAVEL_SPEED);
                    switchState(CORRECTING);
                }
                else{
                    creepToStop(TRAVEL_SPEED);
                    switchState(GOING_FORWARD);
                }   
            }
            else
            {
                Serial.println("Stopped On Point");
                currentDirection = NONE;
                switchState(STOPPED);
            }
        }
        break;
    case STOPPED:
        if(lastStopTime != 0){
            if(currentDirection == FORWARD){
                checkInterStopTimer();
                lastStopTime = 0;
            }
        }

        if (millis() - lastEventUpdate > STOP_DURATION)
        {
            shotCount++;

            lastStopTime = millis();

            creepToStop(TRAVEL_SPEED);
            switchState(GOING_FORWARD);
        }
        break;
    case CORRECTING:
        break;
    case GOING_FORWARD:
        if (magnetSensors.bottomWasTriggered())
        {
            middleCleared = true;
        }
        if (magnetSensors.sideWasTriggered() && middleCleared)
        {
            handleStopInterrupt();
            Serial.println("End Reached");
            
            onEndReached();

            middleCleared = false;
            newSetpoint = -TRAVEL_SPEED;
            currentDirection = BACKWARD;
            switchState(GOING_BACKWARD);
        }
        break;
    case GOING_BACKWARD:
        if (magnetSensors.bottomWasTriggered())
        {
            middleCleared = true;
        }
        if (magnetSensors.sideWasTriggered() && middleCleared)
        {
            handleStopInterrupt();
            Serial.println("Reached Start Position");
            middleCleared = false;

            currentDirection = NONE;
            switchState(AT_START_POSITION);
        }
        break;
    default:
        break;
    }
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Starting setup");
    Serial.println("Setting up encoder");
    speedometer.setupEncoder();
    speedometer.setInverted(true);
    Serial.println("Activating HW Timer");
    setupTimerInterrupt();

    magnetSensors.setup();
    motor.begin();
    pinMode(INTERRUPT_CHECK_PIN, OUTPUT);
    digitalWrite(INTERRUPT_CHECK_PIN, LOW);

    controlLoop.setMode(2);

    Serial.println("Ready to go");

    speedMonitor.begin();
    speedMonitor.setSpeed(0);
    speedMonitor.update();
}

void loop()
{
    updateMotor();

    magnetSensors.update();

    speedMonitor.setSpeed(speedometer.getSpeed());

    int testValues[4] = {roundCounter, incompleteRuns, misses, shotCount};
    speedMonitor.setTestValues(testValues);
    speedMonitor.update();

    runTest();
}

unsigned long lastSpeedPrint = 0;

void updateMotor()
{
    if ((millis() - lastMotorSpeedUpdate) > 300)
    {

        if (millis() - lastSpeedPrint > 1000)
        {

            Serial.print("Speed: ");
            Serial.println(speedometer.getSpeed());
            lastSpeedPrint = millis();
        }

        controlLoop.setInput(speedometer.getSpeed());
        controlLoop.setTarget(newSetpoint);
        motor.setSpeed(controlLoop.calculateOutput());

        lastMotorSpeedUpdate = millis();
    }
}