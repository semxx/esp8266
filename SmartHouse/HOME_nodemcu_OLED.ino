//#define BLYNK_PRINT Serial     // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>    
#include <WiFiUdp.h>    
#include <ArduinoOTA.h> // Библиотека для OTA-прошивки
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>         // Essential for all Blynk Projects
#include <OneWire.h>             //  Для DS18S20, DS18B20, DS1822 
#include <DallasTemperature.h>   //  Для DS18S20, DS18B20, DS1822 
#include <EEPROM.h>
#include <Wire.h>                //  Для  DS1307
#include "DS1307.h"              //  Для  DS1307
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>    // русификация шрифта  http://focuswitharduino.blogspot.ru/2015/03/lcd-nokia-5110.html
#include <Servo.h> 
#include <Shift595.h>

#define OLED_RESET LED_BUILTIN   // просто заглушка, oled на i2c работает без подключения этого контакта
#define Power_GSM_PIN  D9        // GSM Shield при использовании GSM шилда
#define Reset_GSM_PIN            // GSM Shield при использовании GSM шилда

#define SDA            D5        // SDA   GPIO14
#define SCL            D6        // SCL   GPIO12

#define Relay_1        D9        // Реле 1 40A    привязан к датчику  Therm_1
#define Relay_2        D9        // Реле 2 40A   
#define Relay_3        D9        // Реле 3 10A    
#define Relay_4        D9        // Реле 4 10A  
#define Relay_5        D9        // Реле 5 10A  
#define Relay_6        D9        // Реле 6 10A   
#define Speaker        A0        // Динамик 
#define ONE_WIRE_BUS   D7        // Линия датчиков DS18B20
#define btn_Right      D8        // Кнопа смены статусных экранов 
#define ENCODER_ON               // Включить поддержку энкодера
#define R              D1
#define L              D2
#define numOfRegisters 1         // Указываем количество используемых сдвиговых регистров 74HC595

Adafruit_SSD1306 display(OLED_RESET);
HardwareSerial & gprsSerial = Serial1;
SimpleTimer timer;
DS1307 clock;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensorsDS18B20(&oneWire);
Servo myservo;  // create servo object to control a servo 

char auth[] = "4921ca8db3bc4cf6a84613ad405d9094";
char ssid[] = "Xiaomi_2G";
char pass[] = "panatorium";


boolean isAutoHeating = true;      // Переменная принимает значение True, если включили автоподдержание температуры
boolean isStringMessage = false;   // Переменная принимает значение True, если текущая строка является сообщением
boolean isCalling = false;         // Переменная принимает значение True, если звонок
boolean isRelay01 = false;         // Переменная принимает значение True, если реле включено
boolean isRelay02 = false;         // Переменная принимает значение True, если реле включено
boolean isRelay03 = false;         // Переменная принимает значение True, если реле включено
boolean isRelay04 = false;         // Переменная принимает значение True, если реле включено
boolean isRelay05 = false;         // Переменная принимает значение True, если реле включено
boolean isRelay06 = false;         // Переменная принимает значение True, если реле включено
boolean isBlink = false;           // Переменная для мигания
boolean Connected2Blynk = false;
boolean inMenu = false;
boolean encoderR = false;
boolean encoderL = false;

byte MenuTimeoutTimer;

boolean SetH = false; // выделение часов при отображении
boolean SetM = false; // выделение минут при отображении
boolean SetYesNo = false; // выделение Yes/No при установке часов
boolean blink500ms = false; // мигающий бит, инвертируется каждые 500мс
static boolean rotating=false;      // debounce management

boolean plus1sec = false; // ежесекундно взводится
boolean PrintYesNo = false; // показывать ли после времени Yes/No (косвенно - указание на режим установка/отображение)

int Hours = 0; // времянка часов RTC для отображения и установки
int Minutes = 0; // времянка минут RTC для отображения и установки
int Seconds;

String ipString =        "";
String currStr = "";               // переменная для чтения из сомпорта и счения смс и т.д.
String Last_Tel_Number = "";       // переменная для номера от которого пришло смс или звонок
String tmp_msg = "";               // Переменная , в нее пишется стринг для отсылки СМС (не работает с sprintf();)
char   First_Number[] = "+79163770340"; // Номер на который в случае чего будут идти СМС
char   temp_msg[160];                   // Переменная , в нее пишется char для отсылки СМС (работает с sprintf();)

byte num_Screen = 1;   // текущий экран
byte max_Screen = 8;   // всего экранов
byte batt = 0;         // Переменная хранит заряд батареи
byte sgsm = 0;         // Переменная хранит уровень сигнала сети

#define test

#ifdef test
  byte Board_Therm[8] = {0x28,0xFF,0x1C,0xEE,0x87,0x16,0x03,0xF5};
  byte Out_Therm[8]   = {0x28,0xFF,0xA2,0xB5,0x90,0x16,0x04,0xE7};
  byte Therm_1[8]     = {0x28,0xFF,0x91,0xB0,0x87,0x16,0x03,0x1F};
  byte Therm_2[8]     = {0x28,0xFF,0x8D,0xB5,0x87,0x16,0x03,0xC3};
#endif

#ifdef dacha
  byte Board_Therm[8] = {0x28,0xFF,0x1C,0xEE,0x87,0x16,0x03,0xF5};   // 
  byte Out_Therm[8]   = {0x28,0xFF,0xA2,0xB5,0x90,0x16,0x04,0xE7};   // 
  byte Therm_1[8]     = {0x28,0xFF,0x83,0x8F,0x00,0x15,0x02,0x21};    // INPUT
  byte Therm_2[8]     = {0x28,0xFF,0x0B,0x0A,0x62,0x15,0x01,0x84};   // OUTPUT
#endif

int Auto_Temp = 75;       // Дефолтовая автоматически поддерживаемая температура.
int Alarm_Temp = 85;      // Критическия температура, при достежении шлем СМС и все отключаем
int Out_Temp = 0;         // Температура на улице
int Main_Temp = 0;        // Температура на плате
int Floor_1_Temp = 0;     // Температура 1- й этаж
int Floor_2_Temp = 0;     // Температура 2- й этаж
int SaveHistoryHour = 0;  // переменная для хранения значения последнего часа записи значения тепрературы, что бы записывать раз в час
int MenuItem = 0;
int oldEncoderValue  = 0;

//  Ниже не значения, а адреса ячеек ПЗУ
int Addr_Auto_Temp = 0;   // Адрес в ПЗУ для Auto_Temp
int Addr_Lcd_Tot =  1;    // Адрес в ПЗУ для ориентация экрана 0-нормальная 1-перевернутая
int Addr_Temp_1 =   2;    // Адрес в ПЗУ (начало) для хранения значения тепрературы длинна 48 байта=  24 (24 часа) х 2 (2 байта для сохранения int)
int Addr_Temp_2 =  50;    // +48
int Addr_Temp_3 =  98;    // +48
int Addr_Temp_4 = 146;    // +48
//

char Main_Text[11]    = "Main";
char Out_Text[11]     = "Outd";
char Floor_1_Text[11] = "1_flr";
char Floor_2_Text[11] = "2_flr";

unsigned long currentTime = 0;              // сюда просто сохраняем текущее значение Mills
unsigned long Next_Update_Draw = 0;         // Время апдейта экрана
unsigned long Next_Update_Temp = 0;         // Время апдейта температуры
unsigned long Next_Update_Screen_Saver = 0; // Время апдейта экрана
unsigned long EnergySaveMode = 0;           // Время экономить жизнь экрана

// Подключаем сдвиговый регистр 74HC595
int latchPin = 3;                           // Пин подключен к ST_CP входу 74HC595 (Оранжевый > 8)
int clockPin = 1;                           // Пин подключен к SH_CP входу 74HC595  (Коричневый > 12)
int dataPin = D0;                           // Пин подключен к DS входу 74HC595 (Белый > 11)

Shift595 Shifter(dataPin, latchPin, clockPin, numOfRegisters);

// Переменные для Encoder
int  lastEncoded = 0;
int  encoderValue = 0;
int  lastMSB = 0;
int  lastLSB = 0;
long lastencoderValue = 0;

void(* resetFunc) (void) = 0;               // declare reset function at address 0


void setup()
{
  ArduinoOTA.setHostname("BOILER-NodeMCU"); // Задаем имя сетевого порта    
//ArduinoOTA.setPassword((const char *)"0000"); // Задаем пароль доступа для удаленной прошивки   
  ArduinoOTA.begin(); // Инициализируем OTA
//Beep(780, 50);
  Wire.begin(SDA,SCL);
  delay(5);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  //display.display();                          // show splashscreen
  delay(500);  // Clear the buffer.
  display.clearDisplay();                    // clears the screen and buffer
  pinMode(btn_Right, INPUT_PULLUP);           //подтягиваем к кнопке внутренний резистор, что бы не паять его
  pinMode(Power_GSM_PIN, OUTPUT);
  pinMode(Relay_1, OUTPUT);
  pinMode(Relay_2, OUTPUT);
  pinMode(Relay_3, OUTPUT);
  pinMode(Relay_4, OUTPUT);
  pinMode(Relay_5, OUTPUT);
  pinMode(Relay_6, OUTPUT);
  pinMode(Speaker, OUTPUT); //Set buzzerPin as output
  digitalWrite(Speaker, HIGH);
  digitalWrite(Power_GSM_PIN, LOW);
  digitalWrite(Relay_1, LOW);
  digitalWrite(Relay_2, LOW);
  digitalWrite(Relay_3, LOW);
  digitalWrite(Relay_4, LOW);
  digitalWrite(Relay_5, LOW);
  digitalWrite(Relay_6, LOW);
  pinMode(R, INPUT_PULLUP); //  ENCODER RIGHT
  pinMode(L, INPUT_PULLUP); //  ENCODER LEFT
  digitalWrite(R, HIGH);    //  turn pullup resistor on
  digitalWrite(L, HIGH);    //  turn pullup resistor on  
  attachInterrupt(digitalPinToInterrupt(R), handleInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(L), handleInterrupt, CHANGE);
  Shifter.setRegisterPin(1, HIGH);
  Shifter.setRegisterPin(2, HIGH);
  Shifter.setRegisterPin(3, HIGH);
  Shifter.setRegisterPin(4, HIGH);
  EEPROM.begin(512);
  delay(10);
  gprsSerial.begin(9600);
  clock.begin();
  myservo.attach(0);
  Read_Eprom();
  sensorsDS18B20.begin();
  sensorsDS18B20.requestTemperatures();
  UpdateTemp();
  Serial.begin(9600);
  // Check_GSM();
  // SendStatus();
  // fillHistory();
  // clock.fillByYMD(2016,01,10);
  // clock.fillByHMS(22,32,00);
  // clock.setTime();
  // EEPROM.write(addr_Auto_Temp, 24);
  EnergySaveMode =  millis() + 15000; // самое время экономить жизнь OLED
  Last_Tel_Number=First_Number;
  MyWiFi();
  timer.setInterval(500L, timerHalfSec);
}

String GetIpString (IPAddress ip) {
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  return ipStr;
}

void MyWiFi(){
  display.clearDisplay(); 
  int mytimeout = millis() / 1000;
  int x = 0;
  WiFi.begin(ssid, pass);
  delay(500);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    MyPrint(F("  WiFi"),         1 * 6 - 6,  1 * 8 - 8, 2, 1);
    MyPrint(F("Connecting"),     1 * 6 - 6,  4 * 8 - 8, 2, 1);
    MyPrint(F("_"),          1 + x * 6 - 6,  6 * 8 - 8, 2, 1);
    display.display();
    if((millis() / 1000) > mytimeout + 4){ // try for less than 5 seconds to connect to WiFi router
      display.clearDisplay(); 
      break;
    }
  x = x + 1;
  }

   if  (WiFi.status() == WL_CONNECTED) {
      display.clearDisplay(); 
      IPAddress espIP;
      espIP=WiFi.localIP();
      ipString = GetIpString(espIP);
        MyPrint(F("  WiFi"),     1 * 6 - 6, 1 * 8 - 8, 2, 1);
        MyPrint(F("Connected!"), 1 * 6 - 6, 4 * 8 - 8, 2, 1);
        MyPrint(ipString,        1 * 6 - 6, 7 * 8 - 8, 1, 1);
        display.display();
        delay(2500);
    }   
  else {
        mytimeout = millis() / 1000;       
        do {
            MyPrint(F("  WiFi"),    1 * 6 - 6, 1 * 8 - 8, 2, 1);
            MyPrint(F("not found"), 1 * 6 - 1, 4 * 8 - 8, 2, 1); 
            display.display();
           }
           while (((millis() / 1000) > mytimeout + 1)); 

          } 
     
  Blynk.config(auth);
  Connected2Blynk = Blynk.connect(1000);  // 1000 is a timeout of 3333 milliseconds 
  mytimeout = millis() / 1000;
  while (Blynk.connect(1000) == false) { 
    if((millis() / 1000) > mytimeout + 3){ // try for less than 4 seconds
      break;
    }
  }
  display.clearDisplay(); 
}


void CheckConnection(){
  Connected2Blynk = Blynk.connected();
  if(!Connected2Blynk){
    //Serial.println("Not connected to Blynk server");
    MyWiFi();  
  }
  else{
    //Serial.println("Still connected to Blynk server");    
  }
}

void handleInterrupt() {
  int MSB = digitalRead(R);                     //MSB = most significant bit
  int LSB = digitalRead(L);                    //LSB = least significant bit

  int encoded = (MSB << 1) |LSB;             //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded;  //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) 
  { 
    encoderValue --;
    encoderR = true;
    MenuTimeoutTimer = 10;
  }
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
    encoderValue ++;
    encoderL = true;
    MenuTimeoutTimer = 10;
  }

  lastEncoded = encoded; //store this value for next time
 //   Serial.print("Encoder: ");
 //   Serial.println(encoderValue);
  Next_Update_Screen_Saver =  millis() + 60000;    
  EnergySaveMode =  millis() + 45000; // время экономить жизнь OLED
}

void loop()
{
  ArduinoOTA.handle(); // Всегда готовы к прошивке 
  currentTime = millis();                       // считываем время, прошедшее с момента запуска программы
  if (gprsSerial.available()) {                 // Если с порта модема идет передача
    char currSymb = gprsSerial.read();          // читаем символ из порта модема
    //Serial.println(currSymb);
    if ('\n' != currSymb) {
      currStr += String(currSymb);              // не конец строки добавляем в строку символ
    }
    else {                                      // конец строки начинаем ее разбор
//      Parse_Income_String();
    }
  }
  
  if (currentTime > Next_Update_Draw) {         // время перерисовать экран
    ReadButton();
    UpdateDisplay();
    myservo.write(encoderValue);
   // delay(15);
    Next_Update_Draw =  millis() + 100;         // отсчитываем по 0,2 секунды
  }

  if (currentTime > Next_Update_Temp)  {        // время обновить температуру
    Blynk.virtualWrite(V5, encoderValue );
    Blynk.virtualWrite(V6, Floor_1_Temp);
    Blynk.virtualWrite(V7, Floor_2_Temp);
    UpdateTemp();
    // Serial.println(WIFI_getRSSIasQuality(WiFi.RSSI()));
    // CheckConnection();
    Next_Update_Temp =  millis() + 30000;       // отсчитываем по 30 секунд
    // Check_GSM();
  }

  if (currentTime > Next_Update_Screen_Saver && currentTime < EnergySaveMode) {    // время включать скринсейвер на экране
    num_Screen = 0; 
    Next_Update_Screen_Saver =  millis() + 60000;  // отсчитываем по 60 секунд
  }
  
  if (currentTime > EnergySaveMode) {    // время включать скринсейвер на экране
    num_Screen = 10;
    EnergySaver();
    //EnergySaveMode =  millis() + 45000; // время экономить жизнь OLE
  }

  if (plus1sec && !SetH && !SetM) { // если прошла 1 секунда - делаем ежесекундные дела
        plus1sec = false; // сбрасываем до следующей секунды
        clock.getTime();// обновляем часы
        Hours=clock.hour;
        Minutes=clock.minute;
        Seconds=clock.second;
    }
    
  if(Connected2Blynk){
    Blynk.run();  // only process Blyk.run() function if we are connected to Blynk server
  }
  if (num_Screen == 8) 
  {
  //display.clearDisplay();  
  AdjustTime(); 
  //display.display();  
  }
  if (Floor_1_Temp > 79) { 
      do {
          Buzzer(100); //Beep every 500 milliseconds
          delay(150);
         }
    
    while (Floor_1_Temp > 79);
  }

  timer.run();
} // END LOOP


void Buzzer(unsigned char delayms) 
{ 
  analogWrite(Speaker, 1024); //Setting pin to high
  delay(delayms); //Delaying
  analogWrite(Speaker ,0); //Setting pin to LOW
  delay(delayms); //Delaying
}

void Beep(word frq, word dur)
{
    int noteDuration = 1000 / dur;
    tone(Speaker, frq, noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(Speaker);
}

void Ring()
{
  Beep(780, 100);
  Beep(700, 100);
  Beep(626, 150);
}

// чтение из ПЗУ
int EEPROM_int_read(int addr) {
 int num = EEPROM.read(addr);
 return num;
}

// запись в ПЗУ
void EEPROM_int_write(int addr, int num) {
  byte raw[2];
  (int&)raw = num;
  for (byte i = 0; i < 2; i++) EEPROM.write(addr + i, raw[i]);
EEPROM.commit();
}

void ReadButton()
{
 int sensorVal = digitalRead(btn_Right);
    if (sensorVal == HIGH) {                     // переключение информационных экранов
        MenuTimeoutTimer = 10; //таймер таймаута, секунд
      if (num_Screen < max_Screen) {
        num_Screen++;
          Buzzer(100); //Beep every 500 milliseconds
          delay(150);
      }
      else  {
        num_Screen = 1;
      }
      Next_Update_Screen_Saver =  millis() + 30000; // время для включения скринсейвера
      EnergySaveMode =  millis() + 45000; // время экономить жизнь OLED
      Beep(500, 20);
    }
}

void NextBattery()
{
if (batt > 10)  {
      batt = 0;
      isCalling = false;
    }
    else  {
      batt++; // зарядка для батарейки
    }
}    

void SaveHistoty()
{

  if (SaveHistoryHour != clock.hour)
  {
    int adr = (1 + clock.hour) * 2 - 1;

    EEPROM_int_write(adr + Addr_Temp_1, Out_Temp);
    EEPROM_int_write(adr + Addr_Temp_2, Main_Temp);
    EEPROM_int_write(adr + Addr_Temp_3, Floor_1_Temp);
    EEPROM_int_write(adr + Addr_Temp_4, Floor_2_Temp);
    
//    Serial.print("EEPROM addr1:");
//    Serial.print(adr + Addr_Temp_1);
//    Serial.print(" temp1:");
//    Serial.print(Out_Temp);
//
//    Serial.print(" addr2:");
//    Serial.print(adr + Addr_Temp_2);
//    Serial.print(" temp2:");
//    Serial.print(Main_Temp);
//
//    Serial.print(" addr3:");
//    Serial.print(adr + Addr_Temp_3);
//    Serial.print(" temp3:");
//    Serial.print(Floor_1_Temp);
//
//    Serial.print(" addr4:");
//    Serial.print(adr + Addr_Temp_4);
//    Serial.print(" temp4:");
//    Serial.println(Floor_2_Temp);

    SaveHistoryHour = clock.hour;
  }

}

void UpdateTemp()
{
  clock.getTime();
  sensorsDS18B20.requestTemperatures();

  Out_Temp = sensorsDS18B20.getTempC(Out_Therm);
  Main_Temp = sensorsDS18B20.getTempC(Board_Therm);
  Floor_1_Temp = sensorsDS18B20.getTempC(Therm_1);
  Floor_2_Temp = sensorsDS18B20.getTempC(Therm_2);

  // если датчик не подключен выдает -127 на экране не красиво, поставим -88
  if (Floor_1_Temp == -127) {
    Floor_1_Temp = 00;
  }
  if (Floor_2_Temp == -127) {
    Floor_2_Temp = 00;
  }
  if (Main_Temp == -127) {
    Main_Temp = 00;
  }
  if (Out_Temp == -127) {
    Out_Temp = 00;
  }
  SaveHistoty();
/*
  Serial.print("id1:");
  Serial.println(Out_Temp);
  Serial.print("id2:");
  Serial.println(Main_Temp);
  Serial.print("id3:");
  Serial.println(Floor_1_Temp);
  Serial.print("id3:");
  Serial.println(Floor_2_Temp);
*/
  if (isAutoHeating)                   // Если Включено автоподдержание то реагируем
  {
    // 1-й этаж
    if (Floor_1_Temp >= Auto_Temp)     // Если достигли нужную температуру,- выключаем нагрев
    {
      digitalWrite(Relay_1, LOW);
      isRelay01 = false;
    }
    if (Floor_1_Temp <= Auto_Temp - 2 ) // Если Температура упала на 2 градуса,- включаем нагрев
    {
      digitalWrite(Relay_1, HIGH);
      isRelay01 = true;
    }
    
    // 2-й этаж
    if (Floor_2_Temp >= Auto_Temp)      // Если достигли нужную температуру,- выключаем нагрев
    {
      digitalWrite(Relay_2, LOW);
      isRelay02 = false;
    }
    if (Floor_2_Temp <= Auto_Temp - 2 ) // Если Температура упала на 2 градуса,- включаем нагрев
    {
      digitalWrite(Relay_2, HIGH);
      isRelay02 = true;
    }

  }

  if (Floor_1_Temp > Alarm_Temp  || Floor_2_Temp > Alarm_Temp  || Main_Temp > (Alarm_Temp * 2) ) // Если вдруг температура выше допустимой вырубаем все, попутно контролируем температуру на плате
  {
    digitalWrite(Relay_1, LOW);
    digitalWrite(Relay_2, LOW);
    isRelay01 = false;
    isRelay02 = false;
    isAutoHeating = false;
    sprintf(temp_msg, "out=%dC,main=%dC,floor_1=%dC,floor_2=%dC, time: %d:%d", Out_Temp, Main_Temp, Floor_1_Temp, Floor_2_Temp, clock.hour, clock.minute);
//    SendTextMessage(Last_Tel_Number, F("ALARM TEMP. Floor REALAY OFF"), temp_msg);
  }


}

void Read_Eprom()
{
  int val = EEPROM.read(Addr_Auto_Temp);
  if (val < Alarm_Temp && val > 1 )   // температура должна в ПЗУ меньше Аларма и выше 1 градуса, то устанавливаем ее
  {
    Auto_Temp = val;
  }
  else                                 // Если с значением в ПЗУ что-то это не так или не запрограммированно, то ставим температуру по умолчанию
  {
    EEPROM.write(Addr_Auto_Temp, Auto_Temp);
    EEPROM.commit();
  }

  val = EEPROM.read(Addr_Lcd_Tot);
  if (val == 1 )   // ориентация экрана 0-нормальная 1-перевернутая
  {
     // u8g.setRot180();
  }
  else                                 // Если с значением в ПЗУ что-то это не так или не запрограммированно, то ставим температуру по умолчанию
  {
     //u8g.undoRotation();
  }

}

void fillHistory() // Заполнить EEPROM временными данными
{
  for(int val=0; val<(23); val++) 
  {
    int adr = (1 + val) * 2 - 1;

    EEPROM_int_write(adr + Addr_Temp_1, 23);
    EEPROM_int_write(adr + Addr_Temp_2, 22);
    EEPROM_int_write(adr + Addr_Temp_3, 21);
    EEPROM_int_write(adr + Addr_Temp_4, 20);
    EEPROM.commit();
  }  
}
BLYNK_WRITE(1)
{
  int a = param.asInt();
  if (a == 0)
  {
    Shifter.setRegisterPin(1, HIGH);
  } else
    {
    Shifter.setRegisterPin(1, LOW);
    }
 }
 BLYNK_WRITE(2)
{
  int a = param.asInt();
  if (a == 0)
  {
    Shifter.setRegisterPin(2, HIGH);
  } else
    {
    Shifter.setRegisterPin(2, LOW);
    }
 }
  BLYNK_WRITE(3)
{
  int a = param.asInt();
  if (a == 0)
  {
    Shifter.setRegisterPin(3, HIGH);
  } else
    {
    Shifter.setRegisterPin(3, LOW);
    }
 }
   BLYNK_WRITE(4)
{
  int a = param.asInt();
  if (a == 0)
  {
    Shifter.setRegisterPin(4, HIGH);
  } else
    {
    Shifter.setRegisterPin(4, LOW);
    }
 }
