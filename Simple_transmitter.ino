// This code was modified from: https://learn.adafruit.com/adafruit-feather-32u4-radio-with-rfm69hcw-module/using-the-rfm69-radio
// by John Sampson (jcps), with the intention of being a beginner's guide to the RMF69.
// Intended to be used with the Adafruit feathers, but can be modified to use just the RFM69
// **********************************************************************************/
/*
 * https://github.com/LowPowerLab/RFM69/pull/50/files
 * https://github.com/Jcps/RFM69/tree/d1125ec94c5a762a1386f4f1ad19dccef3c926ed
 * 
 * see also :
 * https://bitknitting.wordpress.com/2016/08/02/backyard-automatic-watering-using-a-featherarduino/
 * http://embeddedapocalypse.blogspot.be/2017/01/rfm69hcwlowpowerlab-sendwithretry.html
 * 
 */

#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
//#include <RH_RF69.h>

#include <SPI.h>
#include <stdio.h>

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/CONFIGURE TO FIT YOUR HARDWARE ************
//*********************************************************************************************
#define NETWORKID     1    // The same on all nodes that talk to each other
#define NODEID        1    // The unique identifier of this node

//Match frequency to the hardware version of the radio on your Feather
#define FREQUENCY     RF69_433MHZ
//#define FREQUENCY     RF69_868MHZ
//#define FREQUENCY     RF69_915MHZ
#define IS_RFM69HCW   true // set to 'true' if you are using an RFM69HCW module

//*********************************************************************************************
#define SERIAL_BAUD   115200

// for Feather 32u4 Radio
#define RFM69_CS      8
#define RFM69_IRQ     7
#define RFM69_IRQN    4  // Pin 7 is IRQ 4!
#define RFM69_RST     4

#define VBATPIN A9

/*
// for Feather M0 Radio
#define RFM69_CS      8
#define RFM69_IRQ     3
#define RFM69_IRQN    3  // Pin 3 is IRQ 3!
#define RFM69_RST     4
//
*/
/* ESP8266 feather w/wing
#define RFM69_CS      2
#define RFM69_IRQ     15
#define RFM69_IRQN    digitalPinToInterrupt(RFM69_IRQ )
#define RFM69_RST     16
*/

/* Feather 32u4 w/wing
#define RFM69_RST     11   // "A"
#define RFM69_CS      10   // "B"
#define RFM69_IRQ     2    // "SDA" (only SDA/SCL/RX/TX have IRQ!)
#define RFM69_IRQN    digitalPinToInterrupt(RFM69_IRQ )
*/

/* Feather m0 w/wing
#define RFM69_RST     11   // "A"
#define RFM69_CS      10   // "B"
#define RFM69_IRQ     6    // "D"
#define RFM69_IRQN    digitalPinToInterrupt(RFM69_IRQ )
*/

/* Teensy 3.x w/wing
#define RFM69_RST     9   // "A"
#define RFM69_CS      10   // "B"
#define RFM69_IRQ     4    // "C"
#define RFM69_IRQN    digitalPinToInterrupt(RFM69_IRQ )
*/

/* WICED Feather w/wing
#define RFM69_RST     PA4     // "A"
#define RFM69_CS      PB4     // "B"
#define RFM69_IRQ     PA15    // "C"
#define RFM69_IRQN    RFM69_IRQ
*/

#define LED           13  // onboard blinky
//#define LED           0 //use 0 on ESP8266

int16_t packetnum = 0;  // packet counter, we increment per xmission

RFM69 rf69 = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);

struct radiop{
  char packet[20];
  char pcknr[4];
  char temp[4];
  char batt[6];
};
char charTemp[4];
char charBatt[6];

void setup() {
  while (!Serial); // wait until serial console is open, remove if not tethered to computer. Delete this line on ESP8266
  Serial.begin(SERIAL_BAUD);

  Serial.println("Feather RFM69HCW Reciever");

  // Hard Reset the RFM module
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, HIGH);
  delay(100);
  digitalWrite(RFM69_RST, LOW);
  delay(100);

  // Initialize radio
  rf69.initialize(FREQUENCY,NODEID,NETWORKID);
  if (IS_RFM69HCW) {
    rf69.setHighPower();    // Only for RFM69HCW & HW!
  }
  rf69.setPowerLevel(31); // power output ranges from 0 (5dBm) to 31 (20dBm)

  pinMode(LED, OUTPUT);
  Serial.print("\rReceiving at ");
  Serial.print(FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(" MHz");
 
}


void loop() {
   if(rf69.receiveDone()){ 
       Serial.print((char*)rf69.DATA); //print the message received to serial
       Serial.print("   [RX_RSSI:");Serial.print(rf69.RSSI);Serial.print("]"); // print the RSSI (relative received signal strength), basically how "loud" it was to the reciever.
       Serial.println("-- Done receiving --");
     }
 
   rf69.receiveDone(); //put radio in RX mode
   Serial.flush(); //make sure all serial data is clocked out before sleeping the MCU
  
  struct radiop radiopacket = {
    "Hello World # ",
    "179",
    "",
    "",
  };
  
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  Serial.print("VBat: " ); Serial.println(measuredvbat);
  dtostrf(measuredvbat, 4, 3, charBatt);
  //sprintf(charBatt,"%f",measuredvbat);
  Serial.print("VBat: " ); Serial.println(charBatt);
  itoa(rf69.readTemperature(-2), charTemp, 10);
  memcpy(radiopacket.temp, charTemp, sizeof(radiopacket.temp));
  memcpy(radiopacket.batt, charBatt, sizeof(radiopacket.batt));
    
  Serial.print("Sending "); Serial.println(radiopacket.packet);
  Serial.print("Sending "); Serial.println(radiopacket.pcknr);
  Serial.print("Sending "); Serial.println(radiopacket.temp);
  Serial.print("Sending "); Serial.println(radiopacket.batt);
  Serial.print("Temp :  "); Serial.println(rf69.readTemperature(-2));
  
  // Send a message!
  if (rf69.sendWithRetry(NODEID, (const void*)(&radiopacket), sizeof(radiopacket))){ //target node Id, message as string or byte array, message length
      Blink(LED, 40, 3); //blink LED 3 times, 40ms between blinks
  }

   rf69.receiveDone(); //put radio in RX mode
   Serial.flush(); //make sure all serial data is clocked out before sleeping the MCU
//  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_ON); //sleep Moteino in low power mode (to save battery)
//  Blink(LED, 40, 4);
  delay(5000);  // Wait 1 second between transmits, could also 'sleep' here!

}

void Blink(byte PIN, byte DELAY_MS, byte loops)
{
  for (byte i=0; i<loops; i++)
  {
    digitalWrite(PIN,HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN,LOW);
    delay(DELAY_MS);
  }
}
