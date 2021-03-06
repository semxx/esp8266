/*
   1MB flash sizee
   sonoff header
   1 - vcc 3v3
   2 - rx
   3 - tx
   4 - gnd
   5 - gpio 14
   esp8266 connections
   gpio  0 - button
   gpio 12 - relay
   gpio 13 - green led - active low
   gpio 14 - pin 5 on header
   http://www.esp8266.com/wiki/lib/exe/fetch.php?cache=&media=esp8266-12_mod.png
  Распиновка на NodeMCU as Master (I2C) https://www.cnx-software.com/wp-content/uploads/2015/10/NodeMCU_v0.9_Pinout.png
   ESP8266 GPIO AVAILIBLE: 0, 2, 4, 5, 12, 13, 14, 15
  D0    GPIO - 16 WAKE
  D1    GPIO - 5  ~
  D2    GPIO - 4  ~
  D3    GPIO - 0  FLASH
  D4    GPIO - 2  ~
  D5    GPIO - 14 ~
  D6    GPIO - 12 ~
  D7    GPIO - 13 ~
  D8    GPIO - 15 ~
  D9    GPIO - 3  ~ (rx)
  D10   GPIO - 1  ~ (tx)
*/
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>                // Библиотека для OTA-прошивки
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
#include <OneWire.h>                   //  Для DS18S20, DS18B20, DS1822 
#include <DallasTemperature.h>         //  Для DS18S20, DS18B20, DS1822 
#include <EEPROM.h>
#include <Wire.h>                      //  Для  DS1307
#include <Arduino_I2C_Port_Expander.h> //  Расширяем порты с помощью Arduino PRO mini https://github.com/jaretburkett/Arduino-I2C-Port-Expander
#include <DS3231.h>                    // Подключаем библиотеку для работы с RTC DS3231 https://yadi.sk/d/EPoJicxuvDVUd
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>          // русcификация шрифта http://focuswitharduino.blogspot.ru/2015/03/lcd-nokia-5110.html
#include <SoftwareSerial.h>

// Распиновка на NodeMCU as Master (I2C)
#define OLED_RESET LED_BUILTIN    //  просто заглушка, oled на i2c работает без подключения этого контакта
#define Speaker        A0         //  Динамик 
#define R              D1         //  Encoder Right
#define L              D2         //  Encoder Left
#define ONE_WIRE_BUS   D3         //  Линия датчиков DS18B20
#define SDA            D5         //  SDA   GPIO14
#define SCL            D6         //  SCL   GPIO12
#define SW_TX          D7         //  SoftwareSerial TX pin
#define SW_RX          D4         //  SoftwareSerial RX pin
#define Encoder_SW     D8         //  Кнопа смены статусных экранов 

//Распиновка на Arduino Pro Micro as Slave (I2C) https://giltesa.com/wp-content/uploads/2014/02/arduino-pinouts.jpg
#define Relay_1        2          //  Реле 1 40A ТЭН котла 2 кВт
#define Relay_2        3          //  Реле 2 40A ТЭН котла 4 кВт
#define Relay_3        4          //  Реле 3 10A Циркуляционный насос системы отопления
#define Relay_4        5          //  Реле 4 10A Греющий кабель системы отопления
#define Relay_5        6          //  Реле 5 20A Инфракрасный обогреватель
#define Relay_6        7          //  Реле 6 20A Насос водоснабжения
#define ServoReserved  8          //  Зарезервировано под серво-привод для котла
#define Movement_1     9          //  Датчик движения внутри котельной включает свет и экран
#define Movement_Out_1 10         //  Датчик движения веранда
#define Movement_Out_2 11         //  Датчик движения ворота
#define WaterControl   14         //  A0 Датчик протечки воды
#define Reset_GSM_PIN  15         //  A1 Рестарт GSM-модуля если нет ответа по команде AT

   EXPAND io(0x01);               //initialize an instance of the class with address 0x01

/*
Commands:
EXPAND io2(0x02);              // second port expander
io.digitalWrite(pin, HIGH | LOW); - writes pin high or low
io.digitalRead(pin); - Returns pin value as integer. 0 for low or 1 for high
io.digitalReadPullup(pin); - Same as digital read, but activates the  internal pullup resistor first. 
io.analogRead(pin); - Returns analog read val as int. Must call slaves digital pin number not A0. 
io.analogWrite(pin, 0-255); - writes pwm to pin. Must be a pwm capable pin. 
*/

char auth[] = "9f305a30a35541169260d7e63b169210";
char ssid[] = "Mimimi";
char pass[] = "panatorium";

boolean isAutoHeating = false;     // Переменная принимает значение True, если включили автоподдержание температуры
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
boolean SetH = false;              // выделение часов при отображении
boolean SetM = false;              // выделение минут при отображении
boolean SetYesNo = false;          // выделение Yes/No при установке часов
boolean blink500ms = false;        // мигающий бит, инвертируется каждые 500мс
boolean plus1sec = false;          // ежесекундно взводится
boolean PrintYesNo = false;        // показывать ли после времени Yes/No (косвенно - указание на режим установка/отображение)
boolean BeepEnabled = true;
static boolean rotating=false;     // debounce management

String ipString =        "";
String currStr = "";               // переменная для чтения из сомпорта и счения смс и т.д.
String Last_Tel_Number = "";       // переменная для номера от которого пришло смс или звонок
String tmp_msg = "";               // Переменная , в нее пишется стринг для отсылки СМС (не работает с sprintf();)
char   First_Number[] = "+79163770340"; // Номер на который в случае чего будут идти СМС
char   temp_msg[160];              // Переменная , в нее пишется char для отсылки СМС (работает с sprintf();)

byte MenuTimeoutTimer;
byte num_Screen = 1;               // текущий экран
byte max_Screen = 8;               // всего экранов
byte batt = 0;                     // Переменная хранит заряд батареи
byte sgsm = 0;                     // Переменная хранит уровень сигнала сети
float therm1;
float therm2;
float therm3;
float therm4;
#define test


#ifdef test
  byte Board_Therm[8] = {0x28,0xA0,0xA1,0x07,0x00,0x00,0x80,0xB6};
  byte Out_Therm[8]   = {0x28,0xFF,0x1C,0xEE,0x87,0x16,0x03,0xF5};
  byte Therm_1[8]     = {0x28,0xFF,0xA2,0xB5,0x90,0x16,0x04,0xE7};
  byte Therm_2[8]     = {0x28,0xFF,0x8D,0xB5,0x87,0x16,0x03,0xC3};
#endif

#ifdef dacha
  byte Board_Therm[8] = {0x28,0xFF,0x1C,0xEE,0x87,0x16,0x03,0xF5};      // Датчик t первый этаж
  byte Out_Therm[8]   = {0x28,0xFF,0xA2,0xB5,0x90,0x16,0x04,0xE7};     // Датчик t второй этаж
  byte Therm_1[8]     = {0x28,0xFF,0x83,0x8F,0x00,0x15,0x02,0x21};    // INPUT Котёл обратка
  byte Therm_2[8]     = {0x28,0xFF,0x0B,0x0A,0x62,0x15,0x01,0x84};   // OUTPUT Котел подача
//byte Therm_3[8]     = {0x28,0xFF,0x8D,0xB5,0x87,0x16,0x03,0xC3};  // OUTDOOR Температура за окном
#endif

int Auto_Temp = 70;       // Дефолтовая автоматически поддерживаемая температура.
int Alarm_Temp = 85;      // Критическия температура, при достежении шлем СМС и все отключаем
int Out_Temp = 0;         // Температура на улице
int Main_Temp = 0;        // Температура на плате
int Floor_1_Temp = 0;     // Температура 1- й этаж
int Floor_2_Temp = 0;     // Температура 2- й этаж
int SaveHistoryHour = 0;  // переменная для хранения значения последнего часа записи значения тепрературы, что бы записывать раз в час
int MenuItem = 0;
int oldEncoderValue  = 0;
int Hours = 0;            // времянка часов RTC для отображения и установки
int Minutes = 0;          // времянка минут RTC для отображения и установки
int Seconds;

// Переменные для Encoder
int  lastEncoded = 0;
int  encoderValue = 0;
int  lastMSB = 0;
int  lastLSB = 0;
long lastencoderValue = 0;
unsigned long LAST_DEBOUNCE_TIME = 0;
unsigned long DEBOUNCE_DELAY = 200;
unsigned long ROTATE_DELAY = 50; 
//  Ниже не значения, а адреса ячеек ПЗУ
int Addr_Auto_Temp = 0;   // Адрес в ПЗУ для Auto_Temp
int Addr_Lcd_Tot =  1;    // Адрес в ПЗУ для ориентация экрана 0-нормальная 1-перевернутая
int Addr_Temp_1 =   2;    // Адрес в ПЗУ (начало) для хранения значения тепрературы длинна 48 байта=  24 (24 часа) х 2 (2 байта для сохранения int)
int Addr_Temp_2 =  50;    // +48
int Addr_Temp_3 =  98;    // +48
int Addr_Temp_4 = 146;    // +48

char Main_Text[11]    = "Main";
char Out_Text[11]     = "Outdr";
char Floor_1_Text[11] = "1-flr";
char Floor_2_Text[11] = "2-flr";

unsigned long currentTime = 0;              // сюда просто сохраняем текущее значение Mills
unsigned long Next_Update_Draw = 0;         // Время апдейта экрана
unsigned long Next_Update_Timer = 0;        // Время апдейта таймера
unsigned long Next_Update_Temp = 0;         // Время апдейта температуры
unsigned long Next_Update_Screen_Saver = 0; // Время апдейта экрана
unsigned long EnergySaveMode = 0;           // Время экономить жизнь экрана

SoftwareSerial gprsSerial(SW_RX, SW_TX);    // установка контактов 1 и 3 для программного порта

Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);
SimpleTimer timer;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensorsDS18B20(&oneWire);
DS3231 clock;                 // Связываем объект clock с библиотекой DS3231
RTCDateTime DateTime;         // Определяем сущность структуры RTCDateTime (описанной в библиотеке DS3231) для хранения считанных с часов даты и времени
RTCAlarmTime Alarm1;          // Определяем сущность структуры RTCAlarmTime (описанной в библиотеке DS3231) для хранения считанных с первого будильника настроек даты и времени

/* Подключаем сдвиговый регистр 74HC595

#include <Shift595.h>
#define numOfRegisters 1                    // Указываем количество используемых сдвиговых регистров 74HC595
int latchPin = 3;                           // Пин подключен к ST_CP входу 74HC595 (Оранжевый > 8)
int clockPin = 1;                           // Пин подключен к SH_CP входу 74HC595  (Коричневый > 12)
int dataPin = D10;                          // Пин подключен к DS входу 74HC595 (Белый > 11)

Shift595 Shifter(dataPin, latchPin, clockPin, numOfRegisters);
*/
void setup()
{
    gprsSerial.begin(9600);  delay(50);
    Serial.begin(115200);    delay(50);
    Wire.begin(SDA,SCL);
    delay(5);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);   // initialize with the I2C addr 0x3D (for the 128x64)
    display.display();                           // show splashscreen
    delay(500);  // Clear the buffer.
    display.clearDisplay();                      // clears the screen and buffer
    delay(500);
    Last_Tel_Number=First_Number;  
    pinMode(Encoder_SW, INPUT_PULLUP);           //подтягиваем к кнопке внутренний резистор, что бы не паять его
    pinMode(R, INPUT_PULLUP);                    //  ENCODER RIGHT
    pinMode(L, INPUT_PULLUP);                    //  ENCODER LEFT
    digitalWrite(R, HIGH);                       //  turn pullup resistor on
    digitalWrite(L, HIGH);                       //  turn pullup resistor on  
 //   digitalWrite(Encoder_SW, HIGH);                       //  turn pullup resistor on 
    attachInterrupt(digitalPinToInterrupt(R), handleInterrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(L), handleInterrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(Encoder_SW), handlePush, RISING);
    EEPROM.begin(512);
    delay(10);
    clock.begin();                    // Инициализируем работу с объектом библиотеки DS3231
    clock.enableOutput(false);        // Определяем назначение вывода SQW (INT) для генерации прерываний при сработке будильников
    Read_Eprom();
    sensorsDS18B20.begin();
    sensorsDS18B20.requestTemperatures();
    UpdateTemp();
    MyWiFi();
    GSM_ON();
//  Check_GSM();
    ArduinoOTA.setHostname("BOILER-NodeMCU"); // Задаем имя сетевого порта    
//  ArduinoOTA.setPassword((const char *)"0000"); // Задаем пароль доступа для удаленной прошивки   
    ArduinoOTA.begin(); // Инициализируем OTA
    EnergySaveMode =  millis() + 35000; // самое время экономить жизнь OLED 
    timer.setInterval(500L, timerHalfSec);
    timer.setInterval(30000L, Check_GSM);
//    timer.setInterval(2000L, ReadSlave);
//  Beep(780, 50);
//  gprs_init();
//  SendStatus();
//  fillHistory();
//  EEPROM.write(addr_Auto_Temp, 24);
//  clock.setDateTime(__DATE__, __TIME__);                  // Устанавливаем время на часах, основываясь на времени компиляции скетча
//  clock.setDateTime(2018, 11, 18, 23, 01, 0);              // Установка времени вручную (Год, Месяц, День, Час, Минута, Секунда)
//  setAlarm1(Дата или день, Час, Минута, Секунда, Режим)
//  clock.setAlarm1(0, 0, 0, 10, DS3231_MATCH_S);           // Устанавливаем первый будильник на срабатывание в 10 сек. каждой минуты. Режим DS3231_MATCH_S сообщает о том, что ориентироваться надо на секунды.

/*
    Shifter.setRegisterPin(1, HIGH);
    Shifter.setRegisterPin(2, HIGH);
    Shifter.setRegisterPin(3, HIGH);
    Shifter.setRegisterPin(4, HIGH);
*/

}

String GetIpString (IPAddress ip) {
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  return ipStr;
}

void MyWiFi(){
  display.clearDisplay(); 
  WiFi.disconnect();
  delay(500);
  int mytimeout = millis() / 500;
  int x = 0;
  WiFi.mode(WIFI_STA);
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
        delay(1500);
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
    Serial.println("Not connected to Blynk server");
    Connected2Blynk = Blynk.connect(1000);
  }
  else{
//    Serial.println("Still connected to Blynk server");    
  }
}

void handleInterrupt() {
  int MSB = digitalRead(R);                     //MSB = most significant bit
  int LSB = digitalRead(L);                    //LSB = least significant bit

  int encoded = (MSB << 1) |LSB;             //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded;  //adding it to the previous encoded value
        if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) 
        { 
          if ((millis() - LAST_DEBOUNCE_TIME) > ROTATE_DELAY) {
          encoderValue --;
          Serial.println("Val--"); 
          }
          LAST_DEBOUNCE_TIME = millis(); 
          encoderR = true;
          MenuTimeoutTimer = 10;
  
        }
        if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
          if ((millis() - LAST_DEBOUNCE_TIME) > ROTATE_DELAY) {
          encoderValue ++;
          Serial.println("Val++");
          }
          LAST_DEBOUNCE_TIME = millis();
          encoderL = true;
          MenuTimeoutTimer = 10; 
        }
      
        lastEncoded = encoded;
        Next_Update_Screen_Saver =  millis() + 60000;    
        EnergySaveMode =  millis() + 45000; // время экономить жизнь OLED
  
  }


void handlePush() {
    MenuTimeoutTimer = 10;                      //таймер таймаута, секунд

if ((millis() - LAST_DEBOUNCE_TIME) > DEBOUNCE_DELAY) {

      if (num_Screen < max_Screen) {
        num_Screen++;
  Serial.println("Push");
   //       Buzzer(100);                              //Beep every 500 milliseconds

   //      delay(150);

          
          }
      else  {
        num_Screen = 1;
      }
      Next_Update_Screen_Saver =  millis() + 30000; // время для включения скринсейвера
      EnergySaveMode =  millis() + 45000;           // время экономить жизнь OLED
}

LAST_DEBOUNCE_TIME = millis(); 

}
void loop()
{
  ArduinoOTA.handle();                          // Всегда готовы к прошивке 
  currentTime = millis();                       // считываем время, прошедшее с момента запуска программы

if (Serial.available()){           // Ожидаем команды по Serial...
    gprsSerial.write(Serial.read());    // ...и отправляем полученную команду модему
    gprsSerial.println("");
//    delay(50); // задержка на обработку команды
//    toSerial();
}
  if (gprsSerial.available())                   // Если с порта модема идет передача
  {
    char currSymb = gprsSerial.read();          //  читаем символ из порта модема
    
    if ('\n' != currSymb) {
      currStr += String(currSymb);              // не конец строки добавляем в строку символ
      Serial.write(currSymb);
    }
    else {
      Serial.println("");                                     
      Parse_Income_String();                    // конец строки начинаем ее разбор
    }
  }
  
  if (currentTime > Next_Update_Draw) {         // время перерисовать экран
  //  ReadButton();
    UpdateDisplay();
//  WireIO.analogWrite(pinPwm, map(ldr, 0, 1023, 0, 255));
    Next_Update_Draw =  millis() + 100;         // отсчитываем по 0,2 секунды

  }

  if (currentTime > Next_Update_Temp)  {        // время обновить температуру
    Blynk.virtualWrite(V5, encoderValue );
    Blynk.virtualWrite(V6, therm1);
    Blynk.virtualWrite(V7, therm2);
    Blynk.virtualWrite(V8, therm3);
    Blynk.virtualWrite(V9, therm4);
    Blynk.virtualWrite(V10, Out_Temp);    
    UpdateTemp();
    //Serial.println(WIFI_getRSSIasQuality(WiFi.RSSI()));
    CheckConnection();
    Next_Update_Temp =  millis() + 30000;       // отсчитываем по 30 секунд
    //gprs_init();
  }

  if (currentTime > Next_Update_Screen_Saver && currentTime < EnergySaveMode) {    // время включать скринсейвер на экране
    num_Screen = 0; 
    Next_Update_Screen_Saver =  millis() + 60000;  // отсчитываем по 60 секунд
  }
  
  if (currentTime > EnergySaveMode) {    // время включать скринсейвер на экране
    num_Screen = 10;
    EnergySaver();
  }
  
  if (plus1sec) { // если прошла 1 секунда - делаем ежесекундные дела
        plus1sec = false; // сбрасываем до следующей секунды 
        DateTime = clock.getDateTime();   // Считываем c часов текущие значения даты и времени в сущность DateTime
        Alarm1 = clock.getAlarm1();   
        Hours=DateTime.hour;
        Minutes=DateTime.minute;
        Seconds=DateTime.second;
    }
    
  if(Connected2Blynk){
    Blynk.run();  // only process Blyk.run() function if we are connected to Blynk server
  }
  
  if (Floor_1_Temp > 79) { 
      do {
          Buzzer(100); //Beep every 500 milliseconds
          delay(150);
         }
    while (Floor_1_Temp > 79);
  }
  timer.run();
} // END MAIN LOOP


void Buzzer(unsigned char delayms) 
{ 
  analogWrite(Speaker, 1024);       //Setting pin to high
  delay(delayms);                   //Delaying
  analogWrite(Speaker ,0);          //Setting pin to LOW
  delay(delayms);                   //Delaying
}

void Beep(word frq, word dur)
{
io.digitalWrite(11,1);
//    int noteDuration = 1000 / dur;
//    tone(Speaker, frq, noteDuration);
//    int pauseBetweenNotes = noteDuration * 1.30;
//    delay(pauseBetweenNotes);
//    noTone(Speaker);
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
  
int sensorVal = digitalRead(Encoder_SW);
 //   if (sensorVal == HIGH) {                        // переключение информационных экранов
    //    Beep(626, 150);
        MenuTimeoutTimer = 10;                      //таймер таймаута, секунд
      if (num_Screen < max_Screen) {
        num_Screen++;
          Buzzer(100);                              //Beep every 500 milliseconds
          delay(150);
      }
      else  {
        num_Screen = 1;
      }
      Next_Update_Screen_Saver =  millis() + 30000; // время для включения скринсейвера
      EnergySaveMode =  millis() + 45000;           // время экономить жизнь OLED

}

void NextBattery()
{
if (batt > 10)  {
      batt = 0;
      isCalling = false;
    }
    else  {
      batt++;                                       // зарядка для батарейки
    }
}

void SaveHistoty()
{

  if (SaveHistoryHour != DateTime.hour)
  {
    int adr = (1 + DateTime.hour) * 2 - 1;

    EEPROM_int_write(adr + Addr_Temp_1, Out_Temp);
    EEPROM_int_write(adr + Addr_Temp_2, Main_Temp);
    EEPROM_int_write(adr + Addr_Temp_3, Floor_1_Temp);
    EEPROM_int_write(adr + Addr_Temp_4, Floor_2_Temp);

     /*    Debug
    Serial.print("EEPROM addr1:");
    Serial.print(adr + Addr_Temp_1);
    Serial.print(" temp1:");
    Serial.print(Out_Temp);

    Serial.print(" addr2:");
    Serial.print(adr + Addr_Temp_2);
    Serial.print(" temp2:");
    Serial.print(Main_Temp);

    Serial.print(" addr3:");
    Serial.print(adr + Addr_Temp_3);
    Serial.print(" temp3:");
    Serial.print(Floor_1_Temp);

    Serial.print(" addr4:");
    Serial.print(adr + Addr_Temp_4);
    Serial.print(" temp4:");
    Serial.println(Floor_2_Temp);
*/
     
    SaveHistoryHour = DateTime.hour;
  }

}
void ReadSlave() {      
  
  bool btn = io.digitalReadPullup(13);
   Serial.println(btn);
      if (btn) {
    //     num_Screen = 1;
      } 

  io.ServoRotate(encoderValue);
   
  }
  
  
void UpdateTemp()
{
  sensorsDS18B20.requestTemperatures();
   
  therm1 = sensorsDS18B20.getTempC(Out_Therm);
  therm2 = sensorsDS18B20.getTempC(Board_Therm);
  therm3 = sensorsDS18B20.getTempC(Therm_1);
  therm4 = sensorsDS18B20.getTempC(Therm_2);

  Out_Temp     = round(therm1);
  Main_Temp    = round(therm2);
  Floor_1_Temp = round(therm3);
  Floor_2_Temp = round(therm4);
  Serial.println(clock.readTemperature());
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
    sprintf(temp_msg, "out=%dC,main=%dC,floor_1=%dC,floor_2=%dC, time: %d:%d", Out_Temp, Main_Temp, Floor_1_Temp, Floor_2_Temp, DateTime.hour, DateTime.minute);
    SendTextMessage(Last_Tel_Number, F("ALARM TEMP. Floor REALAY OFF"), temp_msg);
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
//    WireIO.digitalWrite(13, HIGH);
  } else
    {
//    WireIO.digitalWrite(13, LOW);
    }
 }
 BLYNK_WRITE(2)
{
  int a = param.asInt();
  if (a == 0)
  {
//     WireIO.analogWrite(22,encoderValue);
  } 
 }
  BLYNK_WRITE(3)
{
  int a = param.asInt();
  if (a == 0)
  {
//    WireIO.digitalRead(13);
  }
} 
   BLYNK_WRITE(4)
{
  int a = param.asInt();
  if (a == 0)
  {
//    WireIO.analogRead(0);
  } 
 }
   BLYNK_WRITE(8)
{
  int a = param.asInt();
  if (a == 0)
  {
//    WireIO.analogWrite(0, 255);
  } else
    {
//    WireIO.analogWrite(0, 0);
    }
 }
