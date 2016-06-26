#include <SPI.h>
#include <Ethernet.h>
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char server[] = "vasja.ru";
const char httpPost[] PROGMEM =
"POST %s HTTP/1.0\r\n"
"From: MyForm\r\n"
"User-Agent: HTTPTool/1.0\r\n"
"Content-Type: application/x-www-form-urlencoded\r\n"
"Content-Length: %d\r\n"
"\r\n"
"%s\r\n";
IPAddress ip(192,168,0,177);
EthernetClient client;
void setup() {
  Serial.begin(9600);
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac, ip);
  }
  delay(1000);
  Serial.println("connecting...");
  if (client.connect(server, 80)) {
    Serial.println("connected");
    char buf[255];
    sprintf(buf, httpPost, "/post.php", 19, "name=vasj&apass=123");
    client.print(buf);
  } 
  else {
    Serial.println("connection failed");
  }
}

void loop()
{
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    while(true);
  }
}
