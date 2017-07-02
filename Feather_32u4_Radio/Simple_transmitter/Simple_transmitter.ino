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
#include <Wire.h>
#include "cactus_io_BME280_I2C.h"

// Create the BME280 object
BME280_I2C bme;              // I2C using default 0x77 

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

#define LED           13  // onboard blinky

int16_t packetnum = 0;  // packet counter, we increment per xmission

RFM69 rf69 = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);

//1015.30    58.08   22.64 *C  70.95 *F
struct radiop{
  char packet[20];
  char pressure[8];
  char humidity[6];
  char temp1[6];
  char temp2[4];
  char batt[6];
};
char charTemp[4];
char charBatt[6];

void setup() {
  //while (!Serial); // wait until serial console is open, remove if not tethered to computer. Delete this line on ESP8266
  Serial.begin(SERIAL_BAUD);

  //Serial.println("Feather RFM69HCW Reciever");

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

  if (!bme.begin()) {
    //Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  bme.setTempCal(-1);
 
}


void loop() {
  bme.readSensor();
   
  if(rf69.receiveDone()and Serial){ 
    Serial.print((char*)rf69.DATA); //print the message received to serial
    Serial.print("   [RX_RSSI:");Serial.print(rf69.RSSI);Serial.print("]"); // print the RSSI (relative received signal strength), basically how "loud" it was to the reciever.
    Serial.println("-- Done receiving --");
  }
 
  rf69.receiveDone(); //put radio in RX mode
  
  struct radiop radiopacket = {
    "Hello World # ",
    "1792",
    "123",
    "",
    "",
    "",
  };
  
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  dtostrf(measuredvbat, 4, 3, charBatt);
  dtostrf(bme.getPressure_MB(),7,2,radiopacket.pressure);
  dtostrf(bme.getHumidity(),5,2,radiopacket.humidity);
  dtostrf(bme.getTemperature_C(),5,2,radiopacket.temp1);
  itoa(rf69.readTemperature(-2), charTemp, 10);
  memcpy(radiopacket.temp2, charTemp, sizeof(radiopacket.temp2));
  memcpy(radiopacket.batt, charBatt, sizeof(radiopacket.batt));

  if (Serial){
    Serial.print("\rReceiving at ");
    Serial.print(FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
    Serial.println(" MHz");
    
    Serial.print("Sending "); Serial.println(radiopacket.packet);
    Serial.print("Sending "); Serial.println(radiopacket.pressure);
    Serial.print("Sending "); Serial.println(radiopacket.humidity);
    Serial.print("Sending "); Serial.println(radiopacket.temp1);
    Serial.print("Sending "); Serial.println(radiopacket.temp2);
    Serial.print("Sending "); Serial.println(radiopacket.batt);
    Serial.flush(); //make sure all serial data is clocked out before sleeping the MCU
  }
  
  // Send a message!
  if (rf69.sendWithRetry(NODEID, (const void*)(&radiopacket), sizeof(radiopacket))){ //target node Id, message as string or byte array, message length
      Blink(LED, 40, 3); //blink LED 3 times, 40ms between blinks
  }

   rf69.receiveDone(); //put radio in RX mode
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
