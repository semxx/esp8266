#define SERIALDEBUG
#include <Servo.h> 
#include <Wire.h>
#include "WireIO_defs.h"

#define ServoPin    7   // пин серво привода
#define BeepPin     11 // пищалка
#define BeepToneNo  2000 // тон звука "No", герц
#define BeepToneYes 4000 // тон звука "Yes", герц
#define BeepToneNoDuration 200 // длительность звука "No", мс
#define BeepToneYesDuration 200 // длительность звука "Yes", мс

Servo myservo;                              // create servo object to control a servo 
int16_t encoderValue = 0;

volatile command_t _cmd = UNKNOWN;
volatile int8_t _params = 0;
volatile int8_t _pin = -1;

void resetEvent() {
  _cmd = UNKNOWN;
  _params = 0;
  _pin = -1;
}

command_t decodeCmd(uint8_t data) {
  command_t result = UNKNOWN;

  if (((data >> 4) ^ 0x0F) == (data & 0x0F)) {
    result = (command_t)(data & 0x0F);
    if (result > UNKNOWN)
      result = UNKNOWN;
  }

  return result;
}

int8_t decodePin(uint8_t data) {
  if ((data > lastPin) || (data == sdaPin) || (data == sclPin)) // Illegal or used for I2C pin
    return -1;
  else
    return data;
}

void receiveEvent(int numBytes) {

#ifdef SERIALDEBUG
  Serial.print(F("Wire.onReceive("));
  Serial.print(numBytes);
  Serial.println(F(")"));
#endif
  if (numBytes == 0) {
    resetEvent();
#ifdef SERIALDEBUG
    Serial.println(F("Address check"));
#endif
    return;
  }

  uint8_t in;

  while (Wire.available()) {
    if (_cmd == UNKNOWN) {
      do {
        if (! Wire.available())
          return;
        in = Wire.read();
#ifdef SERIALDEBUG
        Serial.print(F("IN: "));
        Serial.println(in);
#endif
        _cmd = decodeCmd(in);
      } while (_cmd == UNKNOWN);
      _pin = -1;
      if (_cmd <= ANALOGREAD)
        _params = 1;
      else if (_cmd <= ANALOGWRITE)
        _params = 2;
#ifdef SERIALDEBUG
      Serial.print(F("Command: "));
      switch (_cmd) {
        case PINMODEINPUT:
        case PINMODEINPUTPU:
        case PINMODEOUTPUT:
          Serial.println(F("pinMode"));
          break;
        case DIGITALREAD:
          Serial.println(F("digitalRead"));
          break;
        case ANALOGREAD:
          Serial.println(F("analogRead"));
          break;
        case DIGITALWRITE:
          Serial.println(F("digitalWrite"));
          break;
        case ANALOGWRITE:
          Serial.println(F("analogWrite"));
          break;
      }
#endif
    }

    if (Wire.available() < _params)
      return;

    if (_cmd <= ANALOGWRITE) { // Read pin
      in = Wire.read();
      --_params;
#ifdef SERIALDEBUG
      Serial.print(F("IN: "));
      Serial.println(in);
#endif
      _pin = decodePin(in);
      if (_pin == -1) {
#ifdef SERIALDEBUG
        Serial.print(F("Illegal pin: "));
        Serial.println(in);
#endif
        while (_params) { // Flush remain data
          Wire.read();
          --_params;
        }
        resetEvent();
        return;
      }
    }

    if ((_cmd >= PINMODEINPUT) && (_cmd <= PINMODEOUTPUT)) {
#ifdef SERIALDEBUG
      Serial.print(F("pinMode("));
      Serial.print(_pin);
      Serial.print(F(", "));
#endif
      if (_cmd == PINMODEINPUT) {
        pinMode(_pin, INPUT);
#ifdef SERIALDEBUG
        Serial.println(F("INPUT)"));
#endif
      } else if (_cmd == PINMODEINPUTPU) {
        pinMode(_pin, INPUT_PULLUP);
#ifdef SERIALDEBUG
        Serial.println(F("INPUT_PULLUP)"));
#endif
      } else {
        pinMode(_pin, OUTPUT);
#ifdef SERIALDEBUG
        Serial.println(F("OUTPUT)"));
#endif
      }
      resetEvent();
    } else if (_cmd == DIGITALWRITE) {
      in = Wire.read();
      --_params;
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
      resetEvent();
    } else if (_cmd == ANALOGWRITE) {
      in = Wire.read();
      --_params;
#ifdef SERIALDEBUG
      Serial.print(F("IN: "));
      Serial.println(in);
#endif
////////////////////////////////////////////////////////////
      if (_pin == 22) {
       encoderValue = in; 
        }
      else 
       
       if (_pin == 23) {
       tone(BeepPin,5000,10);
        } 
        else {
      analogWrite(_pin, in);
        }
      
#ifdef SERIALDEBUG
      Serial.print(F("analogWrite("));
      Serial.print(_pin);
      Serial.print(F(", "));
      Serial.print(in);
      Serial.println(F(")"));
#endif
      resetEvent();
    }
  }
}

void requestEvent() {
#ifdef SERIALDEBUG
  Serial.println(F("Wire.onRequest"));
#endif
  if (_cmd == UNKNOWN) {
#ifdef SERIALDEBUG
    Serial.println(F("Unspecified command!"));
#endif
    resetEvent();
    return;
  }
  if ((_cmd <= ANALOGWRITE) && (_pin == -1)) {
#ifdef SERIALDEBUG
    Serial.println(F("Unspecified pin!"));
#endif
    resetEvent();
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
    resetEvent();
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
    resetEvent();
  }
}

void setup() {
#ifdef SERIALDEBUG
  Serial.begin(115200);
#endif
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
myservo.attach(ServoPin);
}

void loop() {
myservo.write(encoderValue);
  
}
