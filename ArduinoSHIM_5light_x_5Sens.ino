// http://arduino.ru/forum/programmirovanie/upravlenie-svetom-s-pomoshchyu-arduiny-i-datchika-dvizheniya
//#include <avr/wdt.h> 

byte pins[5] = {
  10,9,6,5,3}; // массив выводов  ШИМ-сигнала на лампочки
int states[5] = {
  255,255,255,255,255}; // массив состояний датчиков лампочек здесь 255 - лампа выключена

void setup() 
{    
  // 
  // wdt_enable(WDTO_4S);
  PORTC = 0b11111;  // подтяжка входных выводов на 1 
  PORTD = 0b100;    // подтяжка входных выводов на 1
  // настройка на выход выводов
  for(byte i = 0; i < 5; i++)
    pinMode(pins[i], OUTPUT);     
}

void loop() 
{
 
    // цикл изменения состояния по каждой лампе
    for(byte i = 0; i < 5; i++)
    {
      // когда датчик очередной лампы в 1 - декримент (увеличение светимости) шим сишнала на 3 единицы 
      if((PINC&(1<<i))) states[i] -= 3;
        else 
          {
     // даже если датчик лампы неактивен, но включились другие галогенки - декримент (увеличение светимости) очередной лампы         
            if((PIND&0b100)) states[i] -= 3;
            else states[i]++;
          }        
      states[i] = constrain(states[i], 0, 255);
      analogWrite(pins[i], states[i]);
      delay(3);
    }

  
  
  //wdt_reset();
}
