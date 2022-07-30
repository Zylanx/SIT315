#include <Arduino.h>

#include <float.h>

// Define pin constants
const byte LED_PIN = 13;
const byte METER_PIN = A4;

// Forward declaration, see startTimer comment
void startTimer(double freq);

// Map a double from one range to another
double mapf(double x, double in_min, double in_max, double out_min, double out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setup()
{
    pinMode(LED_PIN, OUTPUT);
    pinMode(METER_PIN, INPUT);

    // Reset the timers settings
    noInterrupts();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    interrupts();

    Serial.begin(9600);
}

void loop()
{
    // Read out and map the pot values
    double potReading = analogRead(METER_PIN);
    double mapped = mapf(potReading, 0.0, 1023.0, 0.0, 10.0);

    // Print the read and mapped values
    Serial.print("Read Value: ");
    Serial.print(potReading);
    Serial.print(", Mapped Value: ");
    Serial.println(mapped);

    // Set the timer
    startTimer(mapped);

    delay(100);
}

// Calculate the compare register value for a given frequency and prescaler.
inline uint16_t calcCompareValue(double freq, uint16_t prescaler)
{
    return F_CPU / ((uint32_t)prescaler * freq) - 1;
}

// Calculate the difference between the requested frequency and the actual frequency
// given a specified prescaler.
inline double calcPrescalerDifference(double freq, uint16_t prescaler)
{
    return abs(freq - (F_CPU / (prescaler * (double)(calcCompareValue(freq, prescaler) + 1))));
}

// Return the index of the smallest prescaler difference in the given array
inline uint8_t getMinPrescaled(double prescalerDifferences[], uint8_t size)
{
    // Set the initial value as the max possible value
    double minValue = DBL_MAX;
    uint8_t minIndex = 0;

    // Loop through each difference, finding the smallest difference
    for (uint8_t i = 0; i < size; i++)
    {
        if (prescalerDifferences[i] < minValue)
        {
            minIndex = i;
            minValue = prescalerDifferences[i];
        }
    }

    return minIndex;
}

// Store the current frequency being generated.
// Ensures that it only sets a new frequency as needed
// NOTE: this could be a static auto variable, but initialisation becomes a pain.
double currentFreq = DBL_MAX;

// Starts or changes the timer.
void startTimer(double freq)
{
    Serial.println("Setting Timer");

    // Only update the timer if the frequency has changed
    if (freq != currentFreq)
    {
        // The frequency is updating, make sure the update is atomic
        noInterrupts();

        Serial.println("Frequency Changed");

        // If the frequency is zero, turn off the interrupt.
        // Otherwise calculate the best prescaler and compare value and use that.
        if (freq == 0)
        {
            Serial.println("Frequency is 0, turning off.");
            TIMSK1 &= ~(1 << OCIE1A);
        } else
        {
            Serial.println("Frequency has changed, setting.");

            // The number of prescaler settings
            const uint8_t size = 5;

            // Store the prescaler and its register setting
            const uint16_t prescalers[size] = {1024, 256, 64, 8, 1};
            const uint8_t prescalerSettings[size] = {0b101, 0b100, 0b011, 0b010, 0b001};

            // Calculate difference from the specified frequency for each prescaler setting
            double prescalerDifferences[size];
            for (uint8_t i = 0; i < size; i++)
            {
                prescalerDifferences[i] = calcPrescalerDifference(freq, prescalers[i]);
            }

            // Select the prescaler that gives the closest output
            uint8_t closest = getMinPrescaled(prescalerDifferences, size);

            // Set the compare value to the compare value of the closest match
            OCR1A = calcCompareValue(freq, prescalers[closest]);

            // Print out the selected prescaler and value
            Serial.print("Closest prescaler: ");
            Serial.print(prescalers[closest]);
            Serial.print(", Compare Value: ");
            Serial.println(calcCompareValue(freq, prescalers[closest]));

            // Reset then set the prescaler
            TCCR1B = (TCCR1B & ~((1 << CS12) | (1 << CS11) | (1 << CS10))) | prescalerSettings[closest];

            // Set CTC mode and enable compare interrupt
            TCCR1B |= (1 << WGM12);
            TIMSK1 |= (1 << OCIE1A);
        }

        // Save the new frequency for future checks.
        currentFreq = freq;

        // Timer updated, renable interrupts
        interrupts();
    }

    Serial.println("\n");
}

ISR(TIMER1_COMPA_vect)
{
    digitalWrite(LED_PIN, digitalRead(LED_PIN) ^ 1);
}
