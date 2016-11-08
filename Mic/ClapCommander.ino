/*
 
 Clap Counter
 
 Uses KY037 to count clap sounds via Analog Input
 
 Analyses signal coming into Analog Pin A0 for minimum increase above the
 baseline. Then waits for the duration of a clap and checks to see if the
 signal has dropped sufficiently. 
 
 It then waits to see if a new clap is detected within a specified timeframe.
 If so, it adds the new clap. If not, it outputs the number of claps counted
 using a Serial.print. 
 
 Note: The values used to detect an increase in sound, drop of in sound and delay for end of sound
       were not calculated, measured or otherwise intelligently obtained.
       Oh no, they were just stabbed at in the dark until it worked a bit.
       There accuracy of counting could be significantly improved by correcting these values.
       
Note: This program uses a dreadful technique of counting program cycles instead of timing a delay.
      This means that if you add some code to do something, even bug testing, then your timing goes out. 
      If you can continuously monitor a time value instead of counting program cycles, please update and re-post this code.
 
Note: You must open the serial monitor to see any results. 
   (Arduino 1.0.1 Serial Monitor is under Tools)
 
 
 The circuit:
 * Pin marked S attached to analog input 0 (marked as A0 on my Arduino Uno)
 * Pin marked G attached to GND (in the Power section of the Aruiono Uno)
 * Pin marked + attached to 5V
 * Pin marked D0 is not attached to anything
 
 Circuit behaviour:
 * No Sound
  - Both LEDs on KY037 are on
  - Pin 13 indicator on Arduino Uno is on
  - TX and RX are off

 * Clap
  - LED under blue box on KY037 dims momentarily
  - Pin 13 indicator turns off for a noticable moment, it may shimmer or flash
  - TX will flash on a short moment after Pin 13 indicator remains on again.
     note, if you clap again before TX flashes, TX will wait again until Pin 13 settles.

 
 Created by Tim Marsh
 modified 09 Mar 2013
 Based on sample code by Tom Igoe
 
 This example code is in the public domain.
 
 http://arduino.cc/en/Tutorial/AnalogInput
 
 */
 

#include <IRremote.h> // это скачанная библиотека 

#define PanasonicAddress      0x4004  

//ВАЖНО !!! ( EE008FE0 ) это код кнопки №1 - моего (ИК)! (инфракрасного пульта)!, -
//-у вашего пульта будет другой код, замените на свой!
#define P1        0xEE008FE0  
#define P2        0xEE008FD0  
#define P3        0xEE008FF0 
#define P4        0xEE008FC8 
#define P5        0xEE008FE8 
#define P6        0xEE008FD8 
#define P7        0xEE008FF8 
#define P8        0xEE008FC4 
#define P9        0xEE008FE4 
#define P0        0xEE008FD4 

#define JVC1      0xFB05 //( FB05 ) это код кнопки пульта от видеокамеры JVC (ZOOM)
#define JVC2      0xFB85

IRsend irsend;

//eventTime, для паузы,  вместо delay,будем использовать функцию millis. 
unsigned long eventTime=0;
 
//переменная переключения режима, при нажатии кнопки она принимает значение +1
int a=0;
int b=0; 
int c=0; 
int d=0; 
int e=0; 
int f=0; 
int g=0;



int sensorPin = A0;    //микрофон подключен сюда. select the input pin for the KY036
int ledPin = 13;      // индикатор чувствительности микрофона.select the pin for the LED (also flashes on Arduino Uno)

// ВАЖНО !!! (PIN 3;) вЫход ИК передатчика, ((PIN 3;)на другой пин изменить нельзя)

//пины управляющие светодиодами, или реле, чем угодно.
int ledPin4 = 4;
int ledPin5 = 5;
int ledPin6 = 6;
int ledPin7 = 7;
int ledPin8 = 8;
int ledPin9 = 9;
int ledPin10 = 10;

//индикация ввода звукового логина и пароля,password and login
int login_ledPin = 11;
int password_ledPin = 12;



int sensorValue = 0;  // variable to store the value coming from the sensor
int intMax = 0;        // Used to store both the baseline from A0 and the level of A0 when it tripped
int intClapCount = 0;  // The number of claps that have been detected in the timeframe
int intSensitivity = 3;  // The increase in value above the baseline from A0 required to register a clap
int intDropOffSense = 3;  // The decrease in A0 value from the new baseline required to register a clap has ended
int intClapDuration = 300;  // The number of program cycles allowed for the echo of the clap to drop off
int intIntervalDuration = 700;  // The number of program cycles allowed for the next clap to start (note: Program cycles is a very dirty way of timing an event and should be avoided)
int intIntervalCount = 0;   // The number of milliseconds to wait before detecting the level of sound after a clap

void setup() {
    Serial.begin(9600);  // I think this sets up the serial monitor 
                         // note; my original value of 115200 failed, but 9600 worked.
  pinMode(ledPin, OUTPUT);  // declare the ledPin as an OUTPUT:
  

  // назначаем пины как выходы
  pinMode(ledPin4, OUTPUT);
  pinMode(ledPin5, OUTPUT);
  pinMode(ledPin6, OUTPUT);
  pinMode(ledPin7, OUTPUT);
  pinMode(ledPin8, OUTPUT);
  pinMode(ledPin9, OUTPUT);
  pinMode(ledPin10, OUTPUT);
  
  pinMode(login_ledPin, OUTPUT);
  pinMode(password_ledPin, OUTPUT);
  
 
  intMax = analogRead(sensorPin) ; // read the analog pin and use the value for the level of background noise
}

void loop() {


  // Check if sufficient program cycles have passed to conclude counting a set of claps
  
  intIntervalCount +=1; // A program cycle has been counted
  if (intIntervalCount > intIntervalDuration)  // TRUE if enough cycles have been counted
    {
      if(intClapCount > 0)  // TRUE if any claps have been counted
      {
        PrintOutput(); // Sends the program control to the bit that outputs to the Serial Monitor
      }
      
      intClapCount = 0;  // Resets the number of claps to zero
      intIntervalCount = 0; // Starts counting program cycles from the beginning
                            // note, this value is also reset to zero after each clap is counted
      
//      Serial.println("New Interval");
    }
  sensorValue = analogRead(sensorPin);  // reads the value coming from A0 analog input
  if (sensorValue > (intMax + intSensitivity)) // Sharp increase in sound above the baseline
  {
    intMax = sensorValue;  // sets the new baseline to be the volume of the clap when it was detected
                           // note, this is not the maximum volume that the clap produced.
    delay(intClapDuration);    // Wait for echoes to fade
    sensorValue = analogRead(sensorPin);  // read A0 again after the clap sound has died off
                                          // note, otherwise you may be detecting talking or some other noise
    if(sensorValue < (intMax - intDropOffSense))   // Sharp decrease in sound down from the volume the clap was detected at 
    {
      NewClap(); // go and do everything that happens when a clap has been detected
    }
    intMax = analogRead(sensorPin) ;  // Set the baseline to be the current background level. 
                                      // This base line may be too high if there is still clap echoes being detected
                                      // A longer delay here may be appropriate.
  }
  

 // This just makes the Pin 13 indicator flash or go off when other things are going on.  
  digitalWrite(ledPin, HIGH);   // turn the ledPin on
  delay(1);          
  digitalWrite(ledPin, LOW);   // turn the ledPin off:   
  delay(0);                  
}

void PrintOutput()

   // Serial.print(" Claps: "); // Prints the text in inverted commas to the Serial Monitor
  //  Serial.println(intClapCount);  // Prints the number of claps to the Serial Monitor and starts a new line
  // note, you can do this in one command if you concatenate the two phrases   

  
                                   
 {
   
  // если хлопнули 3 раза,включаем login_ledPin, на (пауза) 3 секунды, и Выключаем
   //Количество хлопков и паузу можно менять на свой вкус
   if(intClapCount==3) eventTime=millis(),digitalWrite(login_ledPin, HIGH);
       if(millis()-eventTime>3000)  digitalWrite(login_ledPin, LOW);
      
  //Только при условии если, (Одновременно) включен login_ledPin !, и  хлопнули 2 раза,
 // включаем password_ledPin, на (пауза) 5 секунд, и Выключаем
   //Количество хлопков и паузу можно менять на свой вкус
      
      if (digitalRead(login_ledPin) == HIGH  && (intClapCount==2)) eventTime=millis(),digitalWrite(password_ledPin, HIGH);
        if(millis()-eventTime>5000)  digitalWrite(password_ledPin, LOW);
  
  
   
   
   
   
   //только при условии если (Одновременно)включен password_ledPin, и хлопок в ладоши 
   //меняем состояние пина, на выключен или наоборот включен
   
   //Здесь во второй строчке я добавил дополнительное условие ((digitalRead(login_ledPin) == LOW)  &&, 
  // так как команда 2 хлопка,пересекается с моей командой для пароля
   

       if (digitalRead(password_ledPin) ==  HIGH  && (intClapCount==1)){a=a+1;}
       
       if (digitalRead(login_ledPin) == LOW  && (digitalRead(password_ledPin) == HIGH)   && (intClapCount==2)){b=b+1;}       
       if (digitalRead(password_ledPin) ==  HIGH  && (intClapCount==3)){c=c+1;}
       if (digitalRead(password_ledPin) ==  HIGH  && (intClapCount==4)){d=d+1;} 
       if (digitalRead(password_ledPin) ==  HIGH  && (intClapCount==5)){e=e+1;}
       if (digitalRead(password_ledPin) ==  HIGH  && (intClapCount==6)){f=f+1;}
       if (digitalRead(password_ledPin) ==  HIGH  && (intClapCount==7)){g=g+1;}


// начинаем включать светодиоды на пинах
// действие после хлопка, если переменная стала равна 1 то
// произвести переключение пина в режим HIGH, если же нет то вернуть переменную в исходное состояние   
       
  if (a==1){digitalWrite(ledPin4, HIGH);} else {digitalWrite(ledPin4, LOW); a=0;}
  if (b==1){digitalWrite(ledPin5, HIGH);} else {digitalWrite(ledPin5, LOW); b=0;}
  if (c==1){digitalWrite(ledPin6, HIGH);} else {digitalWrite(ledPin6, LOW); c=0;}
  if (d==1){digitalWrite(ledPin7, HIGH);} else {digitalWrite(ledPin7, LOW); d=0;}    
  if (e==1){digitalWrite(ledPin8, HIGH);} else {digitalWrite(ledPin8, LOW); e=0;}
  if (f==1){digitalWrite(ledPin9, HIGH);} else {digitalWrite(ledPin9, LOW); f=0;}
  if (g==1){digitalWrite(ledPin10, HIGH);} else {digitalWrite(ledPin10, LOW); g=0;}

  

  
  
//только при условии если, (Одновременно)включен password_ledPin, и Обнаружен хлопок в ладоши
//Отправляем сигнал через (ИК) передатчик, заранее записанные код кнопк вашего! пульта
// после отправки (ИК)сигнала, Выключаем password_ledPin
// (intClapCount==1)) 1 это количество хлопков

  {  
  //и Здесь во второй строчке я добавил дополнительное условие ((digitalRead(login_ledPin) == LOW)  &&, 
  // так как команда 2 хлопка,пересекается с моей командой для пароля
    
 { if (digitalRead(password_ledPin) == HIGH  && (intClapCount==1)) irsend.sendPanasonic(PanasonicAddress,P1),digitalWrite(password_ledPin, LOW);}
 
{ if (digitalRead(login_ledPin) == LOW  && (digitalRead(password_ledPin) == HIGH  && (intClapCount==2)))
irsend.sendPanasonic(PanasonicAddress,P2),digitalWrite(password_ledPin, LOW);}

 { if (digitalRead(password_ledPin) == HIGH  && (intClapCount==3)) irsend.sendPanasonic(PanasonicAddress,P3),digitalWrite(password_ledPin, LOW);}
 { if (digitalRead(password_ledPin) == HIGH  && (intClapCount==4)) irsend.sendPanasonic(PanasonicAddress,P4),digitalWrite(password_ledPin, LOW);}
 { if (digitalRead(password_ledPin) == HIGH  && (intClapCount==5)) irsend.sendPanasonic(PanasonicAddress,P5),digitalWrite(password_ledPin, LOW);}
 { if (digitalRead(password_ledPin) == HIGH  && (intClapCount==6)) irsend.sendPanasonic(PanasonicAddress,P6),digitalWrite(password_ledPin, LOW);}
 { if (digitalRead(password_ledPin) == HIGH  && (intClapCount==7)) irsend.sendPanasonic(PanasonicAddress,P7),digitalWrite(password_ledPin, LOW);}
 { if (digitalRead(password_ledPin) == HIGH  && (intClapCount==8)) irsend.sendPanasonic(PanasonicAddress,P8),digitalWrite(password_ledPin, LOW);}
 { if (digitalRead(password_ledPin) == HIGH  && (intClapCount==9)) irsend.sendPanasonic(PanasonicAddress,P9),digitalWrite(password_ledPin, LOW);}
 { if (digitalRead(password_ledPin) == HIGH  && (intClapCount==10)) irsend.sendPanasonic(PanasonicAddress,P0),digitalWrite(password_ledPin, LOW);}
    
 }
//Если включился password_ledPin, выключаем login_ledPin 
if(digitalRead(password_ledPin) == HIGH)  digitalWrite(login_ledPin, LOW);
}
void NewClap()
 {
    intClapCount += 1;  // Adds another clap to the count of claps
    intIntervalCount = 0;  // Sets the cycle counter back to zero so that the program waits until the end of a set of claps before printing.
  }
