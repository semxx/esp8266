// http://smart-chip.ru/avto-svet/
//int incomingByte[5]; 
unsigned long time_m; 
const int button4Pin = 2; // Кнопка Большой свет
const int button5Pin = 3; // Кнопка Маленький свет
const int ledPin4 =  4;   // Реле Большой свет
const int ledPin5 =  5;   // Реле Маленький свет
//int incomingByte = 0;      // переменная для хранения байта данных, 
//принятых по последовательному интерфейсу
// изначальное состояние ламп - выключено
boolean led4 = 1;
boolean led5 = 1;
// Состояние кнопок
int button4State = 0;         
int button5State = 0; 
// время отключения автоматики при ручном управлении
unsigned long manual=30000; // 30 сек.
const int PIR=10;
unsigned long led4_m;

//переменная для анализа долгого нажатия.
int fl4 = 0;
int fl5 = 0;

void setup() {
  // инииализация портов реле
  pinMode(ledPin4, OUTPUT); 
  pinMode(ledPin5, OUTPUT);
  // инициализация портов кнопок
  pinMode(button4Pin, INPUT);
  pinMode(button5Pin, INPUT);
  // включаем подтягивающий резистор
  digitalWrite(button4Pin, HIGH);
  digitalWrite(button5Pin, HIGH);
  // Включаем лампы
  digitalWrite(ledPin4, led4);
  digitalWrite(ledPin5, led5);
  // инициализация PIR
  pinMode(PIR, INPUT);

}
void loop(){
  time_m = millis();
  // считываем показания кнопок 
  button4State=digitalRead(button4Pin);
  button5State=digitalRead(button5Pin);
  //delay (100);
  //////////////кнопка 5 (Большой свет)///////////////
  if (button5State == LOW) {  // если кнопка нажата:   
    fl5 = fl5 + 1;
    if (fl5==1){
      led5 = !led5;  
    }
  }
  else fl5=0;

  //////////////кнопка 4 (маленький свет)///////////////
  if (button4State == LOW) {  // если кнопка нажата:   
    fl4 = fl4 + 1;
    if (fl4==1){
      led4 = !led4;
      led4_m=time_m;
    }
  }
  else fl4=0; 

  if (digitalRead(PIR)== HIGH && (time_m-led4_m)>manual && led5==HIGH){// если сработал сенсор и время с момента ручного включения боьше 30 сек.
    led4=LOW; // Включаем свет 
  }

  if (digitalRead(PIR)== LOW && (time_m-led4_m)>manual){// если сработал сенсор и время с момента ручного включения боьше 30 сек.
    led4=HIGH;
    led5=HIGH;
  }
  digitalWrite(ledPin4, led4);
  digitalWrite(ledPin5, led5);

  delay(50); 
}


