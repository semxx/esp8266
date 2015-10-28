// http://cyber-place.ru/showthread.php?t=1488
#include <CyberLib.h>

volatile uint16_t Hot_Count, Cold_Count; 
uint16_t Hot, Cold;
void setup()  
{ 
  D2_In; D2_High; D3_In; D3_High; //сконфигурировать входы и подтянуть к +питания
  attachInterrupt(0, HotWater, FALLING );             // привязываем 0-е прерывание к функции HotCount(). 
  attachInterrupt(1, ColdWater, FALLING );            // привязываем 1-е прерывание к функции ColdCount().
  Serial.begin(57600); 
  wdt_enable (WDTO_1S);    //Сторожевая собака 1сек.   
}  

void loop()  
{  Start
    if (Serial.available()>1)
    {   
       uint8_t inByte = Serial.read();   
        if(inByte=='#')
         { 
          inByte = Serial.read();
          if(inByte=='W')
          {
           delay_ms(200);
            cli();
            Hot=Hot_Count;
            Cold=Cold_Count;
            Hot_Count=0;
            Cold_Count=0;
            sei();
            Serial.print("Hot: "); 
            Serial.print(Hot);
            Serial.print(" Cold: ");  
            Serial.println(Cold);         
          }
         }  
     }
 wdt_reset(); //покормить собаку 
End } 

//*******************************
void ColdWater() 
{ 
  Cold_Count++;  
} 

void HotWater() 
{ 
  Hot_Count++;
}  
