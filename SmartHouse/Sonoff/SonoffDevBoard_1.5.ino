/*
   1MB flash sizee
   sonoff header
   1 - vcc 3v3
   2 - rx
   3 - tx
   4 - gnd
   5 - gpio 14
   esp8266 connections
   gpio  0 - button
   gpio 12 - relay
   gpio 13 - green led - active low
   gpio 14 - pin 5 on header
   http://www.esp8266.com/wiki/lib/exe/fetch.php?cache=&media=esp8266-12_mod.png
  Распиновка на NodeMCU as Master (I2C) https://www.cnx-software.com/wp-content/uploads/2015/10/NodeMCU_v0.9_Pinout.png
   ESP8266 GPIO AVAILIBLE: 0, 2, 4, 5, 12, 13, 14, 15

  D0    GPIO - 16 WAKE
  D1    GPIO - 5  ~
  D2    GPIO - 4  ~
  D3    GPIO - 0  FLASH
  D4    GPIO - 2  ~
  D5    GPIO - 14 ~
  D6    GPIO - 12 ~
  D7    GPIO - 13 ~
  D8    GPIO - 15 ~
  D9    GPIO - 3  ~ (rx)
  D10   GPIO - 1  ~ (tx)
*/
#define SDA            D6         //  SDA   GPIO12
#define SCL            D5         //  SCL   GPIO14
#define   SONOFF_BUTTON             0         //0 - D3
#define   SONOFF_LED                2         //2 - D4
#define   ONE_WIRE_BUS              4         //4 - D2  Линия датчиков DS18B20
#define   RF_PIN                    15         //15 - D8  RF433 Transmitter
#define   SONOFF_AVAILABLE_CHANNELS 4

const int SONOFF_RELAY_PINS[4] =    {12, 13, 14, 5}; //  D6 D7 D5 D1

#define SONOFF_LED_RELAY_STATE      false
#define HOSTNAME "sonoff"

#define INCLUDE_BLYNK_SUPPORT
#define INCLUDE_MQTT_SUPPORT
#define INCLUDE_RF_433_SUPPORT
#define INCLUDE_DS18B20_SUPPORT

/********************************************
   Should not need to edit below this line *
 * *****************************************/

#include "functions.h"
#ifdef INCLUDE_BLYNK_SUPPORT
//#define BLYNK_DEBUG
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
SimpleTimer timer;
static bool BLYNK_ENABLED = true;
#endif

#ifdef INCLUDE_MQTT_SUPPORT
#include <PubSubClient.h>        //https://github.com/Imroy/pubsubclient
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
WiFiClient wclient;
PubSubClient mqttClient(wclient);
static bool MQTT_ENABLED              = true;
int         lastMQTTConnectionAttempt = 0;
#endif

#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoOTA.h>
#include <Ticker.h> //for LED status
#include <EEPROM.h>

#include <Wire.h>
//#include <HTU21D.h>
/*
HTU21D(resolution)
resolution:
HTU21D_RES_RH12_TEMP14 - RH: 12Bit, Temperature: 14Bit, by default
HTU21D_RES_RH8_TEMP12  - RH: 8Bit,  Temperature: 12Bit
HTU21D_RES_RH10_TEMP13 - RH: 10Bit, Temperature: 13Bit
HTU21D_RES_RH11_TEMP11 - RH: 11Bit, Temperature: 11Bit
*/
//HTU21D myHTU21D(HTU21D_RES_RH12_TEMP14);

#define EEPROM_SALT 12667
typedef struct {
  char  bootState[4]      = "off";
  char  blynkToken[33]    = "14730038da9e487cbb78fe18113011f2";
  char  blynkServer[33]   = "blynk-cloud.com";
  char  blynkPort[6]      = "8442";
  char  mqttHostname[33]  = "192.168.2.126";
  char  mqttPort[6]       = "1883";
  char  mqttClientID[24]  = HOSTNAME;
  char  mqttTopic[33]     = HOSTNAME;
  int   salt              = EEPROM_SALT;
} WMSettings;

WMSettings settings;
Ticker ticker;

#ifdef INCLUDE_RF_433_SUPPORT
//   #include <livolo.h>
#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();
//   Livolo livolo(RF_PIN);

//Socket 1416
char *socket1TriStateOn  = "FFFF0FFF0101";
char *socket1TriStateOff = "FFFF0FFF0110";
char *socket2TriStateOn  = "FFFF0FFF1001";
char *socket2TriStateOff = "FFFF0FFF1010";
char *socket3TriStateOn  = "FFFF0FF10001";
char *socket3TriStateOff = "FFFF0FF10010";

//Socket 1401
char *socket11TriStateOn  = "0FFFFFFF0101";
char *socket11TriStateOff = "0FFFFFFF0110";
char *socket12TriStateOn  = "0FFFFFFF1001";
char *socket12TriStateOff = "0FFFFFFF1010";
char *socket13TriStateOn  = "0FFFFFF10001";
char *socket13TriStateOff = "0FFFFFF10010";

void SendRF433(const char *ch) {
  mySwitch.sendTriState(ch);
  Serial.print("Send RF code: ");
  Serial.println (*ch);
}

#endif

#ifdef INCLUDE_DS18B20_SUPPORT

#include <OneWire.h>                   //  Для DS18S20, DS18B20, DS1822 
#include <DallasTemperature.h>         //  Для DS18S20, DS18B20, DS1822

OneWire oneWire(ONE_WIRE_BUS); // http://cdn.chantrell.net/blog/wp-content/uploads/2011/10/DS18B20_Connection.jpg
DallasTemperature sensorsDS18B20(&oneWire);

float Out_Temp, Indoor_Temp, Input_Temp, Output_Temp, Hym;

byte Indoor_t[8] = {0x28, 0xFF, 0x91, 0xB0, 0x87, 0x16, 0x03, 0x1F}; // Температура внутри помещения
byte Out_t[8]    = {0x28, 0xFF, 0xA2, 0xB5, 0x90, 0x16, 0x04, 0xE7}; // Температура на улице
byte Therm_1_t[8] = {0x28, 0xFF, 0x83, 0x8F, 0x00, 0x15, 0x02, 0x21}; // Температура обратка
byte Therm_2_t[8] = {0x28, 0xFF, 0x0B, 0x0A, 0x62, 0x15, 0x01, 0x84}; // Температура выход


void UpdateTemp()
{
  Serial.println("UpdateTemp() has been requested");

  sensorsDS18B20.requestTemperatures();

  Indoor_Temp = sensorsDS18B20.getTempC(Indoor_t);
  Out_Temp = sensorsDS18B20.getTempC(Out_t);
  Input_Temp = sensorsDS18B20.getTempC(Therm_1_t);
  Output_Temp = sensorsDS18B20.getTempC(Therm_2_t);
//  Hym = myHTU21D.readHumidity();
  
  char t1_buffer[15];
  char t2_buffer[15];
  char t3_buffer[15];
  char t4_buffer[15];
  char h1_buffer[15];
  
  dtostrf(Indoor_Temp, 4, 2, t1_buffer);
  dtostrf(Out_Temp,    4, 2, t2_buffer);
  dtostrf(Input_Temp,  4, 2, t3_buffer);
  dtostrf(Output_Temp, 4, 2, t4_buffer);
//  dtostrf(Hym, 4, 2, h1_buffer);
  
#ifdef INCLUDE_MQTT_SUPPORT
  char topic[50];
  sprintf(topic, "%s/thermal-1/status", settings.mqttTopic);
  mqttClient.publish(topic, t1_buffer);

  sprintf(topic, "%s/thermal-2/status", settings.mqttTopic);
  mqttClient.publish(topic, t2_buffer);

  sprintf(topic, "%s/thermal-3/status", settings.mqttTopic);
  mqttClient.publish(topic, t3_buffer);

  sprintf(topic, "%s/thermal-4/status", settings.mqttTopic);
  mqttClient.publish(topic, t4_buffer);
#endif

#ifdef INCLUDE_BLYNK_SUPPORT
  Blynk.virtualWrite(V20, t1_buffer);
  Blynk.virtualWrite(V21, t2_buffer);
  Blynk.virtualWrite(V22, t3_buffer);
  Blynk.virtualWrite(V23, t4_buffer);
//  Blynk.virtualWrite(V25, h1_buffer);
#endif
}

#endif

const int CMD_WAIT = 0;
const int CMD_BUTTON_CHANGE = 1;

int cmd = CMD_WAIT;  //int relayState = HIGH;
int buttonState = LOW;  //inverted button state

static long startPress = 0;

//http://stackoverflow.com/questions/9072320/split-string-into-string-array
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void tick()  //toggle led state
{
  int state = digitalRead(SONOFF_LED);  // get the current state of GPIO1 pin
  digitalWrite(SONOFF_LED, !state);     // set pin to the opposite state
}


void configModeCallback (WiFiManager *myWiFiManager) {  //gets called when WiFiManager enters configuration mode
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}


void updateBlynk(int channel) {
#ifdef INCLUDE_BLYNK_SUPPORT
  int state = digitalRead(SONOFF_RELAY_PINS[channel]);
  Blynk.virtualWrite(channel * 5 + 4, state * 255);
  int v_channel = channel + 1;
  if (state == HIGH) {
    Blynk.setProperty(v_channel * 5 - 4, "color", "#ff4f00");
  }
  else {
    Blynk.setProperty(v_channel * 5 - 4, "color", "#23C48E");
  }
  yield();
  Blynk.virtualWrite(V28,  getCurrentDate() + String("  ") + getCurrentTime() );
  Blynk.setProperty(V28, "label", String("WIFI: ") + String(map(WiFi.RSSI(), -105, -40, 0, 100)) + String("% (") + WiFi.RSSI() + String("dB)") + String(" IP: ") + WiFi.localIP().toString());
#endif
}


void updateMQTT(int channel) {
#ifdef INCLUDE_MQTT_SUPPORT
  int state = digitalRead(SONOFF_RELAY_PINS[channel]);
  char topic[50];
  sprintf(topic, "%s/channel-%d/status", settings.mqttTopic, channel);
  String stateString = state == 0 ? "off" : "on";
  if ( channel >= SONOFF_AVAILABLE_CHANNELS) {
    stateString = "disabled";
  }
  mqttClient.publish(topic, stateString);
  yield();
#endif
}


void setState(int state, int channel) {
  //relay
  digitalWrite(SONOFF_RELAY_PINS[channel], state);

  //led
  if (SONOFF_LED_RELAY_STATE) {
    digitalWrite(SONOFF_LED, (state + 1) % 2); // led is active low
  }
  //blynk
  updateBlynk(channel);

  //MQTT
  updateMQTT(channel);

}

void turnOn(int channel = 0) {
  int relayState = HIGH;
  setState(relayState, channel);
  digitalWrite(SONOFF_LED, LOW);
}

void turnOff(int channel = 0) {
  int relayState = LOW;
  setState(relayState, channel);
  digitalWrite(SONOFF_LED, HIGH);
}

void toggleState() {
  cmd = CMD_BUTTON_CHANGE;
}

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}


void toggle(int channel) {
  Serial.println("toggle state");
  Serial.println(digitalRead(SONOFF_RELAY_PINS[channel]));
  int relayState = digitalRead(SONOFF_RELAY_PINS[channel]) == HIGH ? LOW : HIGH;
  setState(relayState, channel);
  digitalWrite(SONOFF_LED, !relayState);
  if (relayState == HIGH) {
    Blynk.setProperty(SONOFF_RELAY_PINS[channel], "color", "#ff4f00");
  }
  else {
    Blynk.setProperty(SONOFF_RELAY_PINS[channel], "color", "#23C48E");
  }

  //  Blynk.virtualWrite(channel * 5 + 4, state * 255);
}

void restart() {
  //TODO turn off relays before restarting
  ESP.reset();
  delay(1000);
}

void reset() {
  //reset settings to defaults
  //TODO turn off relays before restarting
  /*
    WMSettings defaults;
    settings = defaults;
    EEPROM.begin(512);
    EEPROM.put(0, settings);
    EEPROM.end();
  */
  //reset wifi credentials
  WiFi.disconnect();
  delay(1000);
  ESP.reset();
  delay(1000);
}

#ifdef INCLUDE_BLYNK_SUPPORT
/**********
   VPIN % 5
   0 off
   1 on
   2 toggle
   3 value
   4 led
 ***********/

BLYNK_WRITE_DEFAULT() {
  int pin = request.pin;
  int channel = pin / 5;
  int action = pin % 5;
  int a = param.asInt();
  if (a != 0) {
    switch (action) {
      case 0:
        turnOff(channel);
        break;
      case 1:
        turnOn(channel);
        break;
      case 2:
        toggle(channel);
        break;
      default:
        Serial.print("unknown action");
        Serial.print(action);
        Serial.print(channel);
        break;
    }
    Serial.print("Pin: ");        Serial.println(pin);
    Serial.print("action: ");     Serial.println(action);
    Serial.print("channel: ");    Serial.println(channel);
  }
}
/*
  BLYNK_READ_DEFAULT() {
  // Generate random response
  int pin = request.pin;
  int channel = pin / 5;
  int action = pin % 5;
  Blynk.virtualWrite(pin, digitalRead(SONOFF_RELAY_PINS[channel]));

  }
*/
//restart - button
BLYNK_WRITE(30) {
  int a = param.asInt();
  if (a != 0) {
    restart();
  }
}

//reset - button
BLYNK_WRITE(31) {
  int a = param.asInt();
  if (a != 0) {
    reset();
  }
}

// ext house light  - button
BLYNK_WRITE(32) {
  int a = param.asInt();
  if (a != 0) {
    Serial.println("Light on");
    mySwitch.setPulseLength(179);
    mySwitch.sendTriState(socket3TriStateOn);
    Blynk.setProperty(V32, "color", "#ED9D00");
  }
  else {
    Serial.println("Light off");
    mySwitch.setPulseLength(179);
    mySwitch.sendTriState(socket3TriStateOff);
    Blynk.setProperty(V32, "color", "#23C48E");

  }
}

// ext house light  - button
BLYNK_WRITE(33) {
  int a = param.asInt();
  if (a != 0) {
    Serial.println("Light on");
    mySwitch.setPulseLength(179);
    mySwitch.sendTriState(socket1TriStateOn);
    Blynk.setProperty(V33, "color", "#ED9D00");
  }
  else {
    Serial.println("Light off");
    mySwitch.setPulseLength(179);
    mySwitch.sendTriState(socket1TriStateOff);
    Blynk.setProperty(V33, "color", "#23C48E");

  }
}


BLYNK_WRITE(34) {
  String stateString = "auto";
  int a = param.asInt();
  if (a != 0) {
  char topic[50] = "/broadlink/ac-1/ac_heat";
  mqttClient.publish(topic, stateString);
  yield();  }
  else {  
 
  char topic[50] = "/broadlink/ac-1/ac_off";
  mqttClient.publish(topic, stateString);
  yield();  
  
  }
}
BLYNK_WRITE(35) {
  String stateString = "auto";
  int a = param.asInt();
  if (a != 0) {
  char topic[50] = "/broadlink/ac-2/ac_heat";
  mqttClient.publish(topic, stateString);
  yield();  }
  else {  
 
  char topic[50] = "/broadlink/ac-2/ac_off";
  mqttClient.publish(topic, stateString);
  yield();  
  
  }
}
#endif

#ifdef INCLUDE_MQTT_SUPPORT
void mqttCallback(const MQTT::Publish& pub) {
  Serial.print(pub.topic());
  Serial.print(" => ");
  if (pub.has_stream()) {
    int BUFFER_SIZE = 100;
    uint8_t buf[BUFFER_SIZE];
    int read;
    while (read = pub.payload_stream()->read(buf, BUFFER_SIZE)) {
      Serial.write(buf, read);
    }
    pub.payload_stream()->stop();
    Serial.println("had buffer");
  } else {
    Serial.println(pub.payload_string());
    String topic = pub.topic();
    String payload = pub.payload_string();

    if (topic == settings.mqttTopic) {
      Serial.println("exact match");
      return;
    }

    if (topic.startsWith(settings.mqttTopic)) {
      Serial.println("for this device");
      topic = topic.substring(strlen(settings.mqttTopic) + 1);
      String channelString = getValue(topic, '/', 0);
      if (!channelString.startsWith("channel-")) {
        Serial.println("no channel");
        return;
      }
      channelString.replace("channel-", "");
      int channel = channelString.toInt();
      Serial.println(channel);
      if (payload == "on") {
        turnOn(channel);
      }
      if (payload == "off") {
        turnOff(channel);
      }
      if (payload == "toggle") {
        toggle(channel);
      }
      if (payload == "") {
        updateMQTT(channel);
      }
#ifdef INCLUDE_RF_433_SUPPORT
      if (payload.startsWith("0FFFF") || payload.startsWith("FFFF0") ) {
        Serial.print ("RF_CODE_DETECTED:");
        Serial.println (payload);
        const char *rf_code = payload.c_str();
        SendRF433(rf_code);
      }
#endif
    }
  }

#endif
}

void setup()
{
  Serial.begin(115200);
  pinMode(SONOFF_LED, OUTPUT);  //set led pin as output
  ticker.attach(0.6, tick);  // start ticker with 0.5 because we start in AP mode and try to connect

  const char *hostname = HOSTNAME;

  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setConfigPortalTimeout(180); //timeout - this will quit WiFiManager if it's not configured in 3 minutes, causing a restart

  EEPROM.begin(512);  //custom params
  EEPROM.get(0, settings);
  EEPROM.end();

  if (settings.salt != EEPROM_SALT) {
    Serial.println("Invalid settings in EEPROM, trying with defaults");
    WMSettings defaults;
    settings = defaults;
  }
  WiFiManagerParameter custom_boot_state("boot-state", "on/off on boot", settings.bootState, 33);
  wifiManager.addParameter(&custom_boot_state);

  Serial.println(settings.bootState);

#ifdef INCLUDE_BLYNK_SUPPORT
  Serial.println(settings.blynkToken);
  Serial.println(settings.blynkServer);
  Serial.println(settings.blynkPort);

  WiFiManagerParameter custom_blynk_text("<br/>Blynk config. <br/> No token to disable.<br/>");
  wifiManager.addParameter(&custom_blynk_text);

  WiFiManagerParameter custom_blynk_token("blynk-token", "blynk token", settings.blynkToken, 33);
  wifiManager.addParameter(&custom_blynk_token);

  WiFiManagerParameter custom_blynk_server("blynk-server", "blynk server", settings.blynkServer, 33);
  wifiManager.addParameter(&custom_blynk_server);

  WiFiManagerParameter custom_blynk_port("blynk-port", "port", settings.blynkPort, 6);
  wifiManager.addParameter(&custom_blynk_port);
#endif


#ifdef INCLUDE_MQTT_SUPPORT
  Serial.println(settings.mqttHostname);
  Serial.println(settings.mqttPort);
  Serial.println(settings.mqttClientID);
  Serial.println(settings.mqttTopic);

  WiFiManagerParameter custom_mqtt_text("<br/>MQTT config. <br/> No url to disable.<br/>");
  wifiManager.addParameter(&custom_mqtt_text);

  WiFiManagerParameter custom_mqtt_hostname("mqtt-hostname", "Hostname", settings.mqttHostname, 33);
  wifiManager.addParameter(&custom_mqtt_hostname);

  WiFiManagerParameter custom_mqtt_port("mqtt-port", "port", settings.mqttPort, 6);
  wifiManager.addParameter(&custom_mqtt_port);

  WiFiManagerParameter custom_mqtt_client_id("mqtt-client-id", "Client ID", settings.mqttClientID, 24);
  wifiManager.addParameter(&custom_mqtt_client_id);

  WiFiManagerParameter custom_mqtt_topic("mqtt-topic", "Topic", settings.mqttTopic, 33);
  wifiManager.addParameter(&custom_mqtt_topic);
#endif

  wifiManager.setSaveConfigCallback(saveConfigCallback);  //set config save notify callback

  if (!wifiManager.autoConnect(hostname)) {
    Serial.println("failed to connect and hit timeout");
    ESP.reset();    //reset and try again, or maybe put it to deep sleep
    delay(1000);
  }

  //Serial.println(custom_blynk_token.getValue());
  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("Saving config");

    strcpy(settings.bootState, custom_boot_state.getValue());

#ifdef INCLUDE_BLYNK_SUPPORT
    strcpy(settings.blynkToken, custom_blynk_token.getValue());
    strcpy(settings.blynkServer, custom_blynk_server.getValue());
    strcpy(settings.blynkPort, custom_blynk_port.getValue());
#endif

#ifdef INCLUDE_MQTT_SUPPORT
    strcpy(settings.mqttHostname, custom_mqtt_hostname.getValue());
    strcpy(settings.mqttPort, custom_mqtt_port.getValue());
    strcpy(settings.mqttClientID, custom_mqtt_client_id.getValue());
    strcpy(settings.mqttTopic, custom_mqtt_topic.getValue());
#endif

    Serial.println(settings.bootState);
    Serial.println(settings.blynkToken);
    Serial.println(settings.blynkServer);
    Serial.println(settings.blynkPort);

    EEPROM.begin(512);
    EEPROM.put(0, settings);
    EEPROM.end();
  }

#ifdef INCLUDE_BLYNK_SUPPORT
  //config blynk
  if (strlen(settings.blynkToken) == 0) {
    BLYNK_ENABLED = false;
  }
  if (BLYNK_ENABLED) {
    Blynk.config(settings.blynkToken, settings.blynkServer, atoi(settings.blynkPort));
    Blynk.syncAll();
  }
#endif


#ifdef INCLUDE_MQTT_SUPPORT
  //config mqtt
  if (strlen(settings.mqttHostname) == 0) {
    MQTT_ENABLED = false;
  }
  if (MQTT_ENABLED) {
    mqttClient.set_server(settings.mqttHostname, atoi(settings.mqttPort));
  }
#endif

  //OTA
  ArduinoOTA.onStart([]() {
    Serial.println("Start OTA");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.begin();

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  ticker.detach();

  //setup button
  pinMode(SONOFF_BUTTON, INPUT);
  attachInterrupt(SONOFF_BUTTON, toggleState, CHANGE);

  //initialize output pins array
  for (int ch = 0; ch < SONOFF_AVAILABLE_CHANNELS; ch++)  {
    pinMode(SONOFF_RELAY_PINS[ch], OUTPUT);
    digitalWrite(SONOFF_RELAY_PINS[ch], HIGH);
  }

  if (strcmp(settings.bootState, "on") == 0) {
    turnOn();
  } else {
    turnOff();
  }

  //setup led
  if (!SONOFF_LED_RELAY_STATE) {
    digitalWrite(SONOFF_LED, LOW);
  }

  Serial.println("done setup");

#ifdef INCLUDE_RF_433_SUPPORT

  mySwitch.enableTransmit(RF_PIN);
  mySwitch.setPulseLength(179);

#endif

#ifdef INCLUDE_DS18B20_SUPPORT
  sensorsDS18B20.begin();
  sensorsDS18B20.requestTemperatures();
  timer.setInterval(300000L, UpdateTemp);
#endif
  
/*
  while (myHTU21D.begin() != true)
  {
    Serial.println(F("Si7021 sensor is faild or not connected"));
    delay(5000);
  }
*/

}


void loop()
{

  //ota loop
  ArduinoOTA.handle();

#ifdef INCLUDE_BLYNK_SUPPORT
  //blynk connect and run loop
  if (BLYNK_ENABLED) {
    Blynk.run();
  }
#endif


#ifdef INCLUDE_MQTT_SUPPORT
  //mqtt loop
  if (MQTT_ENABLED) {
    if (!mqttClient.connected()) {
      if (lastMQTTConnectionAttempt == 0 || millis() > lastMQTTConnectionAttempt + 3 * 60 * 1000) {
        lastMQTTConnectionAttempt = millis();
        Serial.println(millis());
        Serial.println("Trying to connect to mqtt");
        if (mqttClient.connect(settings.mqttClientID)) {
          mqttClient.set_callback(mqttCallback);
          char topic[50];
          //sprintf(topic, "%s/+/+", settings.mqttTopic);
          //mqttClient.subscribe(topic);
          sprintf(topic, "%s/+", settings.mqttTopic);
          mqttClient.subscribe(topic);

          sprintf(topic, "%s/rf", HOSTNAME);
          mqttClient.subscribe(topic);

          //TODO multiple relays
          //updateMQTT(0);

          for (int ch = 0; ch < SONOFF_AVAILABLE_CHANNELS; ch++)  {
            updateMQTT(ch);
          }

        } else {
          Serial.println("failed");
        }
      }
    } else {
      mqttClient.loop();
    }
  }
#endif

  //delay(200);
  //Serial.println(digitalRead(SONOFF_BUTTON));
  switch (cmd) {
    case CMD_WAIT:
      break;
    case CMD_BUTTON_CHANGE:
      int currentState = digitalRead(SONOFF_BUTTON);
      if (currentState != buttonState) {
        if (buttonState == LOW && currentState == HIGH) {
          long duration = millis() - startPress;
          if (duration < 1000) {
            Serial.println("short press - toggle relay");
            toggle(0);
          } else if (duration < 5000) {
            Serial.println("medium press - reset");
            restart();
          } else if (duration < 60000) {
            Serial.println("long press - reset settings");
            reset();
          }
        } else if (buttonState == HIGH && currentState == LOW) {
          startPress = millis();
        }
        buttonState = currentState;
      }
      break;
  }

  timer.run();
}
