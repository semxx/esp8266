//
// RotaryBlinker
//
// This example changes how fast the built-in LED blinks by using the
// RotaryEncoder library.
// It starts with an intermediate value (80h).
// * If you rotate it left, it slows down (then it jumps to the fastest).
// * If you rotate it right, it speeds up (then it jumps to the slowest).
// The jumps are because there is no logic to handle wrap-around - an exercise
// for the reader!
//
// Also note that a bias feature has been included. The faster you turn the knob, the
// exponentially quicker the rate changes. This is done by setting a fast rate on
// every detent, then when the millisecond has changed it re-biases the knob lower.
//
// To handle the knob correctly, it needs to INCREASE the time interval when
// dialling LOWER - hence you'll see a negative number in the code.
//
// It also checks for the switch. If it's pressed, it simply turns the LED full on.
//

#include <RotaryEncoder.h>

const byte LED = D1;                   // No built-in LED! Choose... this one!
const unsigned baud = 115200;
const RotaryEncoder::OptPin GND = -1; // Use GND pin
const RotaryEncoder::OptPin VCC = -1; // Use V+ pin
const RotaryEncoder::OptPin SW  =  D4; // Switch
const RotaryEncoder::Pin    DT  =  D3; // Data
const RotaryEncoder::Pin    CLK =  D2; // Clock

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
