#ifndef _RCPROTOCOL_H_
#define _RCPROTOCOL_H_


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

//CRC-8 - based on the CRC8 formulas by Dallas/Maxim
//code released under the therms of the GNU GPL 3.0 license
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




#endif
