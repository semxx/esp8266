#include <RotaryEncoder.h>

// Choose pins that are convenient for the RotaryEncoder's pins
#if defined(ESP8266)

const byte LED = LED_BUILTIN;                   // No built-in LED! Choose... this one!
const unsigned baud = 115200;
const RotaryEncoder::OptPin GND = -1; // Use GND pin
const RotaryEncoder::OptPin VCC = -1; // Use V+ pin
const RotaryEncoder::OptPin SW  =  D4; // Switch
const RotaryEncoder::Pin    DT  =  D3; // Data
const RotaryEncoder::Pin    CLK =  D2; // Clock

#elif defined(ADifferentBoard) // Put the board's definition here

// And then change these assignments
const byte LED = LED_BUILTIN;
const unsigned baud = 115200;
const RotaryEncoder::OptPin GND =  8; // Need 0V
const RotaryEncoder::OptPin VCC =  9; // Need +V
const RotaryEncoder::OptPin SW  = 10; // Switch
const RotaryEncoder::Pin    DT  = 11; // Data
const RotaryEncoder::Pin    CLK = 12; // Clock

#else // Arduino Uno default

const byte LED = LED_BUILTIN;
const unsigned baud = 57600;
const RotaryEncoder::OptPin GND =  8; // Need 0V
const RotaryEncoder::OptPin VCC =  9; // Need +V
const RotaryEncoder::OptPin SW  = 10; // Switch
const RotaryEncoder::Pin    DT  = 11; // Data
const RotaryEncoder::Pin    CLK = 12; // Clock

#endif // Boards

// Tell library which pin does what. Note Optional Pins can be -1
RotaryEncoder encoder(GND, VCC, SW, DT, CLK);

// Setup the board for these definitions
void setup() {
    Serial.begin(baud);
    pinMode(LED, OUTPUT); // Want to see it working

    encoder.Begin(10); // Start, with a re-bias of 10
} // setup()

bool ledON = false;  // Current LED state
byte repeat = 0x80;  // How quickly to restart, in ms
byte count = repeat; // Count down 'til next time!

unsigned last = 0;   // Last time millis() was recorded

void loop() {
    // Have switch override pulsing - turn it hard ON
    if (encoder.Switch()) { // Hard override!
        ledON = false; // Pretend LED is now OFF
        digitalWrite(LED, HIGH);
    } // if
    else {
        int value = encoder.Get(); // Returns positive for clockwise turn
        repeat += -value;   // We want to INCREASE time to slow down blinking
        if (value!=0) {     // Did it change?
            Serial.print(value);    // Print latest value
            Serial.print('=');
            Serial.println(repeat); // Print new result
        } // if
    } // else

    // Is it time to blink the LED again?
    unsigned latest = millis();
    if (last!=latest) {
        last = latest;
        encoder.Rebias();    // Re-bias encoder, to decelerate rotation
        if (--count==0) {    // Reached zero yet?
            ledON = !ledON;  // Yes! Toggle LED
            digitalWrite(LED, ledON ? HIGH : LOW);
            count = repeat;  // Count for next time
       } // if
    } // if
} // loop()
