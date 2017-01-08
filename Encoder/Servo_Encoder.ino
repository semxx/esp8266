#include <Servo.h>
//these pins can not be changed 2/3 are special pins
int encoderPin1 = 2;
int encoderPin2 = 3;

Servo myservo;

int servopos = 0;

volatile int lastEncoded = 0;
volatile long pos = 0;

long lastencoderValue = 0;

int lastMSB = 0;
int lastLSB = 0;

void setup() {
  Serial.begin (9600);
  
  myservo.attach(9);

  pinMode(encoderPin1, INPUT); 
  pinMode(encoderPin2, INPUT);

  digitalWrite(encoderPin1, HIGH); //turn pullup resistor on
  digitalWrite(encoderPin2, HIGH); //turn pullup resistor on

  //call updateEncoder() when any high/low changed seen
  //on interrupt 0 (pin 2), or interrupt 1 (pin 3) 
  attachInterrupt(0, updateEncoder, CHANGE); 
  attachInterrupt(1, updateEncoder, CHANGE);

}

void loop(){
  
if(servopos < 0)
  
for(servopos > pos; servopos = pos; pos -=1){
  myservo.write(servopos * -1);
  delay(5);
}

for(servopos < pos; servopos = pos; pos +=1){
  myservo.write(servopos * -1);
  delay(5); 
}
;


if(servopos > 0)
  
for(servopos > pos; servopos = pos; servopos -=1){
  myservo.write(servopos);
  delay(5);
}

for(servopos < pos; servopos = pos; servopos +=1){
  myservo.write(servopos);
  delay(5); 
}
;


  Serial.println(pos);
  delay(1000); //just here to slow down the output, and show it will work  even during a delay
  Serial.println(servopos);
  delay(1000);
}


void updateEncoder(){
  int MSB = digitalRead(encoderPin1); //MSB = most significant bit
  int LSB = digitalRead(encoderPin2); //LSB = least significant bit

  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) pos ++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) pos --;

  lastEncoded = encoded; //store this value for next time
  
}
