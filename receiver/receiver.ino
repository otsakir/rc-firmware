#include "comm.h"

#include "receiver.h"


// some arduino-specific stuff. We don't want to put that in the more generic receiver.h
#include <SoftwareSerial.h>
SoftwareSerial mySerial(4,2);

void error(const char* msg) {
  mySerial.print("[ERROR] "); mySerial.println(msg);
}

void trace(const char* msg) {
  mySerial.print("[TRACE] "); mySerial.println(msg);
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
  mySerial.begin(38400);	// used for debuging
  mySerial.println("Hello, world?");
  
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
  analogWrite(MOTOR1_PIN, packet.motor1);
  analogWrite(MOTOR2_PIN, packet.motor2);
  digitalWrite(MOTOR1DIR_PIN, bitRead(packet.bits, packetbit_MOTOR1) );
  digitalWrite(MOTOR2DIR_PIN, bitRead(packet.bits, packetbit_MOTOR2) );
}
