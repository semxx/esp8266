#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>   
#include <WiFiUdp.h>
#include <ArduinoOTA.h>           // Библиотека для OTA-прошивки
#include <WiFiClientSecure.h>
#include <ESP8266TelegramBOT.h>
#include <NTPtime.h>
#include <PubSubClient.h>

#define TIMEZONE 3
#define DAYLIGHTSAVINGTIME 0
char timeString[8];

#define BOTtoken "165672905:AAFhk3XgHITZGDA_M2XEoxAhFaOdxl1Wf6Q"  //token of TestBOT
#define BOTname "Neato"
#define BOTusername "neatobot"

char* ssid = "Xiaomi_2G";
char* password = "panatorium";

TelegramBOT bot(BOTtoken, BOTname, BOTusername);

const int LED_BLUE = 2;
const int DOOR_PIN = 1;
 
//////////////////////
// DOOR Variables //
//////////////////////
int DOOR_STATE;
int LAST_DOOR_STATE = LOW;
long LAST_DEBOUNCE_TIME = 0;
long DEBOUNCE_DELAY = 50;
int DOOR_COUNTER = 0;

char* pubTopic = "/ESP8266/DATA";
char* controlTopic = "/ESP8266/CONTROL/#";
char* statTopic = "/ESP8266/STATUS/DOOR/4";
const char* mqtt_server = "10.10.10.25";

String mqtt_user = "admin";
String mqtt_pass = "admin";
String mqtt_client = "DoorState";

long lastMsg = 0;
char msg[50];
int value = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", "admin", "admin")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void setup() {
  Serial.begin(9600);
  delay(50);
//  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
//  delay(50);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.setHostname("DoorState_Informer"); // Задаем имя сетевого порта    
//  ArduinoOTA.setPassword((const char *)"0000"); // Задаем пароль доступа для удаленной прошивки   
  ArduinoOTA.begin(); // Инициализируем OTA  

  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);
  pinMode(DOOR_PIN, INPUT_PULLUP);

  
  bot.begin();      // launch Bot functionalities
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
}
// Debounce DOOR Presses

boolean debounce() {
  boolean retVal = false;
  int reading = digitalRead(DOOR_PIN);
  if (reading != LAST_DOOR_STATE) {
    LAST_DEBOUNCE_TIME = millis();
  }
  if ((millis() - LAST_DEBOUNCE_TIME) > DEBOUNCE_DELAY) {
    if (reading != DOOR_STATE) {
      DOOR_STATE = reading;
      if (DOOR_STATE == HIGH) {
        retVal = true;
      }
    }
  }
  LAST_DOOR_STATE = reading;
  return retVal;
}

void current_time()
{
  unsigned long currentMillis = millis();
  unsigned long currentTime = getTime(TIMEZONE, DAYLIGHTSAVINGTIME);

  int hours = (currentTime  % 86400L) / 3600;
  int minutes = (currentTime % 3600) / 60;
  int seconds = (currentTime % 60);

  sprintf(timeString,"%02d:%02d:%02d",hours, minutes, seconds);
  //BLYNK_LOG("The time is %s", timeString);       // UTC is the time at Greenwich Meridian (GMT)
  Serial.println(timeString);
  
}
void loop() {
  ArduinoOTA.handle(); // Всегда готовы к прошивке 

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  boolean pressed = debounce();
  if (pressed == true) {
   
   DOOR_COUNTER++;

current_time();
    yield();
      String chat_id = "161933663";
      String MsgText = "The door has been opened in: "; 
        MsgText += timeString;   
        bot.sendMessage(chat_id, MsgText, "");
  snprintf (msg, 75, "The door has opened #%ld", DOOR_COUNTER);
  client.publish("DOOR", msg);
  }
}
