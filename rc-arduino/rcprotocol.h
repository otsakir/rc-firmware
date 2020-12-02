#ifndef _RCPROTOCOL_H_
#define _RCPROTOCOL_H_

// Error messages are defined here. In case we run out of memory we can use smaller error descriptions like "E_31" etc.
#define ERROR_LR_TOO_LOW "found LR value out of LR_MIN limit. Pushing limit down"
#define ERROR_LR_TOO_HIGH "found LR value out of LR_MAX limit. Pushing limit up"
#define ERROR_FB_TOO_LOW "found FB value out of FB_MIN limit. Pushing limit down"
#define ERROR_FB_TOO_HIGH "found FB value out of FB_MAX limit. Pushing limit up"


// indexes of bit information in Packet.buttons 0-7
#define bit_HORN 0
#define bit_BREAKS 1
#define bit_FORWARD 2 // going forward or backward
#define bit_RIGHT 4 // turning left or right


struct Packet {
  unsigned char fbNormalized;
  unsigned char lrNormalized;
  unsigned char bits;
  byte crc;

  Packet() {
    fbNormalized = 0;
    lrNormalized = 0;
    bits = 0;
    byte crc = 0; // CAUTION: this should be 0 when packet CRC is calculated. Add it to the packet _after_ the calculation
  }
  
};

// CRC-8 - based on the CRC8 formulas by Dallas/Maxim
// code released under the therms of the GNU GPL 3.0 license
// copied from: https://www.leonardomiliani.com/en/2013/un-semplice-crc8-per-arduino/
byte CRC8(const byte *data, byte len) {
  byte crc = 0x00;
  while (len--) {
    byte extract = *data++;
    for (byte tempI = 8; tempI; tempI--) {
      byte sum = (crc ^ extract) & 0x01;
      crc >>= 1;
      if (sum) {
        crc ^= 0x8C;
      }
      extract >>= 1;
    }
  }
  return crc;
}

void error(const char* msg) {
  Serial.print("[ERROR] "); Serial.println(msg);
}




#endif
