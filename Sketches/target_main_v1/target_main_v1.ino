#include <ESP8266WiFi.h>
#include <SPI.h>          // SPI Protocol
#include <nRF24L01.h>     // Radio Class
#include <RF24.h>         // Radio Class

#define ONE_SEC 1000      // for xmitt delay
#define CE 15             //Pin 15
#define CSN 4             //Pin 4 

//////////////////////////////////////////////////////////////////////////////////////////
// WiFi DEFINITIONS //
//////////////////////////////////////////////////////////////////////////////////////////
const char WIFIPASSWORD[] = "password";

//////////////////////////////////////////////////////////////////////////////////////////
// PIN DEFINITIONS //
//////////////////////////////////////////////////////////////////////////////////////////
const int THING_LED = 5; // Thing's onboard, green LED

//////////////////////////////////////////////////////////////////////////////////////////
// HTML STRINGS //  
//////////////////////////////////////////////////////////////////////////////////////////
String html_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";

String html_init = R"=====(
<!DOCTYPE html>
<html>
 <head>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'/>
  <meta charset='utf-8'>
  <style>
    input[type=text] {
      border: 2px solid black;
      border-radius: 10px;
      width: 45px;
      height: 30px;
      padding: 10px 10px;
      margin: 4px 4px;
      box-sizing: border-box;
      background-color: white;
      color: black;
    }
    
    input[type=radio] {
      width: 15px;
      height: 15px;
      margin: 0px 0px 0px 0px;
    }
    
    input[type=submit] {
      border: 2px solid black;
      border-radius: 10px;
      width: 65px;
      height: 30px;
      margin: 4px 15px;
      box-sizing: border-box;
      background-color: green;
      color: white;
      font-family: Georgia, "Times New Roman", Times, serif;
    }
    
    .form_style {
      max-width: 475px;
      padding: 10px 20px;
      background: #ccd9ff;
      margin: 10px auto;
      padding: 20px;
      border-radius: 10px;
      font-family: Georgia, "Times New Roman", Times, serif;
    }
    
    p{
      color: black; 
      font-size: 20px;
      text-align: left;
      margin: 0px 0px 10px 0px;
    }
    
    body {
      font-size:140%;
    }
     
    #main {
      display: table; 
      margin: auto;  
      padding: 0 10px 0 10px; 
    } 
    
    h2 {
      text-align:center; 
    } 
  </style>

<script>
  let DEFAULT_TIME = 5; 
  function btnOne(){
    let targetMode = "M";
    let conceal = "5";
    let expose = "5";
    let command = ""; 
    //Construct the COMMAND that will be sent to the target
    //Check the mode selected from the radio buttons
    if (document.getElementById("rdbManual1").checked) {
      targetMode = "M";
    }
    else if (document.getElementById("rdbUser1").checked) {
      targetMode = "U";
    }
    else{   //Must be RANDOM
      targetMode = "R";
    }
    //Get the CONCEAL / EXPOSE times
    conceal = document.getElementById("txtConceal1");
    expose = document.getElementById("txtExpose1");

    console.log(conceal);
    console.log(expose);

    if (conceal.value == ""){
      conceal.value = DEFAULT_TIME;
    }
    if (expose.value == ""){
      expose.value = DEFAULT_TIME;
    }
    
    command = "~1" + targetMode + "C" + conceal.value + "E" + expose.value + "!";

    console.log(command);
    ajaxLoad(command);
    
  } //END btnSubmit1 ******************************************************************************

     
 var ajaxRequest = null;
 if (window.XMLHttpRequest)  { ajaxRequest =new XMLHttpRequest(); }
 else { ajaxRequest =new ActiveXObject("Microsoft.XMLHTTP"); }

  function ajaxLoad(ajaxURL){
    if(!ajaxRequest){ alert("AJAX is not supported."); return; }

    ajaxRequest.open("GET",ajaxURL,true);
    ajaxRequest.onreadystatechange = function()
    {
      if(ajaxRequest.readyState == 4 && ajaxRequest.status==200)
      {
        var ajaxResult = ajaxRequest.responseText;
      }
    }
    ajaxRequest.send();
}
</script>

<title>GALEFORCE TARGETS</title>
  </head>
  <body>
    <div id='main'>
      <h2>TARGET CONTROL</h2>
)=====";

String html_1 = R"=====(
  <form class="form_style">
    <p align=left>Target</p>
    <input type="radio" id="rdbManual1" name="mode" value="manual" checked> Manual
    <input type="radio" id="rdbUser1" name="mode" value="user"> User Input
    <input type="radio" id="rdbRandom1" name="mode" value="random"> Random <br>
    <span>
      Conceal Seconds:
      <input type="text" id="txtConceal1" maxlength="2" value="5"><br>
      Expose Seconds:
      <input type="text" id="txtExpose1" maxlength="2" value="5">
      <input type="button" id="btnSubmit1" onclick="btnOne()" value="Submit">
     </span>  
  </form>
)=====";

String html_end = R"=====(
  </div>
 </body>
</html>
)=====";

//////////////////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////////////////
String request = "";                        //variable used to store HTTP requests
String commmand = "";                       //Substring of the request sent to the target
byte target = 0;                            //Represents the active target within a command
WiFiServer server(80);                      //Define a server object utilizing port 80
RF24 radio(CE, CSN);                        //set up radio config
//RF24 radio_2(CE, CSN);                      //set up radio config
//RF24 radio_3(CE, CSN);                      //set up radio config
//RF24 radio_4(CE, CSN);                      //set up radio config
const uint64_t PIPE_1 = 0x01;               //Pipe address for radio_1
const uint64_t PIPE_2 = 0x02;               //Pipe address for radio_2
const uint64_t PIPE_3 = 0x03;               //Pipe address for radio_3
const uint64_t PIPE_4 = 0x04;               //Pipe address for radio_4
char commandArray[10];                      //Data char array to send to target



//////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////////////////
void initHardware(void);
void setupWiFi(void);
void setupRadio(void);
bool serviceWiFi(void);
bool processRequest(String);
void transmitData(void);


//////////////////////////////////////////////////////////////////////////////////////////
// setup() - EXECUTION STARTS HERE
//////////////////////////////////////////////////////////////////////////////////////////
void setup() {          //Execution STARTS HERE
  initHardware();
  setupRadio();
  digitalWrite(THING_LED, HIGH);
  delay(1000);
  digitalWrite(THING_LED, LOW);
  setupWiFi();
  server.begin();                                 //Initiates the server function of the 8266
}

//////////////////////////////////////////////////////////////////////////////////////////
// ININITE LOOP - EXECUTION DOES NOT RETURN FROM HERE
//////////////////////////////////////////////////////////////////////////////////////////
void loop() {

  
  bool wifiActivity = serviceWiFi();
  
}

//////////////////////////////////////////////////////////////////////////////////////////
// Transmit data to the appropriate target
// Param String com: Command string
// Param byte t: target number associated with the command
// Return: void
//////////////////////////////////////////////////////////////////////////////////////////
void transmitData(String com, byte t){
  com.toCharArray(commandArray, 10);
  Serial.println(commandArray);
  switch (t){
    case 1:
      //Set PIPE
      radio.openWritingPipe(PIPE_1);                          // Open radio write pipe
      radio.write(&commandArray, sizeof(commandArray));       //Send data payload
      break;
    case 2:
      break;
    case 3:
      break;
    case 4:
      break;
  }
  
}

//////////////////////////////////////////////////////////////////////////////////////////
// Check is client is trying to connect / request something from the server
// Return: Boolean - True if service a client / False otherwise
//////////////////////////////////////////////////////////////////////////////////////////
bool serviceWiFi(){
  int first, last;                                    //used to extract substring from request
  WiFiClient client = server.available();             //If no client the return
  if (!client) {
    return false;
  }

  request = client.readStringUntil('\r');             // Read the first line of the request
  if (request.indexOf("~")>0){                        //process commands with "~"
    Serial.println("Request: " + request);
    client.flush();
    first = request.indexOf("~");                     //Extract command from request
    last = request.indexOf("!");
    request = request.substring(first, last);
  
    if (request.indexOf("~1")==0){                     //Service request for target one
      //Update HTML code to reflect the request
      Serial.println("Processing Target 1");
      target = 1;
    } 
    else if (request.indexOf("~2")==0){                //Service request for target two
      Serial.println("Processing Target 2");
      target = 2;
    }
    else if (request.indexOf("~3")==0){                //Service request for target three
      Serial.println("Processing Target 3");
      target = 3;
      
    }
    else if (request.indexOf("~4")==0){                //Service request for target four
      Serial.println("Processing Target 4");
      target = 4;
      
    }
    transmitData(request, target);
  }

  client.flush();
  // Send the response to the client
  client.print(html_header);
  client.print(html_init);
  client.print(html_1);
  client.print(html_end);
 
  delay(5);
  Serial.println("Client disonnected");
  return true; 
} //CLOSE serviceWiFi()


//////////////////////////////////////////////////////////////////////////////////////////
// CONFIGURE AND SETUP THE 8266 AS AN ACCESS POINT
//////////////////////////////////////////////////////////////////////////////////////////
void setupWiFi() {
  WiFi.mode(WIFI_AP);

  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "ThingDev-":
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "GALE_TARGET" + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  WiFi.softAP(AP_NameChar, WIFIPASSWORD);
} //CLOSE setupWiFi()

//////////////////////////////////////////////////////////////////////////////////////////
// INITHARDWARE - STARTS THE SERIAL INTERFACE AND CONFIGURES ALL GPIO PINS
//////////////////////////////////////////////////////////////////////////////////////////
void initHardware() {
  Serial.begin(115200);                             //Initialize serial output
  //SETUP GPIO PINS-SET LOW /////////////////////
  pinMode(THING_LED, OUTPUT);
  digitalWrite(THING_LED, LOW);

} //CLOSE initHardware()

//////////////////////////////////////////////////////////////////////////////////////////
// setupRadios - SETUP FOUR DISTINCT RADIOS TO COMMUNICATE TO EACH TARGET
//////////////////////////////////////////////////////////////////////////////////////////
void setupRadio(){
  radio.begin();                        // Turn on radio
  radio.setPALevel(RF24_PA_LOW);        // Set radio power level
  radio.setDataRate(RF24_250KBPS);      // Set radio data rate
  radio.setChannel(124);                // Set radio channel
  radio.openWritingPipe(PIPE_1);        // Open radio write pipe
  radio.stopListening();                // Set up to xmitt
}
