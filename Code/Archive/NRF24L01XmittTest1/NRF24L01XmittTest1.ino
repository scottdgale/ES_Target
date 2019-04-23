/*

*/
//3456789012345678901234567890123456789012345678901234567890123456789

#include <SPI.h>          // SPI Protocol
#include <nRF24L01.h>     // Radio Class
#include <RF24.h>         // Radio Class

#define ONE_SEC 1000      // for xmitt delay

byte message = 0;         // data to send
const uint64_t pipe = 0x0123456789AB;   // Pipe address

RF24 radio(9, 10);        //set up radio config

//3456789012345678901234567890123456789012345678901234567890123456789
//       setup()   setup()   setup()   setup()   setup()   setup()
//       setup()   setup()   setup()   setup()   setup()   setup()
/*

*/
void setup()
{
  radio.begin();                    // Turn on radio
  radio.setPALevel(RF24_PA_LOW);    // Set radio power level
  radio.setDataRate(RF24_250KBPS);  // Set radio data rate
  radio.setChannel(124);            // Set radio channel
  radio.openWritingPipe(pipe);      // Open radio write pipe
  radio.stopListening();          // Set up to xmitt
}

//3456789012345678901234567890123456789012345678901234567890123456789
//       loop()    loop()    loop()    loop()    loop()    loop()
//       loop()    loop()    loop()    loop()    loop()    loop()
/*

*/
void loop()
{
  radio.write(&message, 1);       // Send out payload
  message++;                      // inc message
  delay(ONE_SEC);                 // Delay 1 sec then start again
}
