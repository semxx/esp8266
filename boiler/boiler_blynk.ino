//#define BLYNK_DEBUG
//#define DEBUG //uncomment for debug
#define BLYNK_PRINT Serial    // Comment this out to disable printsand save space
#define LCD_1602_ON
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <livolo.h>
#include <DHT.h>
#include <SimpleTimer.h>
#include <NTPtime.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <Wire.h>
#include <RCSwitch.h>

#ifdef LCD_1602_ON
  #include <LiquidCrystal_I2C.h>
  LiquidCrystal_I2C lcd(0x3F, 20, 4);
#endif

#define TIMEZONE 3
#define DAYLIGHTSAVINGTIME 0
#define DHTPIN 13
#define DHTTYPE DHT22
#define ONE_WIRE_BUS 12

Livolo livolo(14); // transmitter connected to pin #14

SimpleTimer timer;
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
RCSwitch mySwitch = RCSwitch();

//Socket 1416
char* socket1TriStateOn  = "FFFF0FFF0101";
char* socket1TriStateOff = "FFFF0FFF0110";
char* socket2TriStateOn  = "FFFF0FFF1001";
char* socket2TriStateOff = "FFFF0FFF1010";
char* socket3TriStateOn  = "FFFF0FF10001";
char* socket3TriStateOff = "FFFF0FF10010";

//Socket 1401
char* socket11TriStateOn  = "0FFFFFFF0101";
char* socket11TriStateOff = "0FFFFFFF0110";
char* socket12TriStateOn  = "0FFFFFFF1001";
char* socket12TriStateOff = "0FFFFFFF1010";
char* socket13TriStateOn  = "0FFFFFF10001";
char* socket13TriStateOff = "0FFFFFF10010";

char auth[]  = "b92e451545454a85b3602e7f432aeb66";
char auth1[] = "dac663bea5ae4b26b165d8d2d02e5969"; //Auth Tokens for any additional projects

//WidgetLCD lcd(1);

const char* thingSpeakAddress = "api.thingspeak.com";
String thingSpeakAPIKey = "VQ0584HOI2VHUYSH";

String VoidName = "";
float start_time = 0;
float TempC_0 = 0;
float TempC_1 = 0;
float DHT_HYM, DHT_TMP;
float BMP_PRESURE, BMP_ALTITUDE, BMP_TEMPERATURE;

void Start(String vName) {
    VoidName = vName; 
#ifdef DEBUG
   start_time = 0;
   start_time = millis();
#endif
}

void LCD(){
 // lcd.clear();   // initializing the LCD
  
  lcd.setCursor(0, 0);
  lcd.print("HOT:"); // Start Print text to Line 1
  lcd.setCursor(4, 0);
  lcd.print(TempC_0);
  
  lcd.setCursor(8, 0);
  lcd.print(" HY:"); // Start Print text to Line 1
  lcd.setCursor(12, 0);
  lcd.print(DHT_HYM); 

  lcd.setCursor(0, 1);
  lcd.print("OUT:"); // Start Print text to Line 1
  lcd.setCursor(4, 1);
  lcd.print(TempC_1);
  
  lcd.setCursor(8, 1);
  lcd.print(" IN:"); // Start Print text to Line 1
  lcd.setCursor(12, 1);
  lcd.print(BMP_TEMPERATURE); 
  
  }
void End() {
#ifdef DEBUG
  start_time = millis() - start_time ;
  Serial.println(VoidName + " execute time is: " + start_time);
    if (start_time > 5000) {    
      Serial.println("WARNING: "+ VoidName +" execute time is too long: (" + start_time + " ms)");
//        lcd.clear();
//        lcd.print(0, 0, VoidName);
//        lcd.print(0, 1, "Delayed: ");
//        lcd.print(9, 1, start_time);
    }
float start_time = 0;
#endif
}

void dhtRead()
{
Start("dht_Read");

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
     DHT_HYM = h;
     DHT_TMP = t;

     Blynk.virtualWrite(10, t); // virtual pin
     Blynk.virtualWrite(11, h); // virtual pin

  #ifdef DEBUG
     Serial.print("DHT humidity: ");
     Serial.print(h);
     Serial.print(" %\t");
     Serial.print("DHT temperature: ");
     Serial.print(t);
     Serial.println(" C ");
  #endif

 End();
}

void readDallas()
{
Start("read_Dallas");

    sensors.requestTemperatures();

    TempC_0 = sensors.getTempCByIndex(0);
    TempC_1 = sensors.getTempCByIndex(1);
    char t_buffer[15];
    char h_buffer[15];

    dtostrf(TempC_0, 4, 2, t_buffer);
    dtostrf(TempC_1, 4, 2, h_buffer);
    Serial.println(TempC_0);

    Blynk.virtualWrite(V5, t_buffer);
    Blynk.virtualWrite(V6, h_buffer);
    Blynk.virtualWrite(V7, millis() / 60000L);  // Send UpTime


 End();
}


void GetPressure()
{
  Start("Get_Pressure");
  sensors_event_t event;
  bmp.getEvent(&event);

if (event.pressure)
  {
    float temperature;
    bmp.getTemperature(&temperature);
    float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
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
  End();

}

void updateThingSpeak(String tsData) {
  WiFiClient client;
  yield();
  if (!client.connect(thingSpeakAddress, 80)) {
    return;
  }
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
  yield();
  }

void sendThingSpeak(){

Start("sendThingSpeak");

  updateThingSpeak(thingSpeakAPIKey + "&field1=" + String(DHT_TMP) + "&field2=" + String(TempC_0) + "&field3=" + String(DHT_HYM) + "&field4=" + String(BMP_PRESURE) + "&field5=" + String(BMP_ALTITUDE) + "&field6=" + String(BMP_TEMPERATURE));

End();

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
Start("CurrentTime");
  unsigned long currentMillis = millis();
  unsigned long currentTime = getTime(TIMEZONE, DAYLIGHTSAVINGTIME);

  int hours = (currentTime  % 86400L) / 3600;
  int minutes = (currentTime % 3600) / 60;
  int seconds = (currentTime % 60);
  char timeString[8];
  sprintf(timeString,"%02d:%02d:%02d",hours, minutes, seconds);
  //BLYNK_LOG("The time is %s", timeString);       // UTC is thetimeat Greenwich Meridian (GMT)
  Blynk.virtualWrite(V12, timeString );

  // Serial.println(timeString);

 Blynk.run();
 End();
}

 BLYNK_WRITE(0)  // Manual restart ESP
 {
  int a = param.asInt();
    if (a == 0)
  {
    ESP.restart();
  }
 }
 BLYNK_WRITE(1)  // RF433 Switch1
 {
  int a = param.asInt();
    if (a == 0)
  {
mySwitch.setPulseLength(179);
mySwitch.sendTriState(socket1TriStateOff);

  }
  else {
    mySwitch.setPulseLength(179);
    mySwitch.sendTriState(socket1TriStateOn);}
 }
 BLYNK_WRITE(2)  // RF433 Switch2
 {
  int a = param.asInt();
    if (a == 0)
  {
    mySwitch.setPulseLength(179);
    mySwitch.sendTriState(socket2TriStateOff);

  }
  else {
    mySwitch.setPulseLength(179);
    mySwitch.sendTriState(socket2TriStateOn);}
 }
 BLYNK_WRITE(3)  // RF433 Switch3
 {
  int a = param.asInt();
    if (a == 0)
  {
    mySwitch.setPulseLength(179);
    mySwitch.sendTriState(socket3TriStateOff);

  }
  else {    
    mySwitch.setPulseLength(179);
    mySwitch.sendTriState(socket3TriStateOn);}
 }
//////////////////////////////////////////////////////

 /////////////////////////////////////////////////////
  BLYNK_WRITE(14)  // RF433 Switch11
 {
  int a = param.asInt();
    if (a == 0)
  {
    mySwitch.setPulseLength(176);
    mySwitch.sendTriState(socket11TriStateOff);
  }
  else {
    mySwitch.setPulseLength(176);
    mySwitch.sendTriState(socket11TriStateOn);}
 }
 BLYNK_WRITE(15)  // RF433 Switch12
 {
  int a = param.asInt();
    if (a == 0)
  {
    mySwitch.setPulseLength(176);
    mySwitch.sendTriState(socket12TriStateOff);

  }
  else {
    mySwitch.setPulseLength(176);
    mySwitch.sendTriState(socket12TriStateOn);}
 }
 BLYNK_WRITE(16)  // RF433 Switch13
 {
  int a = param.asInt();
    if (a == 0)
  {
    mySwitch.setPulseLength(176);
    mySwitch.sendTriState(socket13TriStateOff);

  }
  else {
    mySwitch.setPulseLength(176);
    mySwitch.sendTriState(socket13TriStateOn);}
 }
 BLYNK_WRITE(17)  // Livolo Switch 1
 {
  int a = param.asInt();
    if (a == 0)
  {
 livolo.sendButton(6400, 120); // blink button #3 every 3 seconds using remote with remoteID #6400
  }
 }
void setup()
{
  Serial.begin(115200);
  delay(50);
  WiFi.mode(WIFI_STA);
  delay(50);
  Blynk.begin(auth, "Xiaomi_2G", "panatorium");

  while (Blynk.connect() == false) {
    // Wait until connected
  }

   Blynk.notify("Device has started!");
// bridge1.setAuthToken(auth1);

//############### Define timers ##########

   timer.setInterval(75000L, current_time);
   timer.setInterval(45000L, dhtRead);
   timer.setInterval(30000L, readDallas);
   timer.setInterval(60000L, sendThingSpeak);
   timer.setInterval(50000L, GetPressure);
   timer.setInterval(5000L, LCD);
  if(!bmp.begin())
  {
    Serial.print("Ooops, no BMP085 detected.");
    while(1);
  }
   dht.begin();
   sensors.begin();
   
  #ifdef DEBUG
    displaySensorDetails();
  #endif
    mySwitch.enableTransmit(14);
    mySwitch.setPulseLength(179);
  //mySwitch.setProtocol(1);
  //mySwitch.setRepeatTransmit(4);
  delay(50);

 #ifdef LCD_1602_ON
  lcd.init();   // initializing the LCD
  lcd.backlight(); // Enable or Turn On the backlight // Setup console
  lcd.setCursor(0, 0);
  lcd.print("ESP SENSORS V1"); // Start Print text to Line 1
  lcd.setCursor(0, 1);      
  lcd.print("Welcome!"); // Start Print Test to Line 2
  delay(2000); 
#endif
/*     setup pins
  for (int i = 0; i < 4; i++) {
    pinMode(channelPins[i], OUTPUT);
    digitalWrite(channelPins[i], LOW);
    delay(10);
  }
 */ 
  }

 void loop(){
   
  Blynk.run();
  timer.run();
  
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
