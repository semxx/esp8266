#include <Arduino.h>
#include <Wire.h>
#include "WireIO.h"

#ifdef POWERSAVE
const uint32_t wakeupDelay = 5;
#endif

bool _WireIO::check() {
  Wire.beginTransmission(slaveAddress);
  bool result = Wire.endTransmission() == 0;
#ifdef POWERSAVE
  if (result) {
    delay(wakeupDelay); // Wait for wake up
    _wakeupTime = millis() + idleTimeout;
  }
#endif

  return result;
}

#ifdef ESP8266
bool _WireIO::begin(int8_t sda, int8_t scl) {
  Wire.begin(sda, scl);
#else
bool _WireIO::begin() {
  Wire.begin();
#endif
#ifdef SUPERSPEED
  Wire.setClock(800000);
#elif defined(FASTSPEED)
  Wire.setClock(400000);
#endif
/*
#if defined(ESP8266)
  Wire.setClockStretchLimit(2000);
#endif
*/

  return check();
}

void _WireIO::pinMode(uint8_t pin, uint8_t mode) {
  if ((pin > lastPin) || ((mode != INPUT) && (mode != INPUT_PULLUP) && (mode != OUTPUT)))
    return;

  uint8_t data = cmdMode;
  if (mode == OUTPUT)
    data |= cmdReadWrite;
  else if (mode == INPUT_PULLUP)
    data |= cmdDigitalAnalog;
  data |= (pin << 3);
#ifdef POWERSAVE
  if (millis() > _wakeupTime) {
    if (! check())
      return;
  } else
    _wakeupTime = millis() + idleTimeout;
#endif
  Wire.beginTransmission(slaveAddress);
  Wire.write(data);
  Wire.endTransmission();
}

bool _WireIO::digitalRead(uint8_t pin) {
  if (pin > lastPin)
    return false;

  uint8_t data = 0;
  data |= (pin << 3);
#ifdef POWERSAVE
  if (millis() > _wakeupTime) {
    if (! check())
      return false;
  } else
    _wakeupTime = millis() + idleTimeout;
#endif
  Wire.beginTransmission(slaveAddress);
  Wire.write(data);
  Wire.endTransmission();
  delay(1);
  Wire.requestFrom(slaveAddress, 1);

  return Wire.read();
}

void _WireIO::digitalWrite(uint8_t pin, bool value) {
  if (pin > lastPin)
    return;

  uint8_t data = cmdReadWrite;
  data |= (pin << 3);
#ifdef POWERSAVE
  if (millis() > _wakeupTime) {
    if (! check())
      return;
  } else
    _wakeupTime = millis() + idleTimeout;
#endif
  Wire.beginTransmission(slaveAddress);
  Wire.write(data);
  Wire.write(value);
  Wire.endTransmission();
}

int16_t _WireIO::analogRead(uint8_t pin) {
  if (pin > lastPin)
    return -1;

  uint8_t data = cmdDigitalAnalog;
  data |= (pin << 3);
#ifdef POWERSAVE
  if (millis() > _wakeupTime) {
    if (! check())
      return -1;
  } else
    _wakeupTime = millis() + idleTimeout;
#endif
  Wire.beginTransmission(slaveAddress);
  Wire.write(data);
  Wire.endTransmission();
  delay(1);
  Wire.requestFrom(slaveAddress, 2);

  int16_t result = Wire.read() << 8;
  result |= Wire.read();

  return result;
}

void _WireIO::analogWrite(uint8_t pin, uint8_t value) {
  if (pin > lastPin)
    return;

  uint8_t data = cmdReadWrite | cmdDigitalAnalog;
  data |= (pin << 3);
#ifdef POWERSAVE
  if (millis() > _wakeupTime) {
    if (! check())
      return;
  } else
    _wakeupTime = millis() + idleTimeout;
#endif
  Wire.beginTransmission(slaveAddress);
  Wire.write(data);
  Wire.write(value);
  Wire.endTransmission();
}

void _WireIO::sendValue(uint16_t value) {
//  uint8_t pin = 0;
  uint16_t data = cmdSendValue;
 // data |= (cmdSendValue << 3);
  Wire.beginTransmission(slaveAddress);
  Wire.write(data);
  Wire.write(value);
  Wire.endTransmission();
}

#ifdef POWERSAVE
uint32_t _WireIO::_wakeupTime = 0;
#endif

_WireIO WireIO;
