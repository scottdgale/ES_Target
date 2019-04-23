#include <SPI.h>          // SPI Protocol
#include <nRF24L01.h>     // Radio Class
#include <RF24.h>         // Radio Class
#include <TimerOne.h>     // Timer Class

#define S1_IN 2           // input for switch 1
#define S2_IN 3           // input for switch 2
#define M_EN 6            // pwm enable signal to motor controller
#define M_IN1 5           // input 1 to motor
#define M_IN2 4           // input 2 to motor
#define PIN_10 10         // used for random seed

#define TIMER_DELAY 10    // timer delay of 10 millisec
#define CE 8    
#define CSN 9
#define INTERRUPT_INTERVAL 200000   //20ms
#define FAST 255
#define MED 200
#define SLOW 140

//////////////////////////////////////////////////////////////////////////////////////
// Global Variables
//////////////////////////////////////////////////////////////////////////////////////
enum STATE {INIT, READY, EXPOSING, EXPOSED, CONCEALING, CONCEALED};
enum STATE myState;
char command[15] = {0};                           // Radio message buffer
String strCommand;                                // String to store command from command[]
const uint64_t PIPE = 0x02;                       // Pipe address
RF24 radio(CE, CSN);                              // set up radio config
const char* T_NAME = "TARGET ONE";                // Target name - will transmit upon initialization
static byte count;                                // used in the ISR to count the number of XXms interrupts that have occured
int concealCount, exposeCount;                    // How long to conceal / how long to expose
int targetCycles, cyclesComplete;                 // How many target expose / conceal iterations - how many have occured
bool exposed;                                     //indicates if the target is exposed
char mode;                                        // indicates mode of operation (M, U, or R)
int minRandom = 1;                                // used in R mode - min random number
int maxRandom = 5;                                // used in R mode - max random number
int dutyCycle = MED;                              // set default PWM duty cycle

//////////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
//////////////////////////////////////////////////////////////////////////////////////
void setupHardware(void);
void setupRadio(void);
void setupTimer(void);
bool checkRadio(void);
bool processCommand(void);
void setRandomExpose(void);
void setRandomConceal(void);
void changeSpeed(void);

//////////////////////////////////////////////////////////////////////////////////////
// setup() Program execution begins here
//////////////////////////////////////////////////////////////////////////////////////
void setup()
{
  setupHardware();
  setupRadio();
  setupTimer();
  enum STATE myState = INIT;
  randomSeed(analogRead(PIN_10));           // Seed random number generator 
}

//////////////////////////////////////////////////////////////////////////////////////
// loop() Infintie loop of exectuion - executes following setup()
//////////////////////////////////////////////////////////////////////////////////////
 void loop()
{
  switch (myState){
    case INIT:
      
      digitalWrite(M_IN1, LOW);                           //Conceal
      digitalWrite(M_IN2, HIGH);
      analogWrite(M_EN, SLOW);                             // reset motor to start position using slow PWM setting
      while(digitalRead(S1_IN)) {}                        // Move target to conceal position (clockwise motor shaft movement)

      digitalWrite(M_IN2, LOW);                           // BREAK motor
      delay(100);
      digitalWrite(M_EN, LOW); 

      exposed = false;
      //targetCycles = 5;                                  // Eventually will become an input
      
      
      Serial.println("Changing to READY");
      myState = READY;
      break;
    
    case READY:
      //Serial.println("READY");
      cyclesComplete = 0;
      if (checkRadio()){
        if (processCommand()){
          myState = EXPOSING;                                  // change STATE
          Serial.println("Changing to REVEALING");
        }
      }
      break;
    
    case EXPOSING:
      Serial.println("EXPOSING");
      digitalWrite(M_IN1, HIGH);                        // Start Motor Reveal Movement
      digitalWrite(M_IN2, LOW);
      analogWrite(M_EN, dutyCycle);                     // set PWM based on user input
      
      while(digitalRead(S2_IN)) {}                      //Spin until the switch is activated
      
      digitalWrite(M_IN2, HIGH);                        // BREAK;
      delay(100);
      digitalWrite(M_EN, LOW); 

      exposed = true;

      if (mode == 'R')
        setRandomExpose();                              // In random mode this will generate expose time 
      
      myState = EXPOSED;                                // change STATE
      Serial.println("Changing to EXPOSED");
      Timer1.restart();                                 // Reset the interrupt timer
      count = 0;                                        // Reset the counter
      
      break;
    
    case EXPOSED:
      if (count >= exposeCount){
        myState = CONCEALING;
        Serial.println("Changing to CONCEALING");
      }
      break;

    case CONCEALING:
      Serial.println("CONCEALING");
      digitalWrite(M_IN1, LOW);                           // Concealthe target
      digitalWrite(M_IN2, HIGH);
      analogWrite(M_EN, dutyCycle);                       // set PWM based on user input
      //digitalWrite(M_EN, HIGH);

      while(digitalRead(S1_IN)) {}                        // Move target to conceal position (clockwise motor shaft movement)

      digitalWrite(M_IN2, LOW);                           // BREAK motor
      delay(100);
      digitalWrite(M_EN, LOW); 

      exposed = false;
      cyclesComplete++;

      if (mode == 'R')
        setRandomConceal();                              // In random mode this will generate expose time 

      myState = CONCEALED;
      Serial.println("Changing to CONCEALED");
      Timer1.restart();                               //Reset the interrupt timer
      count = 0;                                      //Reset the counter
      break;
      
    case CONCEALED:
      if (cyclesComplete == targetCycles){
        myState = READY;
        Serial.println("Changing to READY");
      }
      else if (count >= concealCount){
        myState = EXPOSING;
        Serial.println("Changing to EXPOSING");
        Timer1.restart();                               //Reset the interrupt timer
        count = 0;  
      }
      break;
  }
}

//////////////////////////////////////////////////////////////////////////////////////
// processCommand(): Parses a recieved command and sets targetCount variables
// return true if the command is related to moving the target; otherwise false
//////////////////////////////////////////////////////////////////////////////////////
bool processCommand(){
  uint32_t concealStart, concealEnd, exposeEnd, targetEnd;  
  if (strCommand.indexOf("PW")!= -1){                          // Change the power on the RF signal
    changePower();
  }
  else if (strCommand.indexOf("SP") != -1){                    // Change the PWM duty cylce for the motor
    changeSpeed();
  }
  else{
    concealStart = strCommand.indexOf("C");                      //Extract command from request
    concealEnd = strCommand.indexOf("E");
    exposeEnd = strCommand.indexOf("T");
    targetEnd = strCommand.indexOf("!");
    concealCount = (strCommand.substring(concealStart+1, concealEnd).toInt())*(1000000/INTERRUPT_INTERVAL); 
    exposeCount = (strCommand.substring(concealEnd+1, exposeEnd).toInt())*(1000000/INTERRUPT_INTERVAL);
    
    if (strCommand.indexOf("M")!= -1) {                           // If manual mode then targetCycles = 1
      mode = 'M';
      targetCycles = 1;
    }
    else if (strCommand.indexOf("U") !=-1){
       mode = 'U';
       targetCycles = strCommand.substring(exposeEnd+1, targetEnd).toInt();
    }
    else if (strCommand.indexOf("R") != -1){ // Must be random mode
      mode = 'R';
      targetCycles = strCommand.substring(exposeEnd+1, targetEnd).toInt();
      if (concealCount <= exposeCount){
        minRandom = concealCount;
        maxRandom = exposeCount;
      }
      else{
        minRandom = exposeCount;
        maxRandom = concealCount;
      }
    }
    Serial.println(mode);
    Serial.print("Conceal: ");
    Serial.println(concealCount);
    Serial.print("Delay: ");
    Serial.println(exposeCount);
    Serial.print("Target Cycles: ");
    Serial.println(targetCycles);
    return true; 
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////////////
// setRandomConceal: Generate a random conceal time bounded by user input contained
//    in the command. The min value is contained in concealCount and the maximum
//    value is contained in the exposeCount.
//////////////////////////////////////////////////////////////////////////////////////
void setRandomConceal(){
    // Random conceal / expose generation
    // Limits are between concealCount(min) and exposeCount(max) 
    concealCount = random (minRandom, maxRandom+1);
    Serial.print("Generated random conceal time of: ");
    Serial.println(concealCount);
}

//////////////////////////////////////////////////////////////////////////////////////
// setRandomExpose: Generate a random expose time bounded by user input contained
//    in the command. The min value is contained in concealCount and the maximum
//    value is contained in the exposeCount.
//////////////////////////////////////////////////////////////////////////////////////
void setRandomExpose(){
    // Random conceal / expose generation
    // Limits are between minRandom and maxRandom
    exposeCount = random (minRandom, maxRandom+1);
    Serial.print("Generated random expose time of: ");
    Serial.println(exposeCount);
}

//////////////////////////////////////////////////////////////////////////////////////
// interrupt_SR(): Executes every 20ms (configured in the setupTimer() function
//////////////////////////////////////////////////////////////////////////////////////
void interrupt_SR(){
  count++;
  //Serial.println(count);
  //int s = digitalRead(S1_IN);
  //Serial.println(s);
}

//////////////////////////////////////////////////////////////////////////////////////
// changeSpeed(): Changes the dutycycle of the PWM controling the motor
// Set dutycycle to SLOW, MED, or FAST
//////////////////////////////////////////////////////////////////////////////////////
void changeSpeed(){
  if (strCommand == "~0SPL!"){
    dutyCycle = SLOW;
    Serial.println("Speed set to SLOW"); 
  }   
  else if (strCommand == "~0SPA!"){
    dutyCycle = MED;
    Serial.println("Speed set to MED"); 
  }
  else if (strCommand == "~0SPH!"){
    dutyCycle = FAST;
    Serial.println("Speed set to FAST");
  }    
}

//////////////////////////////////////////////////////////////////////////////////////
// changePower(): Changes the power of the RF radio
// Set power level RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX
//////////////////////////////////////////////////////////////////////////////////////
void changePower(){
  if (strCommand == "~0PWL!"){
    radio.setPALevel(RF24_PA_LOW);
    Serial.println("Power set to low"); 
  }   
  else if (strCommand == "~0PWA!"){
    radio.setPALevel(RF24_PA_HIGH); 
    Serial.println("Power set to high");  
  }
  else if (strCommand == "~0PWH!"){
    radio.setPALevel(RF24_PA_MAX); 
    Serial.println("Power set to max"); 
  }    
}

//////////////////////////////////////////////////////////////////////////////////////
// checkRadio: Checks if there is data to be read in the radio buffer. 
// return: bool - True if there was a command to read / false if nothing to read.
//////////////////////////////////////////////////////////////////////////////////////
bool checkRadio(){
  if (radio.available()){                               //If there is data available to read
    radio.read(&command, sizeof(command));              //read data and store in command buffer
    Serial.println (command);
    String c(command);                                  //save char[] as string
    strCommand = c; 
    Serial.println("Check Radio: " + strCommand);                                
    if (strCommand.indexOf("~")!= -1)                   //validate command (if contains ~)
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
  pinMode (S1_IN, INPUT_PULLUP);         // input for switch 1
  digitalWrite(S1_IN, HIGH);             // One forum said this initializes the pull-up resistor???
  pinMode(S2_IN, INPUT_PULLUP);          // input for switch 2
  pinMode(M_EN, OUTPUT);                 // pwm enable signal to motor controller
  pinMode(M_IN1, OUTPUT);                // input 1 to motor
  pinMode(M_IN2, OUTPUT);                // input 2 to motor

  digitalWrite(M_EN, LOW);               // Set motor inputs to default setting (disabled and brake)
  digitalWrite(M_IN1, LOW);
  digitalWrite(M_IN2, LOW);
}

//////////////////////////////////////////////////////////////////////////////////////
// setupTimer: Setup Timer1 (hardware timer) and associate an interrup_SR() that executes every overflow
//////////////////////////////////////////////////////////////////////////////////////
void setupTimer(void){
  Timer1.initialize(INTERRUPT_INTERVAL);                    //Setup timer for a 20ms interval
  Timer1.attachInterrupt(interrupt_SR);                     //isrHandler() will be called ever 20ms
}
