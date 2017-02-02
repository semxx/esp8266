// http://api.telegram.org/bot165672905:AAFhk3XgHITZGDA_M2XEoxAhFaOdxl1Wf6Q/sendMessage?chat_id=161933663&text=123
#include <SoftwareSerial.h>
#include <SimpleTimer.h>         // Essential for all Blynk Projects
SoftwareSerial gprsSerial(5, 4); // RX, TX
int char_;
int SensorPin = D0;
int SensorState;
int SensorLastState = HIGH;
SimpleTimer timer;

void gprs_send(String data) {  //Процедура отправки данных на сервер
  //отправка данных на сайт
  int d = 400;
  Serial.println("Send start");
  Serial.println("setup url");
//gprsSerial.println("AT+HTTPPARA=\"URL\",\"https://api.telegram.org/bot165672905:AAFhk3XgHITZGDA_M2XEoxAhFaOdxl1Wf6Q/sendMessage?chat_id=161933663&text=Proverka\"");
  gprsSerial.println("AT+HTTPPARA=\"URL\",\"https://api.telegram.org/bot165672905:AAFhk3XgHITZGDA_M2XEoxAhFaOdxl1Wf6Q/sendMessage?chat_id=161933663&text=" + data + "\"");
  delay(d * 2);
  gprsSerial.println("AT+HTTPSSL=1");
  delay(d * 2);
  Serial.println(ReadGSM());
  delay(d);  
  Serial.println(ReadGSM());
  delay(d);
  Serial.println("GET url");
  gprsSerial.println("AT+HTTPACTION=0");
  delay(d * 2);
  Serial.println(ReadGSM());
  delay(d);
  Serial.println("Send done");
}

void setup() {
  delay(3000); //дадим время на инициализацию GSM модулю
  pinMode(SensorPin, INPUT);
  digitalWrite(SensorPin, HIGH);  //вкл. подтягивающий резистор 20ом
  Serial.begin(9600);  //скорость порта
  Serial.println("GPRS test");
  gprsSerial.begin(9600);
  gprs_init();
//  timer.setInterval(20000L, gprs_send);
}

void loop() {
  SensorState = digitalRead(SensorPin);
  if (SensorState != SensorLastState) {  //если смена состояния датчика
    Serial.print("sensor changed to: ");
    Serial.println(SensorState);
    SensorLastState = SensorState;
    gprs_send(String(SensorState));
    delay(100);
  }
  if (gprsSerial.available()) {  //если GSM модуль что-то послал нам, то
    Serial.println(ReadGSM());  //печатаем в монитор порта пришедшую строку
  }
  delay(100);
  timer.run();
}

void gprs_init() {  //Процедура начальной инициализации GSM модуля
  int d = 500;
  int ATsCount = 7;
  String ATs[] = {  //массив АТ команд
    "AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"",  //Установка настроек подключения
    "AT+SAPBR=3,1,\"APN\",\"internet.mts.ru\"",
    "AT+SAPBR=3,1,\"USER\",\"mts\"",
    "AT+SAPBR=3,1,\"PWD\",\"\"",
    "AT+SAPBR=1,1",  //Устанавливаем GPRS соединение
    "AT+HTTPINIT",  //Инициализация http сервиса
    "AT+HTTPPARA=\"CID\",1"  //Установка CID параметра для http сессии
  };
  int ATsDelays[] = {6, 1, 1, 1, 3, 3, 1}; //массив задержек
  Serial.println("GPRG init start");
  for (int i = 0; i < ATsCount; i++) {
    Serial.println(ATs[i]);  //посылаем в монитор порта
    gprsSerial.println(ATs[i]);  //посылаем в GSM модуль
    delay(d * ATsDelays[i]);
    Serial.println(ReadGSM());  //показываем ответ от GSM модуля
    delay(d);
  }
  Serial.println("GPRS init complete");
}



String ReadGSM() {  //функция чтения данных от GSM модуля
  int c;
  String v;
  while (gprsSerial.available()) {  //сохраняем входную строку в переменную v
    c = gprsSerial.read();
    v += char(c);
    delay(10);
  }
  return v;
}
