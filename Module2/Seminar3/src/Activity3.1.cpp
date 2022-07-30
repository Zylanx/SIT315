#include <Arduino.h>

const byte LED_PIN = 13;
const byte METER_PIN = A4;


void setup()
{
    pinMode(LED_PIN, OUTPUT);
    pinMode(METER_PIN, INPUT);

    Serial.begin(9600);

    startTimer();
}

void loop()
{
}

void startTimer()
{
    noInterrupts();

    TCCR1A = 0; // set entire TCCR1A register to 0
    TCCR1B = 0; // same for TCCR1B
    TCNT1 = 0; // initialize counter value to 0

    // set compare match register for 0.5 Hz increments
    OCR1A = 31249; // = 16000000 / (1024 * 0.5) - 1 (must be <65536)
    // turn on CTC mode
    TCCR1B |= (1 << WGM12);
    // Set CS12, CS11 and CS10 bits for 1024 prescaler
    TCCR1B |= (1 << CS12) | (0 << CS11) | (1 << CS10);
    // enable timer compare interrupt
    TIMSK1 |= (1 << OCIE1A);

    interrupts();
}

ISR(TIMER1_COMPA_vect)
{
    digitalWrite(LED_PIN, digitalRead(LED_PIN) ^ 1);
}

