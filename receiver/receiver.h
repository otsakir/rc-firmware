#ifndef _RECEIVER_H_
#define _RECEIVER_H_

// receiver.h is the receiver or vehicle program

#include "comm.h"

#define MOTOR1_PIN 9
#define MOTOR2_PIN 10
#define MOTOR1DIR_PIN 7
#define MOTOR2DIR_PIN 8

#define PACKET_BUFFER_SIZE 10
#define PACKET_SIZE sizeof(Packet)

/*
 * State data for the receiver
 * 
 * readIndex  : index of next byte to read from packetBuffer
 * writeIndex : index of next position to write into packetBuffer. writeIndex should be < readIndex.
 * 
 * It follows that if readIndex == writeIndex the buffer is empty. 
 * Also, not all PACKET_BUFFER_SIZE bytes will ever be full. One will always be empty. The 
 * algorithm does not allow it and relies on that fact..  If this were full, we would end up an ambiguity 
 * for 'readIndex == writeIndex' condition. Would this be an empty buffer or a full one. 
 */
struct ReceiverContext {
	unsigned char packetBuffer[PACKET_BUFFER_SIZE];
	unsigned char readIndex = 0;	// 
	unsigned char writeIndex = 0;	// 
	bool newPacket = false; // this is set to 'true' by serialEvent(). Set it to 'false' loop to allow parsing of new packets
	Packet incomingPacket;
};

extern ReceiverContext receiverContext; // global receiver state

void checkReceived(ReceiverContext ctx);
// from receiver.ino
void onPacketDropped(Packet& droppedPacket);
void onPacketReceived(Packet& packet);

/*
void onPacketReceived(Packet& packet) {
  analogWrite(MOTOR1_PIN, packet.motor1);
  analogWrite(MOTOR2_PIN, packet.motor2);
  digitalWrite(MOTOR1DIR_PIN, bitRead(packet.bits, packetbit_MOTOR1) );
  digitalWrite(MOTOR2DIR_PIN, bitRead(packet.bits, packetbit_MOTOR2) );
}
*/



#endif
