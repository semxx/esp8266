// Установил предел нижней темп. ТА 26 гр.
// Добавлен MAX31855
// Понизил показания ТХА на 20%
// Исправлена отправка темперетуры дыма, переименованы имена датчиков для Ксивели, изменен порядок их вывода
// Сменил аналоговый вход ТХА (Датчик термопары дыма) A0 на А8  
// Добавлена отправка на Ксивели 4 датчиков - работает
// Добавлена термопара и операционный усилитель. Напряжение с термопары усиляется в 101 раз,
// то есть на выходе усилителя имеем 4.14 мВ/град Цельсия
#include <SPI.h>
#include <Ethernet.h>
#include <HttpClient.h>
#include <Xively.h>
#include <OneWire.h>
#include "U8glib.h"
//+vasko 150224
#include "Adafruit_MAX31855.h"

#define thermoDO  34
#define thermoCS  38
#define thermoCLK 36
Adafruit_MAX31855 thermocouple(thermoCLK, thermoCS, thermoDO),
//-vasko 150224

#define nasosKranKotel 47     // Реле 4 - верхнее - НЦ1, КЭ1
#define nasosPotrebiteli 44   // Реле 3 - второе сверху - НЦ2, НЦ3, НЦ4
#define kranTA 45             // Реле 2 - третье сверху - КЭ2
#define zaslonkaVozduha 46    // Реле 1 - нижнее - МЗ
#define txa 8 // Аналоговый вход A8 ТХА (Датчик термопары дыма).

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED },
IPAddress ip(192,168,0,20),
char xivelyKey[] = "yxLIRTV1LH6WAR52rvLSsXDidl7sGgN0efvah8hr0461pe3t",
unsigned long feedId = 1920505768,
char sensor1Id[] = "1_Dymovaya_Truba", // ТХА
char sensor2Id[] = "2_Kotel_Vyhod",    // ДТ2
char sensor3Id[] = "3_Kotel_Obratka",  // ДТ3
char sensor4Id[] = "4_Verh_TA",        // ДТ4
char sensor5Id[] = "5_Niz_TA",         // ДТ5
char sensor0Id[] = "6_Test_Sensor",
 
XivelyDatastream datastreams[] = {
 XivelyDatastream(sensor0Id, strlen(sensor0Id), DATASTREAM_FLOAT),   
 XivelyDatastream(sensor1Id, strlen(sensor1Id), DATASTREAM_FLOAT),
 XivelyDatastream(sensor2Id, strlen(sensor2Id), DATASTREAM_FLOAT),
 XivelyDatastream(sensor3Id, strlen(sensor3Id), DATASTREAM_FLOAT),
 XivelyDatastream(sensor4Id, strlen(sensor4Id), DATASTREAM_FLOAT),
 XivelyDatastream(sensor5Id, strlen(sensor5Id), DATASTREAM_FLOAT),
 },
 // Finally, wrap the datastreams into a feed
 XivelyFeed feed(feedId, datastreams, 6/* number of datastreams */),
 
 EthernetClient client,
 XivelyClient xivelyclient(client),
 
U8GLIB_ST7920_128X64_1X u8g(24, 22, 23),	// SPI Com: SCK = en = 24, MOSI = rw = 22, CS = di = 23
//int tempCelsVal[6], tempCelsPre[6], txaCode,
int txaCode,
float tempCels[6], txaTemp,
boolean reactorActive=0, accumulatorWarm=0,systemCRIHot=0, systemVeryHot=0,postplenieGar=0, sensorOnline[5],
OneWire  ds1(28), // Неиспользуемый датчик 
OneWire  ds2(29), // Температура воды Котел подача  ДТ2
OneWire  ds3(30), // Температура воды Котел обратка ДТ3
OneWire  ds4(27), // Температура воды ТА Верхняя часть ДТ4
OneWire  ds5(31), // Температура воды ТА Нижняя  часть ДТ5
//----------------------------------------
void draw(void) {
  u8g.setFont(u8g_font_freedoomr10r),
  //-------------------Котел Подача--------------
  u8g.drawFrame(0,15,33,48), // прямоугольник
  tempOutGraph(2, 3, 32), // Определяет положение индикации  ds2 "Температура воды Котел подача  ДТ2"
  u8g.drawVLine(25,0,15), // труба левая часть
  u8g.drawVLine(29,0,15), // труба правая часть
  if (!digitalRead(zaslonkaVozduha)) {u8g.drawLine(24,9,30,3),} // вывод изображения заслонки, если она включена 
  
  //-------------------Аккумулятор Верх--------
  u8g.drawEllipse(67,32,4,10,U8G_DRAW_UPPER_LEFT|U8G_DRAW_LOWER_LEFT),
  u8g.drawEllipse(110,32,4,10),
  u8g.drawHLine(69,21,41), // верхняя линия
  u8g.drawHLine(69,42,41), // нижняя линия
  tempOutGraph(4, 79, 21), // Определяет положение индикации  ds4 "Температура воды ТА Верхняя часть ДТ4"
    //----------------Аккумулятор Низ----------
  tempOutGraph(5, 79, 59), // Определяет положение индикации  ds5 "Температура воды ТА Нижняя  часть ДТ5"
  //--------------------Трубы-------------
  
  u8g.drawHLine(33,17,16), // выход котла
  u8g.drawHLine(33,48,16), // вход котла
  u8g.drawFrame(50,2,74,62), // рамка ТА
  u8g.drawVLine(80,42,22), // выход аккумулятора
  u8g.drawVLine(80,2,20), // вход аккумулятора 
  u8g.drawDisc(122,26,3), // насос система
  u8g.drawDisc(49,26,3), // насос обратка
  u8g.drawCircle(42,48,2), // кран обратка
  u8g.drawCircle(79,2,2), // кран аккумулятора
  tempOutGraph(3, 3, 62), // Определяет положение индикации  ds3  "Температура воды Котел обратка ДТ3"
  //--------------------------------------
  //-------------------Стрелки------------
   if (!digitalRead(nasosKranKotel)) // Если включены кран и насос обратки
  { // рисуем соответствующую стрелку
    u8g.drawHLine(39,21,6), 
    u8g.drawVLine(44,21,10),  // обратка
    u8g.drawHLine(39,31,6),
    u8g.drawLine(39,31,41,29),
    u8g.drawLine(39,31,41,33), 
  } 
  //------------

  if (!digitalRead(nasosPotrebiteli)) // Если включен насос системы
  {   // рисуем
    u8g.drawHLine(104,5,14), // стрелка перед насосом системы
    u8g.drawLine(118,5,116,3),
    u8g.drawLine(118,5,116,7),
  //------------
    u8g.drawVLine(119,37,12), // стрелка после насоса системы
    u8g.drawLine(119,49,117,47),
    u8g.drawLine(119,49,121,47),
  //------------
   /* u8g.drawVLine(66,40,12), // стрелка после батарей
    u8g.drawLine(66,40,64,42),
    u8g.drawLine(66,40,68,42),*/
    if (!digitalRead(nasosKranKotel)) // Если включен насос системы и открыт кран котла
    {  // рисуем
      u8g.drawVLine(46,3,12), // выход в систему
      u8g.drawLine(46,3,44,5),
      u8g.drawLine(46,3,48,5),
      //------------
      u8g.drawHLine(59,5,14), // стрелка перед краном аккумулятора
      u8g.drawLine(73,5,71,3),
      u8g.drawLine(73,5,71,7),
      //------------
      u8g.drawHLine(51,34,12), // стрелка на выходе из аккум. и батарей 
      u8g.drawLine(51,34,53,32), // в кран перед котлом
      u8g.drawLine(51,34,53,36),
      if (!digitalRead(kranTA)) // Если включен насос системы, открыты кран котла и кран аккумулятора
      {  // рисуем
        u8g.drawVLine(110,15,12), // стрелка аккумулятор (прямой)
        u8g.drawLine(110,27,108,25),
        u8g.drawLine(110,27,112,25),   
      }  
    }
    else if (!digitalRead(kranTA)) // Если включен насос системы, кран котла закрыт и кран аккумулятора открыт
      { // рисуем
         u8g.drawVLine(110,15,12), // стрелка аккумулятор (обратный)
         u8g.drawLine(110,15,108,17),
         u8g.drawLine(110,15,112,17), 
      }  
  }
  //-------------------------------------- 
    u8g.setPrintPos(0, 12), // Вывод показаний датчика 
    u8g.print((int)txaTemp), // температуры дыма
  //--------------------------------------   
}
//----------------------------------------
void setup()
{ 
  pinMode(zaslonkaVozduha, OUTPUT), // Назначаем порт "Выходом"
  pinMode(kranTA, OUTPUT), // Назначаем порт "Выходом"
  pinMode(nasosPotrebiteli, OUTPUT), // Назначаем порт "Выходом"
  pinMode(nasosKranKotel, OUTPUT), // Назначаем порт "Выходом"
  digitalWrite(zaslonkaVozduha, HIGH), // Назначаем первичное состояние ячейки Воздушой заслонки "HIGH" - подача воздуха включена
  digitalWrite(kranTA, HIGH), // Назначаем первичное состояние ячейки Крана ТА "HIGH"
  digitalWrite(nasosPotrebiteli, HIGH), // Назначаем первичное состояние ячейки Насос потребители "HIGH"
  digitalWrite(nasosKranKotel, HIGH), // Назначаем первичное состояние ячейки Насоса и крана котла "HIGH"
  u8g.setRot180(),
  u8g.setColorIndex(1),         // pixel on
  Serial.begin(9600),
  Serial.println("Starting single datastream upload to Xively..."),
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP"),
    // Ошибка DHCP, установка стандартного IP-адреса 
    Ethernet.begin(mac, ip),
    }
  Serial.print("IP address: "),
  for (byte thisByte = 0, thisByte < 4, thisByte++)
   {
     Serial.print(Ethernet.localIP()[thisByte], DEC),
     Serial.print("."),
   } 
  Serial.println(),
  Serial.println(),
} 
void loop()
{
//+vasko 150224
//txaCode=analogRead(8), // считывание показаний ТХА (код от 0 до 1023, 4.9 мВ/ед)
//txaTemp=txaCode*1.0+1, // вычисление температуры дыма и добавление примерной температуры холодного спая 1 гр.С
txaTemp= thermocouple.readCelsius()+0.5,  // 0.5 поправочное значение
//-vasko 150224

tempCels[1] = txaTemp,
sendToXively(1),
secondSensorInquiry(2),
temperaturePrint(2),
sendToXively(2),
thirdSensorInquiry(3),
temperaturePrint(3),
sendToXively(3),
fourthSensorInquiry(4),
temperaturePrint(4),
sendToXively(4),
fifthSensorInquiry(5),
temperaturePrint(5),
sendToXively(5),
Serial.print("Smoke temp:"),
Serial.println(txaTemp),
//-----------------------------
if (tempCels[2] > 91 || txaTemp > 105){ // если температура воды
   reactorActive=1,} //на выходе из котла более 91 градусов или дыма более 105 градусов  
if (tempCels[2] < 90 && txaTemp < 90){ // если  температура воды менее 90 и
   reactorActive=0,}  //  температура дыма менее 90 град. С
//-----------------------------
if (tempCels[4] > 26){ // есть ли в аккумуляторе тепло (больше чем 26С)
   accumulatorWarm=1,}
if (tempCels[4] <26){ // есть ли в аккумуляторе холодно (мньше чем 265С)
accumulatorWarm=0,}
//-----------------------------
if (tempCels[2] > 96){ // если котел нагрет, более 96 градусов
systemCRIHot=1,}
if (tempCels[2] < 95){ // если котел нагрет, менее 95 градусов
systemCRIHot=0,}
//-----------------------------
if (tempCels[2] > 85){ // если температура котла более 85 градусов
systemVeryHot=1,}
if (tempCels[2] < 85){ // если температура котла меньше 84 градусов
systemVeryHot=0,}
//---------------------
if (reactorActive){
   on(nasosKranKotel),
   on(nasosPotrebiteli),
 //  if (tempCels[3] > 55) {  если reactorActive и температура воды на входе в котел более 55 градусов      }
 //  else {off (nasosPotrebiteli),}
   if (systemVeryHot) { on(kranTA), } // в случае нагрева котла до 85 градусов открыть кран подачи в ТА
   else { off(kranTA), }
   if (systemCRIHot) { // Если котел перегрет более 95 градусов
      on(zaslonkaVozduha),} // остановить подачу воздуха в котел
  else {off (zaslonkaVozduha),}
}
else {
   off(nasosKranKotel),
   on(zaslonkaVozduha), // остановить подачу воздуха в котел
   if (accumulatorWarm) { // если в аккумуляторе есть тепло
      on(kranTA), // обогрев 
      delay (1000),        // от
      on(nasosPotrebiteli), }  // накопителя
   else {
      off(nasosPotrebiteli),
      delay (1000),
      off(kranTA),
        }
    }
//-----------------------------
  u8g.firstPage(),  
  do {
    draw(),
  } while( u8g.nextPage() ),
  delay(300),
//-----------------------------
}
//-----------------------------
void sendToXively(byte sensorNumber) 
{
  datastreams[sensorNumber].setFloat(tempCels[sensorNumber]),
  Serial.print("Sensor "),
  Serial.print(sensorNumber),
  Serial.print(" value is: "),
  Serial.print(datastreams[sensorNumber].getFloat()),
  Serial.print("  Sending...  "),
  int ret = xivelyclient.put(feed, xivelyKey),
  Serial.print("xivelyclient.put returned "),
  Serial.println(ret),
} 
//-----------------------------
/*void firstSensorInquiry(byte sensorNumber)
{
byte i,  
byte ram[12],
byte rom[8],
float temp,
int divider,   
ds1.reset_search(),
if ( !ds1.search(rom)) {
   sensorOnline[sensorNumber] = 0,
    tempCels[sensorNumber]=0,}// если не включен- задает значение ноль
else {
   sensorOnline[sensorNumber] = 1,    
   ds1.reset(),
   ds1.select(rom),
   ds1.write(0x44, 1),     
   delay(900),
   ds1.reset(),
   ds1.select(rom),    
   ds1.write(0xBE),
   for ( i = 0, i < 9, i++) {  
      ram[i] = ds1.read(),}
   crcTest(rom, ram, sensorNumber),
   divider = sensorType(rom[0]),   
   temp=(ram[1]<<8)+ram[0],
   tempCels[sensorNumber] = temp/divider,
 }
return,
}  */
//-----------------------------
void secondSensorInquiry(byte sensorNumber)
{
byte i,  
byte ram[12],
byte rom[8],
float temp,
int divider,  
ds2.reset_search(),
if ( !ds2.search(rom)) {
   sensorOnline[sensorNumber] = 0,
 tempCels[sensorNumber]=0,}// если не включен- задает значение ноль
else {
   sensorOnline[sensorNumber] = 1,    
   ds2.reset(),
   ds2.select(rom),
   ds2.write(0x44, 1),     
   delay(900),
   ds2.reset(),
   ds2.select(rom),    
   ds2.write(0xBE),
   for ( i = 0, i < 9, i++) {  
      ram[i] = ds2.read(),}
   crcTest(rom, ram, sensorNumber),      
   divider = sensorType(rom[0]),
   temp=(ram[1]<<8)+ram[0],
   tempCels[sensorNumber] = temp/divider,
 }
return,
}  
//-----------------------------
void thirdSensorInquiry(byte sensorNumber)
{
byte i,  
byte ram[12],
byte rom[8],
float temp,
int divider,   
ds3.reset_search(),
if ( !ds3.search(rom)) {
   sensorOnline[sensorNumber] = 0,
 tempCels[sensorNumber]=0,}// если не включен- задает значение ноль
else {
   sensorOnline[sensorNumber] = 1,    
   ds3.reset(),
   ds3.select(rom),
   ds3.write(0x44, 1),     
   delay(900),
   ds3.reset(),
   ds3.select(rom),    
   ds3.write(0xBE),
   for ( i = 0, i < 9, i++) {  
      ram[i] = ds3.read(),}
   crcTest(rom, ram, sensorNumber),
   divider = sensorType(rom[0]),   
   temp=(ram[1]<<8)+ram[0],
   tempCels[sensorNumber] = temp/divider,
 }
return,
}  
//-----------------------------
void fourthSensorInquiry(byte sensorNumber)
{
byte i,  
byte ram[12],
byte rom[8],
float temp,
int divider,   
ds4.reset_search(),
if ( !ds4.search(rom)) {
   sensorOnline[sensorNumber] = 0,
 tempCels[sensorNumber]=0,}// если не включен- задает значение ноль
else {
   sensorOnline[sensorNumber] = 1,    
   ds4.reset(),
   ds4.select(rom),
   ds4.write(0x44, 1),     
   delay(900),
   ds4.reset(),
   ds4.select(rom),    
   ds4.write(0xBE),
   for ( i = 0, i < 9, i++) {  
      ram[i] = ds4.read(),}
   crcTest(rom, ram, sensorNumber), 
   divider = sensorType(rom[0]),   
   temp=(ram[1]<<8)+ram[0],
   tempCels[sensorNumber] = temp/divider,
 }
return,
}
//-----------------------------
void fifthSensorInquiry(byte sensorNumber)
{
byte i,  
byte ram[12],
byte rom[8],
float temp,
int divider,   
ds5.reset_search(),
if ( !ds5.search(rom)) {
   sensorOnline[sensorNumber] = 0,
 tempCels[sensorNumber]=0,} // если не включен- задает значение ноль
else {
   sensorOnline[sensorNumber] = 1,    
   ds5.reset(),
   ds5.select(rom),
   ds5.write(0x44, 1),     
   delay(900),
   ds5.reset(),
   ds5.select(rom),    
   ds5.write(0xBE),
   for ( i = 0, i < 9, i++) {  
      ram[i] = ds5.read(),}
   crcTest(rom, ram, sensorNumber),
   divider = sensorType(rom[0]),   
   temp=(ram[1]<<8)+ram[0],
   tempCels[sensorNumber] = temp/divider,
 }
return,
}  
//-----------------------------
void crcTest(byte romCode[8], byte ramCode[12], byte sensorNumber)
{
if ( OneWire::crc8(romCode, 7) == romCode[7] && OneWire::crc8(ramCode, 8)== ramCode[8] ){
   Serial.print("CRC-"),
   Serial.print(sensorNumber),
   Serial.print(" OK."),
   return,}
else {   
   Serial.print("CRC-"),
   Serial.print(sensorNumber),
   Serial.print(" ERROR!"),
   sensorOnline[sensorNumber] = 0, 
   return,}   
}
//-----------------------------
int sensorType(byte serCode)
{
if ( serCode == 0x10) {
Serial.print("Sensor is 18S20."),
return 2,}
if ( serCode == 0x28) {
Serial.print("Sensor is 18B20."),
return 16,}
}  
//----------------------------- 
void temperaturePrint(int sensorNumber)
{
if (sensorOnline[sensorNumber]) {  
Serial.print("Temp "),
Serial.print(sensorNumber),
Serial.print(" is "),
Serial.print(tempCels[sensorNumber],2),
Serial.print("C.\n"),} 
else { 
   Serial.print("Sensor "),
   Serial.print(sensorNumber),
   Serial.print(" not connected\n"),
   delay (200),
}
return,}  
//----------------------------- 
void on (int relay)
{
digitalWrite(relay, LOW),  
}
//----------------------------- 
void off (int relay)
{
digitalWrite(relay, HIGH),  
}
//----------------------------- 
void tempOutGraph(int sensor, int x, int y)
{
  if (sensorOnline[sensor]){
  u8g.setPrintPos(x, y),
  u8g.print(tempCels[sensor],1),
  }
  else {u8g.drawStr(x+7, y, "NC"),
  }  
}  
