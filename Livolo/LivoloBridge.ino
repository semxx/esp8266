/*
  Simple example for receiving

  http://code.google.com/p/rc-switch/
*/

#include <RCSwitch.h>
#include <livolo.h>

RCSwitch mySwitch = RCSwitch();
// keycodes #1: 0, #2: 96, #3: 120, #4: 24, #5: 80, #6: 48, #7: 108, #8: 12, #9: 72; #10: 40, #OFF: 106
// real remote IDs: 6400; 19303
// tested "virtual" remote IDs: 10550; 8500; 7400
// other IDs could work too, as long as they do not exceed 16 bit
// known issue: not all 16 bit remote ID are valid
// have not tested other buttons, but as there is dimmer control, some keycodes could be strictly system
// use: sendButton(remoteID, keycode), see example blink.ino;
Livolo livolo(8); // transmitter connected to pin #8

void setup() {
  Serial.begin(9600);
  /*
   Interupts
  Pin interrupts are supported through attachInterrupt(), detachInterrupt() functions. Interrupts may be attached to any GPIO pin
  except GPIO16, but since GPIO6-GPIO11 are typically used to interface with the flash memory ICs on most esp8266 modules,
  applying interrupts to these pins are likely to cause problems. Standard Arduino interrupt types are supported: CHANGE,
  RISING, FALLING.
   *
   */
  mySwitch.enableReceive(0);  // Receiver on inerrupt 0 => that is pin #2
  livolo.setRepeatTimes(1);
  pinMode(13, OUTPUT);
}

void loop() {
  if (mySwitch.available()) {

    unsigned long value = mySwitch.getReceivedValue();

    if (value == 0) {
      Serial.print("Unknown encoding");
    } else {
      digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
      unsigned int bit = mySwitch.getReceivedBitlength() ;
      unsigned int protocol = mySwitch.getReceivedProtocol();
      Serial.print("Received ");
      Serial.print(value);
      Serial.print(" / ");
      Serial.print(bit );
      Serial.print("bit ");
      Serial.print("Protocol: ");
      Serial.println( protocol);

      brigde(value);
      digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
    }

    mySwitch.resetAvailable();
  }
}

void brigde(unsigned long value) {
  switch (value) {
    case 12417736:
      Serial.println("send 1");
      livolo.sendButton(6400, 96);
      break;
    case 12417730:
      Serial.println("send 2");
      livolo.sendButton(6400, 120);
      break;
  }
}
