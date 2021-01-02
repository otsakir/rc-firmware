

// all settings below are in terms of actual values read from the analog port
int FB_ZERO = 1024/2; // sensible defaults
int LR_ZERO = 1024/2; // sensible defaults
int FB_MAX,FB_MIN = FB_ZERO; 
int LR_MAX, LR_MIN = LR_ZERO;


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


// reads sensor values and bundle then in SensorData
void readSensors(SensorData& packet) {
  long fb = analogRead(FRONTBACK_PIN);
  long lr = analogRead(LEFTRIGHT_PIN); 

  packet.bits = 0;
  if (fb >= FB_ZERO) {
    if (fb > FB_MAX) {
      error(ERROR_FB_TOO_HIGH);
      FB_MAX = fb;
    }
    packet.fbNormalized = ((long)(fb - FB_ZERO)) * 255 / (FB_MAX - FB_ZERO);
    bitSet(packet.bits, sensorbit_FORWARD);
  } else
  if (fb < FB_ZERO) {
    if (fb < FB_MIN) {
      error(ERROR_FB_TOO_LOW);
      FB_MIN = fb;
    }
    packet.fbNormalized = ((long)(FB_ZERO - fb)) * 255 / (FB_ZERO - FB_MIN);
    bitClear(packet.bits, sensorbit_FORWARD);
  }
  if (lr >= LR_ZERO) {
    if (lr > LR_MAX) {
      error(ERROR_LR_TOO_HIGH);
      LR_MAX = lr;
    }
    packet.lrNormalized = ((long)(lr - LR_ZERO)) * 255 / (LR_MAX - LR_ZERO);
    bitSet(packet.bits, sensorbit_RIGHT);
  } else
  if (lr < LR_ZERO) {
    if (lr < LR_MIN) {
      error(ERROR_LR_TOO_LOW);
      LR_MIN = lr;
    }
    packet.lrNormalized = ((long)(LR_ZERO - lr)) * 255 / (LR_ZERO - LR_MIN);
    bitClear(packet.bits, sensorbit_RIGHT);
  }
  // TODO read horn and breaks buttons. For now, set them both to true - 1  
  bitSet(packet.bits, sensorbit_HORN);
  bitSet(packet.bits, sensorbit_BREAKS); 

  Serial.print("FB"); Serial.print("\t: "); Serial.print( bitRead(packet.bits, sensorbit_FORWARD) ? "  -->  " : "  <--  "  ); Serial.print(packet.fbNormalized); Serial.print("\t("); Serial.print(fb); Serial.println(")");
  Serial.print("LR"); Serial.print("\t: "); Serial.print( bitRead(packet.bits, sensorbit_RIGHT) ? "  -->  " : "  <--  "  ); Serial.print(packet.lrNormalized); Serial.print("\t("); Serial.print(lr); Serial.println(")"); 
  Serial.println();
}


void buildPacket(SensorData& sensorData, Packet& packet) {
  packet.reset();
  if (sensorData.lrNormalized == 0) {
    // if no left-right , both motors have the same throttle
    packet.motor1 = (unsigned char) sensorData.fbNormalized;
    packet.motor2 = (unsigned char) sensorData.fbNormalized;
  } else {
    // we have a LR reading
    int motor1 = sensorData.fbNormalized; // start from that and add/sub accordingly. This also covers the case where fbNormalized == 0
    int motor2 = sensorData.fbNormalized; 

    int offset = (int) (((float)sensorData.lrNormalized) * TURN_FACTOR);
    if (sensorbit_FORWARD) {  
      if ( bitRead(sensorData.bits, sensorbit_RIGHT) ) {
        motor1 += offset;
        motor2 -= offset;
      } else {
        motor1 -= offset;
        motor2 += offset;
      }
    } else {
      // moving backwards. Invert offset operation
      if ( ! bitRead(sensorData.bits, sensorbit_RIGHT) ) {
        motor1 += offset;
        motor2 -= offset;
      } else {
        motor1 -= offset;
        motor2 += offset;
      }
    }
    // TODO.the above two branches can be merged with a bitwise operation in the conditional.We can merge them if we need to optimize memory
    
    // now apply thresholds
    if (motor1 < 0) {
      motor1 = -motor1;
      bitSet(packet.bits, packetbit_MOTOR1);
      //     motor1 = 0;
    }
    if (motor2 < 0) {
      motor2 = -motor2;
      bitSet(packet.bits, packetbit_MOTOR2);
      // motor2 = 0;
    }
    if (motor1 > 255)
      motor1 = 255;
    if (motor2 > 255)
      motor2 = 255;

    packet.motor1 = motor1;
    packet.motor2 = motor2;
    
  }

  Serial.print("fb: "); Serial.print(sensorData.fbNormalized); Serial.print("\tlr: "); Serial.print(sensorData.lrNormalized); Serial.println();
  Serial.print("motor1: "); Serial.print(packet.motor1); Serial.print("\t direction: "); Serial.print(bitRead(packet.bits, packetbit_MOTOR1)); Serial.println();
  Serial.print("motor2: "); Serial.print(packet.motor2); Serial.print("\t direction: "); Serial.print(bitRead(packet.bits, packetbit_MOTOR2)); Serial.println("\n");
  
}
