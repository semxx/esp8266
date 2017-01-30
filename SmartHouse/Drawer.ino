void UpdateDisplay()
{
  display.clearDisplay();
  switch (num_Screen)
  {
    case 0:
      DrawScreenSaver();
      break;
    case 1:
      DrawGSM(WIFI_getRSSIasQuality(WiFi.RSSI()));               //DrawGSM(sgsm);
      DrawTime();
      DrawBattery(batt);
      DrawDash();
      DrawHome();
      break;
    case 2:
      DrawBoiler();
      break;
    case 3:
      DrawSetting();
      break;
    case 4:
      DrawSensor();
      break;
    case 5:
      DrawHistPipe();
      break;
    case 6:
      DrawHistLine();
      break;
    case 7:
      PrintYesNo = false;
      PrintRTC(0,20);
     // timer.run();
      break;
    case 8:
      MenuTimeoutTimer = 10; //таймер таймаута, секунд
      //display.clearDisplay(); 
      display.setCursor(0, 40);
      display.print("SET TIME"); //    lcd.print(F("SETUP CLOCK")); 
      delay(200);
      clock.getTime();
      Hours=clock.hour;
      Minutes=clock.minute;
      Seconds=clock.second;
      SetYesNo = false;
      PrintYesNo = true;
      SetTime(0,0); // в позиции 0,1 - запрос ввода времени
      if (MenuTimeoutTimer != 0) {
        if (SetYesNo)
        {
    //      if (BeepEnabled) {
    //        tone(BeepPin,BeepToneYes,BeepToneYesDuration); //звук "YES"
        //}
          clock.fillByHMS(Hours, Minutes, Seconds);    
          clock.setTime();
        }
        else
        {
         // if (BeepEnabled) {
         //   tone(BeepPin,BeepToneNo,BeepToneNoDuration); //звук "NO"
         //}
        }
      }
      else {
       // if (BeepEnabled) {
       //   tone(BeepPin,BeepToneNo,BeepToneNoDuration); //звук "NO"
        //}
      }
      break;
    case 9:
      DrawAbout();
      break;
    case 10:

      break;
    default:
        EnergySaver();
   //   DrawScreenSaver();
  }
  NextBattery();
  display.display();
}

void EnergySaver()
{
  display.clearDisplay(); 
}

void MyPrint(String Str, byte Col, byte Row, byte Size, byte Color)
{
  if (Color == 1) {
    display.setTextColor(WHITE);
  }
  else {
    display.setTextColor(BLACK, WHITE);
  }
  display.setTextSize(Size);
  display.setCursor(Col, Row);
  display.print(Str);
}

void DrawSetting()
{

if (oldEncoderValue != encoderValue) {
      MenuItem = constrain(MenuItem, 0, 4);
      inMenu = true;
      if (encoderValue > oldEncoderValue) {
//        MenuTimeoutTimer = 10; //таймер таймаута, секунд
//        if (encoderR) {
          MenuItem += 1;
        }
        else  {
          MenuItem -= 1;
        }
        if ( MenuItem > 3 ) { // границы пунктов меню
          MenuItem = 1;
        } 
        if ( MenuItem < 1 ) {
          MenuItem = 3;
        }
        //MenuItem = constrain(MenuItem, 0, 8);

oldEncoderValue = encoderValue;
}

if (inMenu) {

  switch (MenuItem)
  {
    case 1:
      display.drawLine(0, 15, 128, 15, WHITE);
      MyPrint(F("Electric"), 3 * 6 - 6, 1 * 8 - 8, 2, 1);
      isAutoHeating = true;
      break;
    case 2:
      display.drawLine(0, 15, 128, 15, WHITE);
      MyPrint(F("Wood mode"), 3 * 6 - 6, 1 * 8 - 8, 2, 1);
      isAutoHeating = false;    
      break;
    case 3:
      MyPrint(F("Saved.."), 1 * 6 - 6, 3 * 8 - 8, 3, 1);    
      //delay (1000);
      inMenu = false;
      break;
      default: break;
  }  
}

else {
      MyPrint(F("SETTINGS"), 2 * 6 - 6, 1 * 8 - 8, 2, 1);
      sprintf(temp_msg, "Alarm Temp: %02dC", Alarm_Temp);
      MyPrint(temp_msg, 2 * 6 - 6, 3 * 8 - 8, 1, 1);
      sprintf(temp_msg, "Auto  Temp: %02dC", Auto_Temp);
      MyPrint(temp_msg, 2 * 6 - 6, 4 * 8 - 8, 1, 1);
      if (isAutoHeating)  {
        MyPrint(F("Heat: ON"), 2 * 6 - 6, 5 * 8 - 8, 1, 1);
      }
      else  {
        MyPrint(F("Heat: OFF"), 2 * 6 - 6, 5 * 8 - 8, 1, 1);
      }
      MyPrint("Numb: " + String(First_Number), 2 * 6 - 6, 7 * 8 - 8, 1, 1);
    }
}

void DrawBoiler(){
//      DrawTime();
 int x = 0;
 display.drawRect(66, 23, 29, 37, WHITE); // бойлер
 display.drawRect(65, 22, 31, 39, WHITE); // бойлер
 display.drawRect(72, 20, 17, 2, WHITE); // крыша
 display.fillRect(106, 23, 13, 9, WHITE); // выход прямоугольник 
 display.fillRect(106, 51, 13, 9, WHITE); // обратка прямоугольник 
 display.drawRect(104, 21, 17, 13, WHITE); // выход рамка 
 display.drawRect(104, 49, 17, 13, WHITE); // обратка рамка 
 display.drawLine(110, 33, 110, 49, WHITE); // перемычка L обратка/выход
 display.drawLine(114, 33, 114, 49, WHITE); // перемычка R обратка/выход
 display.drawLine(71, 27, 104, 27, WHITE); // линия выход
 display.drawLine(71, 55, 104, 55, WHITE); // линия вход
 
 for (int val = 0; val < 4; val++)  {
  display.drawLine(71, 27 + x, 71, 31 + x, WHITE); // Змеевик L
  x = x + 8;
 }
  x = 0;
 for (int val = 0; val < 6; val++)  {
  display.drawLine(71, 31 + x, 89, 31 + x, WHITE); // Змеевик H
  x = x + 4;
 }
 
  x = 0;
 for (int val = 0; val < 3; val++)  {
  display.drawLine(89, 31 + x, 89, 35 + x, WHITE); // Змеевик R
  x = x + 8;
 }
  display.setTextColor(BLACK);
  display.setCursor(107, 24);
  display.print(Floor_1_Temp); // температура подачи
  display.setCursor(107, 52);
  display.print(Floor_2_Temp); // температура обратки
  display.setTextColor(WHITE);
  x = 0;
 for (int val = 0; val < 10; val++)  {
 display.fillRect(123, 58 - x, 6, 5, WHITE); // температурная шкала
  x = x + 8;
 }
  display.drawTriangle(73, 64, 76, 61, 79, 64, WHITE); // пламя 1
  display.drawTriangle(78, 64, 81, 61, 84, 64, WHITE); // пламя 2
  display.drawTriangle(82, 64, 86, 61, 89, 64, WHITE); // пламя 3

  display.fillTriangle(100, 30, 100, 24, 103, 27, WHITE); // стрелка выход
  display.fillTriangle(96, 55, 99, 52, 99, 58, WHITE); // стрелка вход в котел


  display.setTextSize(2);
  display.setCursor(1, 1);
  display.print(Floor_1_Temp);        // температура теплоностителя в левом верхнем углу
  display.drawCircle(28,3,3, WHITE);  // значек градуса


  display.setTextColor(WHITE);
  display.setCursor(34,0);
  display.println("c");
  display.setTextSize(1);
  
 display.drawRect(48, 1, 38, 13, WHITE); // Рамка KW/h
 display.fillRect(50, 3, 34, 9, WHITE); // Фон KW/h 
 display.setTextColor(BLACK);
 display.setCursor(52,4);
 display.println("4KW/h");
 
 display.drawRect(89, 1, 31, 13, WHITE); // Рамка PUMP  
 display.fillRect(91, 3, 27, 9, WHITE); // Фон PUMP 
 display.setCursor(93,4);
 display.println("PUMP");
  

 display.drawRect(1, 49, 50, 13, WHITE); // Рамка KW/h
 display.fillRect(3, 51, 43, 9, WHITE); // Фон KW/h 
 display.setCursor(5,52);
 display.println(encoderValue);

 display.drawRect(27, 33, 17, 13, WHITE); // Рамка 80
 display.fillRect(29, 35, 13, 9, WHITE); // Фон 80 
 display.setCursor(30,36);
 display.println(Alarm_Temp);
 display.setTextColor(WHITE); 

 display.drawRect(1, 33, 23, 13, WHITE); // Рамка PID
 display.setCursor(4,36);
 display.println("PID");
 display.drawRect(1, 17, 60, 13, WHITE); // Рамка POWERFULL
 display.setCursor(4,20);
 display.println("POWERFULL");
// display.setCursor(3,16);
// display.println("MODE: WOOD");
 display.display();




}
  
void DrawScreenSaver()
{
  clock.getTime();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  sprintf(temp_msg, "%02d:%02d" , clock.hour, clock.minute);
  display.setCursor(clock.second, clock.minute / 2 + 16);
  display.print(temp_msg);

  display.setTextSize(1);
  sprintf(temp_msg, "%02d" , clock.second);
  display.setCursor(clock.second + 23, clock.minute / 2 + 12);
  display.print(temp_msg);
}



void DrawSensor()
{
  MyPrint(F("DS18B20"), 4 * 6 - 6, 1 * 8 - 8, 2, 1);
  display.fillRect(0, 16, 127, 9, WHITE);
  MyPrint("     TEMP IN HOME", 1 * 6 - 6 + 2, 3 * 8 - 8 + 1, 1, 0);
  display.drawLine(65, 24, 65, 96, WHITE);

  sprintf(temp_msg, "OutDoor:    %02dC", Out_Temp);
  MyPrint(temp_msg, 2 * 6 - 6, 5 * 8 - 8, 1, 1);
  sprintf(temp_msg, "1-Floor:    %02dC", Floor_1_Temp);
  MyPrint(temp_msg, 2 * 6 - 6, 6 * 8 - 8, 1, 1);
  sprintf(temp_msg, "2-Floor:    %02dC", Floor_2_Temp);
  MyPrint(temp_msg, 2 * 6 - 6, 7 * 8 - 8, 1, 1);
  sprintf(temp_msg, "Mainborad:  %02dC", Main_Temp);
  MyPrint(temp_msg, 2 * 6 - 6, 8 * 8 - 8, 1, 1);
}



void DrawHistPipe()
{
String Str;
byte Addr;
int act_temp;  

if (oldEncoderValue != encoderValue) {
      MenuItem = constrain(MenuItem, 0, 5);
      if (encoderValue > oldEncoderValue) {
          MenuItem += 1;
        }
        else  {
          MenuItem -= 1;
        }
        if ( MenuItem > 4 ) { // границы пунктов меню
          MenuItem = 1;
        } 
        if ( MenuItem < 1 ) {
          MenuItem = 4;
        }
oldEncoderValue = encoderValue;
}
  switch (MenuItem)
  {
    case 1:
            Str = Out_Text;
            Addr = Addr_Temp_1;
            act_temp = Out_Temp;
            break;
    case 2:
            Str = Floor_1_Text;
            Addr = Addr_Temp_3;
            act_temp = Floor_1_Temp;
            break;
    case 3:
            Str = Floor_2_Text;
            Addr = Addr_Temp_4;
            act_temp = Floor_2_Temp;
            break;
    case 4:
            Str = Main_Text;
            Addr = Addr_Temp_2;
            act_temp = Main_Temp;
            break;
      default: MenuItem = 1; 
            break;

  } 
  
  MyPrint(Str, 1 * 6 - 6 + 3, 1 * 8 - 8 + 4, 1, 1);
  sprintf(temp_msg, "%02dC", act_temp);
  MyPrint(temp_msg, 7 * 6 - 6 + 3, 1 * 8 - 8 + 4, 1, 1);
  display.drawRect(0, 0, 66, 16, WHITE);

  int tmpmax = EEPROM_int_read(Addr + 1);
  int tmpmin = tmpmax;

  for (int val = 0; val < 23; val++)  {                    // определяем границы измерений нижний и верхний диапазоны
    int tmp = EEPROM_int_read((1 + val) * 2 - 1 + Addr);   // вычисляем адрес, в зависимости от часа каждый второй байт
    if ( tmp > tmpmax ) {
      tmpmax = tmp;
    }
    if ( tmp < tmpmin ) {
      tmpmin = tmp;
    }
  }

  sprintf(temp_msg, "MAX %02dC", tmpmax);
  MyPrint(temp_msg, 14 * 6 - 6, 1 * 8 - 8, 1, 1);

  sprintf(temp_msg, "MIN %02dC", tmpmin);
  MyPrint(temp_msg, 14 * 6 - 6, 2 * 8 - 8, 1, 1);

  display.drawRect(0, 16, 127, 37, WHITE);
  display.drawRect(0, 52, 127, 12, WHITE);

  for (int x = 0 ; x < 23; x++) {
    int y = map(EEPROM_int_read((1 + x) * 2 - 1 + Addr), tmpmin - 2, tmpmax, 1, 30);
    if (x > clock.hour) {
       display.drawRect(7 + x * 5, 50 - y, 4, y, WHITE);
    }
    else {
       display.fillRect(7 + x * 5, 50 - y, 4, y, WHITE);
    }
  }
  int x = 8 + (clock.hour) * 5;
  display.drawLine(x, 16, x, 53, WHITE);

  sprintf(temp_msg, "%02d" , clock.hour);
  display.drawLine(x, 16, x, 63, WHITE);
}


void DrawHistLine()
{
String Str;
byte Addr;
int act_temp;  

if (oldEncoderValue != encoderValue) {
      MenuItem = constrain(MenuItem, 0, 5);
      if (encoderValue > oldEncoderValue) {
          MenuItem += 1;
        }
        else  {
          MenuItem -= 1;
        }
        if ( MenuItem > 4 ) { // границы пунктов меню
          MenuItem = 1;
        } 
        if ( MenuItem < 1 ) {
          MenuItem = 4;
        }
oldEncoderValue = encoderValue;
}
  switch (MenuItem)
  {
    case 1:
            Str = Out_Text;
            Addr = Addr_Temp_1;
            act_temp = Out_Temp;
            break;
    case 2:
            Str = Floor_1_Text;
            Addr = Addr_Temp_3;
            act_temp = Floor_1_Temp;
            break;
    case 3:
            Str = Floor_2_Text;
            Addr = Addr_Temp_4;
            act_temp = Floor_2_Temp;
            break;
    case 4:
            Str = Main_Text;
            Addr = Addr_Temp_2;
            act_temp = Main_Temp;
            break;
      default: MenuItem = 1;
            break;

  } 
  
  MyPrint(Str, 1 * 6 - 6 + 3, 1 * 8 - 8 + 4, 1, 1);
  sprintf(temp_msg, "%02dC", act_temp);
  MyPrint(temp_msg, 7 * 6 - 6 + 3, 1 * 8 - 8 + 4, 1, 1);
  display.drawRect(0, 0, 66, 16, WHITE);

  int tmpmax = EEPROM_int_read(Addr + 1);
  int tmpmin = tmpmax;

  for (int val = 0; val < 23; val++)  {                     //определяем границы измерений нижний и верхний диапазоны
    int tmp = EEPROM_int_read((1 + val) * 2 - 1 + Addr);    //вычисляем адрес, в зависимости от часа каждый второй байт
    if ( tmp > tmpmax ) {
      tmpmax = tmp;
    }
    if ( tmp < tmpmin ) {
      tmpmin = tmp;
    }
  }

  sprintf(temp_msg, "MAX %02dC", tmpmax);
  MyPrint(temp_msg, 14 * 6 - 6, 1 * 8 - 8, 1, 1);
  sprintf(temp_msg, "MIN %02dC", tmpmin);
  MyPrint(temp_msg, 14 * 6 - 6, 2 * 8 - 8, 1, 1);

  display.drawRect(0, 16, 127, 37, WHITE);
  display.drawRect(0, 52, 127, 12, WHITE);

  int old = map( EEPROM_int_read((1 + 1) * 2 - 1 + Addr), tmpmin, tmpmax, 25, 1);;

  for (int x = 0 ; x < 23; x++) {
    int y = map( EEPROM_int_read((1 + x) * 2 - 1 + Addr), tmpmin, tmpmax, 25, 1);
    display.drawLine(4 + (x - 1) * 5, old + 22, 4 + x * 5, y + 22, WHITE);
    old = y;
  }

  int x = 4 + (clock.hour ) * 5;
  display.drawLine(x, 16, x, 63, WHITE);
}



void DrawAbout()
{
  MyPrint(F("SmartHouse"), 2 * 6 - 6, 1 * 8 - 8, 2, 1);
  MyPrint(F("Controller"), 1 * 6 - 6, 4 * 8 - 8, 2, 1);
  MyPrint(F("  ver 1.1"), 1 * 6 - 6, 6 * 8 - 8, 2, 1);
}


void DrawBattery(word prc)
{
  display.drawRect(102, 4, 3, 7, WHITE);
  display.drawRect(104, 1, 23, 13, WHITE);
  switch (prc)   {
    case 4:
      display.fillRect(106, 3, 4, 9, WHITE);
    case 3:
      display.fillRect(111, 3, 4, 9, WHITE);
    case 2:
      display.fillRect(116, 3, 4, 9, WHITE);
    case 1:
      display.fillRect(121, 3, 4, 9, WHITE);
  }
}

int WIFI_getRSSIasQuality(int RSSI) {
  unsigned long start_time = millis();
  int quality = 0;

  if (RSSI <= -100) {
    quality = 0;
  } else if (RSSI >= -50) {
    quality = 100;
  } else {
    quality = 2 * (RSSI + 100);
  }

  unsigned long load_time = millis() - start_time;
  return quality;
}

void DrawGSM(word signal)
{
  display.drawLine(6, 2, 6, 13, WHITE);
  display.drawTriangle(1, 2, 11, 2, 6, 7, WHITE);
 if  (WiFi.status() == WL_CONNECTED) {     
   if (signal > 40)    display.fillRect(9, 12, 2, 2, WHITE);
   if (signal > 50)    display.fillRect(12, 10, 2, 4, WHITE);
   if (signal > 70)    display.fillRect(15, 6, 2, 8, WHITE);
   if (signal > 85)    display.fillRect(18, 2, 2, 12, WHITE);
  }
}


void DrawTime()
{
  clock.getTime();
  sprintf(temp_msg, "%02d:%02d:%02d" , clock.hour, clock.minute, clock.second);
  MyPrint(temp_msg, 7 * 6 - 6, 1 * 8 - 8, 1, 1);
  sprintf(temp_msg, "%02d/%02d/%02d" , clock.dayOfMonth, clock.month, clock.year + 2000);
  MyPrint(temp_msg, 6 * 6 - 6, 2 * 8 - 8, 1, 1);
}


void DrawHome()
{
  sprintf(temp_msg, "%02dC", Out_Temp);
  MyPrint(temp_msg, 1 * 6 - 6, 3 * 8 - 8, 1, 1);
  sprintf(temp_msg, "%02dC", Floor_2_Temp);
  MyPrint(temp_msg, 4 * 6 - 6, 5 * 8 - 8, 1, 1);
  sprintf(temp_msg, "%02dC", Floor_1_Temp);
  MyPrint(temp_msg, 4 * 6 - 6, 7 * 8 - 8, 1, 1);
  // Дом  крыша
  display.drawLine( 2, 37, 31, 18, WHITE);
  display.drawLine(31, 18, 58, 37, WHITE);
  // стены
  display.drawLine(7, 35,  7, 63, WHITE);
  display.drawLine(7, 63, 53, 63, WHITE);
  display.drawLine(53, 63, 53, 35, WHITE);
  // труба
  display.drawLine(40, 23, 50, 23, WHITE);
  display.drawLine(50, 23, 50, 29, WHITE);
}




void DrawDash()
{

  // информационные поля большие
  display.drawRect(63, 16, 32, 16, WHITE);
  display.drawRect(63, 32, 32, 16, WHITE);
  display.drawRect(95, 16, 32, 16, WHITE);
  display.drawRect(95, 32, 32, 16, WHITE);
  // информационные поля для реле маленькие
  display.drawRect(63, 48, 16, 16, WHITE);
  display.drawRect(79, 48, 16, 16, WHITE);
  display.drawRect(95, 48, 16, 16, WHITE);
  display.drawRect(111, 48, 16, 16, WHITE);

  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(68, 21);
  display.print(F("AUTO"));
  display.setCursor(100, 21);
  display.print(F("CALL"));
  display.setCursor(100, 36);
  display.print("ALAR");

  display.setCursor(70, 36);
  sprintf(temp_msg, "%02dC", Main_Temp);
  display.print(temp_msg);

  display.setCursor(67, 52);
  display.print("3");
  display.setCursor(85, 52);
  display.print("4");
  display.setCursor(101, 52);
  display.print("5");
  display.setCursor(117, 52);
  display.print("6");



  display.setTextColor(BLACK);
  if (isAutoHeating)
  {
    display.fillRect(65, 18, 28, 12, WHITE);
    display.setCursor(68, 21);
    display.print("AUTO");
  }


  if ((Floor_1_Temp > Alarm_Temp) or  (Floor_2_Temp > Alarm_Temp))
  {
    display.fillRect(65, 34, 28, 12, WHITE);
    display.setCursor(68, 36);
    display.print("ALAR");
  }

  if  (isCalling)
  {
    display.fillRect(97, 18, 28, 12, WHITE);
    display.setCursor(100, 21);
    display.print("CALL");
  }
  if (isRelay03)
  {
    display.fillRect(65, 50, 12, 12, WHITE);
    display.setCursor(68, 52);
    display.print("3");
  }
  if (isRelay04)
  {
    display.fillRect(81, 50, 12, 12, WHITE);
    display.setCursor(85, 52);
    display.print("4");
  }
  if (!isRelay05)
  {
    display.fillRect(97, 50, 12, 12, WHITE);
    display.setCursor(101, 52);
    display.print("5");
  }
  if (isRelay06)
  {
    display.fillRect(113, 50, 12, 12, WHITE);
    display.setCursor(117, 52);
    display.print("6");
  }

}
