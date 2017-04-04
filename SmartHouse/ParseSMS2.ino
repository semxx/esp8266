// 27/10/2015
// Команды СМС к дому
// STS            Текущий статус системы
// HST            Запросить историю температур за сутки
// RST            Перезагрузать Ардуино
// AUTO           Включить Автоподдержание t
// AUTO OFF       Выключить Автоподдержание t
// START          Включить все реле + Включить Автоподдержание t
// STOP           Выключить все реле + Выключить Автоподдержание t
// R1 ON          Включить реле  1,2,3,4,5,6
// R1 OFF         Выключить реле 1,2,3,4,5,6
// Help           Подсказка по командам "STS,HST,RST,AUTO,AUTO OFF,START,STOP,R1 ON,R1 OFF,SET TEMP ХХ"
// SET TEMP ХХ    Установить температуру автоподдержания где ХХ температура от 01 до Alarm_Temp 
// SET TIME HH:MM Установить часы и минуты 
// BAL            Запросить Баланс
// GSM            Запросить качество сигнала 

void(* resetFunc) (void) = 0;               // declare reset function at address 0

void Parse_Income_SMS()
{
  Beep(780,100); 

   Serial.println(currStr); 
   
  if ((currStr.startsWith("STS")) or (currStr.startsWith("Sts")))
  {
    SendStatus();
  } 
  else if ((currStr.startsWith(F("HELP"))) or (currStr.startsWith(F("Help"))))
  {    
    SendTextMessage(Last_Tel_Number, F("COMMAND:"),F("STS,HST,RST,AUTO,AUTO OFF,START,STOP,R1 ON,R1 OFF,SET TIME HH:MM,SET TEMP ХХ"));
  }  
  else if ((currStr.startsWith("HST")) or (currStr.startsWith("Hst")))
  {    
    //SendHistory(F("Main"), Addr_Temp_2);
    SendHistory(F("1Flr"), Addr_Temp_3);
    delay(5000);
    SendHistory(F("2Flr"), Addr_Temp_4);
    delay(5000);
    SendHistory(F("OutD"), Addr_Temp_1);
  }
  else if ((currStr.startsWith("RST")) or (currStr.startsWith("Rst")))
  {    
    SendTextMessage(Last_Tel_Number, F("I`m Restart"),"");
    delay(1000);
    resetFunc();      // Вызываем функцию перезагрузки
  } 
  else if ((currStr.startsWith("SET TEMP")) or (currStr.startsWith("Set temp")))
  {
    int val = currStr.substring(9,11).toInt();        // Получим температуру из строки пришедшего СМС
    if (val<Alarm_Temp && val>1)                      // Устанавливаемая температура должна быть меньше Аларма и выше 1 градуса 
    {
      EEPROM.write(Addr_Auto_Temp, val);               // Пишем температуру в EPROM
      Auto_Temp=val;                                   // И устанавливаем ее по умолчанию
      sprintf(temp_msg, " = %dC",  Auto_Temp);  
      SendTextMessage(Last_Tel_Number,"Set new temperature", temp_msg);     
    }
  }

 else if ((currStr.startsWith("SET TIME")) or (currStr.startsWith("Set time")))
  {
    int hh = currStr.substring(9,11).toInt();        // Получим час из строки пришедшего СМС
    int mm = currStr.substring(12,15).toInt();        // Получим час из строки пришедшего СМС
    clock.setDateTime(2017, 3, 13, Hours, Minutes, 0);
    
    DateTime = clock.getDateTime();  
    sprintf(temp_msg, "%d:%d",DateTime.hour, DateTime.minute);
    SendTextMessage(Last_Tel_Number,"Set new time:", temp_msg);        
  }



  
  else if ((currStr.startsWith("AUTO")) or (currStr.startsWith("Auto")))
  {
    isAutoHeating = true;
    digitalWrite(Relay_1, HIGH);
    digitalWrite(Relay_2, HIGH);
    isRelay01 = true;
    isRelay02 = true;
    UpdateTemp();
    SendStatus();
  } 
  else  if ((currStr.startsWith("AUTO OFF")) or (currStr.startsWith("Auto off")))
  {
    isAutoHeating = false;
    digitalWrite(Relay_1, LOW);
    digitalWrite(Relay_2, LOW);
    isRelay01 = false;
    isRelay02 = false;
    UpdateTemp();
    SendStatus();
  } 
  else  if ((currStr.startsWith("START")) or (currStr.startsWith("Start")))
  {
    isAutoHeating = true;
  //  digitalWrite(Relay_1, HIGH); 
    isRelay01 = true;
    delay(500);
 //   digitalWrite(Relay_2, HIGH); 
    isRelay02 = true;
    delay(500);
  //  digitalWrite(Relay_3, HIGH);
    isRelay03 = true; 
    delay(500);
  //  digitalWrite(Relay_4, HIGH);
    isRelay04 = true; 
    delay(500);
  //  digitalWrite(Relay_5, HIGH);
    isRelay05 = true; 
    delay(500);
   // digitalWrite(Relay_6, HIGH);
    isRelay06 = true; 
    delay(500);
    SendTextMessage(Last_Tel_Number,"All Relay" ," ON");
  } 
  else  if ((currStr.startsWith("STOP")) or (currStr.startsWith("Stop")))
  {
    isAutoHeating = false;
//    digitalWrite(Relay_1, LOW); 
    isRelay01 = false;
    delay(500);
//    digitalWrite(Relay_2, LOW); 
    isRelay02 = false;
    delay(500);
//    digitalWrite(Relay_3, LOW);
    isRelay03 = false; 
    delay(500);
//    digitalWrite(Relay_4, LOW);
    isRelay04 = false; 
    delay(500);
//   digitalWrite(Relay_5, LOW);
    isRelay05 = false; 
    delay(500);
 //   digitalWrite(Relay_6, LOW);
    isRelay06 = false; 
    delay(500);
    SendTextMessage(Last_Tel_Number,"All Relay OFF","");
  } 
  else if (currStr.startsWith("R1 ON")) 
  {
 //   digitalWrite(Relay_1, HIGH);
    isRelay01 = true;  
    SendTextMessage(Last_Tel_Number,"Relay 01 ON" ,"");
  } 
  else  if (currStr.startsWith("R1 OFF")) 
  {
//    digitalWrite(Relay_1, LOW);
    isRelay01 = false; 
    SendTextMessage(Last_Tel_Number,"Relay 01" ," OFF");
  } 
  else if (currStr.startsWith("R2 ON")) 
  {
//    digitalWrite(Relay_2, HIGH); 
    isRelay02 = true; 
    SendTextMessage(Last_Tel_Number,"Relay 02" ," ON");
  } 
  else  if (currStr.startsWith("R2 OFF")) 
  {
//    digitalWrite(Relay_2, LOW);
    isRelay02 = false; 
    SendTextMessage(Last_Tel_Number,"Relay 02" ," OFF");
  } 
  else if (currStr.startsWith("R3 ON")) 
  {
 //   digitalWrite(Relay_3, HIGH); 
    isRelay03 = true; 
    SendTextMessage(Last_Tel_Number,"Relay 03" ," ON");
  } 
  else  if (currStr.startsWith("R3 OFF")) 
  {
 //   digitalWrite(Relay_3, LOW);
    isRelay03 = false; 
    SendTextMessage(Last_Tel_Number,"Relay 03" ," OFF");
  } 
  else if (currStr.startsWith("R4 ON")) 
  {
//    digitalWrite(Relay_4, HIGH);
    isRelay04 = true;  
    SendTextMessage(Last_Tel_Number,"Relay 04" ," ON");
  } 
  else  if (currStr.startsWith("R4 OFF")) 
  {
//    digitalWrite(Relay_4, LOW);
    isRelay04 = false; 
    SendTextMessage(Last_Tel_Number,"Relay 04" ," OFF");
  } 
  else if (currStr.startsWith("R5 ON")) 
  {
//    digitalWrite(Relay_5, HIGH);
    isRelay05 = true;  
    SendTextMessage(Last_Tel_Number,"Relay 05" ," ON");
  } 
  else  if (currStr.startsWith("R5 OFF")) 
  {
//    digitalWrite(Relay_5, LOW);
    isRelay05 = false; 
    SendTextMessage(Last_Tel_Number,"Relay 05" ," OFF");
  } 
  else if (currStr.startsWith("R6 ON")) 
  {
 //   digitalWrite(Relay_6, HIGH);
    isRelay06 = true;  
    SendTextMessage(Last_Tel_Number,"Relay 06" ," ON");
  } 
  else  if (currStr.startsWith("R6 OFF")) 
  {
//    digitalWrite(Relay_6, LOW);
    isRelay06 = false; 
    SendTextMessage(Last_Tel_Number,"Relay 06" ," OFF");
  } 
    else if (currStr.startsWith("BAL")) 
     {
     // gprsSerial.println("ATD*100#");  //запрос баланса  с помощью USSD
SendTextMessage("000100","B", "");
     } 
  //  else if (currStr.startsWith("GSM")) 
  //   {
  //    gprsSerial.println("AT+CSQ");  //запрос качество сигнала
  //   } 
  else  // Пришла не Валидная СМС-ка  
  { 
    SendTextMessage(First_Number, "Err.SMS:",currStr);   //шлем ее только на первый установленный номер 
  }

  currStr = "";
  isStringMessage = false;
  Last_Tel_Number = "";
  sprintf(temp_msg,"");  
} 



// 27/10/2015
void Parse_Income_String()
{
     if (currStr.startsWith("+CMT"))     // если текущая строка начинается с "+CMT",то следующая строка является сообщением
      {
        isStringMessage = true;          //  признак того, что это СМС (для дальнейшего парсинга смс)
        Last_Tel_Number=currStr.substring(7,7+12);  // запомним от кого пришла СМС иначе придет вторая строка и нечего будет запоминать. 
      }
      else if (isStringMessage)         // если текущая строка - SMS-сообщение
      { 
        Parse_Income_SMS();                           //  Разбираем СМС сообщение
        num_Screen=1;                                 // будим экран 
        Next_Update_Screen_Saver =  millis() + 15000; // и апдейтим время включения скринсейвера

      }
      else if (currStr.startsWith("+CLIP"))          // если текущая строка начинается с RING это звонок   
      {
        isCalling = true;
        Last_Tel_Number=currStr.substring(8,8+12); 
        Ring();
        num_Screen=1;                                 // будим экран 
        Next_Update_Screen_Saver =  millis() + 45000; // и апдейтим время включения скринсейвера
        EnergySaveMode =  millis() + 45000;
      }
      else if (currStr.startsWith("NO CARRIER"))      // если текущая строка начинается NO CARRIER заначит бросили трубку 
      {
        SendTextMessage(First_Number, "Missed Call:", Last_Tel_Number); 
        isCalling = false;
      }
      else if (currStr.startsWith("Call Ready"))  // шилд зарегистрировался в сети
      {     
        SendStatus();   
      }
      else if (currStr.startsWith("+CSQ"))      // если текущая строка начинается с +CSQ это Качество сигнала
      {
        tmp_msg=currStr.substring(6,6+2);  
        sgsm=tmp_msg.toInt();                    // — уровень сигнала. Может быть в диапазоне 0..31 или 99. Чем больше тем лучше, но 99 значит его отсутствие.       
      }
      else if (currStr.startsWith("+CUSD: 0,"))  // если текущая строка начинается с +CUSD: это баланс от оператора
      {
          SendBalance();
      }
      
      currStr = "";                           // строка из COM  порта разобрана,-  обнуляем её
}
