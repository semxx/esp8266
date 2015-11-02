#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <CyberLib.h>

volatile int val1;
volatile int val2;

LiquidCrystal_I2C lcd(0x3f, 16, 2);

void setup() {
  Serial.begin(9600);
  D2_In; D2_High; D3_In; D3_High; //сконфигурировать входы и подтянуть к +питания  
  attachInterrupt(0, coldWater, FALLING);
  attachInterrupt(1, hotWater, FALLING);
  lcd.init();
  lcd.backlight();
  lcd.print("Water counter");
  lcd.setCursor(0, 1);
  lcd.print("Interrupt based");
  delay(1500);
  lcd.clear();
}

void loop() {

  lcd.setCursor(0, 0);
  lcd.print("COLD: ");
  lcd.print(val1);
  lcd.print(" L.");  
  lcd.setCursor(0, 1);
  lcd.print(" HOT: ");
  lcd.print(val2);
  lcd.print(" L.");  
}
 
void coldWater() {
  val1++;
}
void hotWater() {
  val2++;
}
