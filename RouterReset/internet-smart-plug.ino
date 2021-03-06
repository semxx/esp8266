#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

// GPIO 2
#define GPIO 5

const char* WIFI_SSID            = "Xiaomi_2G";
const char* WIFI_PASSWORD        = "panatorium";

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

void setup() {
  pinMode(GPIO, OUTPUT);
  digitalWrite(GPIO, 1);
  
  Serial.begin(115200);
  delay(1000);
  Serial.println("");
  Serial.println("Internet Smart Plug");
  connectToWifi();
}

void loop() {
  boolean online;
  unsigned short i = 0;

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
  } else {
    Serial.println("nothing to do");
  }

  stopWifiAndReboot();
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

// Turn plug off then on
void cyclePower() {
  Serial.println("turning plug off");
  digitalWrite(GPIO, 0);
  delay(5000);
  digitalWrite(GPIO, 1);
  Serial.println("turning plug on");
}

