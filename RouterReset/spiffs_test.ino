#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <ESP8266TelegramBOT.h>
#include "FS.h"

#define BOTtoken "165672905:AAFhk3XgHITZGDA_M2XEoxAhFaOdxl1Wf6Q"  //token of TestBOT
#define BOTname "Neato"
#define BOTusername "neatobot"

const char ssid[] = "Xiaomi_2G";  //  your network SSID (name)
const char pass[] = "panatorium";       // your network password

// NTP Servers:
//static const char ntpServerName[] = "us.pool.ntp.org";
static const char ntpServerName[] = "time.nist.gov";
//static const char ntpServerName[] = "time-a.timefreq.bldrdoc.gov";
//static const char ntpServerName[] = "time-b.timefreq.bldrdoc.gov";
//static const char ntpServerName[] = "time-c.timefreq.bldrdoc.gov";

const int timeZone = 3;     // Central European Time
//const int timeZone = -5;  // Eastern Standard Time (USA)
//const int timeZone = -4;  // Eastern Daylight Time (USA)
//const int timeZone = -8;  // Pacific Standard Time (USA)
//const int timeZone = -7;  // Pacific Daylight Time (USA)

unsigned int localPort = 8888;  // local port to listen for UDP packets
String timeString;
const int LED_BLUE = 2;

void digitalClockDisplay();
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

void setup() {
  Serial.begin(115200);
 
  if (!SPIFFS.begin()) {
    fail("SPIFFS init failed");
  }
  
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
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
 
  pinMode(LED_BLUE, OUTPUT);
  bot.begin();      // launch Bot functionalities
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
}
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
void digitalClockDisplay()
{
  // digital clock display of the time


  Serial.print(day());
  Serial.print(".");
  Serial.print(month());
  Serial.print(".");
  Serial.print(year());
  Serial.print(" ");
  Serial.print(hour());
  //printDigits(minute());
  //printDigits(second());
  Serial.println();
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

      digitalClockDisplay();


  String text = timeString;
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
    bot.sendMessage("161933663", result, "");
    if (result != text) {
      fail("invalid data in tmp.txt");
    }
  }

    
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
