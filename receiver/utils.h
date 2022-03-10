#ifndef _UTILS_H_
#define _UTILS_H_


void error(const char* msg) {
  Serial.print("[ERROR] "); Serial.println(msg);
}

void trace(const char* msg) {
  Serial.print("[TRACE] "); Serial.println(msg);
}

void warning(const char* msg, int intvalue /*= -32768*/) { // display if other than -32768
  Serial.print("[WARNING] "); Serial.print(msg);
  if (intvalue != -32768) {
  Serial.print(" - "); Serial.print(intvalue); 
  }
  Serial.println();
}



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


#endif
