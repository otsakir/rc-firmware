#include "buttons.h"
#include "periodictask.h"
#include "comm.h"

// some arduino-specific stuff. We don't want to put that in the more generic receiver.h
#include <SoftwareSerial.h>
SoftwareSerial mySerial(4,2);

#include "sender.h"


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
  }
}

void buttonCalibrateHandler(ButtonEvent event, Button& button) {
  mySerial.println("button calibrate pressed");
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
  mySerial.println("started calibration");
  senderContext.CALIBRATING = true;

  senderContext.FB_ZERO = analogRead(FRONTBACK_PIN);
  senderContext.LR_ZERO = analogRead(LEFTRIGHT_PIN);   
  //int FB_MAX,FB_MIN = FB_ZERO;  // TODO : what wasthis ?
  //int LR_MAX, LR_MIN = LR_ZERO;
}

void stopCalibration() {
  senderContext.CALIBRATING = false;
  mySerial.println("stopped calibration");
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
  mySerial.print("FB_ZERO: "); Serial.println(senderContext.FB_ZERO);
  mySerial.print("LR_ZERO: "); Serial.println(senderContext.LR_ZERO);
  mySerial.print("LR_MAX: "); Serial.println(senderContext.LR_MAX);
  mySerial.print("LR_MIN: "); Serial.println(senderContext.LR_MIN);
  mySerial.print("FB_MAX: "); Serial.println(senderContext.FB_MAX);
  mySerial.print("FB_MIN: "); Serial.println(senderContext.FB_MIN);

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
