#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN            12    
#define DHTTYPE           DHT22

#include <OneWire.h> //+
#include <DallasTemperature.h> //+
#include <Wire.h> //+

OneWire myWire(2); // Set GPIO2 on ESP8266 as the oneWire interface.
DallasTemperature sensors(&myWire); //+

float TempC_0; //+
//float TempC_0, TempC_1; //Two sensors are applied in this case.

DHT_Unified dht(DHTPIN, DHTTYPE);

char auth[] = "xxxxxxxxxx";

SimpleTimer timer;

void setup()
{
  Serial.begin(9600);
  Blynk.begin(auth, "xxxxxx", "yyyyyy");

  dht.begin();
  sensors.begin();
  
  // Setup a function to be called every second
  timer.setInterval(5000, sendUptime);
}

void sendUptime()
{
 
 sensors.requestTemperatures();
  TempC_0=sensors.getTempCByIndex(0);
  Blynk.virtualWrite(V3, TempC_0);
  
  Blynk.virtualWrite(V5, millis() / 1000);
  
  sensors_event_t event;  

  dht.temperature().getEvent(&event);
  if (!isnan(event.temperature)) {
    Blynk.virtualWrite(V1, event.temperature);
  }
  dht.humidity().getEvent(&event);
  if (!isnan(event.relative_humidity)) {
    Blynk.virtualWrite(V2, event.relative_humidity);
  }
}

void loop()
{
  Blynk.run();
  timer.run();
}
