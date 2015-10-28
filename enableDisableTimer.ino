bool enableTimer = 0; // Defaults to disable timer (button on vPin 3)
bool timerLights; // Defaults to timer is running (timer widget on vPin 4)

bool manualTree = 0; // Tree lamp override (vPin 5)
bool manualLamp = 0; // Normal lamp override (vPin 6)

void setup()
{
	Blynk.auth(auth stuff here);
}

BLYNK_WRITE(3)  // Timer enable/disable
{
	int setting = param.asInt();
	enableTimer = setting;
}

BLYNK_WRITE(4)  // Actual timer to vPin
{
	int setting = param.asInt();
	timerLights = setting;
}

BLYNK_WRITE(5)  // Manual override for Tree
{
	int setting = param.asInt();
	manualTree = setting;
}

BLYNK_WRITE(6)  // Manual override for Lamp
{
	int setting = param.asInt();
	manualLamp = setting;
}

void loop()
{
	Blynk.run();  // Blynk magic here
	checkTimer(); // My things here
}

void checkTimer()
{
	// If timer is on and timer is running, turn lights on
	if(((enableTimer == 1) && (timerLights == 1)) || (manualTree == 1))
{
  // Timer is running, so lights on!
  digitalWrite(A1, HIGH);
}
else
{
  digitalWrite(A1, LOW);
}

if(((enableTimer == 1) && (timerLights == 1)) || (manualLamp == 1))
{
  // Timer is running, so lights on!
  digitalWrite(A0, HIGH);
}
else
{
  digitalWrite(A0, LOW);
}
}
