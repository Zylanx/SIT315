/*
 * SIT315 M1.T2P - Using Interrupts
 * In this task, we are reading the output of a temperature sensor, and feeding it into a servo.
 * But this time using interrupts instead of polling.
 *
 * The source code is hosted at https://github.com/Zylanx/SIT315
 * Run using PlatformIO
 */

#include <Arduino.h>
#include <Servo.h>

// Define interrupt context handler
#define ATOMIC(X) noInterrupts(); X; interrupts();

// Pin Definitions
#define TEMP_SENSOR A0
#define SERVO_OUTPUT 3

// Temperature Servo
// The servo will have the temperature sensor's output mapped to its input, from 0 to 180 degrees of movement.
Servo tempServo;

inline int tempMap(int rawTemp);
void setupInterrupts();

void setup()
{
    // Start the serial connection
    Serial.begin(115200);

    // Set up the temperature sensor and servo pins
    pinMode(TEMP_SENSOR, INPUT);
    tempServo.attach(SERVO_OUTPUT);

    setupInterrupts();
}

void loop()
{
}

// Handle the servo and temp interrupt request.
ISR(TIMER2_COMPA_vect)
{
    ATOMIC({
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
    });
}

// Maps the temperature to the valid ranges.
// For the sim, these ranges are 20 to 358, corresponding to -40 c to 125 c.
// This range is mapped to the servo output range of 0 to 180 degrees.
inline int tempMap(int rawTemp)
{
    return (int)map(rawTemp, 20, 358, 0, 180);
}

// Set up the timer registers and interrupts for automatically reading the temp
// data and setting the servo.
void setupInterrupts()
{
    // Disable interrupts while setting up
    ATOMIC({
        // Clear registers
        TCCR2A = 0; // set entire TCCR2A register to 0
        TCCR2B = 0; // same for TCCR2B
        TCNT2  = 0; // initialize counter value to 0

        // set compare match register for 61.03515625 Hz increments
        OCR2A = 255; // 61.03515625 = 16000000 / ((255 + 1) * 1024) (must be <256)
        // turn on CTC mode
        TCCR2B |= (1 << WGM21);
        // Set CS22, CS21 and CS20 bits for 64 prescaler
        TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);
        // enable timer compare interrupt
        TIMSK2 |= (1 << OCIE2A);
    });
}