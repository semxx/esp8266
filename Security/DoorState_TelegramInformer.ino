#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <WiFiClientSecure.h>
#include <ESP8266TelegramBOT.h>
#include <SimpleTimer.h>
#include <NTPtime.h>

#define TIMEZONE 3
#define DAYLIGHTSAVINGTIME 0

// Initialize Telegram BOT
#define BOTtoken "165672905:AAFhk3XgHITZGDA_M2XEoxAhFaOdxl1Wf6Q"
#define BOTname "Neato"
#define BOTusername "neatobot"

TelegramBOT bot(BOTtoken, BOTname, BOTusername);
// End of initialize Telegram BOT

/////////////////////////
// Network Definitions //
/////////////////////////

boolean SETUP_MODE;
String SSID_LIST;

/////////////////////////
// Time Definitions //
/////////////////////////

char timeString[8];

/////////////////////
// Pin Definitions //
/////////////////////
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

SimpleTimer timer;

void setup() {

  bot.begin();      // launch Bot functionalities

  initHardware();
  // Try and restore saved settings
  if (loadSavedConfig()) {
    if (checkWiFiConnection()) {
      SETUP_MODE = false;
      // Turn the status led Green when the WiFi has been connected
      digitalWrite(LED_BLUE, HIGH);
      return;
    }
  }
  SETUP_MODE = true;
  setupMode();
}

void loop() {

  // Wait for button Presses
  boolean pressed = debounce();
  if (pressed == true) {
    BUTTON_COUNTER++;


Get_CurrentTime();

String chat_id = "161933663";
String MsgText = "The main door has opened in: ";
       MsgText += timeString;
 
 bot.sendMessage(chat_id, MsgText, "");


  }
  timer.run();
}

void initHardware()
{
  // Serial and EEPROM
  Serial.begin(115200);
  EEPROM.begin(512);
  delay(10);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(LED_BLUE, LOW);

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

/////////////////////////////
// AP Setup Mode Functions //
/////////////////////////////

// Load Saved Configuration from EEPROM
boolean loadSavedConfig() {
  Serial.println("Reading Saved Config....");
  String ssid = "";
  String password = "";
  if (EEPROM.read(0) != 0) {
    for (int i = 0; i < 32; ++i) {
      ssid += char(EEPROM.read(i));
    }
    Serial.print("SSID: ");
    Serial.println(ssid);
    for (int i = 32; i < 96; ++i) {
      password += char(EEPROM.read(i));
    }
    Serial.print("Password: ");
    Serial.println(password);
    WiFi.begin(ssid.c_str(), password.c_str());
    return true;
  }
  else {
    Serial.println("Saved Configuration not found.");
    return false;
  }
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
void Get_CurrentTime()
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

// Build the SSID list and setup a software access point for setup mode
void setupMode() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  delay(100);
  Serial.println("");
  for (int i = 0; i < n; ++i) {
    SSID_LIST += "<option value=\"";
    SSID_LIST += WiFi.SSID(i);
    SSID_LIST += "\">";
    SSID_LIST += WiFi.SSID(i);
    SSID_LIST += "</option>";
  }
  delay(100);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_IP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_SSID);
//  DNS_SERVER.start(53, "*", AP_IP);
//  startWebServer();
  Serial.print("Starting Access Point at \"");
  Serial.print(AP_SSID);
  Serial.println("\"");
}


/////////////////////////
// Debugging Functions //
/////////////////////////

void wipeEEPROM()
{
  EEPROM.begin(512);
  // write a 0 to all 512 bytes of the EEPROM
  for (int i = 0; i < 512; i++)
    EEPROM.write(i, 0);

  EEPROM.end();
}
