#include "comm.h"

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

// some arduino-specific stuff. We don't want to put that in the more generic receiver.h
#include <SoftwareSerial.h>
SoftwareSerial mySerial(4,2);

#include "receiver.h"

void error(const char* msg) {
  mySerial.print("[ERROR] "); mySerial.println(msg);
}

void trace(const char* msg) {
  //mySerial.print("[TRACE] "); mySerial.println(msg);
}

void warning(const char* msg, int intvalue = -32768) { // display if other than -32768
  mySerial.print("[WARNING] "); mySerial.print(msg);
  if (intvalue != -32768) {
  mySerial.print(" - "); mySerial.print(intvalue); 
  }
  mySerial.println();
}


void setup() {
  Serial.begin(9600); 		// used for RF communication
  mySerial.begin(38400);	// used for // used for serial monitoring
  mySerial.println("Hello, world?");
  mySerial.print("sizeof(Packet):"); mySerial.println(sizeof(Packet));
  
  pinMode(MOTOR1_PIN, OUTPUT);
  pinMode(MOTOR2_PIN, OUTPUT);
  pinMode(MOTOR1DIR_PIN,OUTPUT);
  pinMode(MOTOR2DIR_PIN, OUTPUT);
}

int incomingByte = 0; // for incoming serial data

void loop() {
  // check if there is incoming packet ready to be executed
  checkReceived(receiverContext); // decleared in receiver.h
}


/* --- Callbacks --- */

void onPacketDropped(Packet& droppedPacket) {
	mySerial.println("Dropped!");
}

void onPacketReceived(Packet& packet) {
  mySerial.print("Packet received: ");
  bool dir1 = bitRead(packet.bits, packetbit_MOTOR1);
  bool dir2 = bitRead(packet.bits, packetbit_MOTOR2);
  mySerial.print("M1 "); mySerial.print(dir1 ? "-" : "+"); mySerial.print(packet.motor1); mySerial.print("  M2 "); mySerial.print(dir2 ? "-" : "+"); mySerial.println(packet.motor2);
  analogWrite(MOTOR1_PIN, packet.motor1);
  analogWrite(MOTOR2_PIN, packet.motor2);
  digitalWrite(MOTOR1DIR_PIN, dir1);
  digitalWrite(MOTOR2DIR_PIN, dir2);
}
