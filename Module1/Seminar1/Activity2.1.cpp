// Define pin constants.
const uint8_t BTN_PIN = 2;
const uint8_t LED_PIN = 13;

// Define global state storage variables.
uint8_t buttonPrevState = LOW;
uint8_t ledState = LOW;

// Setup Function.
// Runs once on startup, setting up initial state.
void setup()
{
    // Initialise the button as an input with the pullup connected.
    // Ensures the pin does not float.
    pinMode(BTN_PIN, INPUT_PULLUP);
    // Set the LED pin as an output.
    pinMode(LED_PIN, OUTPUT);
    // Setup USB Serial communication.
    Serial.begin(9600);
}

// Repeatedly called by the main loop after during runtime.
void loop()
{
    // Read the current state of the button. Inverted because the pullup makes it active low.
    // NOTE: The circuit should be changed to leave the buttons NC contact floating and the other tied to GND.
    uint8_t buttonState = digitalRead(BTN_PIN);

    // Output the current button and LED states
    Serial.print(buttonState);
    Serial.print(buttonPrevState);
    Serial.println(ledState);


    // If the button has changed state, then toggle the LED
    if(buttonState != buttonPrevState)
    {
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
    }

    // Save the current state
    buttonPrevState = buttonState;

    // Wait for 500 milliseconds before running this function again
    delay(500);
}