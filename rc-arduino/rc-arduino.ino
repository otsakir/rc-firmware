#include "buttons.h"
#include "periodictask.h"
#include "comm.h"
#include "sender.h"

#include <SoftwareSerial.h>

SoftwareSerial mySerial(4,2);

// Receiver 
#include "receiver.h"

// forward declaration for remote-control arduino application 

void buttonCalibrateHandler(ButtonEvent event, Button& button);
void transmitTaskHandler(int dt);



// --- GLOBAL VARIABLES --- */

// BUTTON_PRESS means fires when button is actually released
Button buttonCalibrate(3, BUTTON_PRESS, buttonCalibrateHandler);
// transmit every 1000 msec
PeriodicTask transmitTask(10, transmitTaskHandler);

void helloTaskHandler(int dt) {
  mySerial.println("hello there!");
}

PeriodicTask helloTask(1000, helloTaskHandler);


/* --- H A N D L E R S --- */

void transmitTaskHandler(int dt) {
  if (senderContext.TRANSMITTING) {
    readSensors(senderContext.sensorData);
    applyZeroTolerance(senderContext.sensorData);
    buildPacket(senderContext.sensorData, senderContext.packet); // copy sensor values gathered in sensor-data variable to outgoing packet structure
    transmitPacket(senderContext.packet);

    // RECEIVER
    Packet received_packet;
    receivePacket(received_packet);
    processReceived(received_packet);
  }
  
}

void buttonCalibrateHandler(ButtonEvent event, Button& button) {
  Serial.println("button calibrate pressed");
  if ( !senderContext.CALIBRATING )
    startCalibration();
  else {
    stopCalibration();
    dumpConfig();
  }
}

/* ---  Arduino-specific stuff --- */

void setup() {
  Serial.begin(9600);
  mySerial.begin(38400);
  mySerial.println("Hello, world?");
  
  pinMode(CALIBRATION_PIN, INPUT);
  // initialize zero position in both axis
  senderContext.FB_ZERO = analogRead(FRONTBACK_PIN);
  senderContext.LR_ZERO = analogRead(LEFTRIGHT_PIN); 

  // FOR TESTING ONLY - receiver stuff
  receiverSetup();
  
}

void loop() {
  // check if buttonCalibrate is pressed and trigger its handler if it is
  buttonCalibrate.update();
  // update calibration limits FB_MAX, FB_MIN, LR_MAX, LR_MIN
  if ( senderContext.CALIBRATING )
    calibrate();
  // check if it's time to transmit the packet
  transmitTask.check(millis());
  helloTask.check(millis());
}

/* --- Application code --- */

void startCalibration() {
  stopTransmitting(); // make sure we're not transmitting
  Serial.println("started calibration");
  senderContext.CALIBRATING = true;

  senderContext.FB_ZERO = analogRead(FRONTBACK_PIN);
  senderContext.LR_ZERO = analogRead(LEFTRIGHT_PIN);   
  //int FB_MAX,FB_MIN = FB_ZERO;  // TODO : what wasthis ?
  //int LR_MAX, LR_MIN = LR_ZERO;
}

void stopCalibration() {
  senderContext.CALIBRATING = false;
  Serial.println("stopped calibration");
  dumpConfig();
  // TODO maybe we should have an explicit command of the user to start transmission
  startTransmitting();
}

void startTransmitting() {
  senderContext.TRANSMITTING = true;
}

void stopTransmitting() {
  senderContext.TRANSMITTING = false;
}

void dumpConfig() {
  Serial.print("FB_ZERO: "); Serial.println(senderContext.FB_ZERO);
  Serial.print("LR_ZERO: "); Serial.println(senderContext.LR_ZERO);
  Serial.print("LR_MAX: "); Serial.println(senderContext.LR_MAX);
  Serial.print("LR_MIN: "); Serial.println(senderContext.LR_MIN);
  Serial.print("FB_MAX: "); Serial.println(senderContext.FB_MAX);
  Serial.print("FB_MIN: "); Serial.println(senderContext.FB_MIN);

}

// setup *_MAX,*_MIN calibration limits. Keep calling this while moving the stick to possible positions.
void calibrate() {
  int fb = analogRead(FRONTBACK_PIN);
  int lr = analogRead(LEFTRIGHT_PIN);  
  senderContext.FB_MAX = fb > senderContext.FB_MAX ? fb : senderContext.FB_MAX;
  senderContext.FB_MIN = fb < senderContext.FB_MIN ? fb : senderContext.FB_MIN;
  senderContext.LR_MAX = lr > senderContext.LR_MAX ? lr : senderContext.LR_MAX;
  senderContext.LR_MIN = lr < senderContext.LR_MIN ? lr : senderContext.LR_MIN;
}



void transmitPacket(Packet& packet) {
  packet.crc = 0; // reseting this since it should not be taken into account when calculating CRC. 
  byte crc = CRC8( (byte*)&packet, sizeof(packet));
  packet.crc = crc;
  
  //Serial.print("fb: "); Serial.print(packet.fbNormalized); Serial.print(" - forward: "); Serial.println(bitRead(packet.bits,bit_FORWARD));
  //Serial.print("lr: "); Serial.print(packet.lrNormalized); Serial.print(" - right: "); Serial.println(bitRead(packet.bits,bit_RIGHT));
  //Serial.print("CRC8: "); Serial.println(crc);
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
  memcpy( &returned_packet, &(senderContext.packet), sizeof(Packet) );
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
