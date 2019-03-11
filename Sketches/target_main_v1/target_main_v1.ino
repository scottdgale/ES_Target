#include <ESP8266WiFi.h>

//////////////////////////////////////////////////////////////////////////////////////////
// WiFi DEFINITIONS //
//////////////////////////////////////////////////////////////////////////////////////////
const char WIFIPASSWORD[] = "password";

//////////////////////////////////////////////////////////////////////////////////////////
// PIN DEFINITIONS //
//////////////////////////////////////////////////////////////////////////////////////////
const int THING_LED = 5; // Thing's onboard, green LED
const int PIN15 = 15;
const int PIN4 = 4;

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
String request = "";    //variable used to store HTTP requests
WiFiServer server(80);  //Define a server object utilizing port 80

//////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////////////////
void initHardware();
void setupWiFi();
bool processRequest(String);

//////////////////////////////////////////////////////////////////////////////////////////
// setup() - EXECUTION STARTS HERE
//////////////////////////////////////////////////////////////////////////////////////////
void setup() {          //Execution STARTS HERE
  initHardware();
  digitalWrite(PIN15, HIGH);
  digitalWrite(PIN4, HIGH);
  digitalWrite(THING_LED, HIGH);
  delay(1000);
  digitalWrite(PIN15, LOW);
  digitalWrite(PIN4, LOW);
  digitalWrite(THING_LED, LOW);
  setupWiFi();
  server.begin();                                 //Initiates the server function of the 8266
}

//////////////////////////////////////////////////////////////////////////////////////////
// ININITE LOOP - EXECUTION DOES NOT RETURN FROM HERE
//////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  bool flag = false;
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Read the first line of the request
  request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();
  
 
  if (request.indexOf("**1")>0){
    //Service request for target one
    //Update HTML code to reflect the request
    Serial.println("Processing Target 1");
    if (request.indexOf("M")>0) {
      
    }
    else if (request.indexOf("U")>0){

    }
    else if (request.indexOf("R")>0){

    }
    else{
      //STOP CONDITION
    }

      

  }
  delay (5000);
  client.flush();
  // Send the response to the client
  client.print(html_header);
  client.print(html_init);
  client.print(html_1);
  client.print(html_end);
 
  
  
  delay(5);
  
  Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}


bool processRequest(String request){
  if (request.indexOf("**1**")){
     Serial.println("Target 1: " + request);
     return false;
  }
  else if (request.indexOf("**2**")){
      Serial.println("Target 2: " + request);
      return false;
  }
  else {          //DEFAULT CASE 
      Serial.println("DEFAULT - NO COMMAND");
      return true;
  }
}

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
}

//////////////////////////////////////////////////////////////////////////////////////////
// INITHARDWARE - STARTS THE SERIAL INTERFACE AND CONFIGURES ALL GPIO PINS
//////////////////////////////////////////////////////////////////////////////////////////
void initHardware() {
  Serial.begin(115200);
  //SETUP GPIO PINS and set to LOW
  pinMode(PIN15, OUTPUT);
  pinMode(PIN4, OUTPUT);
  digitalWrite(PIN15, LOW);
  digitalWrite(PIN4, LOW);
  digitalWrite(THING_LED, LOW);
}
