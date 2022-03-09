#ifndef _RECEIVER_H_
#define _RECEIVER_H_

// receiver.h is the receiver or vehicle program


#include "comm.h"
#include "utils.h"


#define MOTOR1_PIN 9
#define MOTOR2_PIN 10
#define MOTOR1DIR_PIN 7
#define MOTOR2DIR_PIN 8

#define PACKET_BUFFER_SIZE 10
#define PACKET_SIZE sizeof(Packet)


struct ReceiverContext {
	bool newPacket = false; // this is set to 'true' by serialEvent(). Set it to 'false' loop to allow parsing of new packets
	Packet incomingPacket;
};

extern ReceiverContext receiverContext; // global receiver state

void checkReceived(ReceiverContext ctx);
// from receiver.ino
void onPacketDropped(Packet& droppedPacket);
void onPacketReceived(Packet& packet);





ReceiverContext receiverContext; // global receiver state



bool verifyCrc(Packet& packet) {
  // keep transmitted crc, set field to 0, recalculate and compare, restore crc field
  byte packet_crc = packet.crc;
  packet.crc = 0;
  bool ret = false;
  if (packet_crc == CRC8((byte*) &packet, sizeof(packet))) {
	  ret = true;
  } 
  // restore value in packet
  packet.crc = packet_crc;
  return ret;
}


/*
void onPacketReceived(Packet& packet) {
  analogWrite(MOTOR1_PIN, packet.motor1);
  analogWrite(MOTOR2_PIN, packet.motor2);
  digitalWrite(MOTOR1DIR_PIN, bitRead(packet.bits, packetbit_MOTOR1) );
  digitalWrite(MOTOR2DIR_PIN, bitRead(packet.bits, packetbit_MOTOR2) );
}
*/

// checks if there is any incoming packet ready for process and triggers its processing
// is responsible for handling ReceiverContext.newPacket
void checkReceived(ReceiverContext ctx) {
    int status = Rf::recv(ctx.incomingPacket);
    if (status > 0) {
		onPacketReceived(ctx.incomingPacket);
		receiverContext.newPacket = false;
	} else
    if ( status < 0) {
        onPacketDropped(receiverContext.incomingPacket);
    }
    // ignore otherwise. No packet available.
}



#endif
