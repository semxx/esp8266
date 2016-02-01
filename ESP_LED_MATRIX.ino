#include <SPI.h>
#include "LedMatrix.h"


#define NUMBER_OF_DEVICES 6
#define CS_PIN 2
LedMatrix ledMatrix = LedMatrix(NUMBER_OF_DEVICES, CS_PIN);
int x = 0;
  
void setup() {
  ledMatrix.init();
  ledMatrix.setIntensity(2); // range is 0-15
  ledMatrix.setText("Indoor:21.2");
  ledMatrix.setNextText("Outdoor:-1.4");
}

void loop() {

  ledMatrix.clear();
  ledMatrix.scrollTextLeft();
  ledMatrix.drawText();
  ledMatrix.commit();
  delay(50);
  x=x+2;
  if (x == 400) {
     ledMatrix.setNextText("Humidity:43%"); 
  }
}
