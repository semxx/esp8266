#include <Shift595.h>
int del = 50;
#define   dataPin          11      // pin 14 on the 74HC595
#define   latchPin          8      // pin 12 on the 74HC595
#define   clockPin         12      // pin 11 on the 74HC595

#define   numOfRegisters    1      // number of shift registers present

Shift595 Shifter(dataPin, latchPin, clockPin, numOfRegisters);

void setup(){
 //Shifter.setRegisterPin(2, HIGH);
 //Shifter.setRegisterPin(3, HIGH);
 //Shifter.setRegisterPin(4, HIGH);
  }

void loop()
{
   for(int i=0; i <= 4; i++){
   Shifter.setRegisterPin(i, HIGH);
  // delay(del);
 //  Shifter.setRegisterPin((i-1), LOW);
   delay(del);
   if (i == 4){
    for(int i=4; i > 0; i--){
   Shifter.setRegisterPin((i), LOW);
   delay(del);
//   Shifter.setRegisterPin((i+1), LOW);
 //  delay(del); 
   }
  }
 }
// delay(1000);
//Shifter.setRegisterPin(1, HIGH);
//   delay(del);
//Shifter.setRegisterPin(1, LOW);
//   delay(del);
}
