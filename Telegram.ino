#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space

#include <ESP8266WiFiMulti.h>
#include <ESP8266TelegramBOT.h>

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
ESP8266WiFiMulti wifiMulti;

// Initialize Telegram BOT
#define BOTtoken "165672905:AAFhk3XgHITZGDA_M2XEoxAhFaOdxl1Wf6Q"  //token of TestBOT
#define BOTname "Neato"
#define BOTusername "neatobot"
ESP8266TelegramBOT bot(BOTtoken, BOTname, BOTusername);
int Bot_mtbs=1000; //mean time between scan messages
long Bot_lasttime; //last time a scan of messages has been done
int Cook_mtbr = 20 * 60000; // time between Cookies refresh
long Cook_lasttime; //last time Cokies have been refreshed
bool Start = false;
// End of initialize Telegram BOT

#define ONE_WIRE_BUS 12  //Data wire plugged to pin 12 (temp sensor)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

char auth[] = "77804dfb38444bb381b9bd01145af4f3";
//Auth Tokens for any additional projects
char auth1[] = "dac663bea5ae4b26b165d8d2d02e5969"; 

float TempC_0 = 0;
float TempC_1 = 0;
float DHT_HYM = 0;
float DHT_TMP = 0;

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
 

  timer.setInterval(60000L, current_time);
  timer.setInterval(15000L, dhtRead);
  timer.setInterval(15000L, readDallas);
  timer.setInterval(60000L, sendThingSpeak);
  timer.setInterval(10000L, TelegramCheck);
  //timer.setInterval(1000L, Bot_ExecMessages);
  
  pinMode(2, OUTPUT); // initialize digital pin 13 as an output.
  bot.begin();      // launch Bot functionalities
}

void current_time()
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

 Blynk.run();

}
void dhtRead()
{
  float exec_time = millis();

 Blynk.run();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

 Blynk.run();


  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
     DHT_HYM = h;
     DHT_TMP = t;
    
     Blynk.virtualWrite(10, t); // virtual pin 
     Blynk.virtualWrite(11, h); // virtual pin   

 Blynk.run();  
 
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

 Blynk.run();

}

void readDallas()
{
  float exec_time = millis();
  sensors.requestTemperatures();
 Blynk.run();
  TempC_0 = sensors.getTempCByIndex(0);
  TempC_1 = sensors.getTempCByIndex(1);
 Blynk.run();
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

 Blynk.run();

}

void sendThingSpeak()
{
  float exec_time = millis();
    // Thingspeak code start here
    if (client.connect(server,80)) {  //   "184.106.153.149" or api.thingspeak.com
Blynk.run();
    String postStr = apiKey;
           postStr +="&field1=";
           postStr += String(DHT_TMP);
           postStr +="&field2=";
           postStr += String(TempC_0);
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

 Blynk.run();
     
     Serial.print("Indoor: ");
     Serial.print(DHT_TMP);
     Serial.print(" C Outdoor: ");
     Serial.print(TempC_0);
     Serial.print(" Humidity: ");
     Serial.print(DHT_HYM);
     Serial.println(" send to Thingspeak");

 
  }
 
  client.stop();
    exec_time = millis() - exec_time ; 
    Serial.print("Send to Thingspeak execute time is: ");
    Serial.println(exec_time);
    exec_time = 0;
 Blynk.run();
}

void loop()
{
  Blynk.run();
  timer.run();
}
/********************************************
 * EchoMessages - function to Echo messages *
 ********************************************/   
/*
void Bot_EchoMessages() {

  for (int i=1; i<bot.message[0][0].toInt()+1; i++)      {
          bot.sendMessage(bot.message[i][4], bot.message[i][5],"");
  }
  bot.message[0][0]="";     // stop analize messages
}

*/
void TelegramCheck()  {
  
  float exec_time = millis();
// if (millis() > Bot_lasttime + Bot_mtbs)  {
    bot.getUpdates(bot.message[0][1]);
  //  Bot_EchoMessages();
      Bot_ExecMessages();    
  //  Bot_lasttime = millis();
//  }

  // Cookies need to be refreshed sometimes
  if (millis() > Cook_lasttime + Cook_mtbr)  {
    //Serial.println("Get Cookies");
    if (!bot.getCookies()) bot.getCookies(); 
    Cook_lasttime = millis();
  }
    exec_time = millis() - exec_time ; 
    Serial.print("TelegramCheck execute time is: ");
    Serial.println(exec_time);
    exec_time = 0;
}
void Bot_ExecMessages() {
float exec_time = millis();
  for (int i = 1; i < bot.message[0][0].toInt() + 1; i++)      {
    bot.message[i][5]=bot.message[i][5].substring(1,bot.message[i][5].length());
    if (bot.message[i][5] == "\/ledon") {
      digitalWrite(2, !HIGH);   // turn the LED on (HIGH is the voltage level)

      bot.sendMessage(bot.message[i][4], "Led is ON", "");
    }
    if (bot.message[i][5] == "\/ledoff") {
      digitalWrite(2, !LOW);    // turn the LED off (LOW is the voltage level)
      bot.sendMessage(bot.message[i][4], "Led is OFF", "");
    }
    if (bot.message[i][5] == "\/temp") {
 Blynk.run();
      //digitalWrite(2, !LOW);    // turn the LED off (LOW is the voltage level)
        char t_buffer[15];
        char h_buffer[15];
        char o_buffer[15];

        dtostrf(DHT_TMP, 4, 2, t_buffer);
        dtostrf(DHT_HYM, 4, 2, h_buffer);
        dtostrf(TempC_0, 4, 2, o_buffer);
  
  char str1[20];
  strcpy (str1,"Outdoor t° is: ");
  strcat (str1,o_buffer);
  puts (str1);
 Blynk.run();
  bot.sendMessage(bot.message[i][4], str1, "");
   
  char str2[20];
  strcpy (str2,"Indoor t° is: ");
  strcat (str2, t_buffer);
  puts (str2);
 Blynk.run();
  bot.sendMessage(bot.message[i][4], str2, "");
   
  char str3[20];
  strcpy (str3,"Hymidity is: ");
  strcat (str3, h_buffer);
  puts (str3);
 Blynk.run();
  bot.sendMessage(bot.message[i][4], str3, "");

    }
    if (bot.message[i][5] == "\/start") {
      String wellcome = "Wellcome from LedBot, your personal Bot on Arduino board";
      String wellcome1 = "/ledon : to switch the Led ON";
      String wellcome2 = "/ledoff : to switch the Led OFF";
      bot.sendMessage(bot.message[i][4], wellcome, "");
      bot.sendMessage(bot.message[i][4], wellcome1, "");
      bot.sendMessage(bot.message[i][4], wellcome2, "");
      Start = true;
    }
    if (bot.message[i][5] == "\/reset") {
      bot.sendMessage(bot.message[i][4], "ESP is going to restart...", "");
      bot.message[0][0] = "";   // All messages have been replied - reset new messages
      delay(1000);
      //ESP.reset();
    }
  }
  bot.message[0][0] = "";   // All messages have been replied - reset new messages

    exec_time = millis() - exec_time ; 
    Serial.print("Bot_ExecMessages execute time is: ");
    Serial.println(exec_time);
    exec_time = 0;
 Blynk.run();
}

 BLYNK_WRITE(1)  // Manual restart ESP 
 {
  int a = param.asInt();
    if (a == 0) 
  {
    ESP.reset();
  }
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

/*
 BLYNK_READ(0) // Led at V0 as Input
{
if (!digitalRead(2) == HIGH) { // if your digital input is high
    led1.on();
  } else {
    led1.off();
}
}*/ 
