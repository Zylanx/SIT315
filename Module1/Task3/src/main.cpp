/*
 * SIT315 M1.T3C - Multiple-Inputs Board
 * In this task, we are reading the output of a temperature sensor, and feeding it into a servo.
 * But this time using interrupts instead of polling.
 * We are then checking if motion has been detected and using that to restrict the movement
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
#define PIR_SENSOR 2  // We must use pin 2, only one of two pins to have a pin interrupt.
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

    // Set up the temperature sensor, pir sensor, and servo pins
    pinMode(TEMP_SENSOR, INPUT);
    pinMode(PIR_SENSOR, INPUT);
    tempServo.attach(SERVO_OUTPUT);

    setupInterrupts();
}

void loop()
{
}

volatile bool motionSensed = false;

void motionSensorChanged()
{
    bool motion = digitalRead(PIR_SENSOR);

    Serial.print("Motion Now ");
    Serial.println(motion ? "Detected" : "Not Detected");

    motionSensed = motion;
}

// Set the angle of the servo based on the temperature
// It also reduces the range when
ISR(TIMER2_COMPA_vect)
{
    ATOMIC({
        // Divide the run-time by 15, since it can only be set down to ~60 hz.
        // The counter is static so it persists between calls
        static uint8_t divider = 0;
        if (divider++ >= 15)
        {
            divider = 0;
            return;
        }

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
//
// If the motion sensor detects movement then it restricts the movement further down.
inline int tempMap(int rawTemp)
{
    Serial.print("Mapped Temp Restricted Range: ");
    Serial.println(motionSensed ? "True" : "False");
    return (int)map(rawTemp, 20, 358, !motionSensed ? 0 : 65, !motionSensed ? 180 : 115);
}

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
       // Set CS22, CS21 and CS20 bits for 1024 prescaler
       TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);
       // enable timer compare interrupt
       TIMSK2 |= (1 << OCIE2A);

       // Setup the PIR sensor
       attachInterrupt(digitalPinToInterrupt(PIR_SENSOR), motionSensorChanged, CHANGE);
   });
}