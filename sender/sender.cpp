#include <Arduino.h>
#include "sender.h"
#include "utils.h"


SenderContext senderContext;

// reads sensor values and bundle then in SensorData
void readSensors(SensorData& packet) {
  long fb = analogRead(FRONTBACK_PIN);
  long lr = analogRead(LEFTRIGHT_PIN); 

  packet.bits = 0;
  if (fb >= senderContext.FB_ZERO) {
    if (fb > senderContext.FB_MAX) {
      error(ERROR_FB_TOO_HIGH);
      senderContext.FB_MAX = fb;
    }
    packet.fbNormalized = ((long)(fb - senderContext.FB_ZERO)) * 255 / (senderContext.FB_MAX - senderContext.FB_ZERO);
    bitClear(packet.bits, sensorbit_BACKWARD);
  } else
  if (fb < senderContext.FB_ZERO) {
    if (fb < senderContext.FB_MIN) {
      error(ERROR_FB_TOO_LOW);
      senderContext.FB_MIN = fb;
    }
    packet.fbNormalized = ((long)(senderContext.FB_ZERO - fb)) * 255 / (senderContext.FB_ZERO - senderContext.FB_MIN);
    bitSet(packet.bits, sensorbit_BACKWARD);
  }
  if (lr >= senderContext.LR_ZERO) {
    if (lr > senderContext.LR_MAX) {
      error(ERROR_LR_TOO_HIGH);
      senderContext.LR_MAX = lr;
    }
    packet.lrNormalized = ((long)(lr - senderContext.LR_ZERO)) * 255 / (senderContext.LR_MAX - senderContext.LR_ZERO);
    bitClear(packet.bits, sensorbit_LEFT);
  } else
  if (lr < senderContext.LR_ZERO) {
    if (lr < senderContext.LR_MIN) {
      error(ERROR_LR_TOO_LOW);
      senderContext.LR_MIN = lr;
    }
    packet.lrNormalized = ((long)(senderContext.LR_ZERO - lr)) * 255 / (senderContext.LR_ZERO - senderContext.LR_MIN);
    bitSet(packet.bits, sensorbit_LEFT);
  }
  // TODO read horn and breaks buttons. For now, set them both to true - 1  
  bitSet(packet.bits, sensorbit_HORN);
  bitSet(packet.bits, sensorbit_BREAKS); 

  mySerial.print("FB"); mySerial.print("\t: "); mySerial.print( bitRead(packet.bits, sensorbit_BACKWARD) ? "  v  " : "  ^  "  ); mySerial.print(packet.fbNormalized); mySerial.print("\t("); mySerial.print(fb); mySerial.println(")");
  mySerial.print("LR"); mySerial.print("\t: "); mySerial.print( bitRead(packet.bits, sensorbit_LEFT) ? "  <  " : "  >  "  ); mySerial.print(packet.lrNormalized); mySerial.print("\t("); mySerial.print(lr); mySerial.println(")"); 
  mySerial.println();
}

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


/*
 * GLOBALS  : No global access
 * 
 */
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

  mySerial.print("fb: "); mySerial.print(sensorData.fbNormalized); mySerial.print("\tlr: "); mySerial.print(sensorData.lrNormalized); mySerial.println();
  mySerial.print("motor1: "); mySerial.print(packet.motor1); mySerial.print("\t direction: "); mySerial.print(bitRead(packet.bits, packetbit_MOTOR1)); mySerial.println();
  mySerial.print("motor2: "); mySerial.print(packet.motor2); mySerial.print("\t direction: "); mySerial.print(bitRead(packet.bits, packetbit_MOTOR2)); mySerial.println("\n");
  
}


void transmitPacket(Packet& packet) {
  packet.crc = 0; // reseting this since it should not be taken into account when calculating CRC. 
  byte crc = CRC8( (byte*)&packet, sizeof(packet));
  packet.crc = crc;

  Serial.write((char*)&packet, sizeof(packet));
  
  //Serial.print("fb: "); Serial.print(packet.fbNormalized); Serial.print(" - forward: "); Serial.println(bitRead(packet.bits,bit_FORWARD));
  //Serial.print("lr: "); Serial.print(packet.lrNormalized); Serial.print(" - right: "); Serial.println(bitRead(packet.bits,bit_RIGHT));
  //Serial.print("CRC8: "); Serial.println(crc);
}

