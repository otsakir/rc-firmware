#include "comm.h"

// some arduino-specific stuff. We don't want to put that in the more generic receiver.h
#include <SoftwareSerial.h>
SoftwareSerial mySerial(4,2);

#include "receiver.h"

void setup() {
  Serial.begin(9600); 		// used for RF communication
  mySerial.begin(38400);	// used for debuging
  mySerial.println("Hello, world?");
  
  receiverSetup();
}

int incomingByte = 0; // for incoming serial data

void loop() {

	if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();

    // say what you got:
    Serial.print("I received: ");
    Serial.println(incomingByte, DEC);
  }
}
