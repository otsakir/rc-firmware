#include <Arduino.h>
#include "receiver.h"
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
	while( Serial.available() ) {
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
