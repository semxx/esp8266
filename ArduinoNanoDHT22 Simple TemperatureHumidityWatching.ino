#include "DHT.h" 
#include <SPI.h>
#include <SoftwareSerial.h>
SoftwareSerial SwSerial(2, 3);
#define BLYNK_PRINT SwSerial
#include <BlynkSimpleSerial.h>
#define DHTPIN 4 // The pin you've connected Data from the DHT to your Arduino
#define DHTTYPE DHT22   // DHT11 or DHT22
DHT dht(DHTPIN, DHTTYPE); 
#include <SimpleTimer.h>

SimpleTimer timer;

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "#################";

void setup()
{
  Serial.begin(9600);
  Blynk.begin(auth);
  timer.setInterval(2000, sendDHT);
}

void sendDHT()
{
//Read the Temp and Humidity from DHT
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Some Ways for Data-Output
  Blynk.virtualWrite(4, h); 
  Blynk.virtualWrite(5, t);
  Blynk.virtualWrite(6, h);
  Blynk.virtualWrite(7, t);
  
