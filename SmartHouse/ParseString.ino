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
        Next_Update_Screen_Saver =  millis() + 15000; // и апдейтим время включения скринсейвера
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
        //  SendBalance();
      }
      
      currStr = "";                           // строка из COM  порта разобрана,-  обнуляем её
}
