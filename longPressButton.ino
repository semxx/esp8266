// EXAMPLE

int longPressMillis = 1000;    // time in millis needed for longpress
boolean buttonState = false

// Blynk On/Off button in app
BLYNK_WRITE(1) {
  if (param.asInt() == 1) {     // if button is pressed
     longPressTimer = timer.setTimeout(longPressMillis, onOffToggle);  // start the timer to run the function in defined time
     } else {
       timer.deleteTimer(longPressTimer);  // Stop and delete the timer if button is released to early
     }
   }
    

// Function called by BLYNK_WRITE(1)
// This function toggles something
void onOffToggle() {
  if (buttonState == true) {
    // do something if true
    buttonState = false;
  } else {
    // do something if not true
   buttonState = true;
  }
}
