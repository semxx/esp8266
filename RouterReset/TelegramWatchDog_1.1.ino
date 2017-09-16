#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <ESP8266TelegramBOT.h>
#include "FS.h"
#include <SimpleTimer.h>

SimpleTimer timer;

#define BOTtoken "165672905:AAFhk3XgHITZGDA_M2XEoxAhFaOdxl1Wf6Q"  //token of TestBOT
#define BOTname "Neato"
#define BOTusername "neatobot"

long hystyresis = 10 * 60 * 1000; // 30 min
long lasttime;   //last time messages' scan has been done

#define GPIO 5

const char* WIFI_SSID            = "Xiaomi_2G";
const char* WIFI_PASSWORD        = "panatorium";

const int REQUEST_TIMEOUT        = 5 * 1000;           // 5 seconds
//const int WAIT_TIME            = 6 * 60 * 60 * 1000; // 6 hours
const int WAIT_TIME              = 1 * 60 * 1000; // 60 sec
const int MAX_TRIES              = 1;
const char* CHECK_HOST1          = "www.google.com";
const char* CHECK_URL1           = "/";
const char* CHECK_HOST2          = "fr.mappy.com";
const char* CHECK_URL2           = "/";

unsigned const int HTTPS_PORT = 443;

// NTP Servers:
static const char ntpServerName[] = "us.pool.ntp.org";
//static const char ntpServerName[] = "time.nist.gov";
//static const char ntpServerName[] = "time-a.timefreq.bldrdoc.gov";
//static const char ntpServerName[] = "time-b.timefreq.bldrdoc.gov";
//static const char ntpServerName[] = "time-c.timefreq.bldrdoc.gov";

const int timeZone = 3;
unsigned int localPort = 8888;  // local port to listen for UDP packets
String timeString;
String text;
const int LED_BLUE = 2;

void GetTimeString();
void sendNTPpacket(IPAddress &address);
time_t getNtpTime();

WiFiUDP Udp;
WiFiClientSecure client;

TelegramBOT bot(BOTtoken, BOTname, BOTusername);

void fail(const char* msg) {
  Serial.println(msg);
    while (true) {
    yield();
  }
}

void connectToWifi() {
  unsigned int i = 0;

  Serial.println("");
  Serial.print("connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.forceSleepWake();
  WiFi.mode(WIFI_STA);  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    i++;
    if (i > 60) {
        Serial.println("can’t connect to wifi");
        markError();
        cyclePower();
        delay(5000);
        ESP.restart();
      }
  }
  Serial.println("");
  Serial.println("wifi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
}

void cyclePower() {     // Turn plug off then on
  Serial.println("turning plug off");
  digitalWrite(GPIO, 0);
  delay(5000);
  digitalWrite(GPIO, 1);
  Serial.println("turning plug on");
}

void stopWifiAndReboot() {
  Serial.print("disabling WIFI then wait: ");
  Serial.println(WAIT_TIME);
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(1); //Needed, at least in my tests WiFi doesn't power off without this for some reason
  delay(WAIT_TIME);
  Serial.println("restarting");
  ESP.restart();
}

void createFile(){
   String err = "1";
    File out = SPIFFS.open("/tmp.txt", "r");
  {
    if (!out) {
    File out = SPIFFS.open("/tmp.txt", "w");
    out.print(err);
    Serial.println("File tmp.txt has been created sucsessfully!"); 
    }
    else {
    Serial.println("File tmp.txt allready exist!"); 
      }
  } 
}

void markError(){
   String err = "1";
    File out = SPIFFS.open("/err.txt", "r");
   {  
    if (!out) {
    File out = SPIFFS.open("/err.txt", "w");
    out.print(err);
    Serial.println("Mark error sucsessfully!"); 
   }
    else {
    Serial.println("Error markeded before!"); 
    } 
  }
}

void cleanError() {
 
 if (!SPIFFS.remove("/err.txt")) {
        Serial.println("remove failed");
      }
  else {
        Serial.println("err.txt has been removed");
    }
  
  }
void UpdateTelegramMessageFile() {
    GetTimeString();
    String text = timeString;
      File out = SPIFFS.open("/tmp.txt", "w");
      out.print(text);
      Serial.println("Sucsessfully updated time for Telegram");
      Serial.println(text);
  }
  
boolean get(const char* host, const char* url) {
  WiFiClientSecure httpsClient;
  boolean status = false;
  Serial.print(">  ");
  Serial.print(host);
  Serial.print(":");
  Serial.print(HTTPS_PORT);

  if (!httpsClient.connect(host, HTTPS_PORT)) {
    Serial.println("");
    Serial.println("connection failed");
    return false;
  }

  Serial.println(url);

  httpsClient.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: ESP8266\r\n" +
               "Connection: close\r\n\r\n");

  unsigned long timeout = millis();
  while (httpsClient.available() == 0) {
    delay(1);
    if (millis() - timeout > REQUEST_TIMEOUT) {
      Serial.println("client Timeout !");
      httpsClient.stop();
      return false;
    }
  }

  if (httpsClient.connected()) {
    String line = httpsClient.readStringUntil('\n');
    Serial.print("< ");
    Serial.println(line);
    status = true;
  }
  httpsClient.stop();
  return status;
}
void setup() {
                Serial.begin(115200);
                delay(1000);
                Serial.println("");
                Serial.println("Internet Smart Plug");
                pinMode(LED_BLUE, OUTPUT);
                pinMode(GPIO, OUTPUT);
                digitalWrite(GPIO, 1);
                if (!SPIFFS.begin()) {
                  fail("SPIFFS init failed");
                }
                connectToWifi();  
                bot.begin();      // launch Bot functionalities
                Udp.begin(localPort);
                Serial.print("Local port: ");
                Serial.println(Udp.localPort());
                Serial.println("waiting for sync");
                setSyncProvider(getNtpTime);
                setSyncInterval(300);
                Serial.println("");
                Serial.println("Internet Smart Plug");
  timer.setInterval(3600000L, mainLoop);
  mainLoop();
  }

String AddZero(String input) {
    String s = input;
       if (s.length()== 1) {   
           s.replace("0", "00");
           s.replace("1", "01");
           s.replace("2", "02");
           s.replace("3", "03");
           s.replace("4", "04");
           s.replace("5", "05");
           s.replace("6", "06");
           s.replace("7", "07");
           s.replace("8", "09");
           s.replace("9", "09");
          return s;
          }
          else  {
                   return s; 
                }
    }

void GetTimeString() {
  int checkYear = year();
  String mnt = "";
  String scd = "";

  mnt += minute();
  scd += second();
 
    if (checkYear != 1970) {
        timeString = "";
        timeString += year();
        timeString += "/";
        timeString += month();
        timeString += "/";
        timeString += day();
        timeString += " ";
        timeString += hour();
        timeString += ":";  
        timeString += AddZero(mnt);
        timeString += ":";
        timeString += AddZero(scd);
      }
      else { Serial.println("Can not get correct date/time from NTP server");
           }
  }


void loop() {
 timer.run();
}

void mainLoop() {
  
    String result;
    String messg;
    boolean online;
    boolean getTime = false;
    unsigned short i = 0;
    
      {
        File in = SPIFFS.open("/tmp.txt", "r");
        if (in) {
            in.setTimeout(0);
            result = in.readString();
        }
        else { 
              fail("failed to open tmp.txt for reading");
             }
    
      }
    
    {
        File in = SPIFFS.open("/err.txt", "r");
        if (in) {
    
            Serial.println("send message.."); 
            Serial.println(result); 
            messg += result;
            messg += " Smart Plug WatchDog is down :-(";
            bot.sendMessage("161933663", messg, "");
            yield();
            GetTimeString();
            yield();
            messg = timeString;
            messg += " Smart Plug WatchDog sucsessfully up!";
            bot.sendMessage("161933663", messg, "");
            Serial.println("send message complete..");
            cleanError(); 
            delay(2000);
            Serial.println("restarting");
            ESP.restart();  
        }
    }
    
      do {
        boolean check1 = get(CHECK_HOST1, CHECK_URL1);
        boolean check2 = get(CHECK_HOST2, CHECK_URL2);
        online = (check1 || check2);
      } while(!online && i++ < MAX_TRIES);
    
      Serial.println("");
      Serial.print("status: ");
      Serial.println(online ? "OK" : "KO");
      Serial.println("");
    
      
      if (!online) {
        markError();
        cyclePower();
        stopWifiAndReboot();
        
      } else {
              GetTimeString();
             //     if (millis() > lasttime + hystyresis || !getTime )  {
                      UpdateTelegramMessageFile();
    
                 // }
    
              }

  }
/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
