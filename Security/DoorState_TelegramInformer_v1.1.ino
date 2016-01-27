#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266TelegramBOT.h>
#include <NTPtime.h>

#define TIMEZONE 3
#define DAYLIGHTSAVINGTIME 0
char timeString[8];

#define BOTtoken "165672905:AAFhk3XgHITZGDA_M2XEoxAhFaOdxl1Wf6Q"  //token of TestBOT
#define BOTname "Neato"
#define BOTusername "neatobot"

String ssid = "Mimimi";
String password = "panatorium";

TelegramBOT bot(BOTtoken, BOTname, BOTusername);

const int LED_BLUE = 2;
const int BUTTON_PIN = 5;
 
//////////////////////
// Button Variables //
//////////////////////
int BUTTON_STATE;
int LAST_BUTTON_STATE = LOW;
long LAST_DEBOUNCE_TIME = 0;
long DEBOUNCE_DELAY = 50;
int BUTTON_COUNTER = 0;


void setup() {
  
  Serial.begin(115200);
  delay(10);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  bot.begin();      // launch Bot functionalities
  WiFi.begin(ssid.c_str(), password.c_str());

    if (checkWiFiConnection()) {
      digitalWrite(LED_BLUE, LOW);
      return;
    }
  
}
 
void loop() {
 
  boolean pressed = debounce();
  if (pressed == true) {
   
   BUTTON_COUNTER++;

current_time();

      String chat_id = "161933663";
      String MsgText = "The door has been opened in: "; 
        MsgText += timeString;   
        bot.sendMessage(chat_id, MsgText, "");
  }
}

// Debounce Button Presses

boolean debounce() {
  boolean retVal = false;
  int reading = digitalRead(BUTTON_PIN);
  if (reading != LAST_BUTTON_STATE) {
    LAST_DEBOUNCE_TIME = millis();
  }
  if ((millis() - LAST_DEBOUNCE_TIME) > DEBOUNCE_DELAY) {
    if (reading != BUTTON_STATE) {
      BUTTON_STATE = reading;
      if (BUTTON_STATE == HIGH) {
        retVal = true;
      }
    }
  }
  LAST_BUTTON_STATE = reading;
  return retVal;
}

// Boolean function to check for a WiFi Connection

boolean checkWiFiConnection() {
  int count = 0;
  Serial.print("Waiting to connect to the specified WiFi network");
  while ( count < 60 ) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      Serial.println("Connected!");
      return (true);
    }
    delay(500);
    Serial.print(".");
    count++;
  }
  Serial.println("Timed out.");
  return false;
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
 
