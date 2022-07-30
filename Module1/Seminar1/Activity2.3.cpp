// Block macro that wraps code blocks to ensure that they are run atomically.
#define ATOMIC(block) noInterrupts(); block; interrupts();

// Define pin constants.
const uint8_t BTN_PIN = 2;
const uint8_t LED_PIN = 13;

// Define global state storage variables.
volatile uint8_t buttonPrevState = LOW;
volatile uint8_t ledState = LOW;

// Handler for the button pin interrupt asynchronously
void button_handler();

// Setup Function.
// Runs once on startup, setting up initial state.
void setup()
{
    ATOMIC({
               // Initialise the button as an input with the pullup connected.
               // Ensures the pin does not float.
               pinMode(BTN_PIN, INPUT_PULLUP);

               // Set the LED pin as an output.
               pinMode(LED_PIN, OUTPUT);

               // Setup USB Serial communication.
               Serial.begin(9600);

               // Attach a change mode interrupt to the button pin to handle when it is pressed or released (or bouncing).
               attachInterrupt(digitalPinToInterrupt(BTN_PIN), button_handler, CHANGE);
           })
}

// Repeatedly called by the main loop after during runtime.
void loop()
{
    // Output the current button and LED states
    Serial.print(!digitalRead(BTN_PIN));
    Serial.print(buttonPrevState);
    Serial.println(ledState);

    delay(500);
}

// Handle the button pin interrupt asynchronously
// It is a change mode interrupt handler so it is called on either signal edge.
void button_handler()
{
    // Ensure that button bouncing doesn't cause the handler to be called multiple times
    ATOMIC({
               // Read the current state of the button. Inverted because the pullup makes it active low.
               uint8_t buttonState = !digitalRead(BTN_PIN);

               // If the button has just gone from low -> high, then toggle the LED
               if (buttonState != buttonPrevState && buttonState)
               {
                   ledState = !ledState;
                   digitalWrite(LED_PIN, ledState);
               }

               // Save the current state
               buttonPrevState = buttonState;
           })
}