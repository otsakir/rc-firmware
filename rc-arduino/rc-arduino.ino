#include "buttons.h"
#include "periodictask.h"
#include "rcprotocol.h"
#include "rcarduino.h"

// Receiver 
#include "receiver.h"



#define FRONTBACK_PIN A0
#define LEFTRIGHT_PIN A1
#define CALIBRATION_PIN 3

// indexes of bit information in Packet.buttons 0-7
#define bit_HORN 0
#define bit_BREAKS 1
#define bit_FORWARD 2 // going forward or backward
#define bit_RIGHT 4 // turning left or right


// --- GLOBAL VARIABLES --- */

bool CALIBRATING = false; // read-only value. should not be edited directly. Only through start/stopCalibration.
bool TRANSMITTING = false;
Packet packet;
// BUTTON_PRESS means fires when button is actually released
Button buttonCalibrate(3, BUTTON_PRESS, buttonCalibrateHandler);
// transmit every 1000 msec
PeriodicTask transmitTask(10, transmitTaskHandler);

// all settings below are in terms of actual values read from the analog port
int FB_ZERO = 1024/2; // sensible defaults
int LR_ZERO = 1024/2; // sensible defaults
int FB_MAX,FB_MIN = FB_ZERO; 
int LR_MAX, LR_MIN = LR_ZERO;


/* --- H A N D L E R S --- */

void transmitTaskHandler(int dt) {
  if (TRANSMITTING) {
    //Serial.println("executing task!");
    readSensors(packet);
    transmitPacket(packet);

    // RECEIVER
    Packet received_packet;
    receivePacket(received_packet);
    processReceived(received_packet);
  }
  
}

void buttonCalibrateHandler(ButtonEvent event, Button& button) {
  Serial.println("button calibrate pressed");
  if ( !CALIBRATING )
    startCalibration();
  else {
    stopCalibration();
    dumpConfig();
  }
}

/* ---  Arduino-specific stuff --- */

void setup() {
  Serial.begin(9600);
  pinMode(CALIBRATION_PIN, INPUT);
  // initialize zero position in both axis
  FB_ZERO = analogRead(FRONTBACK_PIN);
  LR_ZERO = analogRead(LEFTRIGHT_PIN); 

  // FOR TESTING ONLY - receiver stuff
  receiverSetup();
  
}

void loop() {
  // check if buttonCalibrate is pressed and trigger its handler if it is
  buttonCalibrate.update();
  // update calibration limits FB_MAX, FB_MIN, LR_MAX, LR_MIN
  if ( CALIBRATING )
    calibrate();
  // check if it's time to transmit the packet
  transmitTask.check(millis());
}

/* --- Application code --- */

void startCalibration() {
  stopTransmitting(); // make sure we're not transmitting
  Serial.println("started calibration");
  CALIBRATING = true;

  FB_ZERO = analogRead(FRONTBACK_PIN);
  LR_ZERO = analogRead(LEFTRIGHT_PIN);   
  int FB_MAX,FB_MIN = FB_ZERO; 
  int LR_MAX, LR_MIN = LR_ZERO;
}

void stopCalibration() {
  CALIBRATING = false;
  Serial.println("stopped calibration");
  //dumpConfig();
  // TODO maybe we should have an explicit command of the user to start transmission
  startTransmitting();
}

void startTransmitting() {
  TRANSMITTING = true;
}

void stopTransmitting() {
  TRANSMITTING = false;
  
}

void dumpConfig() {
  Serial.print("FB_ZERO: "); Serial.println(FB_ZERO);
  Serial.print("LR_ZERO: "); Serial.println(LR_ZERO);
  Serial.print("LR_MAX: "); Serial.println(LR_MAX);
  Serial.print("LR_MIN: "); Serial.println(LR_MIN);
  Serial.print("FB_MAX: "); Serial.println(FB_MAX);
  Serial.print("FB_MIN: "); Serial.println(FB_MIN);

}

// setup *_MAX,*_MIN calibration limits. Keep calling this while moving the stick to possible positions.
void calibrate() {
  int fb = analogRead(FRONTBACK_PIN);
  int lr = analogRead(LEFTRIGHT_PIN);  
  FB_MAX = fb > FB_MAX ? fb : FB_MAX;
  FB_MIN = fb < FB_MIN ? fb : FB_MIN;
  LR_MAX = lr > LR_MAX ? lr : LR_MAX;
  LR_MIN = lr < LR_MIN ? lr : LR_MIN;
}


void transmitPacket(Packet& packet) {
  packet.crc = 0; // reseting this since it should not be taken into account when calculating CRC. 
  byte crc = CRC8( (byte*)&packet, sizeof(packet));
  packet.crc = crc;
  
  //Serial.print("fb: "); Serial.print(packet.fbNormalized); Serial.print(" - forward: "); Serial.println(bitRead(packet.bits,bit_FORWARD));
  //Serial.print("lr: "); Serial.print(packet.lrNormalized); Serial.print(" - right: "); Serial.println(bitRead(packet.bits,bit_RIGHT));
  //Serial.print("CRC8: "); Serial.println(crc);
}

// reads sensor values and prepares outgoing packet
void readSensors(Packet& packet) {
  long fb = analogRead(FRONTBACK_PIN);
  long lr = analogRead(LEFTRIGHT_PIN); 

  packet.bits = 0;
  if (fb >= FB_ZERO) {
    if (fb > FB_MAX) {
      error(ERROR_FB_TOO_HIGH);
      FB_MAX = fb;
    }
    packet.fbNormalized = ((long)(fb - FB_ZERO)) * 255 / (FB_MAX - FB_ZERO);
    bitSet(packet.bits, bit_FORWARD);
  } else
  if (fb < FB_ZERO) {
    if (fb < FB_MIN) {
      error(ERROR_FB_TOO_LOW);
      FB_MIN = fb;
    }
    packet.fbNormalized = ((long)(FB_ZERO - fb)) * 255 / (FB_ZERO - FB_MIN);
    bitClear(packet.bits, bit_FORWARD);
  }
  if (lr >= LR_ZERO) {
    if (lr > LR_MAX) {
      error(ERROR_LR_TOO_HIGH);
      LR_MAX = lr;
    }
    packet.lrNormalized = ((long)(lr - LR_ZERO)) * 255 / (LR_MAX - LR_ZERO);
    bitSet(packet.bits, bit_RIGHT);
  } else
  if (lr < LR_ZERO) {
    if (lr < LR_MIN) {
      error(ERROR_LR_TOO_LOW);
      LR_MIN = lr;
    }
    packet.lrNormalized = ((long)(LR_ZERO - lr)) * 255 / (LR_ZERO - LR_MIN);
    bitClear(packet.bits, bit_RIGHT);
  }
  // TODO read horn and breaks buttons. For now, set them both to true - 1  
  bitSet(packet.bits, bit_HORN);
  bitSet(packet.bits, bit_BREAKS); 

  //Serial.print("FB"); Serial.print("\t: "); Serial.print( bitRead(packet.bits, bit_FORWARD) ? "  -->  " : "  <--  "  ); Serial.print(packet.fbNormalized); Serial.print("\t("); Serial.print(fb); Serial.println(")");
  //Serial.print("LR"); Serial.print("\t: "); Serial.print( bitRead(packet.bits, bit_RIGHT) ? "  -->  " : "  <--  "  ); Serial.print(packet.lrNormalized); Serial.print("\t("); Serial.print(lr); Serial.println(")"); 
}


/* --- R E C E I V E R --- */


/*
 * Receives the packet from RF validates it and populates the packet structure
 * 
 * Memory handling for packet is done by outside layer. This function assumes
 * it contains  garbage, clears it and populates it by reading data from RF.
 * 
 * For testing purposes, the receiver is implmented in the same file and device 
 * with the sender. Thus, the function will read the packet from the global 
 * variable
 */
void receivePacket(Packet& returned_packet) {
  // TODO receive the packet
  // ...
  memcpy( &returned_packet, &packet, sizeof(Packet) );
  //Serial.print("receivePacket:"); Serial.println(returned_packet.crc);
}

void processReceived(Packet& packet) {
  //Serial.print("motor1 PWM"); Serial.println(packet.fbNormalized);
  //Serial.print("motor2 PWM"); Serial.println(packet.lrNormalized);
  // TODO!!!  convert fbNormalized, lrNormalized values to MOTOR1, MOTOR2 values
  // ...
  
  analogWrite(MOTOR1_PIN, packet.fbNormalized);
  analogWrite(MOTOR2_PIN, packet.lrNormalized);

  digitalWrite(MOTOR1DIR_PIN, bitRead(packet.bits, bit_FORWARD) );
  digitalWrite(MOTOR2DIR_PIN, bitRead(packet.bits, bit_RIGHT) );
}
