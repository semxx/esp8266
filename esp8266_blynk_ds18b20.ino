#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>

#define ONE_WIRE_BUS 4 //Data wire plugged to pin 4 (DS18B20)

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
char auth[] = "77804dfb38444bb381b9bd01145af4f3";
 
// Thingspeak code part I
const char* server = "api.thingspeak.com";
String apiKey = "VQ0584HOI2VHUYSH";
WiFiClient client;
// End of Thingspeak code part I
 
SimpleTimer timer;
 
void setup()
{
  Serial.begin(9600);
  Blynk.begin(auth, "Mimimi", "panatorium");
  sensors.begin();
 
  timer.setInterval(15000, readTemp);

}
 
 
void loop()
{
  Blynk.run();
  timer.run();
}
 
 
void readTemp()
{
  sensors.requestTemperatures();
  float floatTempC_0 = sensors.getTempCByIndex(0);
  float floatTempC_1 = sensors.getTempCByIndex(1);
  char t_buffer[15];
  char h_buffer[15];
  dtostrf(floatTempC_0, 4, 2, t_buffer);
  Blynk.virtualWrite(V5, t_buffer);
  Blynk.run();
  dtostrf(floatTempC_1, 4, 2, h_buffer);
  Blynk.virtualWrite(V6, h_buffer);
  Blynk.run();
 // Send UpTime
  Blynk.virtualWrite(V7, millis() / 60000L);
  Blynk.run();
  // Thingspeak code start here
    if (client.connect(server,80)) {  //   "184.106.153.149" or api.thingspeak.com
    String postStr = apiKey;
           postStr +="&field1=";
           postStr += String(floatTempC_0);
           postStr +="&field2=";
           postStr += String(floatTempC_1);
           postStr += "\r\n\r\n";
 
     client.print("POST /update HTTP/1.1\n");
     client.print("Host: api.thingspeak.com\n");
     client.print("Connection: close\n");
     client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
     client.print("Content-Type: application/x-www-form-urlencoded\n");
     client.print("Content-Length: ");
     client.print(postStr.length());
     client.print("\n\n");
     client.print(postStr);
 
     Serial.print("Temperature: ");
     Serial.print(floatTempC_0);
     Serial.print(" degrees Celcius Humidity: ");
     Serial.print(floatTempC_1);
     Serial.println("% send to Thingspeak");
  }
  client.stop();
  // thingspeak needs minimum 15 sec delay between updates
//delay(20000);
   // End of Thingspeak code
 
}
