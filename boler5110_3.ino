

/* автор Глебов Вадим 
 * готовое устройство 
 * отдельные кнопки управления
 *  
 */
#include <Servo.h> 
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LCD5110_Basic.h>
#include <EEPROM.h>


extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
extern uint8_t BigNumbers[];




//DeviceAddress sensor1 = {0x28, 0xff, 0xDB, 0x76, 0x62, 0x15, 0x1, 0x60};
Servo myservo;  // create servo object to control a servo

// инициализация LCD
//LCD5110 myGLCD(2, 3, 4, 6, 5);//(SCLK)(DIN)(D/C)(RST)(CS)назначаем пины экрана (у вас свои пины)
LCD5110 myGLCD(15, 13, 12, 9, 10);//(SCLK)(DIN)(D/C)(RST)(CS)назначаем пины экрана (у вас свои пины)
//      SCK  - Pin 8
//      MOSI - Pin 9
//      DC   - Pin 10
//      RST  - Pin 11
//      CS   - Pin 12

int Contrast=65; //переменная для экранов Contrast
int m=0; //переменная для экранов меню
int MODE = 0; //
int p0 = 60; // температура вкл насоса
int p1 = 70; // температура закрытия заслонки(поддерживаемая температура теплоносителя)
int p2 = 55; //  температура откл насоса
int p3 = 1; // вкл-откл насоса
int p31 = 1; // вкл-откл насоса
int p4 = 80; // температура включения сигналлизации при повышении
int p5 = 0; // состояние сигналлизации 
int p6 = 0; // температура включения сигналлизации при понижении 
int Temp ; // температура с датчика обратки
int Temp1 ; // температура с датчика подачи
int  vall=0 ;   //положение сервы
int  val=0 ;   //для сервы
int  Dg=1 ;   //для сервы
byte ser ;  // разрешение записи в серву
int lightLev = 30;//уровень подсветки
int lightLevmax = 150;//максимальный уровень подсветки
byte addr ;
boolean Autopump = true; 
boolean Alarm = true;
boolean Alarmd = true;
char* pump[]={"Off", "On "}; // первый элемент с индексом 0, второй - с индексом 1 и т.д.
//пины 
byte gerPin = 19;//вход включения геркона открытия дверки топки
byte servoPin = 17;//выход включения servoprivoda
byte PumpPin = 2;//выход включения насоса
byte lightPin = 11;//pin подсветка
byte SPKPin = 18;// выход сигналлизации
byte keyPin1 = 7;// вход клавиатуры
byte keyPin2 = 6;// вход клавиатуры
byte keyPin3 = 5;// вход клавиатуры
byte keyPin4 = 4;// вход клавиатуры
byte TPin = 3;// вход датчиков температуры

OneWire oneWire(TPin); // вход датчиков 18b20
DallasTemperature ds(&oneWire);
DeviceAddress sensor1 = {0x28, 0xFF, 0x88, 0x10, 0xA1, 0x15, 0x1, 0x95}; //подача
DeviceAddress sensor2 = {0x28, 0xFF, 0xBC, 0x1E, 0xA1, 0x15, 0x3, 0xA7}; //доп датчик


unsigned long PrevMillis ;
unsigned long MillisPassed ;
unsigned long PrMslight ;
unsigned long MsPdlight ;
unsigned long APrevMillis ;
unsigned long AMillisPassed ;





uint8_t char_cel[8] = {
  B00111,B00101,B00111,B00000,B00000,B00000,B00000};

byte key(){ //// для кнопок ЛСДшилда
  int val = analogRead(keyPin1);
  delay (10);
    
    if (!digitalRead(keyPin1)) return 1;         //next кнопка меню в меню и запись параметров и состояния в EEPROM
    else if (!digitalRead(keyPin2)) return 4;  //up увеличение значения отображаемого параметра
    else if (!digitalRead(keyPin3)) return 3;  // prev кнопка возврата
    else if (!digitalRead(keyPin4)) return 2;  //down уменьшение значения отображаемого параметра
    else return 0;   
    PrMslight = millis();//
}

void setup() {
  
//Serial.begin(9600);
  //Установка пинов как выходов
  pinMode(lightPin, OUTPUT);
  pinMode(SPKPin, OUTPUT);
  pinMode(PumpPin, OUTPUT);
  analogWrite(lightPin, lightLev);
  
  myservo.attach(servoPin);  // attaches the servo on pin servoPin to the servo object
  
  //Установка пинов как входов
  pinMode(gerPin, INPUT);
  pinMode(keyPin1, INPUT);
  pinMode(keyPin2, INPUT);
  pinMode(keyPin3, INPUT);
  pinMode(keyPin4, INPUT);
  digitalWrite(gerPin, HIGH);
  digitalWrite(keyPin1, HIGH);
  digitalWrite(keyPin2, HIGH);
  digitalWrite(keyPin3, HIGH);
  digitalWrite(keyPin4, HIGH);
  
  ds.begin();
   
 // Вывод приветствия
  MODE = EEPROM.read(1);
if (MODE==2)p1=85;       
if (MODE==1)p1=75;        
if (MODE==0)p1=65;       
  myGLCD.InitLCD();//инициализацие экрана
  myGLCD.setContrast(Contrast);// контрастность
  myGLCD.setFont(SmallFont);//маленький шрифт 
  myGLCD.clrScr();//чистим экран
  delay(100);
  myGLCD.print("boiler control v2.0", CENTER, 16);//заставка
  delay(500);
  myGLCD.clrScr();//чистим экран
  delay(100); 
  myGLCD.print("VadimGl", CENTER, 16);//заставка
  myGLCD.print("gl_2004@mail.ru", CENTER, 32);//заставка
  delay(1100); 
  myGLCD.clrScr();//чистим экран
 delay(100);
  myGLCD.print("mode:", CENTER, 16);//заставка
  myGLCD.printNumI(MODE+1, RIGHT, 16);
  myGLCD.print("temp:", CENTER, 32);//заставка
  myGLCD.printNumI(p1, RIGHT, 32);
  delay(1100); 
  myGLCD.clrScr();//чистим экран
  
 }

void loop() {
  
  //Обработка нажатия кнопки меню
  if (key()> 0){
    PrMslight = millis();  
  for (int i=0; i<60 ; i++){
        digitalWrite(SPKPin, HIGH);
        delayMicroseconds(200) ;    
        digitalWrite(SPKPin, LOW);
        delayMicroseconds(200) ;    
        }
  analogWrite(lightPin, lightLev);
//для насстроки программы
 // Serial.print(key()); 
 // Serial.println("-");  
  if (key()== 2 && m==0){
  delay (50);
  myGLCD.clrScr();//чистим экран
  EEPROM.write(1, MODE);
  myGLCD.setFont(SmallFont);
  myGLCD.print("  S  A  V  E   ", CENTER, 16);
  delay (500);
  myGLCD.clrScr();//чистим экран
   }
  
  if (key()== 2){
  m--;//уменьшаем переменную уровня меню
  if (m<0) m=0;// то вернуться к началу//если уровень меньше 0
   delay (50);
  myGLCD.clrScr();//чистим экран;
  }
  if (key()== 1)
  {
  m++;//увеличиваем переменную уровня меню
  if (m>1) m=1;//если уровень больше 1 // то вернуться к началу
  delay (50);
  myGLCD.clrScr();//чистим экран;
  }

  
  //+ для р3 вкл насоса в главном меню
    if (key()== 3 && m==0)
  {
  p3++;
  if (p3>1)
  {
  p3=1;
  }
  delay (50);
  Autopump = false;
  }
//- для р3  выкл насоса в главном меню
    
  if (key()== 4 && m==0)
  {
  p3--;
  if (p3<0)
  {
  p3=0;
   }
  delay (50);
  Autopump = true;
  
  }
//выбор режима работы
 if (key()== 3 && m==1)
  {
  MODE--;
  if (MODE<0)MODE=0;
  myGLCD.clrScr();//чистим экран;
  delay (50);
   }
    if (key()== 4 && m==1)
  {
  MODE++;
  if (MODE>2)MODE=2;
  myGLCD.clrScr();//чистим экран;
  delay (50);
   }

//задаем параметры работы в выбраном режиме 

//if (MODE==3)p1=85,p2=55,p4=90,p6=70;       //p1 - температура закрытия заслонки
  if (MODE==2)p1=75,p2=55,p4=80,p6=60;       //p2 - температура откл насоса
  if (MODE==1)p1=65,p2=50,p4=80,p6=50;       //p4 - температура включения сигналлизации при повышении   
  if (MODE==0)p1=60,p2=45,p4=80,p6=10;       //p6 - температура включения сигналлизации при понижении
 }



//подсветка  и переход в главное меню через 10 секунд после нажатия кнопок

   if (MsPdlight > 10000){  //интервал 10 сек
   analogWrite(lightPin, lightLev);
   if (m>0) myGLCD.clrScr();//чистим экран
   m = 0;
  //
  }
  else analogWrite(lightPin, lightLevmax);
   MsPdlight = (millis() - PrMslight);

 //вывод меню
  if (m==0) //главное меню, можно включить насос на постоянную работу
  {
   if (Temp1>99)Temp1=99; 
   //else Dg=24;
    myGLCD.setFont(BigNumbers);
    myGLCD.printNumF(Temp1, 0, 24, 0);
    myGLCD.setFont(SmallFont);
    myGLCD.print("'C", 60, 0);
    myGLCD.print("'C", 40,32);
    myGLCD.print(pump[p3], 64, 16);
    myGLCD.setFont(MediumNumbers);
    myGLCD.printNumI(vall/10, RIGHT, 32);
    myGLCD.printNumF(Temp, 0, 8, 32);
  }
if (m==1)  // 
  {
    myGLCD.setFont(SmallFont);
    
    myGLCD.print("1.Economy", 10, 4);
    myGLCD.print("2.Norma", 10, 20);
    myGLCD.print("3.Higt", 10, 36);
    myGLCD.print("*", 0, MODE*16+4);
    
     }
     
   //управление

  ds.requestTemperatures(); // считываем температуру с датчиков
  
  Temp = ds.getTempC(sensor2);
 Temp1 = ds.getTempC(sensor1);


 //включение насоса

  if (Temp1 >= p0)  p3=1;
if (Temp1 <= 5)  p3=1;
if (Temp1 <= p2 && Autopump == true) p3=0;//выключение насоса если температура ниже температуры отключения
//if (Temp1 <= 0)p3=0; 
 
//включение сигналлизации при превышении

if (Temp1 >= p4) 
{
  if (Alarm && AMillisPassed > 300){//интервал 0,3 сек
  for (int i=0; i<200 ; i++){
          digitalWrite(SPKPin, HIGH);
          delayMicroseconds(200) ;
          digitalWrite(SPKPin, LOW);
          delayMicroseconds(200) ;
                        }
   APrevMillis = millis();
  }
 
  if (key()> 0) Alarm = false; 
 }
  else Alarm = true;
  

//включение сигналлизации при понижении

if (Temp1 <= p6 ) 
{
  if (Alarmd && AMillisPassed > 100){  //интервал 1 сек
for (int i=0; i<500 ; i++){
          digitalWrite(SPKPin, HIGH);
          delayMicroseconds(500) ;
          digitalWrite(SPKPin, LOW);
          delayMicroseconds(500) ; 
    
  }
   APrevMillis = millis();
 }
  if (key()> 0) Alarmd = false; 
 }
  else Alarmd = true;
    AMillisPassed = (millis() - APrevMillis);

    
 p31 = p3 ; // вкл-откл насоса   
if (p3> 0){ 
p3 = 0; 
}
  else  p3 = 1; 
 digitalWrite(PumpPin, p3);  //запись состояния насоса.  
p3 = p31 ;


//подсветка  и переход в главное меню через 10 секунд после нажатия кнопок

if (MsPdlight > 10000){  //интервал 10 сек
   analogWrite(lightPin, lightLev);
   if (m>0) myGLCD.clrScr();//чистим экран
   m = 0;
  }
  else analogWrite(lightPin, lightLevmax);
   MsPdlight = (millis() - PrMslight);


   

//управление сервой
if (digitalRead(gerPin)){   // открытие топки
  vall = 0 ; 
  myGLCD.clrScr();//чистим экран;
                        }
else {
 val = map(Temp1, p1-10, p1, 90, 0);//значение сервы от 90 до 0 при температуре -10 до +0 от установленной
 val = constrain(val, 0, 90);
 vall = val; 
     }
myservo.write(vall);

  }
