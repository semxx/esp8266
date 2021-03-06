#include "LedControl.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include <LiquidCrystal_I2C.h>
#define one_wire_bus 2
#define Speaker        A0  
LiquidCrystal_I2C lcd(0x3F, 16, 2);
OneWire oneWire(one_wire_bus);                                                                                                                                                                                                   
DallasTemperature sensors(&oneWire);

 int dataPin = 12;
 int clkPin = 11;
 int csPin = 10;

 // Переменная numDisplay определяет число матричных 8*8 индикаторов в массиве. Нумерация индикаторов, столбцов и строк начинается с 0
 int numDisplay = 3; 

 // Переменные temp хранят считанную с датчиков температуру
 float temp, temp1; 
 
// символ используемый в прогресс-баре на дисплее
byte customChar[8] = {
  0b11111,
  0b11111,
  0b11111,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

// Массив цифр 5*8
byte digitM[][5] = {{B01111110,B10000001,B10000001,B10000001,B01111110},
                {B10000100,B10000010,B11111111,B10000000,B10000000},
                {B11000010,B10100001,B10010001,B10001001,B10000110},
                {B01000010,B10000001,B10001001,B10001001,B01110110},
                {B00110000,B00101000,B00100100,B00100010,B11111111},
                {B01001111,B10001001,B10001001,B10001001,B01110001},
                {B01111110,B10001001,B10001001,B10001001,B01110010},
                {B00000011,B11100001,B00010001,B00001001,B00000111},
                {B01110110,B10001001,B10001001,B10001001,B01110110},
                {B01001110,B10010001,B10010001,B10010001,B01111110},
                {B00000000,B00000000,B00000000,B00000000,B00000000}
               };

// Массив малых цифр 3*6(десятичные доли градуса)
byte digitD[][3]={{B01111000,B10000100,B01111000},
                  {B10001000,B11111100,B10000000},
                  {B11000100,B10100100,B10011100},
                  {B10000100,B10010100,B11101100},
                  {B00111100,B00100000,B11111000},
                  {B10011100,B10010100,B11110100},
                  {B11111100,B10010100,B11110100},
                  {B11000100,B00100100,B00011100},
                  {B11111100,B10010100,B11111100},
                  {B10111100,B10100100,B11111100}};


// Массив малых цифр 4*6(десятичные доли градуса)
byte digitDv1[][4]={{B01111000,B10000100,B10000100,B01111000},
                  {B00000000,B10001000,B11111100,B10000000},
                  {B10001000,B11000100,B10100100,B10011000},
                  {B01001000,B10000100,B10010100,B01101000},
                  {B00110000,B00101000,B00100100,B11110000},
                  {B01011100,B10010100,B10010100,B01100100},
                  {B01111000,B10010100,B10010100,B01100000},
                  {B10000100,B01000100,B00100100,B00011100},
                  {B01101000,B10010100,B10010100,B01101000},
                  {B00011000,B10100100,B10100100,B01111000}};                  

// Знак разделения целой и дробной частей показаний температуры
byte delimiter[][1] = {{B11000000}};

// Знак плюс +
byte plus[][3] = {{B00010000,B00111000,B00010000}};

// Знак минус -
byte minus[][3] = {{B00001000,B00001000,B00001000}};

// Массив знака градусов и символа Цельсия (С)
byte celsius[][7] = {{B00000011,B00000011,B00000000,B00011110,B00100001,B00100001,B00010010}};
              
// Создаем экземпляр класса LedControl//
//
LedControl lc=LedControl(dataPin, clkPin, csPin, numDisplay);

// Функция возвращает номер активного столбца в пределах индикатора
// Вычисляем номер отображаемого (текущего) столбца в пределах индикатора.
// Как пример:
// Нам нужно вывести массив, начиная с 22-й позиции индикатора
// 1-я позиция: 22-((22/8)*8 + 0) = 22
//
// 22 - (22/8)*8 = 22 - 2*8 = 6
// Т.е. начинаем выводить, начиная с 6-й позиции
//
int fnNumIndicatorColumn(int startPosition, int arrayIndex)
{
  if((startPosition - (startPosition/8)*8 + arrayIndex) > 7)
  { return (startPosition - (startPosition/8)*8 + arrayIndex) - 8;}
  else
  { return ((startPosition - (startPosition/8)*8) + arrayIndex);}
}

// Функция возвращает номер активного индикатора 
int fnCurentIndicator(int startPosition, int arrayIndex)
{
  return (startPosition + arrayIndex)/8;
}

// Функция стирает все индикатоы
void fnClearAllDevices(int numberDev)
{
   for(int l=0; l<numDisplay; l++)
  {
   lc.clearDisplay(l);
  }
}

void setup()
{
   Serial.begin(9600);
   lcd.init();   // initializing the LCD
   lcd.backlight();
   lcd.setCursor(0, 0); //инфо на LCD
   lcd.createChar(0, customChar);
// Устанавливаем яркость индикаторо в. Значение яркости от 0 до 15
    lc.setIntensity(0,7);
    lc.setIntensity(1,7);
    lc.setIntensity(2,7);
    lc.shutdown(0,false);
    lc.shutdown(1,false);
    lc.shutdown(2,false);
    fnClearAllDevices(3);  
    sensors.begin();    // Инициализация датчика температуры
}

// Функция выводит на матрицу заданную строку из массива digitToShow, начиная с заданной позиции
// Номер выводимой строки определяется передаваемым параметром dig 
// Всего число позиций матрицы равно 8*numDisplay, от 0 до 8*numDisplay-1
// Число индикаторов - numDisplay, переменная curDev определяет текущий индикатор, куда пишем столбец
// Число точек в каждом индикаторе - 8
// Переменная k определяет номер выводимого столбца из массива digitToShow
// Переменная i определяет номер столба в пределах индикатора: допустим, нужно вывести первый столбец в 23-ю позицию матрицы;
// 1. определяем номер индикатора (нумерация индикаторов также начинается с 0): curDev = (startPos + k)/8; (23+0)/8 = 2
// 2. определяем номер столбца в пределах индикатора: i = startPos - (startPos/8)*8; 23 - (23/8)*8 = 23 - 16 = 7,
// т.е. первый элемент массива выводим в 7-й столбец 3-го индикатора

// Входной параметр startPos определяет стартовую позицию, с которой начинаем выводить символ

// Другой вариант вывода больших цифр на матрицу
// Функции передаем цифру, которую нужно вывести на индикатор и начальную позицию, с которой начинаем выводить

void fnShowBigDigit(int digit2Show, int startPosition)
{ 
  for(int k=0; k<5; k++)      // Пробегаем по всем элементам выводимого массива, в нашем случае размерность равна 5
  {

// Определяем номер столбца (от 0 до 7) индикатора, в который будем писать. Общее число столбцов от 0 до 31
// Если значение i будет больше, то обнуляем его

  //lc.setColumn(fnCurentIndicator(startPosition,k),fnNumIndicatorColumn(startPosition,k),digitM[digit2Show][k]);
  lc.setRow(fnCurentIndicator(startPosition,k),fnNumIndicatorColumn(startPosition,k),digitM[digit2Show][k]);

  }  
}

// Функция выводит на матрицу малые цифры
void fnShowSmallDigit(int digit2Show, int startPosition)
{
  for(int k=0; k<4; k++)      // Пробегаем по всем элементам выводимого массива, в нашем случае размерность равна 4
  {

// Определяем номер столбца (от 0 до 7) индикатора, в который будем писать. Общее число столбцов от 0 до 31
// Если значение i будет больше, то обнуляем его

  lc.setRow(fnCurentIndicator(startPosition,k),fnNumIndicatorColumn(startPosition,k),digitDv1[digit2Show][k]);
  //lc.setColumn(fnCurentIndicator(startPosition,k),fnNumIndicatorColumn(startPosition,k),digitDv1[digit2Show][k]);

  }  
}

void fnShowTwoBigDigit(double dsTemp)
{
  int k;      // Целая часть значения температуры
  int d10;    // Цифра десятков
  int d1;     // Цифра едениц
  int g;      // дробная часть температуры

// Выделяем первую цифру
  k = dsTemp;
  d10 = k/10;

// Выделяем вторую цифру
  d1 = k - d10*10;

// Выделяем дробную часть

g = dsTemp*10;
g = g - ((d10*10 + d1)*10);

// "Рисуем" старшую цифру, затем младшую цифру
fnShowBigDigit(d10,4);
fnShowBigDigit(d1,10);
fnShowDelimiter();
fnShowSmallDigit(g,18);
}

void fnShowCelsius()
{
  for(int m=0; m<7; m++)
  {
    lc.setColumn(2,m+1,celsius[0][m]);
  }
  
}

/***************************************************** 09/02/2016 ***************************************************/
// Функция вывода знака плюс
// 
void fnShowPlus()
{
  for(int i=0;i<3;i++)
  {
    lc.setRow(0,i,plus[0][i]);
  }  
}

void fnShowMinus()
{
    for(int i=0;i<3;i++)
  {
    lc.setRow(0,i,minus[0][i]);
  }  
}

//
void fnShowDelimiter()
{
  lc.setRow(2,0,delimiter[0][0]);
  }


void Buzzer(unsigned char delayms) 
{ 
  analogWrite(Speaker, 190);       //Setting pin to high
  delay(delayms);                   //Delaying
  analogWrite(Speaker ,0);          //Setting pin to LOW
  delay(delayms);                   //Delaying
}

void loop()
{
  
// Запрашиваем температуру с датчика  
  sensors.requestTemperatures();  
  
// Считываем температуру с датчика и передаем это значение функции
  temp = sensors.getTempCByIndex(0);
  temp1 = sensors.getTempCByIndex(1);
  fnShowTwoBigDigit(temp);
  fnShowPlus();
   lcd.setCursor(0, 1); //инфо на LCD
   lcd.print(F("<:"));
   lcd.print(temp);
   lcd.setCursor(9, 1); //инфо на LCD
   lcd.print(F(">:"));
   lcd.print(F("70.42"));
int scale;
   lcd.setCursor(0, 0); //инфо на LCD
   lcd.print(F("                "));    
scale = map(temp,25,80,0,15);
if (scale == 0) {
   lcd.setCursor(0, 0); //инфо на LCD
   lcd.print(F("Is off..."));  
  }
Serial.println(scale);
 for (int val = 0; val < scale; val++)  {
   lcd.setCursor(val, 0); //инфо на LCD
   lcd.write((uint8_t)0);
 }
    
   //  fnShowCelsius();
if (temp >= 75) {
Buzzer(30);  
}

delay(500);
  }
