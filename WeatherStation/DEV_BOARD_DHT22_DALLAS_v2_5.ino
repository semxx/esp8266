//#define BLYNK_DEBUG
#define BLYNK_PRINT Serial    // Comment this out to disable printsand save space

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

//#define DEBUG //uncomment for debug
#define TIMEZONE 3
#define DAYLIGHTSAVINGTIME 0

#define DHTPIN 13 //pin gpio 13 in sensor
#define DHTTYPE DHT22


DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

//#ifdef DEBUG
//#define AUTH2 "...."
//#else
//#define AUTH4 "...."
//#endif

#define ONE_WIRE_BUS 12  //Data wire plugged to pin 12 (temp sensor)

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

char auth[] = "77804dfb38444bb381b9bd01145af4f3";
//char auth1[] = "dac663bea5ae4b26b165d8d2d02e5969"; //Auth Tokens for any additional projects
//WidgetBridge bridge1(1);
//WidgetLED led1(0);

// Initialize Telegram BOT
#define BOTtoken "165672905:AAFhk3XgHITZGDA_M2XEoxAhFaOdxl1Wf6Q"
#define BOTname "Neato"
#define BOTusername "neatobot"

TelegramBOT bot(BOTtoken, BOTname, BOTusername);
// End of initialize Telegram BOT

String VoidName = "";
float start_time = 0;
float TempC_0 = 0;
float TempC_1 = 0;
float DHT_HYM = 0;
float DHT_TMP = 0;
float BMP_PRESURE = 0;
float BMP_ALTITUDE = 0;
float BMP_TEMPERATURE = 0;


const char* thingSpeakAddress = "api.thingspeak.com";
String thingSpeakAPIKey = "VQ0584HOI2VHUYSH";

WiFiClient client;

SimpleTimer timer;

void setup()
{
  Serial.begin(9600);
  delay(50);
  Blynk.begin(auth, "Mimimi", "panatorium");

  while (Blynk.connect() == false) {
    // Wait until connected
  }
  
   Blynk.notify("Device started");

// Define timers

   timer.setInterval(75000L, current_time);
   timer.setInterval(45000L, dhtRead);
   timer.setInterval(40000L, readDallas);
   timer.setInterval(60000L, sendThingSpeak);
   timer.setInterval(10000L, TelegramCheck);
   timer.setInterval(50000L, GetPressure);
   
   bot.begin();      // launch Bot functionalities
   sensors.begin();
   dht.begin();

  Serial.println("Pressure Sensor Test");
  if(!bmp.begin())
  {
    Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  
  #ifdef DEBUG
    displaySensorDetails();
  #endif

// bridge1.setAuthToken(auth1);
}

void sendThingSpeak(){
  
Metric_Start("sendThingSpeak");

  updateThingSpeak(thingSpeakAPIKey + "&field1=" + String(DHT_TMP) + "&field2=" + String(TempC_0) + "&field3=" + String(DHT_HYM) + "&field4=" + String(BMP_PRESURE) + "&field5=" + String(BMP_ALTITUDE) + "&field6=" + String(BMP_TEMPERATURE));

Metric_End();

}


void dhtRead()
{
Metric_Start("dht_Read");

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

  #ifdef DEBUG
     Serial.print("DHT humidity: ");
     Serial.print(h);
     Serial.print(" %\t");
     Serial.print("DHT temperature: ");
     Serial.print(t);
     Serial.println(" C ");
  #endif

 Blynk.run();
 Metric_End();
}

void readDallas()
{
Metric_Start("read_Dallas");

  sensors.requestTemperatures();
  Blynk.run();
  TempC_0 = sensors.getTempCByIndex(0);
  TempC_1 = sensors.getTempCByIndex(1);

  char t_buffer[15];
  char h_buffer[15];

  dtostrf(TempC_0, 4, 2, t_buffer);
  dtostrf(TempC_1, 4, 2, h_buffer);
  Blynk.virtualWrite(V5, t_buffer);
  Blynk.virtualWrite(V6, h_buffer);
  Blynk.virtualWrite(V7, millis() / 60000L);  // Send UpTime


 Blynk.run();
 Metric_End();
}

void loop()
{
  Blynk.run();
  timer.run();
}

void TelegramCheck()  {
  
  Metric_Start("Telegram_Check");
  
      bot.getUpdates(bot.message[0][1]);
      Bot_ExecMessages();
    
  Metric_End();
}
void Bot_ExecMessages(){
  for (int i = 1; i < bot.message[0][0].toInt() + 1; i++)      {
 //   String EchoResponse = bot.message[i][5];
 //   EchoResponse = UnicodeASCII(EchoResponse);
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
      bot.getUpdates(bot.message[0][1]);
      delay(5000);
      ESP.restart();
    }
//    else {  bot.sendMessage(bot.message[i][4], EchoResponse, "");}

  }

      bot.message[0][0] = "";   // All messages have been replied - reset new messages

 Blynk.run();
}

void GetPressure()
{
  Metric_Start("Get_Pressure");
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
  Metric_End();

}

void updateThingSpeak(String tsData) {
  WiFiClient client;
  if (!client.connect(thingSpeakAddress, 80)) {
    return;
  }
   Blynk.run();
   client.print("POST /update HTTP/1.1\n");
   client.print("Host: api.thingspeak.com\n");
   client.print("Connection: close\n");
   client.print("X-THINGSPEAKAPIKEY: "+thingSpeakAPIKey+"\n");
   client.print("Content-Type: application/x-www-form-urlencoded\n");
   client.print("Content-Length: ");
   client.print(tsData.length());
   client.print("\n\n");
   client.print(tsData);
   client.stop();
   
  #ifdef DEBUG
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
   #endif   
   
Blynk.run();
  }

void displaySensorDetails(void)
{
  sensor_t sensor;
  bmp.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value);
Serial.println(" hPa");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value);
Serial.println(" hPa");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution);
Serial.println(" hPa");
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

void current_time()
{
Metric_Start("CurrentTime");
  unsigned long currentMillis = millis();
  unsigned long currentTime = getTime(TIMEZONE, DAYLIGHTSAVINGTIME);

  int hours = (currentTime  % 86400L) / 3600;
  int minutes = (currentTime % 3600) / 60;
  int seconds = (currentTime % 60);
  char timeString[8];
  sprintf(timeString,"%02d:%02d:%02d",hours, minutes, seconds);
  //BLYNK_LOG("The time is %s", timeString);       // UTC is the timeat Greenwich Meridian (GMT)
  Blynk.virtualWrite(V12, timeString );

  // Serial.println(timeString);

 Blynk.run();
 Metric_End();
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
 String UnicodeASCII(String input) {
      String s = input;
  //  Serial.print("Input value in function replace: ");
  //  Serial.println(s);
    s.replace("\\u0410", "А");
    s.replace("\\u0411", "Б");
    s.replace("\\u0412", "В");
    s.replace("\\u0413", "Г");
    s.replace("\\u0414", "Д");
    s.replace("\\u0415", "Е");
    s.replace("\\u0416", "Ж");
    s.replace("\\u0417", "З");
    s.replace("\\u0418", "И");
    s.replace("\\u0419", "Й");
    s.replace("\\u041a", "К");
    s.replace("\\u041b", "Л");
    s.replace("\\u041c", "М");
    s.replace("\\u041d", "Н");
    s.replace("\\u041e", "О");
    s.replace("\\u041f", "П");
    s.replace("\\u0420", "Р");
    s.replace("\\u0421", "С");
    s.replace("\\u0422", "Т");
    s.replace("\\u0423", "У");
    s.replace("\\u0424", "Ф");
    s.replace("\\u0425", "Х");
    s.replace("\\u0426", "Ц");
    s.replace("\\u0427", "Ч");
    s.replace("\\u0428", "Ш");
    s.replace("\\u0429", "Щ");
    s.replace("\\u042a", "Ъ");
    s.replace("\\u042b", "Ы");
    s.replace("\\u042c", "Ь");
    s.replace("\\u042d", "Э");
    s.replace("\\u042e", "Ю");
    s.replace("\\u042f", "Я");
    s.replace("\\u0430", "а");
    s.replace("\\u0431", "б");
    s.replace("\\u0432", "в");
    s.replace("\\u0433", "г");
    s.replace("\\u0434", "д");
    s.replace("\\u0435", "е");
    s.replace("\\u0436", "ж");
    s.replace("\\u0437", "з");
    s.replace("\\u0438", "и");
    s.replace("\\u0439", "й");
    s.replace("\\u043a", "к");
    s.replace("\\u043b", "л");
    s.replace("\\u043c", "м");
    s.replace("\\u043d", "н");
    s.replace("\\u043e", "о");
    s.replace("\\u043f", "п");
    s.replace("\\u0440", "р");
    s.replace("\\u0441", "с");
    s.replace("\\u0442", "т");
    s.replace("\\u0443", "у");
    s.replace("\\u0444", "ф");
    s.replace("\\u0445", "х");
    s.replace("\\u0446", "ц");
    s.replace("\\u0447", "ч");
    s.replace("\\u0448", "ш");
    s.replace("\\u0449", "щ");
    s.replace("\\u044a", "ъ");
    s.replace("\\u044b", "ы");
    s.replace("\\u044c", "ь");
    s.replace("\\u044d", "э");
    s.replace("\\u044e", "ю");
    s.replace("\\u044f", "я");
    s.replace("\\u0401", "Ё");
    s.replace("\\u0451", "ё");
  //Serial.print("Replaced: ");
  //Serial.println(s);
  return s;
}
*/
void Metric_Start(String vName) 
{
#ifdef DEBUG
  VoidName = vName;
   start_time = 0; 
   start_time = millis();
#endif
}
void Metric_End()
{
#ifdef DEBUG
  start_time = millis() - start_time ;
  Serial.print(VoidName + " execute time is: ");
  Serial.println(start_time);
    if (start_time > 7000) {
      Serial.println("WARNING: "+ VoidName +" execute time is too long: (" + start_time + " ms)");

    }
float start_time = 0; 
#endif
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
