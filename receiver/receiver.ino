
#include <SPI.h>
#include "printf.h"
#include "RF24.h"


/*  Ports used 
 *  
 *  TX - sender board RX
 *  RX - sender board TX
 *  D4 - external serial board/TX
 *  D2 - external serial board/RX
 *  D7 - motor1 dir
 *  D8 - motor2 dir
 *  D9 - motor1 throttle
 *  D10 - motor2 throttle
 *  
 */


#define IS_RC false
#define IS_VEHICLE !IS_RC
#define RF_LEVEL RF24_PA_MIN

// RF24 customizable pins
#define CE_PIN 7
#define CSN_PIN 8

namespace Rf {
    uint8_t sendAddress[6] = "2Node";
    uint8_t receiveAddress[6] = "1Node";
}


#include "receiver.h"

void error(const char* msg) {
  Serial.print("[ERROR] "); Serial.println(msg);
}

void trace(const char* msg) {
  //Serial.print("[TRACE] "); Serial.println(msg);
}

void warning(const char* msg, int intvalue = -32768) { // display if other than -32768
  Serial.print("[WARNING] "); Serial.print(msg);
  if (intvalue != -32768) {
  Serial.print(" - "); Serial.print(intvalue); 
  }
  Serial.println();
}


void setup() {
  Serial.begin(57600);	// used for // used for serial monitoring
  Serial.println("Hello, world?");
  Serial.print("sizeof(Packet):"); Serial.println(sizeof(Packet));
  
  pinMode(MOTOR1_PIN, OUTPUT);
  pinMode(MOTOR2_PIN, OUTPUT);
  pinMode(MOTOR1DIR_PIN,OUTPUT);
  pinMode(MOTOR2DIR_PIN, OUTPUT);
  
  Rf::init();  
}

int incomingByte = 0; // for incoming serial data

void loop() {
  // check if there is incoming packet ready to be executed
  checkReceived(receiverContext); // decleared in receiver.h
}


/* --- Callbacks --- */

void onPacketDropped(Packet& droppedPacket) {
	Serial.println("Dropped!");
}

void onPacketReceived(Packet& packet) {
  Serial.print("Packet received: ");
  bool dir1 = bitRead(packet.bits, packetbit_MOTOR1);
  bool dir2 = bitRead(packet.bits, packetbit_MOTOR2);
  Serial.print("M1 "); Serial.print(dir1 ? "-" : "+"); Serial.print(packet.motor1); Serial.print("  M2 "); Serial.print(dir2 ? "-" : "+"); Serial.println(packet.motor2);
/*  analogWrite(MOTOR1_PIN, packet.motor1);
  analogWrite(MOTOR2_PIN, packet.motor2);
  digitalWrite(MOTOR1DIR_PIN, dir1);
  digitalWrite(MOTOR2DIR_PIN, dir2);*/
}
