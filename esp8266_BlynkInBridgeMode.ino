#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>

#define ONE_WIRE_BUS 12  //Data wire plugged to pin 12 (temp sensor)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const char* server = "api.thingspeak.com";
String apiKey = "VQ0584HOI2VHUYSH";
char auth[] = "77804dfb38444bb381b9bd01145af4f3";

//Auth Tokens for any additional projects
char auth1[] = "1faaad888d3342658d96188230372ee2"; 

WidgetBridge bridge1(1);
WidgetLED led1(0);
//WidgetLED led2(1);

// Thingspeak code part I

WiFiClient client;
SimpleTimer timer;
 
void setup()
{
  pinMode(2,OUTPUT);
  pinMode(13,INPUT);
  
  Serial.begin(115200);
  sensors.begin();
  Blynk.begin(auth, "Mimimi", "panatorium");

  while (Blynk.connect() == false) {
    // Wait until connected
  }
  Blynk.notify("Device started");
  bridge1.setAuthToken(auth1);

  timer.setInterval(60000L, readTemp);
  //timer.setInterval(2000L, kitchenLight);  
  timer.setInterval(3000L, blynkAnotherDevice); 

}
//void kitchenLight()
//{
//  
//}
void readTemp()
{
  sensors.requestTemperatures();
  float floatTempC_0 = sensors.getTempCByIndex(0);
  float floatTempC_1 = sensors.getTempCByIndex(1);
  char t_buffer[15];
  char h_buffer[15];
  dtostrf(floatTempC_0, 4, 2, t_buffer);
  Blynk.virtualWrite(V5, t_buffer); // Send Int temp to application
  dtostrf(floatTempC_1, 4, 2, h_buffer);
  Blynk.virtualWrite(V6, h_buffer); // Send Ext temp to application

  Blynk.virtualWrite(V7, millis() / 60000L); // Send UpTime

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
	 
		   Serial.print("Indoor: ");
		   Serial.print(floatTempC_0);
		   Serial.print(" Outdoor: ");
		   Serial.println(floatTempC_1);
  }
  client.stop();
  // thingspeak needs minimum 15 sec delay between updates

}

 void blynkAnotherDevice()
{  

/*
Blynk.virtualWrite(V1, digitalRead(4));
      if (digitalRead(4) ==! LOW)
        led2.on();
      else
        led2.off();
*/

digitalWrite(2,!digitalRead(13));

      if (digitalRead(13) ==! HIGH)
       //led1.off();
       bridge1.digitalWrite(1, HIGH);
      else
        //led1.on();
        bridge1.digitalWrite(1, LOW);

}

BLYNK_WRITE(2) 
{
  int a = param.asInt();
  if (a == 0) {
	  
    bridge1.digitalWrite(0, LOW); //Turn Off pin 0 on the other device
    } else {

    bridge1.digitalWrite(0, HIGH);//Turns on pin 0 on the other device  
    }
 }

 BLYNK_WRITE(1)  // Manual restart ESP 
 {
  int a = param.asInt();
    if (a == 0) 
	{
		ESP.reset();
	}
 }
 
void loop()
{
  Blynk.run();
  timer.run();
}
