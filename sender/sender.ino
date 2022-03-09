#include "buttons.h"
#include "periodictask.h"

/*  Ports used 
 *  
 *  TX - other board RX
 *  RX - other board TX
 *  D4 - external serial board/TX
 *  D2 - external serial board/RX
 *  D3 - calibration button
 *  A0 - joystick front/back input
 *  A1 - joystick left/right input
 *  
 */


#include "sender.h"

// forward declarations
void startCalibration();
void stopCalibration();
void dumpConfig();
void calibrate();
void startTransmitting();
void stopTransmitting();

// some arduino-specific stuff. We don't want to put that in the more generic receiver.h


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

// forward declaration for remote-control arduino application 

void buttonCalibrateHandler(ButtonEvent event, Button& button);
void transmitTaskHandler(int dt);



// BUTTON_PRESS - fire when button is actually released
Button buttonCalibrate(3, BUTTON_PRESS, buttonCalibrateHandler);
// transmit every 1000 msec
PeriodicTask transmitTask(10, transmitTaskHandler);


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
  Serial.begin(57600); // used for serial monitoring
  Serial.println("sender: hello");
  
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
