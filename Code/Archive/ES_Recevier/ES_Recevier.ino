#include <SPI.h>          // SPI Protocol
#include <nRF24L01.h>     // Radio Class
#include <RF24.h>         // Radio Class

#define RELAY 7           // relay output pin
#define TIMER_DELAY 10    // timer delay of 10 millisec
#define CE 8    
#define CSN 9

//////////////////////////////////////////////////////////////////////////////////////
// Global Variables
//////////////////////////////////////////////////////////////////////////////////////
char command[10] = "";                  // Radio message buffer
const uint64_t PIPE = 0x01;             // Pipe address
RF24 radio(CE, CSN);                    // set up radio config
const char* T_NAME = "TARGET ONE";     // Target name - will transmit upon initialization

//////////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
//////////////////////////////////////////////////////////////////////////////////////
void setupHardware(void);
void setupRadio(void);

//////////////////////////////////////////////////////////////////////////////////////
// setup() Program execution begins here
//////////////////////////////////////////////////////////////////////////////////////
void setup()
{
  setupHardware();
  setupRadio();
  
}

//////////////////////////////////////////////////////////////////////////////////////
// loop() Infintie loop of exectuion - executes following setup()
//////////////////////////////////////////////////////////////////////////////////////
 void loop()
{
  checkRadio();
  
  
}

void checkRadio(){
  //digitalWrite(RELAY, HIGH);           //Toggle LED on every command
  if (radio.available()){                               //If there is data available to read
    radio.read(&command, sizeof(command));              //read data and store in command buffer
    //Serial.println("Test");                                
    
  }
  //digitalWrite(RELAY, LOW);           //Toggle LED on every command
}

void setupRadio(){
  radio.begin();                    // Turn on radio
  radio.setPALevel(RF24_PA_LOW);    // Set power level
  radio.setDataRate(RF24_250KBPS);  // Set data rate
  radio.setChannel(124);            // Set channel
  radio.openReadingPipe(1, PIPE);   // Open pipe
  radio.startListening();           // Start listening
  Serial.println("Start Listening");
}

void setupHardware(){
  Serial.begin(115200);
  //pinMode(RELAY, OUTPUT);                     // relay control
  //digitalWrite(RELAY, LOW);                   // start relay OFF ->(NC)is ON
}
