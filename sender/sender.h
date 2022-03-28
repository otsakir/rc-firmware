#ifndef _SENDER_H_
#define _SENDER_H_

#include "comm.h"
#include "utils.h"



// Error messages are defined here. In case we run out of memory we can use smaller error descriptions like "E_31" etc.
#define ERROR_LR_TOO_LOW "found LR value out of LR_MIN limit. Pushing limit down"
#define ERROR_LR_TOO_HIGH "found LR value out of LR_MAX limit. Pushing limit up"
#define ERROR_FB_TOO_LOW "found FB value out of FB_MIN limit. Pushing limit down"
#define ERROR_FB_TOO_HIGH "found FB value out of FB_MAX limit. Pushing limit up"
#define ERROR_PACKET_BUFFER_FULL "packet buffer full"
#define ERROR_FEWER_BYTES_IN_BUFFER "too few bytes in buffer then expected"
#define STRING_PACKET_CRC_CHECK_FAILED "Packet CRC check failed. Dropping packet"



// indexes of bit information in SensorData 
#define sensorbit_HORN 0
#define sensorbit_BREAKS 1
#define sensorbit_BACKWARD 2 // zero is forward, 1 is backward
#define sensorbit_LEFT 3 // zero is right, 1 is left. These bits work as a reverse operation. Positive is FW/RIGHT

// all settings below are in terms of actual values read from the analog port

struct SensorData {
  unsigned short fbNormalized;
  unsigned short lrNormalized;
  unsigned char bits;

  SensorData() {
    fbNormalized = 0;
    lrNormalized = 0;
    bits = 0;
  }  
};

struct CalibrationInfo {
  int FB_ZERO = 1024/2; // sensible defaults
  int LR_ZERO = 1024/2; // sensible defaults
  int FB_MAX = 1024/2;
  int FB_MIN = 1024/2;
  int LR_MAX = 1024/2;
  int LR_MIN = 1024/2;
  
  void dump(Stream& serial) {
	  serial.print("FB_ZERO: "); serial.println(FB_ZERO);
	  serial.print("LR_ZERO: "); serial.println(LR_ZERO);
	  serial.print("FB_MAX: "); serial.println(FB_MAX);
	  serial.print("FB_MIN: "); serial.println(FB_MIN);
	  serial.print("LR_MAX: "); serial.println(LR_MAX);
	  serial.print("LR_MIN: "); serial.println(LR_MIN);
  }
};

struct SenderContext {
  bool CALIBRATING; // read-only value. should not be edited directly. Only through start/stopCalibration.
  bool TRANSMITTING = false;
  SensorData sensorData;
  Packet packet;
  CalibrationInfo calInfo;  
};


// EEPROM
#define EEPROM_SCHEMA_VERSION 1
#define EEPROM_STATUS_BIT_CALIBRATED 0


struct EepromContent {
	unsigned char signature = 255; // by default != 'G' i.e. not initialized
	unsigned char version;
	unsigned char status;
	CalibrationInfo calibrationInfo;
	
};

struct EepromTool {
	
	EepromContent eepromContent;
		
	// load raw contents to RAM ,initialized and stores them. Always call when application starts
	void init() {
		EEPROM.get(0, eepromContent);
		if (eepromContent.signature != 'G') {
			eepromContent.signature = 'G';
			eepromContent.status = 0;
			bitClear(eepromContent.status, EEPROM_STATUS_BIT_CALIBRATED);
			// won't set calibrationInfo
			EEPROM.put(0, eepromContent);
			Serial.println("Initialized EEPROM");
		}
	}
	
	bool isCalibrated() {
		return bitRead(eepromContent.status, EEPROM_STATUS_BIT_CALIBRATED);
	}
	
	bool saveCalibration(const CalibrationInfo& calInfo) {
		if (eepromContent.signature == 'G') {
			eepromContent.calibrationInfo = calInfo;
			bitSet(eepromContent.status, EEPROM_STATUS_BIT_CALIBRATED);
			EEPROM.put(0, eepromContent);
			Serial.println("Saved contents to EEPROM");
			return true;
		} else  {
			return false;
		}
		
	}
	
	bool loadCalibration(CalibrationInfo& calInfo) {
		if (eepromContent.signature == 'G' && bitRead(eepromContent.status, EEPROM_STATUS_BIT_CALIBRATED)) {
			EEPROM.get((char*)&eepromContent.calibrationInfo - (char*)&eepromContent, eepromContent.calibrationInfo);
			return true;
		}
		return false;
	}
	
};



// reads sensor values and bundle then in SensorData
void readSensors(SensorData& packet);
void applyZeroTolerance(SensorData& packet);
void buildPacket(SensorData& sensorData, Packet& packet);
void transmitPacket(Packet& packet);


// State
SenderContext senderContext;


// --- Implementation ---


// reads sensor values and bundle then in SensorData
void readSensors(SensorData& packet) {
  long fb = analogRead(FRONTBACK_PIN);
  long lr = analogRead(LEFTRIGHT_PIN); 

  packet.bits = 0;
  if (fb >= senderContext.calInfo.FB_ZERO) {
    if (fb > senderContext.calInfo.FB_MAX) {
      error(ERROR_FB_TOO_HIGH);
      senderContext.calInfo.FB_MAX = fb;
    }
    packet.fbNormalized = ((long)(fb - senderContext.calInfo.FB_ZERO)) * 255 / (senderContext.calInfo.FB_MAX - senderContext.calInfo.FB_ZERO);
    bitClear(packet.bits, sensorbit_BACKWARD);
  } else
  if (fb < senderContext.calInfo.FB_ZERO) {
    if (fb < senderContext.calInfo.FB_MIN) {
      error(ERROR_FB_TOO_LOW);
      senderContext.calInfo.FB_MIN = fb;
    }
    packet.fbNormalized = ((long)(senderContext.calInfo.FB_ZERO - fb)) * 255 / (senderContext.calInfo.FB_ZERO - senderContext.calInfo.FB_MIN);
    bitSet(packet.bits, sensorbit_BACKWARD);
  }
  if (lr >= senderContext.calInfo.LR_ZERO) {
    if (lr > senderContext.calInfo.LR_MAX) {
      error(ERROR_LR_TOO_HIGH);
      senderContext.calInfo.LR_MAX = lr;
    }
    packet.lrNormalized = ((long)(lr - senderContext.calInfo.LR_ZERO)) * 255 / (senderContext.calInfo.LR_MAX - senderContext.calInfo.LR_ZERO);
    bitSet(packet.bits, sensorbit_LEFT);
  } else
  if (lr < senderContext.calInfo.LR_ZERO) {
    if (lr < senderContext.calInfo.LR_MIN) {
      error(ERROR_LR_TOO_LOW);
      senderContext.calInfo.LR_MIN = lr;
    }
    packet.lrNormalized = ((long)(senderContext.calInfo.LR_ZERO - lr)) * 255 / (senderContext.calInfo.LR_ZERO - senderContext.calInfo.LR_MIN);
    bitClear(packet.bits, sensorbit_LEFT);
  }
  // TODO read horn and breaks buttons. For now, set them both to true - 1  
  bitSet(packet.bits, sensorbit_HORN);
  bitSet(packet.bits, sensorbit_BREAKS); 

  //mySerial.print("FB"); mySerial.print("\t: "); mySerial.print( bitRead(packet.bits, sensorbit_BACKWARD) ? "  v  " : "  ^  "  ); mySerial.print(packet.fbNormalized); mySerial.print("\t("); mySerial.print(fb); mySerial.println(")");
  //mySerial.print("LR"); mySerial.print("\t: "); mySerial.print( bitRead(packet.bits, sensorbit_LEFT) ? "  <  " : "  >  "  ); mySerial.print(packet.lrNormalized); mySerial.print("\t("); mySerial.print(lr); mySerial.println(")"); 
  //mySerial.println();
}

// round  X,Y value to 0 if too small
void applyZeroTolerance(SensorData& packet) {
  if (packet.fbNormalized < ZERO_THRESHOLD) {
    packet.fbNormalized = 0;
    bitClear(packet.bits, sensorbit_BACKWARD); 
  }
  if (packet.lrNormalized < ZERO_THRESHOLD) {
    packet.lrNormalized = 0;
    bitClear(packet.bits, sensorbit_LEFT);
  }
}

void buildPacket(SensorData& sensorData, Packet& packet) {
  packet.reset();
  bool backward = bitRead(sensorData.bits, sensorbit_BACKWARD);
  bool left = bitRead(sensorData.bits, sensorbit_LEFT);
  bool motor1dir = backward;
  bool motor2dir = backward;
  
  if (sensorData.lrNormalized == 0) {
    // if no left-right , both motors have the same throttle
    packet.motor1 = (unsigned char) sensorData.fbNormalized;
    packet.motor2 = (unsigned char) sensorData.fbNormalized;
  } else {
    // we have a LR reading
    int motor1 = sensorData.fbNormalized; // start from that and add/sub accordingly. This also covers the case where fbNormalized == 0
    int motor2 = sensorData.fbNormalized; 

    int offset = (int) (((float)sensorData.lrNormalized) * TURN_FACTOR);
    if (! backward ) {  // if forward
      if ( !left ) { // if right
        motor1 += offset;
        motor2 -= offset;
      } else { // if left
        motor1 -= offset;
        motor2 += offset;
      }
    } else {
      // moving backwards. Invert offset operation
      if ( ! left ) { //if right
        motor1 += offset;
        motor2 -= offset;
      } else {
        motor1 -= offset;
        motor2 += offset;
      }
    }
    // TODO.the above two branches can be merged with a bitwise operation in the conditional.We can merge them if we need to optimize memory

    // now apply thresholds
    if (motor1 > 255)
      motor1 = 255;
    if (motor2 > 255)
      motor2 = 255;
    // see if we need to invert motor direction
    if (motor1 < 0) {
      motor1 = -motor1;
      motor1dir = !motor1dir;
    }
    if (motor2 < 0) {
      motor2 = -motor2;
      motor2dir = !motor2dir;
    }

    // populate outgoing packet
    packet.motor1 = motor1;
    packet.motor2 = motor2;
  }
  bitWrite(packet.bits, packetbit_MOTOR1, motor1dir);
  bitWrite(packet.bits, packetbit_MOTOR2, motor2dir);

  //mySerial.print("fb: "); mySerial.print(sensorData.fbNormalized); mySerial.print("\tlr: "); mySerial.print(sensorData.lrNormalized); mySerial.println();
  //mySerial.print("motor1: "); mySerial.print(packet.motor1); mySerial.print("\t direction: "); mySerial.print(bitRead(packet.bits, packetbit_MOTOR1)); mySerial.println();
  //mySerial.print("motor2: "); mySerial.print(packet.motor2); mySerial.print("\t direction: "); mySerial.print(bitRead(packet.bits, packetbit_MOTOR2)); mySerial.println("\n");
  
}


void transmitPacket(Packet& packet) {
  packet.crc = 0; // reseting this since it should not be taken into account when calculating CRC. 
  byte crc = CRC8( (byte*)&packet, sizeof(packet));
  packet.crc = crc;

  //Serial.write((char*)&packet, sizeof(packet));
  if (! Rf::send(packet) ) {
      Serial.println(F("Transmission failed or timed out"));
  } else {
      //Serial.println("successfully sent");
      //Serial.print("M1 "); Serial.print(packet.motor1); Serial.print("  M2 "); Serial.println(packet.motor2);
  }
  
  //Serial.print("fb: "); Serial.print(packet.fbNormalized); Serial.print(" - forward: "); Serial.println(bitRead(packet.bits,bit_FORWARD));
  //Serial.print("lr: "); Serial.print(packet.lrNormalized); Serial.print(" - right: "); Serial.println(bitRead(packet.bits,bit_RIGHT));
  //Serial.print("CRC8: "); Serial.println(crc);
}



#endif
