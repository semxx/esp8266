// exmaple code from Sarunyu Chompupon
// connect RST to GPIO16

#include <ESP8266WiFi.h>

extern "C" {
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "cont.h"
}

const char* ssid     = "ssid";
const char* password = "password";

WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  Serial.println("Awake");
  delay(10000);
  Serial.println("test");
  delay(10000);
  Serial.println("Sleep for 1 min");
  // Call API-Function
  system_deep_sleep(60000000);
  delay(100);
  Serial.println("Awake after deep sleep");
}
