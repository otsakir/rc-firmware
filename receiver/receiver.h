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


#include "utils.h"


ReceiverContext receiverContext; // global receiver state


// the size of the data that are waiting to be read in the buffer
unsigned char readySizeInBuffer(ReceiverContext& ctx) {
  return (ctx.writeIndex + PACKET_BUFFER_SIZE - ctx.readIndex) % PACKET_BUFFER_SIZE;
}

// try to form a valid packet with the oldest bytes in the buffer. If invalid we should discard the first byte.
// packet is used as a temporary memory location for the packet. It will be altered even if a proper packet is not extracted
bool tryPacket(Packet& packet, ReceiverContext context) {
  unsigned char* ppacket = (unsigned char*) &packet;
  unsigned char count = 0; // bytes read from buffer
  unsigned char ri = context.readIndex;
  while (ri != context.writeIndex && count < PACKET_SIZE ) {
    ppacket[count] = context.packetBuffer[ri];
    count ++;
    ri ++;
    if (ri >= PACKET_BUFFER_SIZE)
      ri = 0;
  }
  
  // we should always reach PACKET_SIZE since the other break condition should not occur if the caller has confirmed
  // that there are enough bytes in the buffer to form a packet
  
  if (count == PACKET_SIZE) {
    return true;
  } else
    return false; // not enough bytes to form a packet
}

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

void serialEvent() {
	unsigned char wi; // temporary write index
  int avail = Serial.available();
	mySerial.print("serialEvent: available(): "); mySerial.println(avail); 
	while( avail ) {
    // there is a pending byte see where we would put it
		wi = receiverContext.writeIndex;
		wi ++;
		if (wi == PACKET_BUFFER_SIZE)
			wi = 0;
		if (wi == receiverContext.readIndex) {
			error(ERROR_PACKET_BUFFER_FULL);
			return; // no nothing and hope some chars will be read until the next serialEvent
		}
		// ok, there is more than one position available in the buffer
		unsigned char a_byte = Serial.read();
		receiverContext.packetBuffer[receiverContext.writeIndex] = a_byte;
		receiverContext.writeIndex = wi;
		trace(TRACE_RECEIVED_BYTE_FROM_SERIAL);
		
		// now try to form a packet from the data in the packet buffer
		while (!receiverContext.newPacket && readySizeInBuffer(receiverContext) >= PACKET_SIZE) {
      mySerial.println("trying to form a packet");
			Packet packet;
			if ( tryPacket(packet, receiverContext) ) {
				// packet updated
				if (verifyCrc(packet)) {
					memcpy(&receiverContext.incomingPacket, &packet, sizeof(receiverContext.incomingPacket) ); // make a copy of it
					receiverContext.newPacket = true;
					return;
				} else {
					warning(STRING_PACKET_CRC_CHECK_FAILED, packet.index);
					onPacketDropped(packet);
				}
			} else {
				error(ERROR_FEWER_BYTES_IN_BUFFER);
			}

			// discard one byte
			receiverContext.readIndex ++;
			if (receiverContext.readIndex >= PACKET_BUFFER_SIZE)
				receiverContext.readIndex = 0;
		}
   avail = Serial.available();
	}
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
	if (ctx.newPacket) {
		onPacketReceived(ctx.incomingPacket);
		receiverContext.newPacket = false;
	}
}



#endif
