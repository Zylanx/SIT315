/*
 * SIT315 M1.T4C - More-Inputs-Timer Board
 * In this task, we are reading the output of a temperature sensor, and feeding it into a servo.
 * But this time using interrupts instead of polling.
 * We are then checking if motion has been detected and using that to restrict the movement.
 * We have now also added two buttons that allow restricting the movement further to one half of the side, and selecting
 * which side.
 *
 * The source code is hosted at https://github.com/Zylanx/SIT315
 * Run using PlatformIO
 */

#include <Arduino.h>

#include <Servo.h>
#include <YetAnotherPcInt.h>

// Define interrupt context handler
#define ATOMIC(X) noInterrupts(); X; interrupts();

// Pin Definitions
#define TEMP_SENSOR A0
#define PIR_SENSOR 2  // We must use pin 2, only one of two pins to have a pin interrupt.
#define SERVO_OUTPUT 3
#define ENABLE_HALF_RANGE_PIN 11

#define SWAP_HALF_RANGE_PIN 8
#define HALF_RANGE_LED LED_BUILTIN

// Temperature Servo
// The servo will have the temperature sensor's output mapped to its input, from 0 to 180 degrees of movement.
Servo tempServo;

// Forward Declarations
inline int tempMap(int rawTemp);
void setupInterrupts();


void setup()
{
    // Start the serial connection
    Serial.begin(115200);

    // Set up the temperature sensor, pir sensor, the half range buttons, the half range led, and servo pins
    pinMode(TEMP_SENSOR, INPUT);
    pinMode(PIR_SENSOR, INPUT);
    pinMode(ENABLE_HALF_RANGE_PIN, INPUT_PULLUP);
    pinMode(SWAP_HALF_RANGE_PIN, INPUT_PULLUP);
    pinMode(HALF_RANGE_LED, OUTPUT);
    tempServo.attach(SERVO_OUTPUT);

    // Initialise the half range led
    digitalWrite(HALF_RANGE_LED, LOW);

    setupInterrupts();
}

void loop()
{
}

// Set the angle of the servo based on the temperature
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

// Storage for when motion is sensed
volatile bool motionSensed = false;

// Called when motion has just been sensed, or the motion sensed has timed out.
// Read the state and save it into the volatile variable.
void motionSensorChanged()
{
    bool motion = digitalRead(PIR_SENSOR);

    Serial.print("Motion Now ");
    Serial.println(motion ? "Detected" : "Not Detected");

    motionSensed = motion;
}

// Storage for the half range buttons.
volatile bool half_range_enable = false;
volatile bool half_range_side = false; // False = left, True = right.

// Called when the half range enable button is pressed to enable or disable the half range restriction.
void enableHalfRangeChanged()
{
    // Invert the enable variable and display the value.
    half_range_enable = !half_range_enable;
    digitalWrite(HALF_RANGE_LED, half_range_enable);

    Serial.print("Half Range ");
    Serial.println(half_range_enable ? "Enabled" : "Disabled");
}

// Called when the half range side button is pressed to swap from left to right and vice-versa.
void halfRangeSideChanged()
{
    // Invert the side variable
    half_range_side = !half_range_side;

    Serial.print("Half Range Side: ");
    Serial.println(half_range_enable ? "Right" : "Left");
}

// Maps the temperature to the valid ranges.
// For the sim, these ranges are 20 to 358, corresponding to -40 c to 125 c.
// This range is mapped to the servo output range of 0 to 180 degrees.
//
// If the motion sensor detects movement then it restricts the movement further down.
//
// It also restricts the movement to the left or right half when the half range buttons are pressed.
inline int tempMap(int rawTemp)
{
    Serial.print("Mapped Temp Restricted Range: ");
    Serial.println(motionSensed ? "True" : "False");

    if (half_range_enable)
    {
        Serial.print("Mapped Temp Half Side: ");
        Serial.println(half_range_enable ? "Right" : "Left");
    }

    // Set the left and right bounds of the range.
    // Normal is 0 - 180, left is 0 - 90, and right is 90 - 180.
    int left = !half_range_enable || !half_range_side ? 0 : 90;
    int right = !half_range_enable || half_range_side ? 180 : 90;

    // If the motion sensor sensed movement, then set the range to 50% either side of the midpoint between the bounds.
    if (motionSensed)
    {
        int midpointOffset = ((right - left) / 2) / 2;

        left += midpointOffset;
        right -= midpointOffset;
    }

    return (int)map(rawTemp, 20, 358, left, right);
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

        // Attach PCINT to the Half Range Buttons
        PcInt::attachInterrupt(ENABLE_HALF_RANGE_PIN, enableHalfRangeChanged, FALLING);
        PcInt::attachInterrupt(SWAP_HALF_RANGE_PIN, halfRangeSideChanged, FALLING);
    });
}