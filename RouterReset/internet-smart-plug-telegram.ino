#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <ESP8266TelegramBOT.h>
#include "FS.h"

#define BOTtoken "165672905:AAFhk3XgHITZGDA_M2XEoxAhFaOdxl1Wf6Q"  //token of TestBOT
#define BOTname "Neato"
#define BOTusername "neatobot"

#define GPIO 5

const int REQUEST_TIMEOUT        = 5 * 1000;           // 5 seconds
//const int WAIT_TIME              = 6 * 60 * 60 * 1000; // 6 hours
const int WAIT_TIME              = 1 * 60 * 1000; // 60 sec
const int MAX_TRIES              = 10;
const char* CHECK_HOST1          = "www.google.com";
const char* CHECK_URL1           = "/";
const char* CHECK_HOST2          = "fr.mappy.com";
const char* CHECK_URL2           = "/";
const char* MONITOR_HOST         = "yandex.ru";
const char* MONITOR_URL_UP       = "/internet";
const char* MONITOR_URL_DOWN     = "/maps";
unsigned const int HTTPS_PORT = 443;

//const char ssid[] = "Xiaomi_2G";  //  your network SSID (name)
//const char pass[] = "panatorium";       // your network password

const char* WIFI_SSID            = "Xiaomi_2G";
const char* WIFI_PASSWORD        = "panatorium";
// NTP Servers:
//static const char ntpServerName[] = "us.pool.ntp.org";
static const char ntpServerName[] = "time.nist.gov";
//static const char ntpServerName[] = "time-a.timefreq.bldrdoc.gov";
//static const char ntpServerName[] = "time-b.timefreq.bldrdoc.gov";
//static const char ntpServerName[] = "time-c.timefreq.bldrdoc.gov";

const int timeZone = 3;

unsigned int localPort = 8888;  // local port to listen for UDP packets
String timeString;
String text;
const int LED_BLUE = 2;

void GetTimeString();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);

WiFiUDP Udp;
WiFiClientSecure client;

time_t getNtpTime();
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
    delay(500);
    Serial.print(".");
    i++;
    if (i > 50) {
        Serial.println("can’t connect to wifi");
        cyclePower();
        delay(5000);
        ESP.restart();
        //stopWifiAndReboot();
      }
  }
  Serial.println("");
  Serial.println("wifi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
}
// Turn plug off then on
void cyclePower() {
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
  return status;
}
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("");
  Serial.println("Internet Smart Plug");
  connectToWifi();  

  if (!SPIFFS.begin()) {
    fail("SPIFFS init failed");
  }

  bot.begin();      // launch Bot functionalities
  
/*  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("IP number assigned by DHCP is ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
*/
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
 
  pinMode(LED_BLUE, OUTPUT);
  pinMode(GPIO, OUTPUT);
  digitalWrite(GPIO, 1);
  Serial.println("");
  Serial.println("Internet Smart Plug");


  }

/*  
  {

    Serial.println("\n\nFS test\n");
    if (!SPIFFS.format()) {
      fail("format failed");
    }
    Dir root = SPIFFS.openDir("/");
    int count = 0;
    while (root.next()) {
      ++count;
    }
    if (count > 0) {
      fail("some files left after format");
    }
  }
*/


/*
  String text = "write test";
  {
    File out = SPIFFS.open("/tmp.txt", "w");
    if (!out) {
      fail("failed to open tmp.txt for writing");
    }
    out.print(text);
  }

  {
    File in = SPIFFS.open("/tmp.txt", "r");
    if (!in) {
      fail("failed to open tmp.txt for reading");
    }
    Serial.printf("size=%d\r\n", in.size());
    if (in.size() != text.length()) {
      fail("invalid size of tmp.txt");
    }
    Serial.print("Reading data: ");
    in.setTimeout(0);
    String result = in.readString();
    Serial.println(result);
    if (result != text) {
      fail("invalid data in tmp.txt");
    }
  }

  {
    for (int i = 0; i < 10; ++i) {
      String name = "seq_";
      name += i;
      name += ".txt";

      File out = SPIFFS.open(name, "w");
      if (!out) {
        fail("can't open seq_ file");
      }

      out.println(i);
    }
  }
  {
    Dir root = SPIFFS.openDir("/");
    while (root.next()) {
      String fileName = root.fileName();
      File f = root.openFile("r");
      Serial.printf("%s: %d\r\n", fileName.c_str(), f.size());
    }
  }

  {
    Dir root = SPIFFS.openDir("/");
    while (root.next()) {
      String fileName = root.fileName();
      Serial.print("deleting ");
      Serial.println(fileName);
      if (!SPIFFS.remove(fileName)) {
        fail("remove failed");
      }
    }
  }

  {
    File tmp = SPIFFS.open("/tmp1.txt", "w");
    tmp.println("rename test");
  }

  {
    if (!SPIFFS.rename("/tmp1.txt", "/tmp2.txt")) {
      fail("rename failed");
    }
    File tmp2 = SPIFFS.open("/tmp2.txt", "r");
    if (!tmp2) {
      fail("open tmp2 failed");
    }
  }

  {
    if (!SPIFFS.format()) {
      fail("format failed");
    }
    Dir root = SPIFFS.openDir("/");
    int count = 0;
    while (root.next()) {
      ++count;
    }
    if (count > 0) {
      fail("some files left after format");
    }
  }

  Serial.println("success");
  */

time_t prevDisplay = 0; // when the digital clock was displayed

/*
String AddZero(String input) {
      String s = input;
  //  Serial.print("Input value in function replace: ");
  //  Serial.println(s);
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
*/
void GetTimeString()
{
  timeString = "";
  timeString += year();
  timeString += ".";
  timeString += month();
  timeString += ".";
  timeString += day();
  timeString += " ";
  timeString += hour();
  timeString += ":";  
  timeString += minute();
  timeString += ":";
  timeString += second();
}

void loop() {

boolean online;
boolean isSend = false;
unsigned short i = 0;
if (!isSend) {
    File in = SPIFFS.open("/tmp.txt", "r");
    if (!in) {
      fail("failed to open tmp.txt for reading");
    }
    Serial.printf("size=%d\r\n", in.size());
 //   if (in.size() != text.length()) {
 //     fail("invalid size of tmp.txt");
 if (in.size() > 0) {

        Serial.print("Reading data: ");
        in.setTimeout(0);
        String result = in.readString();
        Serial.println(result); 
        bot.sendMessage("161933663", result, "");
        isSend = true;
        delay(700);
      }
  }

/* 
  {
    File in = SPIFFS.open("/tmp.txt", "r");
    if (!in) {
      fail("failed to open tmp.txt for reading");
    }
    Serial.printf("size=%d\r\n", in.size());
 //   if (in.size() != text.length()) {
 //     fail("invalid size of tmp.txt");
 if (in.size() > 0) {

        Serial.print("Reading data: ");
        in.setTimeout(0);
        String result = in.readString();
        Serial.println(result);        yield();
        bot.sendMessage("161933663", result, "");
        yield();
      }
//    if (result != text) {
//      fail("invalid data in tmp.txt");
    }
*/
if (isSend) {
  do {
    boolean check1 = get(CHECK_HOST1, CHECK_URL1);
    boolean check2 = get(CHECK_HOST2, CHECK_URL2);
    online = (check1 || check2);
  } while(!online && i++ < MAX_TRIES);

  Serial.println("");
  Serial.print("status: ");
  Serial.println(online ? "OK" : "KO");
  Serial.println("");

 // get(MONITOR_HOST, online ? MONITOR_URL_UP : MONITOR_URL_DOWN);

  if (!online) {
    cyclePower();
    GetTimeString();

  
  } else {
    Serial.println("nothing to do");
  }
}
/*
    text = timeString;
  {
    File out = SPIFFS.open("/tmp.txt", "w");
    if (!out) {
      fail("failed to open tmp.txt for writing");
    }
    out.print(text);
  }
  stopWifiAndReboot();
*/
delay(20000);
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
