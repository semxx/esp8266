void UpdateDisplay()
{
  display.clearDisplay();
  switch (num_Screen)
  {
    case 0:
      DrawScreenSaver();
      break;
    case 1:
      DrawGSM(27); //DrawGSM(sgsm);
      DrawTime();
      DrawBattery(batt);
      DrawDash();
      DrawHome();
      break;
    case 2:
      DrawSensor();
      break;
    case 3:
      DrawSetting();
      break;
    case 4:
      DrawHistPipe(Main_Text, Addr_Temp_2, Main_Temp);
      break;
    case 5:
      DrawHistPipe(Out_Text, Addr_Temp_1, Out_Temp);
      break;
    case 6:
      DrawHistPipe(Floor_1_Text, Addr_Temp_3, Floor_1_Temp);
      break;
    case 7:
      DrawHistLine(Floor_2_Text, Addr_Temp_4, Floor_2_Temp);
      break;
    case 8:
      DrawAbout();
      break;
    case 9:

      break;
    case 10:

      break;
    default:
      DrawScreenSaver();
  }
  NextBattery();
  display.display();
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




void DrawSetting()
{
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




void DrawHistPipe(char Str[11] , byte Addr, int act_temp)
{

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


void DrawHistLine(String Str , byte Addr, int act_temp)
{
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


void DrawGSM(word signal)
{
  display.drawLine(6, 2, 6, 13, WHITE);
  display.drawTriangle(1, 2, 11, 2, 6, 7, WHITE);
  if (signal > 7)     display.fillRect(9, 12, 2, 2, WHITE);
  if (signal > 13)    display.fillRect(12, 10, 2, 4, WHITE);
  if (signal > 19)    display.fillRect(15, 6, 2, 8, WHITE);
  if (signal > 25)    display.fillRect(18, 2, 2, 12, WHITE);
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



