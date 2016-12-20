bool timerOnOff = 0;        //флаг состояния таймера 0-выкл., 1-вкл.
 
void handler(){
              USE_SERIAL.println("100001111100000"); // пуляет в терминал мусор когда таймер работает
}
 
void setup(){
   pinMode(5, INPUT);
//************************************
    timer1_isr_init();
    timer1_enable(3, 0, 1);
    timer1_write(268430);
    timer1_attachInterrupt(handler);
    timerOnOff = 1;           // таймер включен
//************************************
 
}
void loop(){
//************************************  
    if(digitalRead(5)){                           // проверяем управляющий пин, если 1 то убиваем таймер
                             if(timerOnOff){            // проверяем флаг состояния таймера, если 1 то убиваем таймер
                                              timer1_detachInterrupt(); // убиваем таймер
                                              timerOnOff = 0;           // таймер выключен
                             }
    }else{                                              // если управляющий пин 0 то запускаем таймер
              if(timerOnOff==0){                       // проверяем флаг состояния таймера, если 0 то запускаем таймер
                            timer1_enable(3, 0, 1);
                            timer1_write(268430);
                            timer1_attachInterrupt(handler);
                            timerOnOff = 1;             // таймер включен
              }
    }
//************************************  
}
 
