#include <SPI.h>          // SPI Protocol
#include <nRF24L01.h>     // Radio Class
#include <RF24.h>         // Radio Class
#include <TimerOne.h>     // Timer Class

#define RELAY 7           // relay output pin
#define TIMER_DELAY 10    // timer delay of 10 millisec
#define CE 8    
#define CSN 9
#define INTERRUPT_INTERVAL 200000   //20ms

//////////////////////////////////////////////////////////////////////////////////////
// Global Variables
//////////////////////////////////////////////////////////////////////////////////////
enum STATE {INIT, READY, THROWING, LOADING, WAITING};
enum STATE myState;
char command[10] = "";                            // Radio message buffer
String strCommand;                                // String to store command from command[]
const uint64_t PIPE = 0x01;                       // Pipe address
RF24 radio(CE, CSN);                              // set up radio config
const char* T_NAME = "TARGET ONE";                // Target name - will transmit upon initialization
static byte count;                                // used in the ISR to count the number of XXms interrupts that have occured
const byte THROW_WAIT = 2;                        //time in 20ms increments it takes to throw a pigeion
const byte LOAD_WAIT = 9;                         //time in 20ms increments it takes for the thrower to reload
int targetCount, delayCount;                      //tracks the number of pigeons to throw
bool relayFired;                                  //indicates whether the relay has been activated yet


//////////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
//////////////////////////////////////////////////////////////////////////////////////
void setupHardware(void);
void setupRadio(void);
void setupTimer(void);
bool checkRadio(void);

//////////////////////////////////////////////////////////////////////////////////////
// setup() Program execution begins here
//////////////////////////////////////////////////////////////////////////////////////
void setup()
{
  setupHardware();
  setupRadio();
  setupTimer();
  enum STATE myState = INIT;
}

//////////////////////////////////////////////////////////////////////////////////////
// loop() Infintie loop of exectuion - executes following setup()
//////////////////////////////////////////////////////////////////////////////////////
 void loop()
{
  
  switch (myState){
    case INIT:
      Serial.println("Changing to READY");
      relayFired = false;
      myState = READY;
      break;
    
    case READY:
      //Serial.println("READY");
      if (checkRadio()){
        processCommand();
        myState = THROWING;                             //change STATE
        Serial.println("Changing to THROWING");
        Timer1.restart();                               //Reset the interrupt timer
        count = 0;                                      //Reset the counter
      }
      break;
    
    case THROWING:
      //Serial.println("THROWING");
      if (!relayFired){                                 //Executes the first time in the STATE
         digitalWrite(RELAY, HIGH);                     //TURN PIN ON FOR 1 SECOND
         targetCount--;                                 //Decrement the number of targets to throw
         relayFired = true;
      }
      if (count >= THROW_WAIT){
        digitalWrite(RELAY, LOW);               //TURN PIN OFF
        myState = LOADING;                      //change STATE
        Serial.println("Changing to LOADING");
        Timer1.restart();                               //Reset the interrupt timer
        count = 0;                                      //Reset the counter
      }
      break;
    
    case LOADING:
      if (count >= LOAD_WAIT){
        relayFired = false;
        myState = WAITING;
        Serial.println("Changing to WAITING");
        Timer1.restart();                               //Reset the interrupt timer
        count = 0;                                      //Reset the counter
      }
      break;

    case WAITING:
      if (targetCount > 0){
        if(count >= delayCount){
          myState = THROWING;
          Serial.println("Changing to THROWING");
          Timer1.restart();                               //Reset the interrupt timer
          count = 0;                                      //Reset the counter
        }
      }
      else{
        myState = READY;
        Serial.println("Changing to READY");
      }
      break;
  }
}

//////////////////////////////////////////////////////////////////////////////////////
// processCommand(): Parses a recieved command and sets targetCount variables
//////////////////////////////////////////////////////////////////////////////////////
void processCommand(){
  uint32_t targetStart, targetEnd, delayEnd;  
  targetStart = strCommand.indexOf("C");                      //Extract command from request
  targetEnd = strCommand.indexOf("E");
  delayEnd = strCommand.indexOf("!");
  targetCount = strCommand.substring(targetStart+1, targetEnd).toInt();
  delayCount = (strCommand.substring(targetEnd+1, delayEnd).toInt())*(1000000/INTERRUPT_INTERVAL); 
  Serial.print("Target Count: ");
  Serial.println(targetCount);
  Serial.print("Delay: ");
  Serial.println(delayCount);
}

//////////////////////////////////////////////////////////////////////////////////////
// interrupt_SR(): Executes every 20ms (configured in the setupTimer() function
//////////////////////////////////////////////////////////////////////////////////////
void interrupt_SR(){
  count++;
  //Serial.println(count);
}

//////////////////////////////////////////////////////////////////////////////////////
// checkRadio: Checks if there is data to be read in the radio buffer. 
// return: bool - True if there was a command to read / false if nothing to read.
//////////////////////////////////////////////////////////////////////////////////////
bool checkRadio(){
  if (radio.available()){                               //If there is data available to read
    radio.read(&command, sizeof(command));              //read data and store in command buffer
    String c(command);                                  //save char[] as string
    strCommand = c; 
    Serial.println("Check Radio: " + strCommand);                                
    return true;
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////////////
// setupRadio: Setup the RF24 power, datarate, channel and pipe
//////////////////////////////////////////////////////////////////////////////////////
void setupRadio(){
  radio.begin();                    // Turn on radio
  radio.setPALevel(RF24_PA_LOW);    // Set power level RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX
  radio.setDataRate(RF24_250KBPS);  // Set data rate
  radio.setChannel(124);            // Set channel
  radio.openReadingPipe(1, PIPE);   // Open pipe
  radio.startListening();           // Start listening
  Serial.println("Start Listening");
}

//////////////////////////////////////////////////////////////////////////////////////
// setupRadio: Setup the RF24 power, datarate, channel and pipe
//////////////////////////////////////////////////////////////////////////////////////
void setupHardware(){
  Serial.begin(9600);
  pinMode(RELAY, OUTPUT);                     // relay control
  digitalWrite(RELAY, LOW);                   // start relay OFF ->(NC)is ON
}

//////////////////////////////////////////////////////////////////////////////////////
// setupTimer: Setup Timer1 (hardware timer) and associate an interrup_SR() that executes every overflow
//////////////////////////////////////////////////////////////////////////////////////
void setupTimer(void){
  Timer1.initialize(INTERRUPT_INTERVAL);                    //Setup timer for a 20ms interval
  Timer1.attachInterrupt(interrupt_SR);         //isrHandler() will be called ever 20ms
}
