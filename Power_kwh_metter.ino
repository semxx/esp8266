#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
    #include <SPI.h>
    #include <Ethernet.h>
    #include <BlynkSimpleEthernet.h>
    #include <SimpleTimer.h>
    
    // You should get Auth Token in the Blynk App.
    // Go to the Project Settings (nut icon).
    char auth[] = "xxxxxxxxxxxxxxxx";
    
    SimpleTimer timer;
    
    long pulseCount = 0;   
    //Used to measure power.
    unsigned long pulseTime,lastTime;
    
    //power and energy
    double power, elapsedkWh;
    
    //Number of pulses per wh - found or set on the meter.
    int ppwh = 1; //1000 pulses/kwh = 1 pulse per wh
    
    void setup()
    {
      Serial.begin(9600);
    
    // KWH interrupt attached to IRQ 1  = pin3
      attachInterrupt(1, onPulse, FALLING);
      
      Blynk.begin(auth);
     
      // Setup a function to be called every second
      timer.setInterval(1000L, sendUptime);
      
    }
    void sendUptime()
    {
      // You can send any value at any time.
      // Please don't send more that 10 values per second.
     
      Blynk.virtualWrite(1, power);
    }
    
    void loop()
    {
     Blynk.run(); // Initiates Blynk
     timer.run(); // Initiates SimpleTimer
     
    }
    
    // The interrupt routine
    void onPulse()
    {
    
    //used to measure time between pulses.
      lastTime = pulseTime;
      pulseTime = micros();
    
    //pulseCounter
      pulseCount++;
    
    //Calculate power
      power = (3600000000.0 / (pulseTime - lastTime))/ppwh;
      
      //Find kwh elapsed
      elapsedkWh = (1.0*pulseCount/(ppwh*1000)); //multiply by 1000 to pulses per wh to kwh convert wh to kwh
    
    //Print the values.
      Serial.print(power,4);
      Serial.print(" ");
      Serial.println(elapsedkWh,3);
    }
