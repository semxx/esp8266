/*
Что куда и зачем. 

--- MAX6675 термопара--- 
13- SCK
12 - CS
11 - S0
------- градусники -----
10 - DS18B20
------- серво ----------
9 - SERVO
--------  модем --------
5- RX
3- TX
 -----------------------
4 - светодиод
----------кнопки--------
вверх А2
средняя А1
низ  А0 
------  дисплей --------
A4   SCL
A5   SDA
-----------------------
6 - бузер

*/ //Библиотеки длямодулей 
#include <SoftwareSerial.h>   //  сом порт для модема
SoftwareSerial mySerial(5, 3); // RX, TX
//////////////////////////// дисплей
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 4
#include <Wire.h>
Adafruit_SSD1306 display(OLED_RESET);
/////////////////// термометры DS18B20
#include <DallasTemperature.h>
#include <OneWire.h>
//#define ONE_WIRE_BUS 10
OneWire oneWire(10);
DallasTemperature sensors(&oneWire);
/////////////////////термопара max6675
#include <max6675.h>
MAX6675 thermocouple(13, 12, 11);//(thermoCLK, thermoCS, thermoDO)
/////////////////////Сервопривод
#include <Servo.h>
Servo myservo; 
//////////////////// EEPROM память.
#include <EEPROM.h>
/////////////// переменные
 byte temp;   // заданная температура 
 byte tA;  // гистерезис
 byte mK; // максимальная температура котла
 byte uG; //  температура управляющая заслонкой
 byte mU;  // максимальный угол поворота сервы.
 byte val;  //расчетный угол поворота сервы
 int i=0;   // счетчик времени.
 byte p=0; // флаг гистерезиса макс. температуры
 String num=""; // строка номера телефона
  byte n;   // переменная цикла
  int cf=0;  // цифра мобильного номера
        byte col=10; // курсор
        String B = ""; // буфер модема
        byte ch = 0; // байт с порта модема
        unsigned long cM = millis();
        long lT = 2000; // 2 сек время простоя  показ температуры
float tempUP ;  // температура с верхнего градусника
float tempDOWN ; //  температура с нижнего градусника
float tempUl ;   // температура с градусника котла
    
    
void setup(){  
  bip();
   EEPROM.get(5,temp); // читаем сохраненные значения в переменные
   EEPROM.get(10,tA);
   EEPROM.get(15,mK);
   EEPROM.get(20,uG);
   EEPROM.get(25,mU);
    for(  n=30;n<=38;n++)  //читаем номер телефона
        { 
         num=num+String(EEPROM.read(n));
         delay(15);
        }
 myservo.attach(9); //  сервопривод на 9 ноге.
//////////////////// кнопки.
pinMode(A0, INPUT);           // назначить выводу порт ввода
digitalWrite(A0, HIGH); 
pinMode(A1, INPUT);           // назначить выводу порт ввода
digitalWrite(A1, HIGH); 
pinMode(A2, INPUT);           // назначить выводу порт ввода
digitalWrite(A2, HIGH); 
pinMode(4, OUTPUT); 
  sensors.setWaitForConversion(1);   // не ждать пока градусники ds18b20 отдуплятся 
   display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // запускаем диплей
   display.setTextWrap(0);
   display.setTextColor(WHITE);
   display.setTextSize(2);
   mySerial.begin(9600);       // запускаем соединение с модемом 
   delay(200);
}


void loop() {
      sensors.requestTemperatures();      // опрос датчиков температуры
      tempUP=sensors.getTempCByIndex(0);   // верхний
      tempDOWN=sensors.getTempCByIndex(1); // нижний
      tempUl=thermocouple.readCelsius();   // термопара
     
      if(tempUP <= tA){;bip();bip();bip();bip();bip();bip();i++;} else {i=0;}// Авария температура
      if (i==1 || i>150){ mySerial.println("ATD+380"+num+";");i=2;}          // упала, зумер, дозвон.
      
      if(uG<1){uG=1;} // считаем угол поворота сервы
        val= mU / (uG/(temp-tempUP) ); 
        if (val<=0){val=0;}
        if (val>mU){val=mU;}
        if (tempUP>=temp){val=0;}
        if (tempUP<=temp-uG){val=mU;}
        if (tempUl >= mK && p==0){val=0;p=1;} // если котел перегрет закрываем шибр
        if (p==1) 
           { if (tempUl >= mK-5 )      // гистерезис котла 5 градусов ждать пока остынет
             { val=0;} else { p=0;}}
   myservo.write(val);       //  крутим серву.    
   Ds();  // выводим данные о температуре на экран
   digitalWrite(4, HIGH); // включили светодиод
   cM = millis(); 
   while(millis()-cM<=lT)  // основной цикл опроса кнопок. 
    {
      if( millis()-cM > lT/1000 && p==0){ digitalWrite(4, LOW);} // моргалка выключили светодиод 
      if (analogRead(A1)<100) // нажата центральная
      { i=0;
        while(analogRead(A1)<100 && i<10000) {i++;} // крутим цикл  пока нажата кнопка и i меньше 10000
        if (i>=9999){bip();configGSM(); i=0;}  // нажата центральная  и  удержана -  вызов меню настройки телефона
        else { bip(); menu();  i=0; } // нажата центральная кнопка вызов меню настройки температур и угла сервы.
      }
      if ( analogRead(A0)<100 || analogRead(A2)<100)  { bip();temp=vvod("TEM-PA"," t'C ", temp,5);} // изменение настройки температуры
     } 
      digitalWrite(4, LOW); ch = 0; 
      while (mySerial.available()) { // опрос модема, сохраняем входную строку в переменную val
       ch = mySerial.read(); B += char(ch);}
     if( B != "")  //  если модем что то прислал
     {
      display.clearDisplay();  //Вывод на дисплей что ответил модем 
      display.setTextSize(2);
      display.setCursor(0,0);
      display.println("modem:");
      display.setCursor(0,30);
      display.println(B);
      display.display();
      bip(); B=""; delay(1000);        
     } B="";
     

}

//______________Настройка номера телефона__________________________
void configGSM(){
 
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0,0);
      display.println(("Tel:"));
      display.setCursor(0,16);
      display.println("0"+num);
      display.display();
      display.setCursor(0,16);
        for(  n=30;n<=38;n++)
        { cf = EEPROM.read(n);
          delay(15);
          display.setTextColor(BLACK, WHITE); 
          display.setCursor(col,16);
          display.println(cf);
          display.display();
          while( analogRead(A1)>100)
           {
            if (analogRead(A0)<100) {bip(); cf=cf+1;if(cf>9){cf=9;};display.setCursor(col,16);display.println(cf); display.display();}
            if (analogRead(A2)<100) {bip(); cf=cf-1;if(cf<0){cf=0;};display.setCursor(col,16);display.println(cf); display.display();}
           }
           display.setTextColor(WHITE,BLACK);
           display.setCursor(col,16);
           display.println(cf);
           display.display();
           bip();
           delay(500);
           col=col+12;
           if( EEPROM.read(n)!= cf){EEPROM.write(n,cf);bip();}
        }
}  

//________________основное меню________________________
void menu() {  
  lT = 4000;
   mK= vvod("MAX TEM-PA","KOTEL", mK,15);
   uG= vvod("TEM.3AKP.","3ATBOPA", uG,20);
   mU= vvod("MAX PA3MAX","3ATBOPA \x12", mU,25);
   tA=   vvod("TEM-PA","ABAP.TPEBO", tA,10);
   lT = 2000;
}
  
//_______Меню настройки значений ____________________
int vvod(String t1,String t2, byte v,byte e){

 int tp=0;
  EEPROM.get(e,tp); // запоминаем  начальное значение 
   delay(100);
   display.setTextColor(WHITE);
    cM = millis();
    while(millis()-cM<=lT)
     {
      if (analogRead(A2) < 100 ) { bip();v=v-1;cM = millis(); if(v<0){v=0;} delay(100);}
      if (analogRead(A0) < 100 ) { bip();v=v+1; cM = millis();if(v>255){v=255;}delay(100);} 
       if (analogRead(A1) < 100 ) {bip();bip();cM = 0; delay(100);}
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0,0);
      display.println((t1));
      display.setCursor(0,20);
      display.println((t2));
       display.setTextSize(4);
       display.setCursor(40,37);
       display.println(v);
       display.display();
     }
     if( v != tp) { EEPROM.put(e,v);bip();} // если значение поменялось записываем в еером
 return v;
}

//_____________Вывод значений на экран температуры и угла сервы______________
void Ds() {

  display.setTextColor(WHITE);
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,0);
  display.println(("BEPX\x18"));
  display.setCursor(0,16);
  display.println(("3EM.\x19"));
  display.setCursor(0,32);
  display.println(("KOTEL"));
  display.setCursor(0,50);
  display.println(("3ATBOP"));
  display.setCursor(70,0);
    if(tempUP >= temp )
      {display.setTextColor(BLACK, WHITE); 
       display.println(tempUP);
       display.setTextColor(WHITE,BLACK);}
    else {  display.println(tempUP);}
  display.setCursor(70,16);
  display.println(tempDOWN);
  display.setCursor(70,32);
    if(tempUl >= mK || p==1)
      {display.setTextColor(BLACK, WHITE); 
       display.println(tempUl);
       display.setTextColor(WHITE,BLACK);}
     else  {  display.println(tempUl);}
   display.setCursor(80,50);
   display.println(val);
   display.display();
 }

// --------- бикалка -------------
void bip(){

tone(6,4000); delay(150);  noTone(6);
}


