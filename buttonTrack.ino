BLYNK_WRITE(V0)
{
  y = param.asInt();
  Serial.print(y);
  if ( y == 1)
  {
   digitalWrite(6, HIGH);
  }
  else if ( y == 0)
  {
    digitalWrite(6, LOW);
   }
}


void loop()
{
  buttonack ();
}
void buttonack()
{
  if ( y == 1 && z == 0)
  {
    Blynk.virtualWrite(V12, 255);
    BLYNK_LOG("LED12: on"); 
    z = 1;
  }
  if ( z == 1 && currenttime - buttonled > 30000)
  {
    Blynk.virtualWrite(V12, 255);
    buttonled = currenttime;
  }
  if ( y == 0 && z ==  1 )
  {
    Blynk.virtualWrite(V12, 0);
    BLYNK_LOG("LED12: off");
    z = 0;
  }
    if ( z == 0 && currenttime - buttonled > 30000)
  {
    Blynk.virtualWrite(V12, 0);
    buttonled = currenttime;
  }
}
