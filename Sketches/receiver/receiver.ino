//////////////////////////////////////////////////////////////////////////////////////////
// PIN DEFINITIONS //
//////////////////////////////////////////////////////////////////////////////////////////
const int ONBOARD_LED = 13; // Thing's onboard, green LED




void setup() {
  initHardware();

}



void loop() {
  digitalWrite (ONBOARD_LED, !digitalRead(ONBOARD_LED));
  delay (200);

  
}


//////////////////////////////////////////////////////////////////////////////////////////
// INITHARDWARE - STARTS THE SERIAL INTERFACE AND CONFIGURES ALL GPIO PINS
//////////////////////////////////////////////////////////////////////////////////////////
void initHardware() {
  Serial.begin(115200);
  //SETUP GPIO PINS and set to LOW
  pinMode(ONBOARD_LED, OUTPUT);
  
}