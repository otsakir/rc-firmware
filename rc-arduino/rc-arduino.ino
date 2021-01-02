#include "buttons.h"
#include "periodictask.h"
#include "rcprotocol.h"
#include "sender.h"
#include "sender_core.h"

// Receiver 
#include "receiver.h"

// forward declaration for remote-control arduino application 

void buttonCalibrateHandler(ButtonEvent event, Button& button);
void transmitTaskHandler(int dt);



// --- GLOBAL VARIABLES --- */

bool CALIBRATING = false; // read-only value. should not be edited directly. Only through start/stopCalibration.
bool TRANSMITTING = false;
SensorData sensorData;
Packet packet;
// BUTTON_PRESS means fires when button is actually released
Button buttonCalibrate(3, BUTTON_PRESS, buttonCalibrateHandler);
// transmit every 1000 msec
PeriodicTask transmitTask(10, transmitTaskHandler);


/* --- H A N D L E R S --- */

void transmitTaskHandler(int dt) {
  if (TRANSMITTING) {
    readSensors(sensorData);
    applyZeroTolerance(sensorData);
    buildPacket(sensorData, packet); // copy sensor values gathered in sensor-data variable to outgoing packet structure
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


void applyZeroTolerance(SensorData& packet) {
  if (packet.fbNormalized < ZERO_THRESHOLD) 
    packet.fbNormalized = 0;
  if (packet.lrNormalized < ZERO_THRESHOLD)
    packet.lrNormalized = 0;
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
  
  analogWrite(MOTOR1_PIN, packet.motor1);
  analogWrite(MOTOR2_PIN, packet.motor2);

  digitalWrite(MOTOR1DIR_PIN, bitRead(packet.bits, packetbit_MOTOR1) );
  digitalWrite(MOTOR2DIR_PIN, bitRead(packet.bits, packetbit_MOTOR2) );
}
