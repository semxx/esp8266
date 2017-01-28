
void timerHalfSec()
{
  blink500ms = !blink500ms; // инверсия мерцающего бита
  if(blink500ms) {
    plus1sec = true; // ежесекундно взводится
    if (MenuTimeoutTimer != 0) {
      MenuTimeoutTimer --; // ежесекундный декремент этого таймера
    }
  }
}

void AdjustTime(){

  }
void PrintRTC(char x, char y)
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(x, y);
  
  if (SetH&&blink500ms) {
    display.print("  ");    //  lcd.print(F("  "));
  }
  else {
    if (Hours < 10) {
      display.print("0");   //  lcd.print(F("0"));
    }   
    display.print(Hours);   //  lcd.print(Hours);
  }
 
  // мигающее двоеточие, если не в режиме установки времени
  if (!(SetH||SetM||PrintYesNo||blink500ms))
  {
     display.print(" ");  //  lcd.print(F(" "));
  }
  else {
     display.print(":");   //  lcd.print(F(":"));
  }
 
  if (SetM&&blink500ms) {
    display.print("  ");    //      lcd.print(F("  "));
  }
  else {
    if (Minutes < 10) {
     display.print("0");   //  lcd.print(F("0"));
    }   
     display.print(Minutes);   //      lcd.print(Minutes);
  }
     display.print(" ");  //  lcd.print(F(" "));
 
  if (PrintYesNo) {
    display.print("[");    //    lcd.print(F("["));
    if (!(SetH||SetM||blink500ms))
    {
    display.print("  ");    //      lcd.print(F("   "));
    }
    else {
      if (SetYesNo)
      {
    display.print("YES");    //        lcd.print(F("YES"));
      }
      else {
    display.print("NO");    //        lcd.print(F("NO "));
      }
    }
    display.print("]");    //    lcd.print(F("]"));
  }
 display.display();
 timer.run();
}
void SetTime(char x, char y)
{
  // ========= set hours
  SetH = true;
  do
  {
     
    PrintRTC(x,y);
    rotating = true;  // reset the debouncer
if (encoderValue > oldEncoderValue){
      Hours += 1;
      if(Hours > 23) {
        Hours = 0;
      };
      oldEncoderValue = encoderValue;
    }
if (encoderValue < oldEncoderValue) {
      Hours -= 1;
      if(Hours < 0) {
        Hours = 23;
      };
      oldEncoderValue = encoderValue;
    }
Serial.println(MenuTimeoutTimer);
Serial.println(digitalRead(btn_Right));    
//    timer.run();
  }
while ((digitalRead(btn_Right)==1)|(MenuTimeoutTimer==0));  

  //if (BeepEnabled) {
  //  tone(BeepPin,4000,50); //звук "YES"
  
 SetH = false;
  //delay(200);
  // ========= set minutes
  SetM = true;
  do {

    PrintRTC(0,0);
    rotating = true;  // reset the debouncer
if (encoderValue > oldEncoderValue){
      Minutes += 1;
      if(Minutes > 59) {
        Minutes = 0;
      };
      oldEncoderValue = encoderValue;
    }
if (encoderValue < oldEncoderValue){
      Minutes -= 1;
      if(Minutes < 0) {
        Minutes = 59;
      };
      oldEncoderValue = encoderValue;
    }
 
//  timer.run();
  }
  while ((digitalRead(btn_Right)==1)|(MenuTimeoutTimer==0));
 // if (BeepEnabled) {
 //   tone(BeepPin,4000,50); //звук "YES"
  
  if (PrintYesNo) {
    SetM = false;
    //delay(200);
    // ========= set yes-no
    SetYesNo = false;
    do {
      PrintRTC(0,0);
      rotating = true;  // reset the debouncer
      if (encoderValue  != oldEncoderValue){
        SetYesNo = !SetYesNo;
      oldEncoderValue = encoderValue;
      }
//    timer.run();
    }
    while ((digitalRead(btn_Right)==1)|(MenuTimeoutTimer==0));
    //delay(200);
  } 
}
