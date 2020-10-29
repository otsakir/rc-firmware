#ifndef _RCPROTOCOL_H_
#define _RCPROTOCOL_H_


struct Packet {
  unsigned char fbNormalized;
  unsigned char lrNormalized;
  unsigned char bits;

  Packet() {
    fbNormalized = 0;
    lrNormalized = 0;
    bits = 0;
  }
  
};



#endif
