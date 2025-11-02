/* CAN Loopback Example
* This example sends a message once a second and receives that message
*   no CAN bus is required.  This example will test the functionality
*   of the protocol controller, and connections to it.
*  
*   Written By: Cory J. Fowler - October 5th 2016
*   Adjusted by:
*/


#include <mcp_can.h>
#include <SPI.h>


// CAN TX Variables
unsigned long prevTX = 0;                                        // Variable to store last execution time
const unsigned int invlTX = 1000;                                // One second interval constant
byte data[] = {0xAA, 0x55, 0x01, 0x10, 0xFF, 0x12, 0x34, 0x56};  // Generic CAN data to send
//byte data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // Generic CAN data to send


// CAN RX Variables
long unsigned int rxId;
unsigned char len;
unsigned char rxBuf[8];


// Serial Output String Buffer
char msgString[128];


// Module wiring
//MCP2515 VCC to NodeMCU 3V3
//MCP2515 GND to NodeMCU GND
//MCP2515 CS to NodeMCU D8 (GPIO15)
//MCP2515 SO to NodeMCU D6 (GPIO12)
//MCP2515 SI to NodeMCU D7 (GPIO13)
//MCP2515 SCK to NodeMCU D5 (GPIO14)
//MCP2515 INT to NodeMCU D1 (GPIO5)
// CAN0 INT and CS
#define CAN0_INT 5      // Set INT to pin D2 related to GPIO15
#define CAN0_CS 2       // Set CS to pin D8 related to GPIO5
MCP_CAN CAN0(CAN0_CS);               


void setup()
{
 Serial.begin(115200);  // CAN is running at 500,000BPS; 115,200BPS is SLOW, not FAST, thus 9600 is crippling.


 // Initialize MCP2515 running at 8MHz with a baudrate of 500kb/s and the masks and filters disabled.
 if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
  Serial.println("MCP2515 Initialized Successfully!");
else
   Serial.println("Error Initializing MCP2515...");
 pinMode(CAN0_INT, INPUT);                           // Configuring pin for /INT input
 // Uncomment to go out of loopback mode
 CAN0.setMode(MCP_NORMAL);




 Serial.println("MCP2515 Library Normal Example...");
}


void loop()
{
 if(!digitalRead(CAN0_INT))                          // If CAN0_INT pin is low, read receive buffer
 {
   CAN0.readMsgBuf(&rxId, &len, rxBuf);              // Read data: len = data length, buf = data byte(s)~
  
   if((rxId & 0x80000000) == 0x80000000)             // Determine if ID is standard (11 bits) or extended (29 bits)
     sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
   else
     sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);
    Serial.print(msgString);
    if((rxId & 0x40000000) == 0x40000000){            // Determine if message is a remote request frame.
     sprintf(msgString, " REMOTE REQUEST FRAME");
     Serial.print(msgString);
   } else {
     for(byte i = 0; i<len; i++){
       sprintf(msgString, " 0x%.2X", rxBuf[i]);
       Serial.print(msgString);
     }
   }
      
   Serial.println();
 }
  if(millis() - prevTX >= invlTX){                    // Send this at a one second interval.
   prevTX = millis();
   byte sndStat = CAN0.sendMsgBuf(0x200, 8, data);
  
   if(sndStat == CAN_OK)
     Serial.println("Message Sent Successfully!");
   else
     Serial.println("Error Sending Message...");
 }
}


/*********************************************************************************************************
 END FILE
*********************************************************************************************************/
