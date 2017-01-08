#define A D1
#define B D2
#include "SimpleTimer.h"
SimpleTimer timer;

#define ENCODER_ON                              // Включить поддержку энкодера

#ifdef ENCODER_ON
  uint8_t encoderDirection = 0;                 // Направление поворота энкодера
  bool encoderFlagA = false;
  bool encoderFlagB = false;
  int encoderResetTimer = 0;
  int encoderResetInterval = 1000;                     // Интервал сброса флагов
#endif //ENCODER_ON


#ifdef ENCODER_ON
void encoderInterrupts() {
  attachInterrupt(digitalPinToInterrupt(A), encoderReadPinA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(B), encoderReadPinB, CHANGE);
}

void encoderSetup() {
  pinMode(A, INPUT_PULLUP);
  pinMode(B, INPUT_PULLUP);
  encoderInterrupts();
  encoderResetTimer = timer.setInterval(encoderResetInterval, encoderReset);
}

void encoderReset() {
  if (!encoderFlagA && !encoderFlagB) return;
  encoderDirection = 0;
  encoderFlagA = false;
  encoderFlagB = false;
  timer.restartTimer(encoderResetTimer);
  encoderInterrupts();
  Serial.println("encoderReset()");
}

void encoderAction(bool reverse=false) {
  if (reverse == true) {
    Serial.println("Encoder <= Reverse direction");
  } else {
    Serial.println("Encoder => Forward direction");
  }
  encoderReset();
}

void encoderReadPinA() {
  if (encoderFlagA == false) {
    if (digitalRead(B) == LOW) return;
    Serial.println("Flag A");
    encoderFlagA = true;
    detachInterrupt(A);
    if (encoderDirection == 0) {
      encoderDirection = 1;
    } else if (encoderDirection == 2) {
      encoderAction(true); //Reverse direction
    }
  }
}

void encoderReadPinB() {
  if (encoderFlagB == false) {
    if (digitalRead(A) == LOW) return;
    Serial.println("Flag B");
    encoderFlagB = true;
    detachInterrupt(B);
    if (encoderDirection == 0) {
      encoderDirection = 2;
    } else if (encoderDirection == 1) {
      encoderAction();
    }
  }
}
#endif //ENCODER_ON

void setup() {
  Serial.begin(115200);
  delay(100);
  #ifdef ENCODER_ON
    encoderSetup();
  #endif //ENCODER_ON

}

void loop() {
timer.run();

}
