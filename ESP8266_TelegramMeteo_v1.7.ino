//#define BLYNK_DEBUG
#define DEBUG //uncomment for debug
#define BLYNK_PRINT Serial    // Comment this out to disable printsand save space

/*
 *  This sketch based on the example simple HTTP-like server.
 *  The server will perform 3 functions depending on the request
 *  1.  http://ESP8266-IP:SVRPORT/gpio/0 will set the GPIO16 low,
 *  2.  http://ESP8266-IP:SVRPORT/gpio/1 will set the GPIO16 high
 *  3.  http://ESP8266-IP:SVRPORT/?request=GetSensors will return a json string with sensor values
 *
 *  ESP8266-IP is the IP address of the ESP8266 module
 *  SVRPORT is the TCP port number the server listens for connections on
 */
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <OneWire.h>
#include <DHT.h>
#include <NTPtime.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <Wire.h>
#include <WiFiClientSecure.h>
#include <ESP8266TelegramBOT.h>
#include <RCSwitch.h>
#include <livolo.h>
#include <UtilityFunctions.h>
#include <SimpleTimer.h>

#define BOTtoken "165672905:AAFhk3XgHITZGDA_M2XEoxAhFaOdxl1Wf6Q"
#define BOTname "Neato"
#define BOTusername "neatobot"
#define TIMEZONE 3
#define DAYLIGHTSAVINGTIME 0
#define DHTPIN 13
#define DHTTYPE DHT22
#define ONE_WIRE_BUS 12
#define TEMPERATURE_PRECISION 12 // Lower resolution

extern "C" {
#include "user_interface.h"
}
//Server actions
#define SET_LED_OFF 0
#define SET_LED_ON  1
#define Get_SENSORS 2

#define SERBAUD 9600
#define SVRPORT 80
#define ONEJSON 0
#define FIRSTJSON 1
#define NEXTJSON 2
#define LASTJSON 3

//GPIO pin assignments
#define LED_IND 4      // LED used for initial code testing (not included in final hardware design)

WiFiServer server(SVRPORT);
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
TelegramBOT bot(BOTtoken, BOTname, BOTusername);
SimpleTimer timer;
Livolo livolo(14); // transmitter connected to pin #14
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
OneWire  ds(12);
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

char auth[]  = "77804dfb38444bb381b9bd01145af4f3";
char auth1[] = "dac663bea5ae4b26b165d8d2d02e5969"; //Auth Tokens for any additional projects

WidgetLCD lcd(1);

const char* thingSpeakAddress = "api.thingspeak.com";
String thingSpeakAPIKey = "VQ0584HOI2VHUYSH";

const char* ssid = "Mimimi";          //your wifi ssid
const char* password = "panatorium";  //your wifi password

const uint8_t ipadd[4] = {10,0,0,32};  //static ip assigned to ESP8266
const uint8_t ipgat[4] = {10,0,0,1};    //local router gateway ip
const uint8_t ipsub[4] = {255,255,255,0};

//globals
int lc=0;
int hours,minutes,seconds;
unsigned long currentMillis, currentTime;

bool complete=false;
float start_time = 0;
float TempC_0,TempC_1,bt,bp,ba,dhtt,dhth,t_light;
String btmp,bprs,balt,VoidName;
char tin[20],tout[20],light[5],dhhumi[20],dhtemp[20],szT[30]; 
uint32_t state=0;

void Start(String vName) {
#ifdef DEBUG
   VoidName = vName;
   start_time = 0;
   start_time = millis();
#endif
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

void printStatus(char * status, int s) {

    if(s>=0) Serial.println(s);
    else Serial.println("");
    delay(100);
}
void startWIFI(void) {
  // Connect to WiFi network
  Serial.println();
  delay(10);
  Serial.println();
  delay(10);
  Serial.print("Connecting to ");
  delay(10);
  Serial.println(ssid);
  delay(10);
  WiFi.config(ipadd, ipgat, ipsub); 
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("ESP8266 IP: ");
  Serial.println(WiFi.localIP());

  Serial.print("ESP8266 WebServer Port: ");
  Serial.println(SVRPORT);
  delay(300);

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
     Serial.print(dhtt);
     Serial.print(" C Outdoor: ");
     Serial.print(TempC_0);
     Serial.print(" Humidity: ");
     Serial.print(dhth);
     Serial.print(" Presure: ");
     Serial.print(bprs);
     Serial.print(" Altitude: ");
     Serial.print(balt);
     Serial.print(" BMP_Temp: ");
     Serial.print(btmp);
     Serial.println(" send to Thingspeak");
   #endif

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

void Bot_ExecMessages(){
  for (int i = 1; i < bot.message[0][0].toInt() + 1; i++)      {
 //   String EchoResponse = bot.message[i][5];
 //   EchoResponse = UnicodeASCII(EchoResponse);
     bot.message[i][5]=bot.message[i][5].substring(1,bot.message[i][5].length());
    if (bot.message[i][5] == "\/ledon") {
      mySwitch.sendTriState(socket1TriStateOn);
      delay(100);
      mySwitch.sendTriState(socket2TriStateOn);
      delay(100);
      mySwitch.sendTriState(socket3TriStateOn);

      bot.sendMessage(bot.message[i][4], "Light is ON", "");
    }
    if (bot.message[i][5] == "\/ledoff") {
      mySwitch.sendTriState(socket1TriStateOff);
      delay(100);
      mySwitch.sendTriState(socket2TriStateOff);
      delay(100);
      mySwitch.sendTriState(socket3TriStateOff);
      bot.sendMessage(bot.message[i][4], "Light is OFF everywere...", "");
    }
    if (bot.message[i][5] == "\/weather") {
      
        char t_buffer[15];
        char h_buffer[15];
        char o_buffer[15];
        char r_buffer[15];

//        dtostrf(DHT_TMP, 4, 2, t_buffer);
//        dtostrf(DHT_HYM, 4, 2, h_buffer);
//        dtostrf(TempC_0, 4, 2, o_buffer);
//        dtostrf(BMP_PRESURE, 4, 2, r_buffer);

        char str1[20];
        strcpy (str1,"Outdoor t° is: ");
        strcat (str1,o_buffer);
        puts (str1);

        bot.sendMessage(bot.message[i][4], str1, "");

        char str2[20];
        strcpy (str2,"Indoor t° is: ");
        strcat (str2, t_buffer);
        puts (str2);
        bot.sendMessage(bot.message[i][4], str2, "");

        char str3[20];
        strcpy (str3,"Hymidity is: ");
        strcat (str3, h_buffer);
        puts (str3);
        bot.sendMessage(bot.message[i][4], str3, "");

        char str4[20];
        strcpy (str4,"Pressure (mmHg) is: ");
        strcat (str4, r_buffer);
        puts (str4);
        bot.sendMessage(bot.message[i][4], str4, "");
    }
    if (bot.message[i][5] == "\/start") {

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

void TelegramCheck()  {

  Start("Telegram_Check");

      bot.getUpdates(bot.message[0][1]);
      Bot_ExecMessages();

  End();
}

void readSensorIsr() {
  yield();
  switch(state++) {
    case 0: // Read Indoor Temperature
      Start("getTempDs18b20");
        sensors.requestTemperatures();
        TempC_0 = sensors.getTempCByIndex(0);
        char t_buffer[15];
        dtostrf(TempC_0, 4, 2, t_buffer);
        Blynk.virtualWrite(V5, t_buffer);
       // getTempDs18b20(1, tin);
       // Serial.println(tin);
       // Blynk.virtualWrite(V5, tin);
      End();
      break;
    case 1: // Read Outdoor Temperature
     // getTempDs18b20(2, tout);
     // Blynk.virtualWrite(V6, tout);
      break;
    case 2: // Read DHT22 Humidity and Temperature
      Start("Read DHT22");
       dhth = dht.readHumidity();
       dhtt = dht.readTemperature();

      if (isnan(dhth) || isnan(dhtt)) {
      Serial.println("Failed to read from DHT sensor!");
      //return;
      }

       Blynk.virtualWrite(10, dhtt); // virtual pin
       Blynk.virtualWrite(11, dhth); // virtual pin

      #ifdef DEBUG
       Serial.print("DHT humidity: ");
       Serial.print(dhth);
       Serial.print(" %\t");
       Serial.print("DHT temperature: ");
       Serial.print(dhtt);
       Serial.println(" C ");
      #endif
      End();
      break;
    case 3: // Read luminosity A0
      Start("Read luminosity A0");
         t_light = analogRead(A0);
         ftoa(t_light,light, 0);
      End();
      break;
    case 4: //Read Baro Pressure
      Start("READ BMP180");
      sensors_event_t event;
      bmp.getEvent(&event);

    if (event.pressure)
      {
      float temperature;
      bmp.getTemperature(&temperature);
      float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
      bprs = event.pressure*0.75006375541921;
      balt = (bmp.pressureToAltitude(seaLevelPressure, event.pressure));
      btmp = temperature;
      Blynk.virtualWrite(V8, bprs);
      Blynk.virtualWrite(V9, balt);
      Blynk.virtualWrite(V13, btmp);
      }
      else
      {
      Serial.println("Sensor error");
      }
      End();
      break;
    case 5: // Get current time from NTP server
      Start("CurrentTime");
      currentMillis = millis();
      currentTime = getTime(TIMEZONE, DAYLIGHTSAVINGTIME);
      hours = (currentTime  % 86400L) / 3600;
      minutes = (currentTime % 3600) / 60;
      seconds = (currentTime % 60);
      char timeString[8];
      sprintf(timeString,"%02d:%02d:%02d",hours, minutes, seconds);
        //BLYNK_LOG("The time is %s", timeString);       // UTC is thetimeat Greenwich Meridian (GMT)
      Blynk.virtualWrite(V12, timeString );
      End();
      break;
    case 6:

      break;
    case 7:
      Blynk.virtualWrite(V7, millis() / 60000L);  // Send UpTime
    state = 0;
      break;
    default:
      break;
  }
  ESP.wdtFeed(); 
  yield();
}
void sendThingSpeak(){

Start("sendThingSpeak");

  updateThingSpeak(thingSpeakAPIKey + "&field1=" + dhtt + "&field2=" + String(TempC_0) + "&field3=" + dhth + "&field4=" + bprs + "&field5=" + balt + "&field6=" + btmp);

End();

}
void jsonAdd(String *s, String key,String val) {
    *s += '"' + key + '"' + ":" + '"' + val + '"';
}
void jsonEncode(int pos, String * s, String key, String val) {
    switch (pos) {
      case ONEJSON:      
      case FIRSTJSON:
        *s += "{\r\n";
        jsonAdd(s,key,val);
        *s+= (pos==ONEJSON) ? "\r\n}" : ",\r\n";
        break;
      case NEXTJSON:    
        jsonAdd(s,key,val);
        *s+= ",\r\n";
        break;
      case LASTJSON:    
        jsonAdd(s,key,val);
        *s+= "\r\n}";
        break;
    }
}
void killclient(WiFiClient client, bool *busy) {
  lc=0;
  delay(1);
  client.flush();
  client.stop();
  complete=false;
  *busy = false;  
}
void sysloop() {
  static bool busy=false;
  static int timeout_busy=0;
  //connect wifi if not connected
  if (WiFi.status() != WL_CONNECTED) {
    delay(1);
    startWIFI();
    return;
  }
  //return if busy
  if(busy) {
    delay(1);
    if(timeout_busy++ >10000) {
      printStatus((char *)" Status: Busy timeout-resetting..",-1);
      ESP.reset(); 
      busy = false;
    }
    return;
  }
  else {
    busy = true;
    timeout_busy=0;
  }
  delay(1);
  //Read 1 sensor every 2.5 seconds
  if(lc++>2500) {
    lc=0;
    printStatus((char *)" ",state);
    readSensorIsr(); 
    busy = false;
    return;   
  }
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
     busy = false;
     return;
  } 
  // Wait until the client sends some data
  while((!client.available())&&(timeout_busy++<5000)){
    delay(1);
    if(complete==true) {
      killclient(client, &busy);
      return;
    }
  }
  //kill client if timeout
  if(timeout_busy>=5000) {
    killclient(client, &busy);
    return;
  }
  
  complete=false; 
  ESP.wdtFeed(); 
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  client.flush();
  if (req.indexOf("/favicon.ico") != -1) {
    client.stop();
    complete=true;
    busy = false;
    return;
  }
  Serial.print("Recieve http: ");  
  Serial.println(req);
  delay(100);
  
    // Match the request
  int val;
  if (req.indexOf("/gpio/0") != -1)
    val = SET_LED_OFF;
  else if (req.indexOf("/gpio/1") != -1)
    val = SET_LED_ON;
  else if (req.indexOf("/?request=GetSensors") != -1) {
    val = Get_SENSORS;
    //Serial.println("Get Sensor Values Requested");
  }  
  else {
    Serial.println("invalid request");
    client.stop();
    complete=true;
    busy = false;
    return;
  }
  client.flush();

  // Prepare Response header
  String s = "HTTP/1.1 200 OK\r\n";
  String v ="";
  ESP.wdtFeed(); 
      
  switch (val) {
    case SET_LED_OFF:
    case SET_LED_ON:
      // Set GPIO4 according to the request
      digitalWrite(LED_IND , val);
  
      // Prepare the response for GPIO state
      s += "Content-Type: text/html\r\n\r\n";
      s += "<!DOCTYPE HTML>\r\nGPIO is now ";
      s += (val)?"high":"low";
      s += "</html>\n";
      // Send the response to the client
      client.print(s);
      break;
    case Get_SENSORS:
  updateThingSpeak(thingSpeakAPIKey + "&field1=" + dhtt + "&field2=" + String(TempC_0) + 
  "&field3=" + dhth + "&field4=" + bprs + "&field5=" + balt + "&field6=" + btmp);
      //Create JSON return string
      s += "Content-Type: application/json\r\n\r\n";
      jsonEncode(FIRSTJSON,&s,"DateTime", TempC_0);      
      jsonEncode(NEXTJSON,&s,"DHT_TEMP_INDOOR", dhth); 
      jsonEncode(NEXTJSON,&s,"DHT_TEMP_INDOOR", dhth); 
      jsonEncode(NEXTJSON,&s,"DHT_HYMIDITY", dhth);
      jsonEncode(NEXTJSON,&s,"BMP180_PRESSURE", bprs);
      jsonEncode(NEXTJSON,&s,"BMP180_TEMPERATURE", btmp);
      jsonEncode(NEXTJSON,&s,"BMP180_ALTITUDE", balt);
      jsonEncode(NEXTJSON,&s,"A0_Lighting", light);
      v = system_get_free_heap_size();
      jsonEncode(NEXTJSON,&s,"SYS_Heap", v);
      v = millis()/60000L;
      jsonEncode(LASTJSON,&s,"SYS_Time", v);

      client.print(s);      // Send the response to the client
      yield();
      //ESP.wdtFeed(); 
      break;
    default:
      break;
   }

    delay(1);
    v ="";
    s ="";
    val = 0;
    Serial.println("Ending it: Client disconnected");
    delay(150);
    complete=true;
    busy = false;
    ESP.wdtFeed(); 
  

  // The client will actually be disconnected 
  // when the function returns and 'client' object is destroyed
}


void setup() {
  Serial.begin(SERBAUD);
  delay(10);
  startWIFI();

  Blynk.begin(auth, "Mimimi", "panatorium");
   while (Blynk.connect() == false) {
    // Wait until connected
  }

   Blynk.notify("Device has started!");
//   bridge1.setAuthToken(auth1);

//############### Define timers ##########

   timer.setInterval(60000L, sendThingSpeak);
// timer.setInterval(30000L, TelegramCheck);


  if(!bmp.begin())
  {
    Serial.print("Ooops, no BMP085 detected.");
    while(1);
  }
   dht.begin();
   bot.begin();      // launch TelegramBot functionalities
   DeviceAddress outsideThermometer = { 0x28, 0xFF, 0x83, 0x8F, 0x00, 0x15, 0x02, 0x21 };
   sensors.begin();
   sensors.setResolution(outsideThermometer, TEMPERATURE_PRECISION);
   mySwitch.enableTransmit(14);
   mySwitch.setPulseLength(179); 
  
   // Set Indicator LED as output
  
   pinMode(LED_IND , OUTPUT);
   digitalWrite(LED_IND, 0);

  #ifdef DEBUG
    displaySensorDetails();
  #endif

  // Print Free Heap
  printStatus((char *)" Status: Setup complete",-1);
  delay(500);
  
}

void loop() {
  
    sysloop();
    Blynk.run();
    timer.run();
}


 BLYNK_WRITE(0)  // Manual restart ESP
 {
  int a = param.asInt();
    if (a == 0)
  {
    ESP.reset();
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
BLYNK_WRITE(4)  // Telegram Check
 {
  int a = param.asInt();
    if (a == 0)
  {
    TelegramCheck();
  }
 }
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
 /*
void getTempDs18b20(int sensor, char * val) {
  
  byte i;
  byte present = 1;
  byte type_s;
  byte addr[8];
  byte data[12];
  float celsius, fahrenheit;
  
  switch(sensor) {

    case 1:  //Outside Sensor Device 0 Address: 28 FF 83 8F 00 15 02 21
    addr[0]=0x28;
    addr[1]=0xFF;
    addr[2]=0x83;
    addr[3]=0x8F;
    addr[4]=0x00;
    addr[5]=0x15;
    addr[6]=0x02;
    addr[7]=0x21;
  break;
    case 2:  //Inside Sensor
    addr[0]=0x28;
    addr[1]=0xFF;
    addr[2]=0x0B;
    addr[3]=0x0A;
    addr[4]=0x62;
    addr[5]=0x15;
    addr[6]=0x01;
    addr[7]=0x84;
  break;
    case 3:  //Attic Sensor
    addr[0]=0x10;
    addr[1]=0xCB;
    addr[2]=0x45;
    addr[3]=0x2F;
    addr[4]=0x00;
    addr[5]=0x08;
    addr[6]=0x00;
    addr[7]=0x3B;
  break;
  }
 
  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }

  type_s = 0; //DS18B20
  ds.reset();
  ds.select(addr);
  ds.write(0x44,1);         // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad
  
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }
  ds.reset();
  unsigned int raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // count remain gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  ftoa(celsius,val, 2);
  Serial.println(celsius);

}
*/
