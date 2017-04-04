// 27/10/2015
/*
AT+SAPBR=3,1,"CONTYPE","GPRS"
AT+SAPBR=3,1,"APN","internet.mts.ru"
AT+SAPBR=3,1,"USER","mts"
AT+SAPBR=3,1,"PWD",""
AT+SAPBR=1,1
AT+HTTPINIT
AT+HTTPPARA="CID",1
AT+HTTPPARA="URL","https://api.telegram.org/bot165672905:AAFhk3XgHITZGDA_M2XEoxAhFaOdxl1Wf6Q/sendMessage?chat_id=161933663&text=System started over SIM800L and GPRS"
AT+HTTPSSL=1
AT+HTTPACTION=0

ERORS:
600 Not HTTP PDU
601 Network Error
602 No memory
603 DNS Error
604 Stack Busy


LOG: RETURNED VALUE

AT+SAPBR=3,1,"CONTYPE","GPRS"
OK
AT+SAPBR=3,1,"APN","internet.mts.ru"
OK
AT+SAPBR=3,1,"USER","mts"
OK
AT+SAPBR=3,1,"PWD",""
OK 
AT+SAPBR=1,1
OK
AT+HTTPINIT
OK
AT+HTTPPARA="CID",1
OK
AT+HTTPPARA="URL","https://api.telegram.org/bot165672905:AAFhk3XgHITZGDA_M2XEoxAhFaOdxl1Wf6Q/sendMessage?chat_id=161933663&text=System started over SIM800L and GPRS"
OK
AT+HTTPSSL=1
OK
AT+HTTPACTION=0
OK

+HTTPACTION: 0,200,280

*/

void GSM_ON()   
{ 
//  digitalWrite(Reset_GSM_PIN, LOW);
//  delay(300);
//  digitalWrite(Reset_GSM_PIN, HIGH);
//  delay(700);
//  digitalWrite(Reset_GSM_PIN, LOW);

 gprsSerial.print(F("AT+CLTS=1\r")); // Разрешаем GSM модулю устанавливать локальное время сотового оператора   
 delay(50); // задержка на обработку команды
 toSerial();
 gprsSerial.print(F("AT+CMGF=1\r"));
 delay(50);
 toSerial();
 gprsSerial.print(F("AT+IFC=1, 1\r"));
 delay(150); // задержка на обработку команды
 toSerial();
 gprsSerial.print(F("AT+CNMI=1,2,2,1,0\r")); 
 delay(300);
 toSerial();
  
  
}  

void gprs_init() {  //Процедура начальной инициализации GSM модуля
  int d = 200;
  int ATsCount = 11;
  String ATs[] = {  //массив АТ команд
    "AT+CLTS=1",
    "AT+CMGF=1",
    "AT+IFC=1, 1",
    "AT+CNMI=1,2,2,1,0",
    "AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"",  //Установка настроек подключения
    "AT+SAPBR=3,1,\"APN\",\"internet.mts.ru\"",
    "AT+SAPBR=3,1,\"USER\",\"mts\"",
    "AT+SAPBR=3,1,\"PWD\",\"\"",
    "AT+SAPBR=1,1",  //Устанавливаем GPRS соединение
    "AT+HTTPINIT",  //Инициализация http сервиса
    "AT+HTTPPARA=\"CID\",1"  //Установка CID параметра для http сессии
  };
  int ATsDelays[] = {2, 2, 2, 2, 25, 2, 2, 2, 10, 10, 2}; //массив задержек
  Serial.println("GPRS init start");
  for (int i = 0; i < ATsCount; i++) {
    Serial.println(ATs[i]);  //посылаем в монитор порта
    gprsSerial.println(ATs[i]);  //посылаем в GSM модуль
    yield();
    delay(d * ATsDelays[i]);
    yield();
    toSerial();
//    yield();
//    delay(d);
//    SendStatus();
  }
  Serial.println("GPRS init complete");
}
void Check_GSM() // Проверка что модем отвечает, если нет то включим его.
{
  gprsSerial.print(F("AT\r"));
  //Serial.println("Check_GSM()");
  //gprsSerial.println("AT+CSQ");  //запрос качество сигнала
  delay(50);  // даем время модему ответить

  if (!gprsSerial.available())  { 
    GSM_ON();
    //gprs_init();
//    yield();
    Serial.println("GSM(FAIL) TURN GSM ON");
  }
  else {
     Serial.println("GSM(OK)");
  }
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

void SendHistory(String Str , byte Addr)
{
//  DateTime = clock.getDateTime();  
  tmp_msg="Out:"; 
  for(int val=0; val<(23); val++) 
  {
    if (val==DateTime.hour) {
      tmp_msg= String(tmp_msg)  + String(">;"); 
    }
    else
    {
      int adr=(1+val)*2-1+Addr;
      int tmp=EEPROM_int_read(adr);
      tmp_msg= String(tmp_msg) + String(tmp) +String(";"); 
    }
  } 
  Serial.println(tmp_msg);
  SendTextMessage(First_Number, Str, tmp_msg ); 
}

void SendStatus()
{
//  DateTime = clock.getDateTime(); 
  sprintf(temp_msg, "out=%dC,main=%dC,floor_1=%dC,floor_2=%dC, time: %d:%d",Out_Temp,Main_Temp,Floor_1_Temp,Floor_2_Temp,DateTime.hour, DateTime.minute);
 
  if (isAutoHeating)   {   
    SendTextMessage(Last_Tel_Number,"Auto Heat,", temp_msg);  
  }
  else 
  {     
    SendTextMessage(Last_Tel_Number,"Stand By,", temp_msg);  
  }  
}

void SendTextMessage(String sms_number, String sms_text1, String sms_text2) 
{
//  Beep(780,100); 
  if (sms_number = "") sms_number = String(First_Number);
  gprsSerial.print(F("AT+CMGS="));         
  delay(300);
  gprsSerial.print((char)34);            // передача в порт символа "
  gprsSerial.print((String)sms_number);  // передача номера телефона
  gprsSerial.print((char)34);   
  gprsSerial.print((char)13);
  delay(300);
  gprsSerial.println((String)sms_text1); // передача текста сообщения
  delay(500);
  gprsSerial.println((String)sms_text2); // передача текста сообщения
  delay(500);
  gprsSerial.print((char)26);            // передача ^Z
  gprsSerial.print((char)13);            // передача Enter
  Beep(780,100); 
}

void toSerial()
{
  while(gprsSerial.available()!=0)
  {
    Serial.write(gprsSerial.read());
  }
}


void SendBalance() // Проверка ответа модема , ловим смс с балансом от оператора
{
  int i = currStr.indexOf('UAH');
  String balance=currStr.substring(1,159);      //  получаем баланс на сим карте из смс
  SendTextMessage(Last_Tel_Number, "My balance : ", balance);
}






// --------------------------------------------------------------------------------
//  http://we.easyelectronics.ru/blog/part/369.html  
//  AT — простейшая команда, при которой GPRS Shield ничего не делает, в случае правильного подключения возвращает в ответ OK
//  ATA — команда принять входящий вызов. Сам факт вызова устанавливается регулярно приходящими с платы во время звонка сообщениями RING
//  AT+CHUP — отклонить вызов
//  AT+CLCC — позволяет узнать номер звонящего
//  AT+IPR=19200  установить скорость модема   
//  AT+CSQ   качество сигнала — уровень сигнала. Может быть в диапазоне 0..31 или 99. Чем больше тем лучше, но 99 значит его отсутствие.
//
//  AT+CREG=1 — разрешить выдачу сообщения о смене регистрации в сети. Может принимать значения 0..2, где
//       0, — запрет выдачи сообщений.
//       1, — разрешить сообщение о смене статуса регистрации.
//       2, — разрешает выдачу сообщений, плюс выдает еще немного невнятной информации
//
//  AT+CREG? — получить тип регистрации в сети. Вернется что-то вроде +CREG: 0,1 Где:
//       0, — нет сообщения о смене регистрации в сети. 
//       1, — текущее состояние.
//       0 — не зарегистрирован, поиск сети не ведется,
//       1 — зарегистрирован в своей домашней сети,
//       2 — не зарегистрирован, идет поиск сети,
//       3 — регистрация отклонена,
//       4 — модуль сам не знает что происходит
//       5 — зарегистрирован в роуминге.
//
// -------------------------------------------------------------------------------
//  Ответы Модема
//
//  Приходящая первая строка СМС 
//      +CMT: "+79646353331","","13/11/15,11:59:51+16"
//  Входящий звонок
//      +CLIP: "+79646353331",145,"",,"",0
//
//    GPRS.
//    Настройка и установка GPRS соединения:
//    AT+SAPBR=3,1,«CONTYPE»,«GPRS»
//    AT+SAPBR=3,1,«APN»,«internet.beeline.ru»
//    AT+SAPBR=3,1,«USER»,«beeline»
//    AT+SAPBR=3,1,«PWD»,«beeline»
//    AT+SAPBR=1,1 — установка GPRS связи
//    AT+SAPBR=2,1 — полученный IP адрес
//    +SAPBR: 1,1,«10.229.9.115»
//    AT+SAPBR=4,1 — текущие настройки соединения
//    AT+SAPBR=0,1 — разорвать GPRS соединение    
