// 27/10/2015

void GSM_ON()   
{ 
  digitalWrite(Power_GSM_PIN, LOW);
  delay(300);
  digitalWrite(Power_GSM_PIN, HIGH);
  delay(700);
  digitalWrite(Power_GSM_PIN, LOW);
  gprsSerial.print(F("AT+CLTS=1\r")); // Разрешаем GSM модулю устанавливать локальное время сотового оператора
  delay(50);  // даем время модему ответить
  gprsSerial.print(F("AT+CMGF=1\r"));
  delay(50);  // даем время модему ответить
  gprsSerial.print(F("AT+IFC=1, 1\r"));
  delay(50);  // даем время модему ответить
  gprsSerial.print(F("AT+CNMI=1,2,2,1,0\r")); 
  delay(50);  // даем время модему ответить
  Beep(780,100); 
  
}  


void SendHistory(String Str , byte Addr)
{
  clock.getTime();
  tmp_msg="Out:"; 
  for(int val=0; val<(23); val++) 
  {
    if (val==clock.hour) {
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
  //SendTextMessage(First_Number, Str, tmp_msg ); 
}


void Check_GSM() // Проверка что модем отвечает, если нет то включим его.
{
  //gprsSerial.print(F("AT\r"));
  Serial.println("Check_GSM()");
  gprsSerial.println("AT+CSQ");  //запрос качество сигнала
  delay(200);  // даем время модему ответить
  if (!gprsSerial.available())  { 
    GSM_ON();
    Serial.println("Check_GSM(FAIL) TURN GSM ON");
  }
  else {
     Serial.println("Check_GSM(OK)");
  }
}



void SendStatus()
{
  clock.getTime();
  sprintf(temp_msg, "out=%dC,main=%dC,floor_1=%dC,floor_2=%dC, time: %d:%d",Out_Temp,Main_Temp,Floor_1_Temp,Floor_2_Temp,clock.hour, clock.minute);
 
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
  Beep(780,100); 
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




/* 
void SendBalance() // Проверка ответа модема , ловим смс с балансом от оператора
{
  int i = currStr.indexOf('UAH');
  String balance=currStr.substring(10,i-3);      //  получаем баланс на сим карте из смс
  SendTextMessage(Last_Tel_Number, "My balance : ", balance);
}
*/





// --------------------------------------------------------------------------------
//
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
