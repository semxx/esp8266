      //house air is cooler than set temp

      if ((setCooling == 1) && (livingTemp > houseTemp) && (houseTemp < setTemp) && (externTemp > houseTemp) && (roofTemp > houseTemp))
      {
        digitalWrite(roofVent, LOW); // roof vent closed
		

void doSomething()
{
  onCycles++;
  Blynk.virtualWrite(V5, onCycles);
  BLYNK_LOG("Wrote V5: %d", int(onCycles));
 
  WiFi.disconnect();
  ESP.deepSleep(10000000, WAKE_RF_DEFAULT); // Sleep for 10 second
}


BLYNK_CONNECTED() {
Blynk.syncAll();
BLYNK_LOG("Synced all");
}

BLYNK_CONNECTED() {
      Blynk.syncAll();
}

//////////////////////////////////////////////////////////////////////////////////////

#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

//#define BLYNK_DEBUG        // Optional, this enables more detailed prints

char auth[] = "xxxxxxxxxxxxxxxxxxxxx";
WidgetLED led1(V3); //register to virtual pin 3


double onCycles = 0;

BLYNK_WRITE(V5) 
{
    onCycles = param.asDouble();
    BLYNK_LOG("Read V5: %f", onCycles);
}


// This function sends the number of cycles the board was started
void doSomething()
{
  onCycles++;
  Blynk.virtualWrite(V5, onCycles);
  BLYNK_LOG("Wrote V5: %d", int(onCycles));
 
  led1.on();  
  delay(10);
  led1.off();

  WiFi.disconnect();
  ESP.deepSleep(10000000, WAKE_RF_DEFAULT); // Sleep for 10 second
}


void setup()
{
  Serial.begin(115200);
  Serial.println("Serial started");

  //pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output - destroys serial protocol!

  Blynk.begin(auth,"SSID", "Password");

  while (Blynk.connect() == false) {
    // Wait until connected
      Serial.println("Blynking");
    delay(10);
  }
    Serial.println("Connected");

  Blynk.syncAll();

  BLYNK_LOG("Synced");
  doSomething();
  
}

void loop()
{
  Blynk.run(); // Initiates Blynk
}
