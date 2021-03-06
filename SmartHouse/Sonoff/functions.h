#include <TimeLib.h>
/**************************************************************

                       General Functions

 **************************************************************/
/*
  getCurrentTime() - returns the current time as String 11:59:59
*/
String getCurrentTime() {
  String extraZeroH, extraZeroM, extraZeroS;
  if (hour() < 10) extraZeroH = '0';
  if (minute() < 10) extraZeroM = '0';
  if (second() < 10) extraZeroS = '0';
  return String(extraZeroH + hour()) + ':' + extraZeroM + minute() + ':' + extraZeroS + second();
}
/*
  getCurrentDate() - returns the current date as String DD-MM-YYYY
*/
String getCurrentDate() {
  return String(day()) + '-' + monthShortStr(month()) + '-' + year();
}
/*
  printTimeDate() - prints the current date and time to terminal with line break
*/
void printTimeDate() {
//  terminal.println("-----------------------");
//  terminal.println( getCurrentDate() + String(" | ") + getCurrentTime() );
}
/*
  printOutput() - easy print to terminal with date function
*/
void printOutput(String a) {
  printTimeDate();
//  terminal.println(a);
//  terminal.flush();
}
/*
  formatTime() - Format millis in to MM:SS
*/
String formatTime(long milliSeconds) {
  long hours = 0, mins = 0, secs = 0;
  String returned, secs_o, mins_o;
  secs = milliSeconds / 1000;
  mins = secs / 60; 
  hours = mins / 60; 
  secs = secs - (mins * 60); 
  mins = mins - (hours * 60); 
  if (secs < 10 && mins) secs_o = "0";
  if (mins) returned += mins + String("m ");
  returned += secs_o + secs + String("s");
  return returned;
}
float a = 17.271;
float b = 237.7;
float calculateDewPointGamma(float temperature, float humidity)
{
  float gamma = ((a * temperature) / (b + temperature)) + log(humidity / 100);

  return gamma;
}


float calculateDewPoint(float temperature, float humidity)
{
  float gammavalue = calculateDewPointGamma(temperature, humidity);
 
  float dewpoint = (b * gammavalue) / (a - gammavalue);

  return dewpoint;
 
}



float calculateHumidexE(float dewpoint)
{
  float e = 5417.7530 * ((1/273.16) - (1 / (273.16 + dewpoint)));

  return e;
}


float calculateHumidex(float temperature, float dewpoint)
{
  float humidex = temperature + 0.5555 * ( 6.11 * exp(calculateHumidexE(dewpoint)) - 10);

  return humidex;
}

