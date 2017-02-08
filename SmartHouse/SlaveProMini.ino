#include "WireIO.h"

#define EXAMPLE 1

#if EXAMPLE == 1
const int8_t pinBtn = 2;
#elif EXAMPLE == 2
const int8_t pinLdr = 14; // A0
#endif
const int8_t pinPwm = 9;

void setup() {
  Serial.begin(115200);
#ifdef ESP8266
  Serial.println();
#endif
  while (! WireIO.begin()) {
    Serial.println(F("Cannot connect to slave device!"));
    delay(1000);
  }
#if EXAMPLE == 1
  WireIO.pinMode(pinBtn, INPUT_PULLUP);
#elif EXAMPLE == 2
  WireIO.pinMode(pinLdr, INPUT);
#endif
  WireIO.pinMode(pinPwm, OUTPUT);
}

void loop() {
#if EXAMPLE == 1
  static uint8_t light = 0;

  bool btn = WireIO.digitalRead(pinBtn);
  if (! btn) {
    if (light < 255)
      light += 5;
  } else
    light = 0;
  WireIO.analogWrite(pinPwm, light);
  Serial.println(light);
  delay(50);
#elif EXAMPLE == 2
  int16_t ldr = WireIO.analogRead(pinLdr);
  WireIO.analogWrite(pinPwm, map(ldr, 0, 1023, 0, 255));
  Serial.println(ldr);
  delay(50);
#endif
}
