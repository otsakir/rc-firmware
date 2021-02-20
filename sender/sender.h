#ifndef _SENDER_H_
#define _SENDER_H_

#include "comm.h"

// sender.h is about remote-control

#define FRONTBACK_PIN A0
#define LEFTRIGHT_PIN A1
#define CALIBRATION_PIN 3
#define ZERO_THRESHOLD 10 // fb/lr threshold value under which it's considered zero. It's 10 from 255.
#define TURN_FACTOR 1 // that's a factor affecting how quickly to turn

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

struct SenderContext {
  bool CALIBRATING = false; // read-only value. should not be edited directly. Only through start/stopCalibration.
  bool TRANSMITTING = false;
  SensorData sensorData;
  Packet packet;

  int FB_ZERO = 1024/2; // sensible defaults
  int LR_ZERO = 1024/2; // sensible defaults
  int FB_MAX,FB_MIN = FB_ZERO; 
  int LR_MAX, LR_MIN = LR_ZERO;  
  
};

extern SenderContext senderContext;

extern SerialClass mySerial;


// reads sensor values and bundle then in SensorData
void readSensors(SensorData& packet);

void applyZeroTolerance(SensorData& packet);

void buildPacket(SensorData& sensorData, Packet& packet);

void transmitPacket(Packet& packet);


#endif
