// http://www.instructables.com/id/How-to-use-Virtual-Pins-in-Blynk/?ALLSTEPS

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "8d82695b6d7cxxxxxxxxx";
int rcvpin=A0;
void setup()
{
  Serial.begin(9600);
  Blynk.begin(auth, "TP-LINK", "yyy");
   pinMode(rcvpin,INPUT);
}
  
void loop() {  
   int ldrval=analogRead(rcvpin);
if(ldrval<100)

{
Blynk.virtualWrite(5, HIGH);
}
else {
     
      Blynk.virtualWrite(5, LOW);
      }
    
    Blynk.run();
     }
   

