// last save 01.12.2015
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <SimpleTimer.h>
#include <NTPtime.h>


#define TIMEZONE 3
#define DAYLIGHTSAVINGTIME 0
#define DHTPIN 13 //pin gpio 13 in sensor
#define DHTTYPE DHT22   // DHT 22 Change this if you have a DHT11
DHT dht(DHTPIN, DHTTYPE);

#define ONE_WIRE_BUS 12  //Data wire plugged to pin 12 (temp sensor)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

char auth[] = "77804dfb38444bb381b9bd01145af4f3";
//Auth Tokens for any additional projects
char auth1[] = "dac663bea5ae4b26b165d8d2d02e5969"; 

float TempC_0 = 0;
float TempC_1 = 0;
float DHT_HYM = 0;

//WidgetBridge bridge1(1);
//WidgetLED led1(0);

const char* server = "api.thingspeak.com";
String apiKey = "VQ0584HOI2VHUYSH";

WiFiClient client;

SimpleTimer timer;
 
void setup()
{
  Serial.begin(9600);
  sensors.begin();
  dht.begin();
  Blynk.begin(auth, "Mimimi", "panatorium");

  while (Blynk.connect() == false) {
    // Wait until connected
  }
  Blynk.notify("Device started");
//  bridge1.setAuthToken(auth1);
 

  timer.setInterval(60000L, CurrentTime);
  timer.setInterval(10000L, dhtRead);
  timer.setInterval(10000L, readDallas);
  timer.setInterval(60000L, sendThingSpeak);

}

void CurrentTime()
{
  unsigned long currentMillis = millis();
  unsigned long currentTime = getTime(TIMEZONE, DAYLIGHTSAVINGTIME);

  int hours = (currentTime  % 86400L) / 3600;
  int minutes = (currentTime % 3600) / 60;
  int seconds = (currentTime % 60);
  char timeString[8];
  sprintf(timeString,"%02d:%02d:%02d",hours, minutes, seconds);
  //BLYNK_LOG("The time is %s", timeString);       // UTC is the time at Greenwich Meridian (GMT)
  Blynk.virtualWrite(V12, timeString );
  Serial.println(timeString);

}
void dhtRead()
{
  float exec_time = millis();
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  


  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
     DHT_HYM = h;
    
     Blynk.virtualWrite(10, t); // virtual pin 
     Blynk.virtualWrite(11, h); // virtual pin   
  
     Serial.print("DHT humidity: ");
     Serial.print(h);
     Serial.print(" %\t");
     Serial.print("DHT temperature: ");
     Serial.print(t);
     Serial.println(" C ");
 exec_time = millis() - exec_time ; 
    Serial.print("dhtRead execute time is: ");
    Serial.println(exec_time);
exec_time = 0;
}

void readDallas()
{
  float exec_time = millis();
  sensors.requestTemperatures();
  TempC_0 = sensors.getTempCByIndex(0);
  TempC_1 = sensors.getTempCByIndex(1);
  char t_buffer[15];
  char h_buffer[15];

  dtostrf(TempC_0, 4, 2, t_buffer);
  dtostrf(TempC_1, 4, 2, h_buffer);
  Blynk.virtualWrite(V5, t_buffer);
  Blynk.virtualWrite(V6, h_buffer);
  Blynk.virtualWrite(V7, millis() / 60000L);  // Send UpTime
 exec_time = millis() - exec_time ; 
    Serial.print("Read Dallas execute time is: ");
    Serial.println(exec_time);
exec_time = 0;
}

void sendThingSpeak()
{
  float exec_time = millis();
    // Thingspeak code start here
    if (client.connect(server,80)) {  //   "184.106.153.149" or api.thingspeak.com
    String postStr = apiKey;
           postStr +="&field1=";
           postStr += String(TempC_0);
           postStr +="&field2=";
           postStr += String(TempC_1);
           postStr +="&field3=";
           postStr += String(DHT_HYM);
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
     Serial.print("Indoor: ");
     Serial.print(TempC_0);
     Serial.print(" C Outdoor: ");
     Serial.print(TempC_1);
     Serial.print(" Humidity: ");
     Serial.print(DHT_HYM);
     Serial.println(" send to Thingspeak");
      
  }
 
  client.stop();
 exec_time = millis() - exec_time ; 
    Serial.print("Send to Thingspeak execute time is: ");
    Serial.println(exec_time);
exec_time = 0;
}

void loop()
{
  Blynk.run();
  timer.run();
}

/*
void blynkAnotherDevice()
{  
digitalWrite(2,!digitalRead(13));

      if (digitalRead(13) ==! HIGH)
       //led1.off();
       bridge1.digitalWrite(1, HIGH);
      else
        //led1.on();
        bridge1.digitalWrite(1, LOW);
}
*/
/*
BLYNK_WRITE(2) 
{
  int a = param.asInt();
  if (a == 0) 
  {
    bridge1.digitalWrite(0, LOW); //Turn Off pin 0 on the other device
  } else 
    {
      bridge1.digitalWrite(0, HIGH);//Turns on pin 0 on the other device  
    }
 }
*/
 BLYNK_WRITE(1)  // Manual restart ESP 
 {
  int a = param.asInt();
    if (a == 0) 
  {
    ESP.reset();
  }
 }
/*
 BLYNK_READ(0) // Led at V0 as Input
{
if (!digitalRead(2) == HIGH) { // if your digital input is high
    led1.on();
  } else {
    led1.off();
}
}*/ 
