//#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <SPI.h>
#include <Ethernet.h>
#include <BlynkSimpleEthernet.h>
#include <SimpleTimer.h>

char auth[] = "xxxxxxxxx";

SimpleTimer timer;

#define echoPin 8 // Echo Pin
#define trigPin 9 // Trigger Pin

long duration, distance; // Duration used to calculate distance

void RepeatTask()
{
  /* The following trigPin/echoPin cycle is used to determine the
 distance of the nearest object by bouncing soundwaves off of it. */ 
 digitalWrite(trigPin, LOW); 
 delayMicroseconds(2); 

 digitalWrite(trigPin, HIGH);
 delayMicroseconds(10); 
 
 digitalWrite(trigPin, LOW);
 duration = pulseIn(echoPin, HIGH);
 
 //Calculate the distance (in cm) based on the speed of sound.
 distance = duration/58.2;

 //Serial.println(distance);
}

void setup() {
 Serial.begin(9600);
 Blynk.begin(auth);
 pinMode(trigPin, OUTPUT);
 pinMode(echoPin, INPUT);
 timer.setInterval(1000, RepeatTask);
}

BLYNK_READ(V8)
{
Blynk.virtualWrite(8,distance);// virtualpin 8 distance
}

void loop() {
  Blynk.run(); // Initiates Blynk
  timer.run(); // Initiates SimpleTimer
}
