#include <PID_v1.h>//библиотека ПИД-ругулятора
#include //библиотека серво привода
 
double Setpoint, Input, Output;
 
Servo myservo;//серво привод
 
PID myPID(&Input, &Output, &Setpoint,8.6,0.5,1, DIRECT);//создаем ПИД-регулятор
 
boolean revers;
 
void setup()
{
Setpoint = 20;//заданная температура в салоне автомобиля
Input = analogRead(0)/9.31;//читаем показания с датчика температуры LM35
myPID.SetOutputLimits(0, 180);//устанавливаем границы выходного сигнала для ПИД-регулятора
if (Setpoint<Input){//если начальная температура больше заданной
revers=true;
myPID.SetControllerDirection(REVERSE);//ПИД-регулятор используем обратный
}
myPID.SetMode(AUTOMATIC);//включаем ПИД-регулятор
}
 
void loop()
{
Input = analogRead(0)/9.31;//анализируем температуру салона
myPID.Compute();//считаем выходной сигнал ПИД-регулятора
myservo.attach(3,630,2540);//включаем серву на 3 цифровом пине, с длинной импульса от 630 до 2540(указываем границы поворота сервы)
if (revers)//если пид регулятор обратный, то сервой управляем также относительно противоположной крайней точки
myservo.write(180-Output);
else
myservo.write(Output);
delay(1000);//ждем серву
myservo.detach();//отключаемся от сервы
}
