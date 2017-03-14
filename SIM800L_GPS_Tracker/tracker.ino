// http://alielectronics.ru/blog/diy-gps-tracker-sim808-and-arduino-nano

#include <SoftwareSerial.h>
SoftwareSerial GSMport(3, 2); // RX, TX

typedef struct {
    String latitude;
    String longitude;
    String datetime;
} MyDataGPS;

MyDataGPS dataGPS;
void gprs_init(void);
MyDataGPS getGPSLocation(void);
void gprs_send(String dataa, String datab, int dataA0, int dataA1);
String ReadGSM();

void setup() {

  Serial.begin(9600);
  GSMport.begin(9600);
  GSMport.println("AT+CLIP=1"); //устанавливаем чтобы номер входящего звонка определялся
  delay(300);
  GSMport.println("AT+DDET=1"); //устанавливаем прием DTMF
  delay(300);
  gprs_init();

  digitalWrite(A1,1); //подтяжка к 1
}

void loop() {
  char c;
  String str;

if (analogRead(A1)<100) {
  dataGPS = getGPSLocation();
  Serial.println("Level A1: "+String(analogRead(A1)));
  gprs_send(dataGPS.latitude, dataGPS.longitude, analogRead(A0), analogRead(A1));
}

  while (GSMport.available()) {  //отправляем данные с GSM в Serial
    c = GSMport.read();
    Serial.write(c);
    delay(10);
  }
  while (Serial.available()) {  //сохраняем входную от консоли строку в переменную str пока не конец строки
    c = Serial.read();
    GSMport.write(c);
    //str += c;
    //if (c == '\n') {
    // AnalyseCmd(&str);
    // str = "";
    delay(10);
  }
}


MyDataGPS getGPSLocation(void) {  //функция определения местоположения по GPS
  String v,v1,v2;
  String data[5];
  MyDataGPS d;
  int a = 0,b = 0;
  GSMport.println("AT+CGNSPWR=1");
  delay(1000);
  Serial.println(ReadGSM());
  GSMport.println("AT+CGNSINF");
  delay(400);
  delay(400);
  v = ReadGSM();
  Serial.println(v);
    
 for(int i=0;i<5;i++){ //парсинг данных между запятыми в массив строк
  a = v.indexOf(",",a);  //первое вхождение запятой
  if(a!=-1){
    b = v.indexOf(",",a+1);  //второе вхождение запятой после нашей первой запятой если a не -1
    data[i] = v.substring(a+1,b);
    Serial.println(String("Poluchennaya data: " + String(i)+" - "+ data[i]));
    a = b;
  }
 }
  d.datetime = data[1];
  d.latitude = data[2];
  d.longitude = data[3];
  return d;
}

void gprs_init(void) {  //Процедура начальной инициализации GSM модуля для GPRS
  int d = 500;
  Serial.println("---------------GPRS init start----------------");
  delay(d * 4);
  GSMport.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");  //Установка настроек подключения
  delay(d * 5);
  Serial.println(ReadGSM());  //показываем ответ от GSM модуля
  delay(d * 5);
  GSMport.println("AT+SAPBR=3,1,\"APN\",\"internet.ltmsk\""); //Установка APN
  delay(d);
  Serial.println(ReadGSM());  //показываем ответ от GSM модуля
  delay(d * 2);
  GSMport.println("AT+SAPBR=3,1,\"USER\",\"\""); //Установка имени пользователя для сети
  delay(d);
  Serial.println(ReadGSM());  //показываем ответ от GSM модуля

  GSMport.println("AT+SAPBR=3,1,\"PWD\",\"\""); //Установка пароля для сети
  delay(d);
  Serial.println(ReadGSM());  //показываем ответ от GSM модуля
  delay(d * 2);
  GSMport.println("AT+SAPBR=1,1");  //Устанавливаем соединение GPRS
  delay(d * 2);
  Serial.println(ReadGSM());  //показываем ответ от GSM модуля
  delay(d * 2);
  GSMport.println("AT+SAPBR=2,1"); //Проверяем как настроилось
  delay(d);
  Serial.println(ReadGSM());  //показываем ответ от GSM модуля
  delay(d * 5);

  Serial.println("----------------GPRS init complete-------------");
  Serial.println("");
}

void gprs_deinit(){
  
}

String ReadGSM() {  //функция чтения данных от GSM модуля
  char c;
  String str;
  while (GSMport.available()) {  //сохраняем входную строку в переменную str
    c = GSMport.read();
    str += c;
    delay(20);
  }
  str = "<<< " + str;
  return str;
}


void gprs_send(String dataa, String datab, int dataA0, int dataA1) {  //Процедура отправки данных на сервер
  //отправка данных на сайт
  String tempstr;
  Serial.println("Send start");
  GSMport.println("AT+HTTPINIT");
  delay(1000);
  Serial.println(ReadGSM());
  GSMport.println("AT+HTTPPARA=\"CID\",1");
  delay(1000);
  Serial.println(ReadGSM());
  Serial.println("setup url");
  tempstr = String("AT+HTTPPARA=\"URL\",\"alielectronics.ru/gps/write.php?a=" + dataa + "&b=" + datab + "&a0=" + String(dataA0) +"&a1=" + String(dataA1) +"\"");
  GSMport.println(tempstr);
  Serial.println(tempstr);
  delay(4000);
  Serial.println(ReadGSM());
  Serial.println("GET url");
  GSMport.println("AT+HTTPACTION=0");
  delay(5000);
  Serial.println(ReadGSM());
  Serial.println("Send done");
  GSMport.println("AT+HTTPTERM");
  delay(200);
  Serial.println(ReadGSM());
}
