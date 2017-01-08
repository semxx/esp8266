// Вызывате WDT Reset

long lastencoderValue = 0;
int lastEncoded = 0;
int encoderValue = 0;
int lastMSB = 0;
int lastLSB = 0;
#define R D1
#define L D2
void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(R, INPUT_PULLUP); 
  pinMode(L, INPUT_PULLUP);

  digitalWrite(R, HIGH); //turn pullup resistor on
  digitalWrite(L, HIGH); //turn pullup resistor on  

  attachInterrupt(R, handleInterrupt, CHANGE);
  attachInterrupt(L, handleInterrupt, CHANGE);
}

void handleInterrupt() {
  int MSB = digitalRead(R); //MSB = most significant bit
  int LSB = digitalRead(L); //LSB = least significant bit

  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;

  lastEncoded = encoded; //store this value for next time
}

void loop() {
    Serial.print("Encoder: ");
    Serial.println(encoderValue);
    delay(1000);
}
