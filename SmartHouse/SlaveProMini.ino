#define SERIALDEBUG

#include <Wire.h>
#include "WireIO_defs.h"
#include <Servo.h> 
#ifdef POWERSAVE
#include <avr/sleep.h>
#endif

Servo myservo;                              // create servo object to control a servo 

volatile command_t _cmd = NONE;
volatile int8_t _pin = -1;

#ifdef POWERSAVE
volatile uint32_t timeToSleep = 0;
#endif

inline void eventComplete() {
  _cmd = NONE;
  _pin = -1;
}

void receiveEvent(int numBytes) {
#ifdef POWERSAVE
  timeToSleep = millis() + idleTimeout;
#endif
#ifdef SERIALDEBUG
  Serial.print(F("Wire.onReceive("));
  Serial.print(numBytes);
  Serial.println(F(")"));
#endif
  if (numBytes == 0) {
    _cmd = NONE;
    _pin = -1;
#ifdef SERIALDEBUG
    Serial.println(F("Address check"));
#endif
    return;
  }
  while (Wire.available()) {
    uint8_t in = Wire.read();
#ifdef SERIALDEBUG
    Serial.print(F("IN: "));
    Serial.println(in);
#endif
    if (_cmd == NONE) {
      _pin = in >> 3;
      if ((_pin > lastPin) || (_pin == sdaPin) || (_pin == sclPin)) { // Illegal or used for I2C pin
#ifdef SERIALDEBUG
        Serial.print(F("Illegal pin: "));
        Serial.println(_pin);
#endif
        _pin = -1;
        return;
      }
      if (in & cmdMode) {
        _cmd = PINMODE;
      } else {
        if (in & cmdReadWrite) {
          if (in & cmdDigitalAnalog)
            _cmd = ANALOGWRITE;
          else
            _cmd = DIGITALWRITE;
        } else {
          if (in & cmdDigitalAnalog)
            _cmd = ANALOGREAD;
          else
            _cmd = DIGITALREAD;
        }
      }
#ifdef SERIALDEBUG
      Serial.print(F("Pin: "));
      Serial.println(_pin);
      Serial.print(F("Command: "));
      switch (_cmd) {
        case PINMODE:
          Serial.println(F("pinMode"));
          break;
        case DIGITALREAD:
          Serial.println(F("digitalRead"));
          break;
        case ANALOGREAD:
          Serial.println(F("AnalogRead"));
          break;
        case DIGITALWRITE:
          Serial.println(F("DigitalWrite"));
          break;
        case ANALOGWRITE:
          Serial.println(F("AnalogWrite"));
          break;
      }
#endif
    }
    if (_cmd == PINMODE) {
#ifdef SERIALDEBUG
      Serial.print(F("pinMode("));
      Serial.print(_pin);
      Serial.print(F(", "));
#endif
      if (in & cmdReadWrite) { // 0 - input, 1 - output
        pinMode(_pin, OUTPUT);
#ifdef SERIALDEBUG
        Serial.println(F("OUTPUT)"));
#endif
      } else {
        if (in & cmdDigitalAnalog) { // 1 - pullup
          pinMode(_pin, INPUT_PULLUP);
#ifdef SERIALDEBUG
          Serial.println(F("INPUT_PULLUP)"));
#endif
        } else {
          pinMode(_pin, INPUT);
#ifdef SERIALDEBUG
          Serial.println(F("INPUT)"));
#endif
        }
      }
      eventComplete();
    } else if (_cmd == DIGITALWRITE) {
      if (Wire.available()) {
        in = Wire.read();
#ifdef SERIALDEBUG
        Serial.print(F("IN: "));
        Serial.println(in);
#endif
        digitalWrite(_pin, in);
#ifdef SERIALDEBUG
        Serial.print(F("digitalWrite("));
        Serial.print(_pin);
        Serial.print(F(", "));
        Serial.print(in);
        Serial.println(F(")"));
#endif
        eventComplete();
      }
    } else if (_cmd == ANALOGWRITE) {
      if (Wire.available()) {
        in = Wire.read();
#ifdef SERIALDEBUG
        Serial.print(F("IN: "));
        Serial.println(in);
#endif
        analogWrite(_pin, in);
#ifdef SERIALDEBUG
        Serial.print(F("analogWrite("));
        Serial.print(_pin);
        Serial.print(F(", "));
        Serial.print(in);
        Serial.println(F(")"));
#endif
        eventComplete();
      }
    }
  }
}

void requestEvent() {
#ifdef POWERSAVE
  timeToSleep = millis() + idleTimeout;
#endif
#ifdef SERIALDEBUG
  Serial.println(F("Wire.onRequest"));
#endif
  if ((_cmd != DIGITALREAD) && (_cmd != ANALOGREAD)) {
    _cmd = NONE;
    _pin = -1;
#ifdef SERIALDEBUG
    Serial.println(F("Illegal command!"));
#endif
    return;
  }
  if (_pin == -1) {
    _cmd = NONE;
#ifdef SERIALDEBUG
    Serial.println(F("Illegal pin!"));
#endif
    return;
  }
  if (_cmd == DIGITALREAD) {
    uint8_t val = digitalRead(_pin);
    Wire.write(val);
#ifdef SERIALDEBUG
    Serial.print(F("digitalRead("));
    Serial.print(_pin);
    Serial.print(F(") = "));
    Serial.println(val);
#endif
    eventComplete();
  } else if (_cmd == ANALOGREAD) {
    int16_t val = analogRead(_pin);
    Wire.write(val >> 8);
    Wire.write(val & 0xFF);
#ifdef SERIALDEBUG
    Serial.print(F("analogRead("));
    Serial.print(_pin);
    Serial.print(F(") = "));
    Serial.print(val);
#endif
    eventComplete();
  }
}
void setup() {
//#ifdef SERIALDEBUG
  Serial.begin(115200);
//#endif
  Wire.begin(slaveAddress);
#ifdef SUPERSPEED
  Wire.setClock(800000);
#elif defined(FASTSPEED)
  Wire.setClock(400000);
#endif
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
#ifdef SERIALDEBUG
  Serial.print(F("Wire.begin("));
  Serial.print(slaveAddress);
  Serial.println(F(")"));
#endif
#ifdef POWERSAVE
  timeToSleep = millis() + idleTimeout;
#endif

myservo.attach(7);

}

void loop() {

int encoderValue = analogRead(14);
Serial.println(encoderValue);
myservo.write(encoderValue);
delay(500);

#ifdef POWERSAVE
  if (millis() > timeToSleep) {
#ifdef SERIALDEBUG
    Serial.println(F("Sleeping"));
    Serial.flush();
#endif
    uint8_t _ADCSRA = ADCSRA;
    // Disable ADC
    ADCSRA = 0;
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    // Do sleep
    sleep_cpu();
    // Wake up
    sleep_disable();
    // Enable ADC
    ADCSRA = _ADCSRA;
    // Release TWI bus
    TWCR = bit(TWEN) | bit(TWIE) | bit(TWEA) | bit(TWINT);
    // Reinit TWI bus
    Wire.begin(slaveAddress);
#ifdef SUPERSPEED
    Wire.setClock(800000);
#elif defined(FASTSPEED)
    Wire.setClock(400000);
#endif
#ifdef SERIALDEBUG
    Serial.println(F("Waked up"));
#endif
    _cmd = NONE;
    _pin = -1;
    timeToSleep = millis() + idleTimeout;
  }
#endif


}
