#include <DallasTemperature.h>
#include <OneWire.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <EEPROM.h>
#include "DHT.h"

/*
   Установки PIN входов и выходов
*/
#define ADKeyboardPin 0 // Analog In
#define FireSensorIn 1 // Analog In

#define DHTPIN 11 // Digital In
#define WindFanOut 3 // Digital Out PWM
#define PumpRelayOut 5 // Digital Out
#define BurnPinIn 6 // Digital In
#define ShnackRelayOut 7 // Digital Out
#define LampRelayOut 8 // Digital Out
#define DS18B20Pin 9 // Digital IN
#define ErrorRelayPin 10 // Digital Out

#define SDAPin A4 // Analog Out
#define SCLPin A5 // Analog Out

#define DHTTYPE DHT22
#define REVERSRELAY 0 // type relay (high level on or low level on)
#define DSBIT 9

DeviceAddress t_pod = {0x28, 0xFF, 0xCE, 0x0A, 0x62, 0x16, 0x03, 0x9D};
DeviceAddress t_obr = {0x28, 0xFF, 0xF9, 0x6D, 0x71, 0x16, 0x05, 0x2D};
DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x3F, 16, 2);
OneWire  oneWire(DS18B20Pin);
DallasTemperature sensors(&oneWire);

/*
   Основные заводские настройки
*/

// Тц | Время цикла | Сумма времени подачи и паузы работы привода шнека в режимах нагрева и поддержания 20 секунд
int Tc = 20;

// Тпр* | Подача розжиг | Время подачи топлива в режиме розжига 60 секунд
int Tpr = 60;

// Тпн* | Подача нагрев | Время подачи в режиме нагрева 8 секунд
int Tpn = 5;

// Тпп* | Подача поддержание | Время подачи в режиме поддержания 3 секунд
int Tpp = 1;

// Wp | Вентилятор розжиг | Интенсивность работы вентилятора при розжиге 50%
int Wr = 50;

// Wн* | Вентилятор нагрев | Интенсивность работы вентилятора в режиме нагрева 75%
int Wn = 75;

// Wп | Вентилятор поддержание | Интенсивность работы вентилятора в режиме поддержания 30%
int Wp = 30;

// Wо* | Вентилятор ожидание | Интенсивность работы вентилятора в режиме ожидания пуска 50%
int Wo = 50;

// tу* | Установка температуры | Пороговое значение температуры теплоносителя для перехода из режима нагрева в режим поддержания 60 ˚С
int tu = 60;

// tг | Гистерезис установки температуры | Разность между температурами перехода из нагрева в поддержание и температурой обратного перехода из поддержания в нагрев 2 ˚С
int tg = 2;

// Тф | Время фиксации пламени | Продолжительность задержки перехода из розжига к нагреву после появления сигнала пламени или обратного перехода к розжигу после исчезновения сигнала наличия пламени 30 секунд
int Tf = 30;

// Тр | Время розжига | Продолжительность работы лампы розжига. По истечении этого времени должен появиться сигнал наличия пламени 7 минут
int Tr = 7;

// Тв | Время выжигания | Длительность продувки на максимальной мощности вентилятора для очищения жаровни от продуктов горения 5 минут
int Tv = 5;

// Fу | Установка пламени | Минимальная яркость пламени перехода от розжига к рабочим режимам и обратно 20%
int Fu = 20;

// ta | Перегрев | Температура аварийного отключенияподачи топлива при превышении максимально допустимого значения температуры теплоносителя в трубе подачи 90˚С
int ta = 90;

// tп | Температура подачи | Текущие показания датчика температуры подачи ˚С
int tp = 0;

// tо | Температура обратки | Текущие показания датчика температуры обратки ˚С
int to = 0;

// tв | Температура воздуха | Текущие показания датчика температуры воздуха в помещении ˚С
int tv = 0;

// Fп | Пламя | Текущие показания датчика пламени %
int Fp = 0;

// St | Этап цикла | 0 - ожидание пуска, 1 - Розжиг, 2 - Нагрев, 3 - Поддержание, 4 - Выжигание
int St = 0;


/*
   Временные интервалы, тайминги и служебные промежуточные переменные
*/
long menu = 0;
int Error = 0;

// Текущие микросекунды
unsigned long currentMillis = 0;
unsigned long oneSecondStepPoll = 0;
unsigned long displaySecondStepPoll = 0;
int DisplayPage = 0;
int CurrPWMStatus = 0;
int CurrLampStatus = 0;

// Для проверки наличия пламени
int Fire = 0;
int Firecount = 0;
int FirstBurnStep = 0; // Этапы розжига: 0 - Загрузка топлива, 1 - Розжиг
int stat_BurnPinIn = 0; // Включена горелка или нет
unsigned long CurrBurnTimeStatus = 0;
unsigned long MillisBySt0, MillisBySt1, MillisBySt4, MillisBySt6 = 0;

// Для работы шнека по периоду
unsigned long ShnackStopMillis = 0;
int ShnackStep = 0; // Этапы цикла шнека: 0 - Начало цикла, 1 - Подача, 2 - Ожидание начала нового цикла
int CurrShnackStatus = 0;
unsigned long ShnackNextStartMillis = 0;
int CurrPumpStatus = 0; // Статус циркуляционника

/*
   Системные Функции
*/

void debug() {
  Serial.print(currentMillis);
  Serial.print("\t Step: ");  Serial.print(St);
  Serial.print("\t Fire: ");  Serial.print(Fire);
  Serial.print("\t Fire %: ");  Serial.print(Fp);
  Serial.print("\t Firecount: ");  Serial.print(Firecount);
  Serial.print("\t CurrBurnTimeStatus: ");  Serial.print(CurrBurnTimeStatus);
  Serial.print("\t Podacha: ");  Serial.print(tp);
  Serial.print("\t Obratka: ");  Serial.print(to);
  Serial.print("\t Fan %: "); Serial.print(CurrPWMStatus); Serial.print("%");
  Serial.print("\t Shnack: ");  Serial.print(CurrShnackStatus);
  Serial.print("\t ShnackStep: ");  Serial.print(ShnackStep);
  Serial.print("\t ShnackStopMillis: ");  Serial.print(ShnackStopMillis);
  Serial.print("\t ShnackNextStartMillis: ");  Serial.print(ShnackNextStartMillis);
  Serial.print("\t FirstBurnStep: ");  Serial.print(FirstBurnStep);
  Serial.println("");
}

void setupPins() {
  pinMode(WindFanOut, OUTPUT);
  pinMode(FireSensorIn, INPUT);
  pinMode(LampRelayOut, OUTPUT);
  pinMode(ShnackRelayOut, OUTPUT);
  pinMode(ErrorRelayPin, OUTPUT);
  pinMode(BurnPinIn, INPUT);
  pinMode(ADKeyboardPin, INPUT);
  pinMode(PumpRelayOut, OUTPUT);

  setPinStatus("off", PumpRelayOut, CurrPumpStatus, REVERSRELAY);
  setPinStatus("off", ShnackRelayOut, CurrShnackStatus, 0);
  setPinStatus("off", LampRelayOut, CurrLampStatus, REVERSRELAY);
  SetPWM(WindFanOut, 15);
  Ds18b20Setup();
}

void setPinStatus(String stat, int pin, int &info, int is_revers)
{
  if (stat == String("on")) {
    if (is_revers == 1) {
      digitalWrite(pin, LOW);
    } else {
      digitalWrite(pin, HIGH);
    }
    info = 1;
  } else {
    if (is_revers == 1) {
      digitalWrite(pin, HIGH);
    } else {
      digitalWrite(pin, LOW);
    }
    info = 0;
  }
}

void getTemperature(DeviceAddress deviceAddress, int &ret)
{
  int tempC = sensors.getTempC(deviceAddress);
  if (tempC == -127.00 || tempC == 0.00 || tempC == 255.94 || tempC == 139.38 || tempC == 85.0) {
    // return
  } else {
    ret = tempC;
  }
}

void Ds18b20Setup() {
  sensors.begin();
  sensors.setResolution(t_pod, DSBIT);
  sensors.setResolution(t_obr, DSBIT);
  sensors.requestTemperatures();
}

void ds18b20Poll()
{
  getTemperature(t_pod, tp);
  getTemperature(t_obr, to);

  sensors.setWaitForConversion(false);
  sensors.requestTemperatures();
}

// Функция перевода секунд в микросекунды
long SecToMicroSec(long seco) {
  long retval = 0;
  retval = seco * 1000;
  return retval;
}

// Функция перевода минут в микросекунды
long MinToMicroSec(long minutes) {
  long retval = 0;
  retval = minutes * 60000;
  return retval;
}

// Функция установки напряжения на аналоговый PWM выход в процентах
void SetPWM(int pin, long percent) {
  CurrPWMStatus = percent;
  long value = (long) (255 / 100 * percent);
  analogWrite(pin, value);
}

void GetAllInfo() {
  stat_BurnPinIn = digitalRead(BurnPinIn);
  float Fptmp = analogRead(FireSensorIn);
  Fptmp =  100 - ((Fptmp / 1023) * 100);
  Fp = (long) Fptmp;
  pollDHT();
  ds18b20Poll();
}

void pollDHT() {
  float curr_dht;
  //float h = dht.readHumidity();
  curr_dht = dht.readTemperature();

  if (isnan(curr_dht)) {
    return;
  }
  tv = curr_dht;
}

void ShowInfo() {
  // output_string - куда выводить, массив char
  // "%02i %2c %4.3f" формат вывода аргументов:
  // %02(два поля - сначала нули)i(int)
  // %2(два поля)c(char)
  // %4(4 поля всего).2(после запятой)f(float)
  // про форматы - htp://www.chitay.org/c/13/printf.htm
  char output_string[16];


  if (menu == 0) {
    if (DisplayPage == 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      sprintf(output_string, "tp=%02i to=%02i", tp, to);
      lcd.print(output_string);


      lcd.setCursor(0, 1);
      sprintf(output_string, "tu=%02i tv=%02i", tu, tv);
      lcd.print(output_string);

    }

    if (DisplayPage == 2) {
      lcd.clear();
      lcd.setCursor(0, 0);
      sprintf(output_string, "Fp=%02i W=%02i St=%02i", Fp, CurrPWMStatus, St);
      lcd.print(output_string);

      lcd.setCursor(0, 1);
      sprintf(output_string, "L=%02i Error=%02i", CurrLampStatus, Error);
      lcd.print(output_string);
      DisplayPage = 0; // Повторяем по кругу. Обязательный флаг на последнем элементе меню.
    }
  }
}

void GetDataFromEEPROM() {
  int currconfig = 0;

  currconfig = EEPROM_int_read(0);

  if (currconfig == 1)
  {
    Tc = EEPROM_int_read(2);
    Tpr = EEPROM_int_read(4);
    Tpn = EEPROM_int_read(6);
    Tpp = EEPROM_int_read(8);
    Wr = EEPROM_int_read(10);
    Wn = EEPROM_int_read(12);
    Wp = EEPROM_int_read(14);
    Wo = EEPROM_int_read(16);
    tu = EEPROM_int_read(18);
    tg = EEPROM_int_read(20);
    Tf = EEPROM_int_read(22);
    Tr = EEPROM_int_read(24);
    Tv = EEPROM_int_read(26);
    Fu = EEPROM_int_read(28);
    ta = EEPROM_int_read(30);
  } else {
    SaveDataToEEPROM();
  }
}

void SaveDataToEEPROM() {
  EEPROM_int_write(0, 1);
  EEPROM_int_write(2, Tc);
  EEPROM_int_write(4, Tpr);
  EEPROM_int_write(6, Tpn);
  EEPROM_int_write(8, Tpp);
  EEPROM_int_write(10, Wr);
  EEPROM_int_write(12, Wn);
  EEPROM_int_write(14, Wp);
  EEPROM_int_write(16, Wo);
  EEPROM_int_write(18, tu);
  EEPROM_int_write(20, tg);
  EEPROM_int_write(22, Tf);
  EEPROM_int_write(24, Tr);
  EEPROM_int_write(26, Tv);
  EEPROM_int_write(28, Fu);
  EEPROM_int_write(30, ta);
}


// 0 - работаем, 1 - закончил работу
int ShnackWorkTime(unsigned long Time, int period) {
  long workTime = SecToMicroSec(Time);
  long diff = 0;

  workTime = workTime / 10;

  if (ShnackStopMillis == 0) {
    ShnackStopMillis = currentMillis + workTime;
  }
  diff = currentMillis - ShnackStopMillis;

  if (period == 0) {
    if (diff > 0) {
      // Stop
      setPinStatus("off", ShnackRelayOut, CurrShnackStatus, 0);
      ShnackStopMillis = 0;
      return 1;
    } else {
      setPinStatus("on", ShnackRelayOut, CurrShnackStatus, 0);
      // Start
      return 0;
    }
  } else {
    if (ShnackStep == 0) {
      ShnackNextStartMillis = currentMillis + SecToMicroSec(Tc);
      ShnackStep = 1;
    }
    if (ShnackStep == 1) {
      if (diff > 0) {
        // Stop
        setPinStatus("off", ShnackRelayOut, CurrShnackStatus, 0);
        ShnackStep = 2;
      } else {
        // Start
        setPinStatus("on", ShnackRelayOut, CurrShnackStatus, 0);
      }
    }
    if (ShnackStep == 2 && currentMillis >= ShnackNextStartMillis) {
      ShnackStep = 0;
      ShnackStopMillis = 0;
    }
  }
}


void CheckWorkStep() {
  // Если tо<tу, автоматически включается режим нагрева (включая гистерезис)

  if (tp - tg < tu) {
    St = 2;
  }

  // Если tо>tу, автоматически включается режим поддержания заданной температуры (включая гистерезис)
  if (tp + tg > tu) {
    St = 3;
  }

  //Если температура теплоносителя в трубе подачи (tп) превышает температуру аварийного отключения (tа), загорается красный индикатор , подача топлива прекращается и не возобновляется до тех пор, пока значение (tп) не станет меньше или равно (tа). На экране ПУ появляется сообщение об ошибке.
  if (tp > ta || to > ta) {
    St = 5;
  }

  // Розжиг, если потухли на нагреве или поддержании горения
  if ((St == 2 || St == 3) && Fire == 0) {
    St = 4;
  }
}

// Return 0 - Идёт проверка, 1 - Появилось пламя, 2 - Истекло время ожидания пламени
int checkFireByTime(unsigned long chktime, unsigned long &CheckFireByTimeMillis) {
  // Начинаем проверку.
  if (CheckFireByTimeMillis == 0) {
    CheckFireByTimeMillis = currentMillis;
  }
  // Превышено время розжига
  if (CheckFireByTimeMillis > 0 && currentMillis - CheckFireByTimeMillis > chktime ) {
    if (Fire == 1) {
      return 1;  // Огонь есть
    } else {
      return 2; // Истекло время поджига
    }
  }

  return 0;
}

void SetLogiq() {
  int CurrShnackStatusRet = 0;
  tu = calc_int_out();
  if (St != 0) {
    MillisBySt0 = 0;
  }
  if (St != 1) {
    MillisBySt1 = 0;
  }
  if (St != 4) {
    MillisBySt4 = 0;
  }

  if (St != 6) {
    MillisBySt6 = 0;
  }

  // Режим ожидания пуска
  if (St == 0 && stat_BurnPinIn == 1) {
    Error = 0;
    FirstBurnStep = 0;
    // В режиме ожидания пуска включается вентилятор на мощности, соответствующей режиму поддержания
    SetPWM(WindFanOut, Wp);
    // todo check why not visible fire after reboot
    if (Fire == 1) {
      CheckWorkStep();
    } else {
      // Если пламя отсутствует, то по истечении времени фиксации пламени горелка переходит в режим розжига и на экране появляется надпись «розжиг»
      long checkTf = SecToMicroSec(Tf) + 5000;
      CurrBurnTimeStatus = checkFireByTime(checkTf, MillisBySt0);
      if (CurrBurnTimeStatus == 2) {
        St = 1;
        ShnackStopMillis = 0;
      }
      if (CurrBurnTimeStatus == 1) {
        CheckWorkStep();
      }
    }
  }

  if (St == 0 && stat_BurnPinIn == 0) {
    SetPWM(WindFanOut, 15);
  }

  // Розжиг
  if (St == 1) {
    if (FirstBurnStep == 0) {
      CurrShnackStatusRet = ShnackWorkTime(Tpr, 0);
      if (CurrShnackStatusRet == 1) {
        FirstBurnStep = 1;
      }
    } else {
      setPinStatus("on", LampRelayOut, CurrLampStatus, REVERSRELAY);
      SetPWM(WindFanOut, Wr);
      if (Fire == 1) {
        CheckWorkStep();
      } else {
        // время розжига и кол-во попыток. А также сигнализируем о неисправности.
        long checkTr = MinToMicroSec(Tr);
        CurrBurnTimeStatus = checkFireByTime(checkTr, MillisBySt1);
        if (CurrBurnTimeStatus == 2) {
          Error = 1;
          St = 6;
        }
      }
    }
  }

  if (St != 1) {
    setPinStatus("off", LampRelayOut, CurrLampStatus, REVERSRELAY);
  }

  // Нагрев
  if (St == 2) {
    FirstBurnStep = 0;
    SetPWM(WindFanOut, Wn);
    ShnackWorkTime(Tpn, Tc);
    CheckWorkStep();
  }


  // Поддержание
  if (St == 3) {
    FirstBurnStep = 0;
    SetPWM(WindFanOut, Wp);
    ShnackWorkTime(Tpp, Tc);
    CheckWorkStep();
  }

  if (St >= 2) {
    setPinStatus("on", PumpRelayOut, CurrPumpStatus, REVERSRELAY);
  } else {
    setPinStatus("off", PumpRelayOut, CurrPumpStatus, REVERSRELAY);
  }

  // Выжигание
  if (St == 4) {
    SetPWM(WindFanOut, 100);
    FirstBurnStep = 0;
    long checkTv = MinToMicroSec(Tv);
    CurrBurnTimeStatus  = checkFireByTime(checkTv, MillisBySt4);
    if (CurrBurnTimeStatus == 2) {
      St = 0;
    }
  }

  // Перегрев обратки или подачи
  if (St == 5) {
    SetPWM(WindFanOut, Wp);
    FirstBurnStep = 0;
    CheckWorkStep();
  }

  // Ошибка и остановка горелки
  if (St == 6) {
    FirstBurnStep = 0;
    if (Error == 1)
    {
      SetPWM(WindFanOut, 100);
      long checkTv = MinToMicroSec(Tv);
      CurrBurnTimeStatus  = checkFireByTime(checkTv, MillisBySt6);
      if (CurrBurnTimeStatus == 2) {
        Error = 0;
      }
    } else {
      SetPWM(WindFanOut, 15);
    }
  }

  if (St >= 4) {
    setPinStatus("off", ShnackRelayOut, CurrShnackStatus, 0);
  }

  // Если горелка не работает
  if (stat_BurnPinIn == 0 && St != 0) {
    St = 4;
  }

  if (stat_BurnPinIn == 1 && St == 4) {
    if (Fire == 1) {
      St = 2;
    }
  }


}

// Определение пламени
void checkFire() {

  if (Fp >= Fu) {
    if (Firecount < Tf) {
      Firecount++;
    }

  } else {
    if (Firecount > 0) {
      Firecount--;
    }
  }

  if (Firecount == 0) {
    Fire = 0;
  }

  if (Firecount == Tf) {
    Fire = 1;
  }
}


/*
   Show and edit menu
*/

void action(String actionval, int &val, int minimal, int maximal)
{
  if (actionval == "Up") {
    if (val < maximal) {
      val++;
    }
    if (val >= maximal) {
      val = minimal;
    }
  }
  if (actionval == "Down") {
    if (val > minimal) {
      val--;
    }
    if (val <= minimal) {
      val = maximal;
    }
  }
}

void ShowMenu(String actionval) {
  char output_string[16];

  switch (menu) {

    // Тц | Время цикла | Сумма времени подачи и паузы работы привода шнека в режимах нагрева и поддержания 20 секунд
    case 1:
      action(actionval, Tc, 5, 99);
      lcd.clear();
      lcd.setCursor(0, 0);
      sprintf(output_string, "SETUP Tc=%02is", Tc);
      lcd.print(output_string);

      lcd.setCursor(0, 1);
      lcd.print("05-99s Def: 20s");

      break;

    // Тпр* | Подача розжиг | Время подачи топлива в режиме розжига 60 секунд
    case 2:
      action(actionval, Tpr, 1, 99);
      lcd.clear();
      lcd.setCursor(0, 0);
      sprintf(output_string, "SETUP Tpr=%02is", Tpr);
      lcd.print(output_string);

      lcd.setCursor(0, 1);
      lcd.print("01-99s Def: 60s");
      break;

    // Тпн* | Подача нагрев | Время подачи в режиме нагрева 8 секунд
    case 3:
      action(actionval, Tpn, 1, 99);
      lcd.clear();
      lcd.setCursor(0, 0);
      sprintf(output_string, "SETUP Tpn=%02is", Tpn);
      lcd.print(output_string);
      lcd.setCursor(0, 1);
      lcd.print("01-99s Def: 8s");

      break;

    // Тпп* | Подача поддержание | Время подачи в режиме поддержания 3 секунд
    case 4:
      action(actionval, Tpp, 1, 99);
      lcd.clear();
      lcd.setCursor(0, 0);
      sprintf(output_string, "SETUP Tpp=%02is", Tpp);
      lcd.print(output_string);
      lcd.setCursor(0, 1);
      lcd.print("01-99s Def: 3s");

      break;

    // Wp | Вентилятор розжиг | Интенсивность работы вентилятора при розжиге 50%
    case 5:
      action(actionval, Wr, 15, 99);
      lcd.clear();
      lcd.setCursor(0, 0);
      sprintf(output_string, "SETUP Wr=%02i%%", Wr);
      lcd.print(output_string);
      lcd.setCursor(0, 1);
      lcd.print("15-99% Def: 50%");

      break;

    // Wн* | Вентилятор нагрев | Интенсивность работы вентилятора в режиме нагрева 75%
    case 6:
      action(actionval, Wn, 15, 99);
      lcd.clear();
      lcd.setCursor(0, 0);
      sprintf(output_string, "SETUP Wn=%02i%%", Wn);
      lcd.print(output_string);
      lcd.setCursor(0, 1);
      lcd.print("15-99% Def: 75%");

      break;

    // Wп | Вентилятор поддержание | Интенсивность работы вентилятора в режиме поддержания 30%
    case 7:
      action(actionval, Wp, 15, 99);
      lcd.clear();
      lcd.setCursor(0, 0);
      sprintf(output_string, "SETUP Wp=%02i%%", Wp);
      lcd.print(output_string);
      lcd.setCursor(0, 1);
      lcd.print("15-99% Def: 30%");

      break;

    // Wо* | Вентилятор ожидание | Интенсивность работы вентилятора в режиме ожидания пуска 50%
    case 8:
      action(actionval, Wo, 15, 99);
      lcd.clear();
      lcd.setCursor(0, 0);
      sprintf(output_string, "SETUP Wo=%02i%%", Wo);
      lcd.print(output_string);
      lcd.setCursor(0, 1);
      lcd.print("15-99% Def: 50%");

      break;

    // tу* | Установка температуры | Пороговое значение температуры теплоносителя для перехода из режима нагрева в режим поддержания 60 ˚С
    case 9:
      action(actionval, tu, 20, 80);
      lcd.clear();
      lcd.setCursor(0, 0);
      sprintf(output_string, "SETUP tu=%02iC", tu);
      lcd.print(output_string);
      lcd.setCursor(0, 1);
      lcd.print("20-80C Def: 60C");

      break;

    // tг | Гистерезис установки температуры | Разность между температурами перехода из нагрева в поддержание и температурой обратного перехода из поддержания в нагрев 2 ˚С
    case 10:
      action(actionval, tg, 1, 5);
      lcd.clear();
      lcd.setCursor(0, 0);
      sprintf(output_string, "SETUP tg=%02iC", tg);
      lcd.print(output_string);
      lcd.setCursor(0, 1);
      lcd.print("01-05C Def: 02C");

      break;

    // Тф | Время фиксации пламени | Продолжительность задержки перехода из розжига к нагреву после появления сигнала пламени или обратного перехода к розжигу после исчезновения сигнала наличия пламени 30 секунд
    case 11:
      action(actionval, Tf, 10, 60);
      lcd.clear();
      lcd.setCursor(0, 0);
      sprintf(output_string, "SETUP Tf=%02is", Tf);
      lcd.print(output_string);
      lcd.setCursor(0, 1);
      lcd.print("10-60s Def: 30s");

      break;

    // Тр | Время розжига | Продолжительность работы лампы розжига. По истечении этого времени должен появиться сигнал наличия пламени 7 минут
    case 12:
      action(actionval, Tr, 1, 15);
      lcd.clear();
      lcd.setCursor(0, 0);
      sprintf(output_string, "SETUP Tr=%02im", Tr);
      lcd.print(output_string);
      lcd.setCursor(0, 1);
      lcd.print("01-15m Def: 07m");

      break;

    // Тв | Время выжигания | Длительность продувки на максимальной мощности вентилятора для очищения жаровни от продуктов горения 5 минут
    case 13:
      action(actionval, Tv, 1, 15);
      lcd.clear();
      lcd.setCursor(0, 0);
      sprintf(output_string, "SETUP Tv=%02im", Tv);
      lcd.print(output_string);
      lcd.setCursor(0, 1);
      lcd.print("01-15m Def: 05m");

      break;

    // Fу | Установка пламени | Минимальная яркость пламени перехода от розжига к рабочим режимам и обратно 20%
    case 14:
      action(actionval, Fu, 10, 50);
      lcd.clear();
      lcd.setCursor(0, 0);
      sprintf(output_string, "SETUP Fu=%02i%%", Fu);
      lcd.print(output_string);
      lcd.setCursor(0, 1);
      lcd.print("10-50% Def: 20%");

      break;

    // ta | Перегрев | Температура аварийного отключенияподачи топлива при превышении максимально допустимого значения температуры теплоносителя в трубе подачи 90˚С
    case 15:
      action(actionval, ta, 70, 90);
      lcd.clear();
      lcd.setCursor(0, 0);
      sprintf(output_string, "SETUP ta=%02iC", ta);
      lcd.print(output_string);
      lcd.setCursor(0, 1);
      lcd.print("70-90C Def: 90C");

      break;

    case 16:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Save and Quit");

      if (actionval == "Select") {
        lcd.clear();
        DisplayPage = 0;
        SaveDataToEEPROM();
        menu = 0;
      }
      break;

    // NO Save
    case 17:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Quit no save");

      if (actionval == "Select") {
        lcd.clear();
        DisplayPage = 0;
        GetDataFromEEPROM();
        menu = 0;
      }
      break;

    // Load Defaults
    case 18:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Load Defaults");

      if (actionval == "Select") {
        EEPROM_int_write(0, 0);
        lcd.clear();
        lcd.print("NOW RESTART!");
        delay(100000000);
      }
      break;
    default:
      break;
  }
}

void CheckMenu() {
  int x;
  x = analogRead (ADKeyboardPin);
  if (x < 1000 || menu != 0)
  {
    ShowMenu("none");
    delay(50);
    if (x > 1000) return;

    if (x < 60) {
      // Right
      if (menu > 0) {
        menu++;
      }
      if (menu >= 19) {
        menu = 1;
      }

      ShowMenu("none");
      delay(100);
    }
    else if (x < 200) {
      // Up
      ShowMenu("Up");
    }
    else if (x < 400) {
      // Down
      ShowMenu("Down");
    }
    else if (x < 600) {
      // Left
      if (menu >= 1) {
        menu--;
      }
      if (menu == 0) {
        menu = 18;
      }
      ShowMenu("none");
      delay(100);
    }
    else if (x < 800) {
      // Select
      if (menu == 0) {
        menu++;
      }
      ShowMenu("Select");
      delay(100);
    }
    delay(50);
  }
}

// чтение
int EEPROM_int_read(int addr) {
  byte raw[2];
  for (byte i = 0; i < 2; i++) raw[i] = EEPROM.read(addr + i);
  int &num = (int&)raw;
  return num;
}

// запись
void EEPROM_int_write(int addr, int num) {
  byte raw[2];
  (int&)raw = num;
  for (byte i = 0; i < 2; i++) EEPROM.write(addr + i, raw[i]);
}

float calc_temp() {
  //
  // Функция расчитывает температуру контура отпления
  //
  float a, b, c, x;
  float temp_v, temp_n;
  float iv_k = 1.0; // Коэффициент кривой TODO вынести в конфиг и EEPROM
  // Температура контура отопления в зависимости от наружной температуры
  // Tn = ax2 + bx + c
  // a = -0,21k — 0,06
  // b = 6,04k + 1,98
  // с = -5,06k + 18,06
  // x = -0.2*t1 + 5

  x = (-0.2 * tv) + 5;
  a = (-0.21 * iv_k) - 0.06;
  b = (6.04 * iv_k) + 1.98;
  c = (-5.06 * iv_k) + 18.06;

  temp_n = (a * x * x) + (b * x) + c;

  // Расчетная температура конура отопления
  // T = Tn + Tk + Tt
  temp_v = temp_n;

  // Ограничиваем температуру
  if (temp_v > 75) temp_v = 75;
  if (temp_v < 0) temp_v = 0;

  return temp_v;
}


int calc_int_out() {
  //
  // Расчитываем значение переменной int_out на основе temp_v
  // или выходное значение платы управления на основе расчетной температуры конура отопления
  //
  int int_out;
  float temp_v = calc_temp();

  // Температура котла зависит от значений платы управления нелинейно, поэтому воспользуемся табличными данными
  // Номер элемента в массиве совпадает с расчетным значением int_out
  //int temptable[] = {0, 0, 32, 35, 38, 41, 43, 46, 48, 51, 53, 55, 57, 59, 60, 62, 63, 64, 66, 68, 69, 70, 71, 72, 74, 75, 76, 77, 78, 79, 79, 80};
  int temptable[] = {24, 28, 32, 35, 38, 41, 43, 46, 48, 51, 53, 55, 57, 59, 60, 62, 63, 64, 66, 68, 69, 70, 71, 72, 74, 75, 76, 77, 78, 79, 79, 80};

  for (int i = 1; i <= 31; i++) {
    // ищем подходящую темепературу
    if (temp_v <= temptable[i]) {
      float j1, j2;
      j1 = temptable[i] - temp_v;
      j2 = temp_v - temptable[i - 1];
      // выбираем лучшее значение
      if (j2 > j1) {
        int_out = i;
      } else {
        int_out = i - 1;
      }
      break;
    }
  }
  return temptable[int_out];
}


/*
   Функции микроконтроллера
*/
void setup() {
  Serial.begin(115200);
  dht.begin();
  lcd.init();
  lcd.backlight();// Включаем подсветку дисплея
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Controller v0.1");
  lcd.setCursor(0, 1);
  lcd.print("By V.Dyakov");
  GetDataFromEEPROM();
  setupPins();
  delay(3000);
}

void loop() {
  currentMillis = millis();
  CheckMenu();
  SetLogiq();
  if (currentMillis - oneSecondStepPoll >= 2000) {
    GetAllInfo();
    checkFire();
    debug();
    oneSecondStepPoll = currentMillis;
  }

  if (currentMillis - displaySecondStepPoll >= 3000) {
    DisplayPage++;
    ShowInfo();
    displaySecondStepPoll = currentMillis;
  }
}
