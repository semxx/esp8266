#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

const char* ssid = "Mimimi";
const char* password = "panatorium";

const char* host = "api.telegram.org";
const int httpsPort = 443;

// SHA1 fingerprint of the certificate
const char* fingerprint = "‎37 21 36 77 50 57 f3 c9 28 d0 f7 fa 4c 05 35 7f 60 c1 20 44";
String url = "/bot165672905:AAFhk3XgHITZGDA_M2XEoxAhFaOdxl1Wf6Q/sendMessage?chat_id=161933663&text=";

  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void sendMessage(String MsgText) {
int i = 0;
/*
  if (client.verify(fingerprint, host)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }
 */
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    //return;
  }
  Serial.print("requesting URL: ");  Serial.println(url);
  Serial.print("connecting to ");    Serial.println(host);
 

  client.print(String("POST ") + url + MsgText +" HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: ESP8266\r\n" +
               "Connection: close\r\n\r\n");

 // Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
 //     Serial.println("headers received");
      break;
    }
}
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"ok\":true")) {
    Serial.println("Message send sucsessfuly!");
  } 
    else {
     for ( i = 0; i < 5; i++)  {   
         if (client.connect(host, httpsPort)) {

         client.print(String("POST ") + url + MsgText +" HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: ESP8266\r\n" +
               "Connection: close\r\n\r\n");
               line = client.readStringUntil('\n');
               Serial.println(line);
                 if (line = "HTTP/1.1 200 OK") {
                  Serial.println("Message sent at the second attempt!");
                  client.stop();
                  return;
                 }
        }
          else {
            Serial.println("Connection failed. Trying again...");
             }
      }
    Serial.println("Message sent failed");
    client.stop();
    return;
    }
  Serial.print("reply was: ");
  Serial.println(line);
  client.stop();
}

void loop() {

String Text1 = "This is test message";
String Text2 = "Is delivered?";

  sendMessage(Text1);
  sendMessage(Text2);  

delay(20000);
 
}
