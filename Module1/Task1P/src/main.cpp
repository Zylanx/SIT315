/*
 * SIT315 M1.T1P - Build a simple Sense-Think-Act Board
 * In this task, we are reading the output of a temperature sensor, and feeding it into a servo.
 */

#include <Arduino.h>
#include <Servo.h>

// Sim Config Constants
#define DELAY_TIME 250  // Used to keep the sim running at a decent rate.

// Pin Definitions
#define TEMP_SENSOR A0
#define SERVO_OUTPUT 3

// Temperature Servo
// The servo will have the temperature sensor's output mapped to its input, from 0 to 180 degrees of movement.
Servo tempServo;

inline int tempMap(int rawTemp);

void setup()
{
    // Start the serial connection
    Serial.begin(115200);

    // Set up the temperature sensor and servo pins
    pinMode(TEMP_SENSOR, INPUT);
    tempServo.attach(SERVO_OUTPUT);
}

void loop()
{
    // Read and map the temperature sensor input - SENSE, then THINK
    int rawTemp = analogRead(A0);
    int mappedTemp = tempMap(rawTemp);

    // Output to the servo - ACT
    tempServo.write(mappedTemp);

    // Print the temperature inputs
    Serial.print("Raw Temp: ");
    Serial.println(rawTemp);
    Serial.print("Servo Degrees: ");
    Serial.println(mappedTemp);
    Serial.println(); // Slightly annoying that I have to do this. Alternative is one print with an sprintf into a buffer

    delay(DELAY_TIME);
}

// Maps the temperature to the valid ranges.
// For the sim, these ranges are 20 to 358, corresponding to -40 c to 125 c.
// This range is mapped to the servo output range of 0 to 180 degrees.
inline int tempMap(int rawTemp)
{
    return (int)map(rawTemp, 20, 358, 0, 180);
}