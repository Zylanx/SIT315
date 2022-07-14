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
#define ATOMIC(X) (noInterrupts(); (X) interrupts();)


// Setup Interrupt clocking
#define clock 1

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
}

void loop()
{
}

ISR(TIMER1_COMPA_vect)
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

void setupInterrupts()
{
    // Disable interrupts while setting up
    ATOMIC({
        // Clear registers
        TCCR1A = 0;
        TCCR1B = 0;
        TCNT1 = 0;

        // 15624 = (16000000/(4 * 256)) - 1
        OCR1A = 15624;
        // Prescaler 256
        TCCR1B |= (1 << CS12);
        // CTC
        TCCR1B |= (1 << WGM12);
        // Output Compare Match A Interrupt Enable
        TIMSK1 |= (1 << OCIE1A);
    });
}