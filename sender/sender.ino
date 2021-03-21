#include "buttons.h"
#include "periodictask.h"

#include <SoftwareSerial.h>
SoftwareSerial mySerial(4,2);

#include "sender.h"

// some arduino-specific stuff. We don't want to put that in the more generic receiver.h


void error(const char* msg) {
  mySerial.print("[ERROR] "); mySerial.println(msg);
}

void trace(const char* msg) {
  mySerial.print("[TRACE] "); mySerial.println(msg);
}

void warning(const char* msg, int intvalue /*= -32768*/) { // display if other than -32768
  mySerial.print("[WARNING] "); mySerial.print(msg);
  if (intvalue != -32768) {
  mySerial.print(" - "); mySerial.print(intvalue); 
  }
  mySerial.println();
}

// forward declaration for remote-control arduino application 

void buttonCalibrateHandler(ButtonEvent event, Button& button);
void transmitTaskHandler(int dt);



// BUTTON_PRESS - fire when button is actually released
Button buttonCalibrate(3, BUTTON_PRESS, buttonCalibrateHandler);
// transmit every 1000 msec
PeriodicTask transmitTask(1000, transmitTaskHandler);


/* --- Handlers --- */

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
  Serial.begin(9600); // used for RF comminication
  mySerial.begin(38400); // used for serial monitoring
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
  mySerial.print("FB_ZERO: "); mySerial.println(senderContext.FB_ZERO);
  mySerial.print("LR_ZERO: "); mySerial.println(senderContext.LR_ZERO);
  mySerial.print("LR_MAX: "); mySerial.println(senderContext.LR_MAX);
  mySerial.print("LR_MIN: "); mySerial.println(senderContext.LR_MIN);
  mySerial.print("FB_MAX: "); mySerial.println(senderContext.FB_MAX);
  mySerial.print("FB_MIN: "); mySerial.println(senderContext.FB_MIN);

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
