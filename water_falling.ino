/* 
Тупо регистрируем замыкания контактов
*/

int pin = 2;       // 2-й цифровой пин, нулевое прерывание
long counter = 0;  // счётчик импульсов

void setup() {
  // Настраиваем последовательный интерфейс, для вывода информации 
  Serial.begin(9600);
  
  //Настраиваем цифровой вход
  pinMode(pin, INPUT);      // Сюда будем подключать подтягивающий резюк
  digitalWrite(pin, HIGH);  // "Подключаем" подтягивающий резистор (реализован внутри ATmega)
  attachInterrupt(0, count, FALLING); // задаём обработчик прерывания 0 (2-й пин). 
                                      // прерывание будет при изменении уровня с HIGHT на LOW
                                      // вызывать функицю count
}

void loop(){
  // основной рабочий цикл
  
  Serial.println(counter); // выводим в консоль значение счётчика
}


void count(){
  // вызывается прерыванием 0 от 2-го цифрового входа
  counter++;  // ясен перец, увеличиваем переменную. Утекло ещё 5 литров
}