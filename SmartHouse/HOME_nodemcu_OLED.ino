
#include <OneWire.h>            //  Для DS18S20, DS18B20, DS1822 
#include <DallasTemperature.h>  //  Для DS18S20, DS18B20, DS1822 
#include <EEPROM.h>
#include <Wire.h>               //  Для  DS1307
#include "DS1307.h"             //  Для  DS1307
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>   // русификация шрифта  http://focuswitharduino.blogspot.ru/2015/03/lcd-nokia-5110.html


#define OLED_RESET LED_BUILTIN        // просто заглушка, oled на i2c работает без подключения этого контакта
#define Power_GSM_PIN  D5        //GSM Shield при использовании GSM шилда который садиться прямо на мегу 
#define Reset_GSM_PIN           //GSM Shield при использовании GSM шилда который садиться прямо на мегу
 
// i2c для олед дисплея 128 X 64  и часов dip-ds1307
//#define SDA             D4     // SDA
//#define SCL             D3     // SCL

#define Relay_1        D5        // Реле 1 40A    привязан к датчику  Therm_1
#define Relay_2        D5        // Реле 2 40A   
#define Relay_3        D5        // Реле 3 10A    
#define Relay_4        D5        // Реле 4 10A  
#define Relay_5        D5        // Реле 5 10A  
#define Relay_6        D5        // Реле 6 10A   
#define Speaker        3        // Динамик 
#define ONE_WIRE_BUS   D7       // Линия датчиков DS18B20
#define btn_Right      D0         // Кнопа смены статусных экранов 
//#define ENCODER_ON                              // Включить поддержку энкодера

#ifdef ENCODER_ON

  #include "SimpleTimer.h"

  uint8_t encoderDirection = 0;                 // Направление поворота энкодера
  bool encoderFlagA = false;
  bool encoderFlagB = false;
  int encoderResetTimer = 0;
  int encoderResetInterval = 2000;                     // Интервал сброса флагов
  
  SimpleTimer timer;
  
#endif //ENCODER_ON

DS1307 clock;
Adafruit_SSD1306 display(OLED_RESET);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensorsDS18B20(&oneWire);
HardwareSerial & gprsSerial = Serial1;

boolean isAutoHeating = true;     // Переменная принимает значение True, если включили автоподдержание температуры
boolean isStringMessage = false;   // Переменная принимает значение True, если текущая строка является сообщением
boolean isCalling = false;         // Переменная принимает значение True, если звонок
boolean isRelay01 = false;         // Переменная принимает значение True, если реле включено
boolean isRelay02 = false;         // Переменная принимает значение True, если реле включено
boolean isRelay03 = false;         // Переменная принимает значение True, если реле включено
boolean isRelay04 = false;         // Переменная принимает значение True, если реле включено
boolean isRelay05 = false;         // Переменная принимает значение True, если реле включено
boolean isRelay06 = false;         // Переменная принимает значение True, если реле включено
boolean isBlink = false;           // Переменная для мигания

String currStr = "";               // переменная для чтения из сомпорта и счения смс и т.д.
String Last_Tel_Number = "";       // переменная для номера от которого пришло смс или звонок
String tmp_msg = "";               // Переменная , в нее пишется стринг для отсылки СМС (не работает с sprintf();)

char First_Number[] = "+79163770340"; // Номер на который в случае чего будут идти СМС
char temp_msg[160];                   // Переменная , в нее пишется char для отсылки СМС (работает с sprintf();)

byte num_Screen = 1;   // текущий экран
byte max_Screen = 8;   // всего экранов
byte batt = 0;         // Переменная хранит заряд батареи
byte sgsm = 0;         // Переменная хранит уровень сигнала сети



 #define dacha

#ifdef dacha
  byte Board_Therm[8] = {0x28,0xFF,0x1C,0xEE,0x87,0x16,0x03,0xF5};
  byte Out_Therm[8]   = {0x28,0xFF,0xA2,0xB5,0x90,0x16,0x04,0xE7};
  byte Therm_1[8]     = {0x28,0xFF,0x91,0xB0,0x87,0x16,0x03,0x1F};
  byte Therm_2[8]     = {0x28,0xFF,0x8D,0xB5,0x87,0x16,0x03,0xC3};
#endif

int Out_Temp = 0;       // Температура на улице
int Main_Temp = 0;      // Температура на плате
int Floor_1_Temp = 0;   // Температура 1- й этаж
int Floor_2_Temp = 0;   // Температура 2- й этаж

char Main_Text[11]    = "Main";
char Out_Text[11]     = "Outd";
char Floor_1_Text[11] = "1_flr";
char Floor_2_Text[11] = "2_flr";

int Auto_Temp = 75;     // Дефолтовая автоматически поддерживаемая температура.
int Alarm_Temp = 85;    // Критическия температура, при достежении шлем СМС и все отключаем

//  Ниже не значения, а адреса ячеек ПЗУ
int Addr_Auto_Temp = 0;   // Адрес в ПЗУ для Auto_Temp
int Addr_Lcd_Tot =  1;    // Адрес в ПЗУ для ориентация экрана 0-нормальная 1-перевернутая
int Addr_Temp_1 =   2;    // Адрес в ПЗУ (начало) для хранения значения тепрературы длинна 48 байта=  24 (24 часа) х 2 (2 байта для сохранения int)
int Addr_Temp_2 =  50;    // +48
int Addr_Temp_3 =  98;    // +48
int Addr_Temp_4 = 146;    // +48
//

int SaveHistoryHour = 0;  // переменная для хранения значения последнего часа записи значения тепрературы, что бы записывать раз в час

unsigned long currentTime = 0;              // сюда просто сохраняем текущее значение Mills
unsigned long Next_Update_Draw = 0;         // Время апдейта экрана
unsigned long Next_Update_Temp = 0;         // Время апдейта температуры
unsigned long Next_Update_Screen_Saver = 0; // Время апдейта экрана

void(* resetFunc) (void) = 0;                    // declare reset function at address 0

void setup()
{
  Beep(780, 50);
  Last_Tel_Number=First_Number;
  #ifdef ENCODER_ON
    encoderSetup();
  #endif //ENCODER_ON
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.display();                          // show splashscreen
  display.clearDisplay();                     // clears the screen and buffer

  pinMode(btn_Right, INPUT_PULLUP);           //подтягиваем к кнопке внутренний резистор, что бы не паять его
  pinMode(Power_GSM_PIN, OUTPUT);
  pinMode(Relay_1, OUTPUT);
  pinMode(Relay_2, OUTPUT);
  pinMode(Relay_3, OUTPUT);
  pinMode(Relay_4, OUTPUT);
  pinMode(Relay_5, OUTPUT);
  pinMode(Relay_6, OUTPUT);
  digitalWrite(Power_GSM_PIN, LOW);
  digitalWrite(Relay_1, LOW);
  digitalWrite(Relay_2, LOW);
  digitalWrite(Relay_3, LOW);
  digitalWrite(Relay_4, LOW);
  digitalWrite(Relay_5, LOW);
  digitalWrite(Relay_6, LOW);

  EEPROM.begin(512);

  Serial.begin(9600);
  gprsSerial.begin(9600);
  clock.begin();
  sensorsDS18B20.begin();
  Read_Eprom();

  Check_GSM();

  sensorsDS18B20.requestTemperatures();
  UpdateTemp();
  SendStatus();
  //  fillHistory();
  // clock.fillByYMD(2016,01,07);
  // clock.fillByHMS(21,51,00);
  // clock.setTime();
//  EEPROM.write(addr_Auto_Temp, 24);
}

void loop()
{
  currentTime = millis();                       // считываем время, прошедшее с момента запуска программы

  if (gprsSerial.available()) {                 // Если с порта модема идет передача
    char currSymb = gprsSerial.read();         //  читаем символ из порта модема
    //Serial.println(currSymb);
    if ('\n' != currSymb) {
      currStr += String(currSymb);             // не конец строки добавляем в строку символ
    }
    else {                                      // конец строки начинаем ее разбор
      Parse_Income_String();
    }
  }
  
  if (currentTime > Next_Update_Draw) {          // время перерисовать экран
    ReadButton();
    UpdateDisplay();
    Next_Update_Draw =  millis() + 100;  // отсчитываем по 0,2 секунды
  }

  if (currentTime > Next_Update_Temp)  {         // время обновить температуру
    UpdateTemp();
    Next_Update_Temp =  millis() + 30000;           // отсчитываем по 30 секунд
  //  Check_GSM();
  }

  if (currentTime > Next_Update_Screen_Saver) {    // время включать скринсейвер на экране
    num_Screen = 0; 
    Next_Update_Screen_Saver =  millis() + 60000;  // отсчитываем по 60 секунд
  }
} // END LOOP


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
 // byte raw[2];
 // for (byte i = 0; i < 2; i++) raw[i] = EEPROM.read(addr + i);
 // int &num = (int&)raw;
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
    if (sensorVal == LOW) {                     // переключение информационных экранов
      if (num_Screen < max_Screen) {
        num_Screen++;
      }
      else  {
        num_Screen = 1;
      }
      Next_Update_Screen_Saver =  millis() + 30000; // время для включения скринсейвера
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


//
//    Serial.print(" addr2:");
//    Serial.print(adr + Addr_Temp_2);
//    Serial.print(" temp2:");
//    Serial.print(Main_Temp);
//
//   
//    Serial.print(" addr3:");
//    Serial.print(adr + Addr_Temp_3);
//    Serial.print(" temp3:");
//    Serial.print(Floor_1_Temp);
//
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
    Floor_1_Temp = -88;
  }
  if (Floor_2_Temp == -127) {
    Floor_2_Temp = -88;
  }
  if (Main_Temp == -127) {
    Main_Temp = -88;
  }
  if (Out_Temp == -127) {
    Out_Temp = -88;
  }
  SaveHistoty();

  Serial.print("id1:");
  Serial.println(Out_Temp);
  Serial.print("id2:");
  Serial.println(Main_Temp);
  Serial.print("id3:");
  Serial.println(Floor_1_Temp);
  Serial.print("id3:");
  Serial.println(Floor_2_Temp);

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
