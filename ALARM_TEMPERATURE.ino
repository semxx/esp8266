
/* Код написан на скорую руку, красоты нет,но работает)) 
 *  особо не напрягался просто выхватил куски из других проэктов.
 *   что куда 
 * DS18B20   - D8
 * кнопки      A0 A1 A2 
 * дисплей     А4   A5 
 * бипер  пин  D13
 * 
 * 
 * 
 */
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#define ONE_WIRE_BUS 8
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
#include <EEPROM.h>
int tempB; 
int tempL; 
///////////////////////для графика////////////////////////////////
int tempLog[48];
#define sizeTrend 24
int in1[sizeTrend];
int temp[24];// ={-11,-8,-6,-3,-1,0,-2,0,3,5,6,8,11,10,7,4,3,2,1,3,5,7,9,10}; // масив суточной температуры
 // значения просто для проверки  работы графика,  после включения что бы не был пустым :) через сутки наполнится
// хранить график в епром невижу смысла. 
int convert(int y, int mn, int mx, byte yTrend, byte heightTrend, int *in ){
  int ny=y;
  ny=map(ny,mn,mx,heightTrend-1+yTrend,yTrend);
  //ny=map(ny,mn,mx,yTrend,heightTrend-1+yTrend);
  return ny;
}
////////////////////////////////////////////////////////////

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
int biper=0;
float tempBig ;  // градусник в большой тепличке
float tempLit ;   // градусник в маленько тепличке
float tempUl ;     // градусник  улица
float tempGR;    // температура для графика
    unsigned long currentMillis = millis();
    unsigned long Millis = millis();
    long loopTime = 2000; // 2 сек время простоя  показ температуры
    long TimeM = 60000; // 1 минута
    byte chas=0;
    byte minut=0;



   
void setup(){   
EEPROM.get(20,tempL);
EEPROM.get(10,tempB);
pinMode(A0, INPUT);           // назначить выводу порт ввода
digitalWrite(A0, HIGH); 
pinMode(A1, INPUT);           // назначить выводу порт ввода
digitalWrite(A1, HIGH); 
pinMode(A2, INPUT);           // назначить выводу порт ввода
digitalWrite(A2, HIGH); 
  unsigned long currentMillis = millis();
  long loopTime = 10; // 2 сек время простоя  показ температуры
  sensors.setWaitForConversion(1); 
  tempBig=sensors.getTempCByIndex(0);          
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextWrap(0);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  digitalWrite(11,0);
}
/////////////////////////////////////////////////////////////////////////
void loop() {
  if (millis()-Millis>=TimeM)
 {
    Millis=millis();
    minut++; 
    tempGR=tempGR+tempUl; //сумируем каждую минуту показания уличного градусника
    if(minut>=60)
    {
      minut=0; tempGR=tempGR /60;  temp[24]= tempGR;tempGR=0;  // получаем среднюю температуру за час 
      for(int i=0; i<=23;i++){temp [i]=temp[i+1]; }  // закидываем в массив  смещая значения элементов на -1
    }
    // tempUl
  }
  Tdisplay_4();  // Вывод  на экран.
 

 
}
//_____________________________________________________________________________________________________
void Tdisplay_4()// вывод всего что намеряли на экран.
{
  display.clearDisplay();
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  tempBig=sensors.getTempCByIndex(0); 
  display.println(tempBig);
  display.setTextSize(3);
  display.setCursor(0,40);
  display.println(utf8rus("БОЛЬШАЯ  "));
  display.display();
  if ( tempB>=tempBig) {bip();}
  display.clearDisplay();
    currentMillis = millis();
    while(millis()-currentMillis<=loopTime)
    {
     if ( analogRead(A0)<100 || analogRead(A1)<100 || analogRead(A2)<100)  { menu();}
    }
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  tempLit=sensors.getTempCByIndex(1); 
  display.println(tempLit);
  display.setTextSize(2.5);
  display.setCursor(0,40);
  display.println(utf8rus(" МАЛЕНЬКАЯ "));
  display.display();
  if ( tempL>=tempLit) {bip();}
  display.clearDisplay();
   currentMillis = millis();
   while(millis()-currentMillis<=loopTime)
   {
    if ( analogRead(A0)<100 || analogRead(A1)<100 || analogRead(A2)<100)  { menu();}
   }
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  tempUl=sensors.getTempCByIndex(2); 
  display.println( tempUl);
  display.setTextSize(3);
  display.setCursor(0,40);
  display.println(utf8rus(" УЛИЦА   "));
  display.display();
   currentMillis = millis();
   while(millis()-currentMillis<=loopTime)
    {
     if ( analogRead(A0)<100 || analogRead(A1)<100 || analogRead(A2)<100)  { menu();}
    }
   display.setTextSize(1);
   display.clearDisplay();
   for (int i=0; i<24; i++)
   {display.clearDisplay();
    drawTrend(95,63,0,0,temp[i],in1);
   }
   display.display();
   currentMillis = millis();
   while(millis()-currentMillis<=loopTime)
   {
    if ( analogRead(A0)<100 || analogRead(A1)<100 || analogRead(A2)<100)  { menu();}
   }
 sensors.requestTemperatures();
 if (tempL<tempLit || tempB<tempBig ) {biper=0;} // бибикаем если температура ниже установок
}
//////////////////////////////////////////////////////////////////////////////////////////
void menu() // меню настройки минимальных температур для 1 и 2 градусника
{
 if (tempL>=tempLit || tempB>=tempBig ) {biper=1; tone(13,2550); delay(100); noTone(13);tone(13,600); delay(200); noTone(13);}
 currentMillis = millis();
 while(millis()-currentMillis<=loopTime)
  {
    if (analogRead(A0) < 100 ) { tempB=tempB-1;currentMillis = millis(); }
    if (analogRead(A2) < 100 ) { tempB=tempB+1; currentMillis = millis();} 
    delay(100);
    display.clearDisplay();
    display.setTextSize(3);
    display.setCursor(0,0);
    display.println(utf8rus("БОЛЬШАЯ  "));
    display.setTextSize(4);
    display.setCursor(40,30);
    display.println(tempB);
    display.display();
  }
  display.clearDisplay();
  currentMillis = millis();
  while(millis()-currentMillis<=loopTime)
  {
  if (analogRead(A0) < 100 ) { tempL=tempL-1;currentMillis = millis(); }
  if (analogRead(A2) < 100 ) { tempL=tempL+1; currentMillis = millis();} 
  delay(100);
  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(0,0);
  display.println(utf8rus("МАЛЕНЬКАЯ"));
  display.setTextSize(4);
  display.setCursor(40,30);
  display.println(tempL);
  display.display();
  }
  display.clearDisplay();
  if ( tempB != EEPROM.read(10)) { EEPROM.put(10,tempB);tone(13,2550); delay(100); noTone(13);}
  if ( tempL != EEPROM.read(20)) { EEPROM.put(20,tempL);tone(13,2550); delay(100); noTone(13);}
}
/////////////////////////////////////////////////////////////////////////////////////////////////
void bip() // бибикалка
{
if (biper==1){return;}
tone(13,2550); delay(100); noTone(13);
tone(13,2550); delay(100); noTone(13);
tone(13,2550); delay(100); noTone(13);
tone(13,2550); delay(100); noTone(13);
tone(13,2550); delay(100); noTone(13);
return;
}
////////////////////////////////////////////////////////////////////////////////////////
// графики 
void drawTrend(int widthTrend, int heightTrend, int xTrend, byte yTrend, int cn, int *in){
   //добавляем новое значение
  in[sizeTrend-1]= cn ;//analogRead(cn);
  byte oldX=0;
  byte oldY=0+yTrend;
  int mn=1000;
  int mx=0;
  double k=0.0;
  //сдвигаем график
  for  (byte x=0;x<(sizeTrend-1);x++){
    in[x]=in[x+1];
  }

  //поиск мин и макс
  for (byte i=0;i<sizeTrend;i++){
    if (in[i]>mx){
      mx=in[i];
    }
    if (in[i]<mn){
      mn=in[i];
    }
  }
  if(mn==mx){
    mx=mn+1;
    mn=mn-1;
  }
  //display.drawLine(xTrend+oldX, oldY, xTrend+nxt_x, y, WHITE);
  //формирование буфера вывода
  for(byte x=0;x<sizeTrend-1;x++){
    byte y=convert(in[x],mn,mx,yTrend,heightTrend,in);
    byte nxt_x=map(x,0,sizeTrend-1,0,widthTrend);
    //отрисовка тренда
    display.drawLine(xTrend+oldX, oldY, xTrend+nxt_x, y, WHITE);
    
    oldX=nxt_x;
    oldY=y;

    //отрисовка рамки
    display.drawRect(xTrend, yTrend, widthTrend, heightTrend, WHITE);
 
  } 

  //вывод минимума и максимума
  display.setCursor(xTrend+widthTrend+3,yTrend);
  display.println(mx);
  display.setCursor(xTrend+widthTrend+3,yTrend+heightTrend-8);
  display.println(mn);
  display.setCursor(xTrend+widthTrend+3,yTrend+(heightTrend/2)-4);
  display.println(in[sizeTrend-1]); //текущее
 // display.drawLine(0, , 95, , WHITE);
    oldX=0;
  oldY=convert(in[0],mn,mx,yTrend,heightTrend,in);
  
  for(byte x=0;x<sizeTrend-1;x++){
    byte y=convert(0,mn,mx,yTrend,heightTrend,in);
    byte nxt_x=map(x,0,sizeTrend-1,0,widthTrend);
    //отрисовка нулевой линии
    display.drawLine(xTrend+oldX, oldY, xTrend+nxt_x, y, WHITE);
    oldX=nxt_x;
    oldY=y;
 
  } 
}

/*   тут бибикалка с музыкой, звездные войны и другие мелодии :) 
tone(13, 700, 300);
delay(600);
tone(13, 700, 300);
delay(600);
tone(13, 780, 150);
delay(300);

tone(13, 700, 150);
delay(300);
tone(13, 625, 450);
delay(600);
tone(13, 590, 150);
delay(300);
tone(13, 520, 150);
delay(300);
tone(13, 460, 450);
delay(600);
tone(13, 350, 450);
delay(600);
delay(600);
tone(13, 350, 450);
delay(600);
tone(13, 460, 450);
delay(600);
tone(13, 520, 150);
delay(300);
tone(13, 590, 150);
delay(300);
tone(13, 625, 450);
delay(600);
tone(13, 590, 150);
delay(300);
tone(13, 520, 150);
delay(300);
tone(13, 700, 1350);
delay(1800);
tone(13, 700, 300);
delay(600);
tone(13, 700, 300);
delay(600);
tone(13, 780, 150);
delay(300);
tone(13, 700, 150);
delay(300);
tone(13, 625, 450);
delay(600);
tone(13, 590, 150);
delay(300);
tone(13, 520, 150);
delay(300);
tone(13, 460, 450);
delay(600);
tone(13, 350, 450);
delay(600);
delay(600);
tone(13, 350, 450);
delay(600);
tone(13, 625, 450);
delay(600);
tone(13, 590, 150);
delay(300);
tone(13, 520, 150);
delay(300);
tone(13, 700, 450);
delay(600);
tone(13, 590, 150);
delay(300);
tone(13, 520, 150);
delay(300);
tone(13, 460, 1350);
delay(5000);


tone(13, 392, 350);
delay(350);
tone(13, 392, 350);
delay(350);
tone(13, 392, 350);
delay(350);
tone(13, 311, 250);
delay(250);
tone(13, 466, 100);
delay(100);
tone(13, 392, 350);
delay(350);
tone(13, 311, 250);
delay(250);
tone(13, 466, 100);
delay(100);
tone(13, 392, 700);
delay(700);

tone(13, 587, 350);
delay(350);
tone(13, 587, 350);
delay(350);
tone(13, 587, 350);
delay(350);
tone(13, 622, 250);
delay(250);
tone(13, 466, 100);
delay(100);
tone(13, 369, 350);
delay(350);
tone(13, 311, 250);
delay(250);
tone(13, 466, 100);
delay(100);
tone(13, 392, 700);
delay(700);

tone(13, 784, 350);
delay(350);
tone(13, 392, 250);
delay(250);
tone(13, 392, 100);
delay(100);
tone(13, 784, 350);
delay(350);
tone(13, 739, 250);
delay(250);
tone(13, 698, 100);
delay(100);
tone(13, 659, 100);
delay(100);
tone(13, 622, 100);
delay(100);
tone(13, 659, 450);
delay(450);

tone(13, 415, 150);
delay(150);
tone(13, 554, 350);
delay(350);
tone(13, 523, 250);
delay(250);
tone(13, 493, 100);
delay(100);
tone(13, 466, 100);
delay(100);
tone(13, 440, 100);
delay(100);
tone(13, 466, 450);
delay(450);

tone(13, 311, 150);
delay(150);
tone(13, 369, 350);
delay(350);
tone(13, 311, 250);
delay(250);
tone(13, 466, 100);
delay(100);
tone(13, 392, 750);
delay(750);
//delay(5000);
  /*
   tone(13,2550);
     delay(50);
      noTone(13);
      delay(50);
      tone(13,2550);
     delay(50);
      noTone(12);
     delay(50);
     tone(13,2550);
     delay(100);
      noTone(13);
     delay(100);
      tone(13,1800);
      delay(500);
       noTone(13);
      delay(200);
     tone(13,2500);
       delay(500);
        noTone(13);
        delay(100);
         tone(13,10800);
         delay(200);
         noTone(13);
         delay(200);
         tone(13,10800);
         delay(200);
         noTone(13);
   delay(2000);
   */


