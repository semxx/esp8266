#include "blynk/blynk.h"
#include "blynk/BlynkSimpleSparkCore.h"
#include "SparkCorePolledTimer/SparkCorePolledTimer.h"
#include "Adafruit_DHT/Adafruit_DHT.h"
#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22		// DHT 22 (AM2302)
#define BLYNK_PRINT Serial

SparkCorePolledTimer updateTimer(5000); //Create a timer object and set it's timeout in milliseconds
void OnTimer(void);   //Prototype for timer callback method

DHT dht(DHTPIN, DHTTYPE);
float h, t, f, c, 
int m, mRaw, l, lRaw;

char auth[] = "MY_AUTH_TOKEN";  

void setup() {
    Serial.begin(9600);
    updateTimer.SetCallback(OnTimer);
    Serial.println("DHTxx test!");
    dht.begin();
    delay(5000); // Allow board to settle
    pinMode(A4, OUTPUT);
    pinMode(A5, OUTPUT);
    Blynk.begin(auth);
}

void loop() {

    Blynk.run();
    updateTimer.Update();
    
    h = dht.getHumidity();
    t = dht.getTempCelcius();
    mRaw = analogRead(A4);
    lRaw = analogRead(A5);
    m = map(mRaw, 400, 3200, 0, 100);
    l = map(lRaw, 400, 3200, 0, 100);
  
// Check if any reads failed and exit early (to try again).
	if (isnan(h) || isnan(t) || isnan(f)) {
		Serial.println("Failed to read from DHT sensor!");
		return;
	}
}

void OnTimer(void) {  //Handler for the timer, will be called automatically
  
  t = ((int) (t * 10) / 10.0);
  h = ((int) (h * 10) / 10.0);

  Blynk.virtualWrite(21, t);
  Blynk.virtualWrite(22, h);
  Blynk.virtualWrite(23, m);
  Blynk.virtualWrite(24, l);
}
