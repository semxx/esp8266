//#define BLYNK_DEBUG
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space

//#define HOME //uncomment for home server
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <SimpleTimer.h>
#include <NTPtime.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <Wire.h>
#include <WiFiClientSecure.h>
#include <ESP8266TelegramBOT.h>

#define TIMEZONE 3
#define DAYLIGHTSAVINGTIME 0

#define DHTPIN 13 //pin gpio 13 in sensor
#define DHTTYPE DHT22
#define interruptPIN 15

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

#ifdef HOME
#define AUTH2 "...."
#else
#define AUTH4 "...."
#endif

#define ONE_WIRE_BUS 12  //Data wire plugged to pin 12 (temp sensor)

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

char auth[] = "77804dfb38444bb381b9bd01145af4f3";
char auth1[] = "dac663bea5ae4b26b165d8d2d02e5969"; //Auth Tokens for any additional projects
WidgetBridge bridge1(1);
//WidgetLED led1(0);

// Initialize Telegram BOT
#define BOTtoken "165672905:AAFhk3XgHITZGDA_M2XEoxAhFaOdxl1Wf6Q"  //token of TestBOT
#define BOTname "Neato"
#define BOTusername "neatobot"

TelegramBOT bot(BOTtoken, BOTname, BOTusername);
// End of initialize Telegram BOT

float TempC_0 = 0;
float TempC_1 = 0;
float DHT_HYM = 0;
float DHT_TMP = 0;
float BMP_PRESURE = 0;
float BMP_ALTITUDE = 0;
float BMP_TEMPERATURE = 0;

const int Pin = 0;
const int Pin2 = 2;
volatile int state0 = 0;
volatile int state2 = 0;

const char* thingSpeakAddress = "api.thingspeak.com";
String thingSpeakAPIKey = "VQ0584HOI2VHUYSH";

WiFiClient client;

SimpleTimer timer;
 
void setup()
{
  Serial.begin(9600);
// pinMode(interruptPIN, INPUT_PULLUP);
// attachInterrupt(digitalPinToInterrupt(interruptPIN), interruptHandler, CHANGE);

  pinMode(Pin, INPUT_PULLUP);
  pinMode(Pin2, INPUT_PULLUP);
 
//  attachInterrupt(Pin, coldimpulse, FALLING);
//  attachInterrupt(Pin2, hotimpulse, FALLING);
  delay(50);
	
  Blynk.begin(auth, "Mimimi", "panatorium");

  while (Blynk.connect() == false) {
    // Wait until connected
  }
  
   Blynk.notify("Device started");
// pinMode(2, OUTPUT); // initialize digital pin 13 as an output. 
   bridge1.setAuthToken(auth1);

   timer.setInterval(60000L, current_time);
   timer.setInterval(15000L, dhtRead);
   timer.setInterval(15000L, readDallas);
   timer.setInterval(60000L, sendThingSpeak);
   timer.setInterval(10000L, TelegramCheck);
   timer.setInterval(20000L, GetPressure);
// timer.setInterval(5000L, Bot_ExecMessages);
  
  bot.begin();      // launch Bot functionalities
  sensors.begin();
  dht.begin();

  Serial.println("Pressure Sensor Test"); Serial.println(""); 
  /* Initialise sensor BMP085*/
  if(!bmp.begin())
  {
    /* There was a problem detecting the BMP085 ... check your connections */
    Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }

  displaySensorDetails();

}

void interruptHandler()
{
 //static unsigned long last_interrupt_time = 0;
 //unsigned long interrupt_time = millis();
// if (interrupt_time - last_interrupt_time > 100)
// {
  Serial.println("Interrupt detected");
 //}
 //last_interrupt_time = interrupt_time;
}

void sendThingSpeak(){
updateThingSpeak("field1=" + String(DHT_TMP) + "&field2=" + String(TempC_0) + "&field3=" + String(DHT_HYM) + "&field4=" + String(BMP_PRESURE) + "&field5=" + String(BMP_ALTITUDE) + "&field6=" + String(BMP_TEMPERATURE));
}

void updateThingSpeak(String tsData) {
  WiFiClient client;
  if (!client.connect(thingSpeakAddress, 80)) {
    return;
  }
   client.print(F("GET /update?key="));
   client.print(thingSpeakAPIKey);
   client.print(F("&"));
   client.print(tsData);
   client.print(F(" HTTP/1.1\r\nHost: api.thingspeak.com\r\n\r\n"));
   client.println();
// Serial output
   Serial.print("Indoor: ");
   Serial.print(DHT_TMP);
   Serial.print(" C Outdoor: ");
   Serial.print(TempC_0);
   Serial.print(" Humidity: ");
   Serial.print(DHT_HYM);
   Serial.print(" Presure: ");
   Serial.print(BMP_PRESURE);
   Serial.print(" Altitude: ");
   Serial.print(BMP_ALTITUDE);
   Serial.print(" BMP_Temp: ");
   Serial.print(BMP_TEMPERATURE);
   Serial.println(" send to Thingspeak");
  
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
  
      bot.getUpdates(bot.message[0][1]);
      Bot_ExecMessages();    

}
void Bot_ExecMessages(){

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
    if (bot.message[i][5] == "\/weather") {
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

  bot.sendMessage(bot.message[i][4], str1, "");
 Blynk.run();    
  char str2[20];
  strcpy (str2,"Indoor t° is: ");
  strcat (str2, t_buffer);
  puts (str2);

  bot.sendMessage(bot.message[i][4], str2, "");
 Blynk.run();   
  char str3[20];
  strcpy (str3,"Hymidity is: ");
  strcat (str3, h_buffer);
  puts (str3);
  bot.sendMessage(bot.message[i][4], str3, "");
 Blynk.run();
    }
    if (bot.message[i][5] == "\/start") {
      String wellcome = "Wellcome from LedBot, your personal Bot on Arduino board";
      String wellcome1 = "/ledon : to switch the Led ON";
      String wellcome2 = "/ledoff : to switch the Led OFF";
      bot.sendMessage(bot.message[i][4], wellcome, "");
      bot.sendMessage(bot.message[i][4], wellcome1, "");
      bot.sendMessage(bot.message[i][4], wellcome2, "");
    }
    if (bot.message[i][5] == "\/reset") {
      bot.sendMessage(bot.message[i][4], "ESP is going to restart...", "");
      bot.message[0][0] = "";   // All messages have been replied - reset new messages
      delay(1000);
      ESP.reset();
    }
  }
  bot.message[0][0] = "";   // All messages have been replied - reset new messages

    exec_time = millis() - exec_time ; 
    Serial.print("Bot_ExecMessages execute time is: ");
    Serial.println(exec_time);
    exec_time = 0;
 Blynk.run();
}

void GetPressure()
{
  sensors_event_t event;
  bmp.getEvent(&event);

if (event.pressure)
  {
    Serial.print("Pressure:    ");
    Serial.print(event.pressure*0.75006375541921);
    Serial.println(" mmHg");
    float temperature;
    bmp.getTemperature(&temperature);
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");

    /* Then convert the atmospheric pressure, and SLP to altitude         */
    /* Update this next line with the current SLP for better results      */
    float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
    Serial.print("Altitude:    "); 
    Serial.print(bmp.pressureToAltitude(seaLevelPressure, event.pressure)); 
    Serial.println(" m");
    Serial.println("");
    BMP_PRESURE = event.pressure*0.75006375541921;
    BMP_ALTITUDE = (bmp.pressureToAltitude(seaLevelPressure, event.pressure));
    BMP_TEMPERATURE = temperature;
  Blynk.virtualWrite(V8, BMP_PRESURE);
  Blynk.virtualWrite(V9, BMP_ALTITUDE);
  Blynk.virtualWrite(V13, BMP_TEMPERATURE);
  Blynk.run();
  }
  else
  {
    Serial.println("Sensor error");
  }
}

void displaySensorDetails(void)
{
  sensor_t sensor;
  bmp.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" hPa");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" hPa");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" hPa");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
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

 BLYNK_WRITE(1)  // Manual restart ESP 
 {
  int a = param.asInt();
    if (a == 0) 
  {
    ESP.reset();
  }
 }
 
void coldimpulse() {
  state0++;
  Serial.println(state0);
}

void hotimpulse() {
  state2++;
  Serial.println(state2);
}
/*
void sendThingSpeak()
{
  float exec_time = millis();
    // Thingspeak code start here
    if (client.connect(thingSpeakAddress,80)) {  //   "184.106.153.149" or api.thingspeak.com
Blynk.run();
    String postStr = thingSpeakAPIKey;
           postStr +="&field1=";
           postStr += String(DHT_TMP);
           postStr +="&field2=";
           postStr += String(TempC_0);
           postStr +="&field3=";
           postStr += String(DHT_HYM);
           postStr +="&field4=";
           postStr += String(BMP_PRESURE);
           postStr +="&field5=";
           postStr += String(BMP_ALTITUDE); 
           postStr +="&field6=";
           postStr += String(BMP_TEMPERATURE); 
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
     Serial.print(" Presure: ");
     Serial.print(BMP_PRESURE);
     Serial.print(" Altitude: ");
     Serial.print(BMP_ALTITUDE);
     Serial.print(" BMP_Temp: ");
     Serial.print(BMP_TEMPERATURE);
     Serial.println(" send to Thingspeak");

  }
 
  client.stop();
    exec_time = millis() - exec_time ; 
    Serial.print("Send to Thingspeak execute time is: ");
    Serial.println(exec_time);
    exec_time = 0;
 Blynk.run();
}
*/
 
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
