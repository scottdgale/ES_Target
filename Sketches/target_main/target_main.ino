#include <ESP8266WiFi.h>

//////////////////////
// WiFi Definitions //
//////////////////////
const char WIFIPASSWORD[] = "password";

/////////////////////
// Pin Definitions //
/////////////////////
const int LED_PIN = 5; // Thing's onboard, green LED
const int PIN15 = 15;
const int PIN4 = 4;

/////////////////////
// Global Variables//
/////////////////////
String html_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";

String html_1 = R"=====(
<!DOCTYPE html>
<html>
 <head>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'/>
  <meta charset='utf-8'>
  <style>
    body {font-size:140%;} 
    #main {display: table; margin: auto;  padding: 0 10px 0 10px; } 
    h2 {text-align:center; } 
    .button { padding:10px 10px 10px 10px; width:100%;  background-color: #50FF50; font-size: 120%;}
    .button_ON { padding:10px 10px 10px 10px; width:100%;  background-color: #50FF50; font-size: 120%;}
    .button_OFF { padding:10px 10px 10px 10px; width:100%;  background-color: #757575; font-size: 120%;}
  </style>

<script>
  function switchLED4(){
     let c = document.getElementById("LED4"); 
     console.log (c); 
     
     let btn4 = document.getElementById("LED4").value;
     if (btn4 =="Turn ON the LED")
     {
       document.getElementById("LED4").value = "Turn OFF the LED";
       ajaxLoad('LED4ON'); 
     }
     else
     {
       document.getElementById("LED4").value = "Turn ON the LED";
       ajaxLoad('LED4OFF');
     }
  }
  function switchLED15(){
     let c = document.getElementById("LED15"); 
     console.log (c); 
     
     let btn15 = document.getElementById("LED15").value;
     if (btn15 == "Turn ON the LED")
     {
       document.getElementById("LED15").value = "Turn OFF the LED";
       ajaxLoad('LED15ON'); 
     }
     else
     {
       document.getElementById("LED15").value = "Turn ON the LED";
       ajaxLoad('LED15OFF');
     }
  }
     
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

<title>LED Control</title>
  </head>
  <body>
    <div id='main'>
      <h2>LED Control</h2>
)=====";

String html_2 = R"=====(
  <input type="button" id="LED4" class="button" onclick="switchLED4()" value="Turn ON the LED"  />
  )=====";
String html_3 = R"=====(
  <input type="button" id="LED15" class="button" onclick="switchLED15()" value="Turn ON the LED"  />
  )=====";

String html_4 = R"=====(
  </div>
 </body>
</html>
)=====";

String request = "";    //variable used to store HTTP requests
WiFiServer server(80);  //Define a server object utilizing port 80

//Function Prototypes
void initHardware();
void setupWiFi();

void setup() {          //Execution STARTS HERE
  initHardware();
  digitalWrite(PIN15, HIGH);
  delay(1000);
  digitalWrite(PIN15, LOW);
  digitalWrite(PIN4, HIGH);
  delay(1000);
  digitalWrite(PIN4, LOW);
  setupWiFi();
  server.begin();       //Initiates the server function of the 8266
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Read the first line of the request
  request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();


  if(request.indexOf("LED4ON") > 0 )  { digitalWrite(PIN4, HIGH);  }
  else if  ( request.indexOf("LED4OFF") > 0 ) { digitalWrite(PIN4, LOW);   }
  else {
    boolean pinStatus = digitalRead(PIN4);
    if (pinStatus==HIGH) { 
      html_2.replace("Turn ON the LED","Turn OFF the LED");   
    }
    else { 
      html_2.replace("Turn OFF the LED","Turn ON the LED");  
    }
  }
  if(request.indexOf("LED15ON") > 0 )  { digitalWrite(PIN15, HIGH);  }
  else if  ( request.indexOf("LED415OFF") > 0 ) { digitalWrite(PIN15, LOW);   }
  else {
    boolean pinStatus = digitalRead(PIN15);
    if (pinStatus==HIGH) { 
      html_3.replace("Turn ON the LED","Turn OFF the LED");   
    }
    else { 
      html_3.replace("Turn OFF the LED","Turn ON the LED");  
    }
  }

  client.flush();

  // Send the response to the client
  client.print(html_header);
  client.print(html_1);
  client.print(html_2);
  client.print(html_3);
  client.print(html_4);
  
  delay(5);
  
  Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}

void setupWiFi() {
  WiFi.mode(WIFI_AP);

  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "ThingDev-":
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "SG_Target_" + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  WiFi.softAP(AP_NameChar, WIFIPASSWORD);
}

void initHardware() {
  Serial.begin(115200);
  //SETUP GPIO PINS and set to LOW
  pinMode(PIN15, OUTPUT);
  pinMode(PIN4, OUTPUT);
  digitalWrite(PIN15, LOW);
  digitalWrite(PIN4, LOW);
  
}
