#ifndef _RCPROTOCOL_H_
#define _RCPROTOCOL_H_

/*
 * Communication stuff
 * 
 * Functionality placed here will be used by both the sender and receiver
 * 
 */

// Error messages are defined here. In case we run out of memory we can use smaller error descriptions like "E_31" etc.
#define ERROR_LR_TOO_LOW "found LR value out of LR_MIN limit. Pushing limit down"
#define ERROR_LR_TOO_HIGH "found LR value out of LR_MAX limit. Pushing limit up"
#define ERROR_FB_TOO_LOW "found FB value out of FB_MIN limit. Pushing limit down"
#define ERROR_FB_TOO_HIGH "found FB value out of FB_MAX limit. Pushing limit up"
#define ERROR_PACKET_BUFFER_FULL "packet buffer full"
#define ERROR_FEWER_BYTES_IN_BUFFER "too few bytes in buffer then expected"
#define STRING_PACKET_CRC_CHECK_FAILED "Packet CRC check failed. Dropping packet"

#define TRACE_RECEIVED_BYTE_FROM_SERIAL "received byte form serial"


// indexes of bit information in Packet.buttons 0-7
#define packetbit_HORN 0
#define packetbit_BREAKS 1
#define packetbit_MOTOR1 2  // left one .Reverse motor 1 direction.
#define packetbit_MOTOR2 3  // right one


struct Packet {
  unsigned char motor1; // left motor (as we move forward)
  unsigned char motor2; // right motor
  unsigned char bits;
  unsigned char index;  // increased by one for each different packet sent.
  byte crc;

  Packet() : Packet(0,0,0,0,0) {
  }
  
  Packet(unsigned char m1, unsigned char m2, unsigned char b, unsigned char i, byte c) {
	  motor1 = m1;
	  motor2 = m2;
	  bits = b;
	  index = i;
	  crc = c;
  }

  void reset() {
    motor1 = 0;
    motor2 = 0;
    bits = 0;
    index = 0;
    crc = 0;    
  }
  
};

byte CRC8(const byte *data, byte len);



#endif
