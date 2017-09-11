#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiUDP.h>
 

const byte targetMacAddress[] = { 0x90, 0x2b, 0x34, 0x5c, 0x97, 0xd4 }; // MAC адрес компа, который нужно разбудить
const byte targetIPAddress[] = { 10, 10, 10, 255 }; // IP адрес компа, который нужно разбудить. Также можно указать широковещательный адрес сети, указав в 4м сегменте 255
const int targetWOLPort = 9;  // WOL порт. Обычно 7 или 9
const unsigned int localUdpPort = 12345;
 

WiFiUDP Udp;
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "14730038da9e487cbb78fe18113011f2";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Xiaomi_2G";
char pass[] = "panatorium";

void sendWOL() {
  const int magicPacketLength = 102;
  byte magicPacket[magicPacketLength] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
 
  for (int ix=6; ix < magicPacketLength; ix++)
    magicPacket[ix] = targetMacAddress[ix % 6];
 
  Udp.begin(localUdpPort);
  Udp.beginPacket(targetIPAddress, targetWOLPort);
  Udp.write(magicPacket, magicPacketLength);
  Udp.endPacket();
  Udp.stop();
Serial.println("Send magicPacket");
}

void setup()
{
  // Debug console
  Serial.begin(115200);

  Blynk.begin(auth, ssid, pass);
}

void loop()
{
  Blynk.run();
}
BLYNK_WRITE(31) {
  int a = param.asInt();
  if (a != 0) {
    sendWOL();
  }
}
