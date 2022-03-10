#include "buttons.h"
#include "periodictask.h"

#include <SPI.h>
#include "printf.h"
#include "RF24.h"

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


#define IS_RC true
#define IS_VEHICLE !IS_RC
#define RF_LEVEL RF24_PA_MIN

// pin mapping
#define FRONTBACK_PIN A0
#define LEFTRIGHT_PIN A1
#define CALIBRATION_PIN 3
#define ZERO_THRESHOLD 20 // fb/lr threshold value under which it's considered zero. It's 10 from 255.
#define TURN_FACTOR 1 // that's a factor affecting how quickly to turn
// RF24 customizable pins
#define CE_PIN 7
#define CSN_PIN 8

// RF24 address names
namespace Rf {
    uint8_t sendAddress[6] = "1Node";
    uint8_t receiveAddress[6] = "2Node";
}

#include "sender.h"


// forward declarations
void startCalibration();
void stopCalibration();
void dumpConfig();
void calibrate();
void startTransmitting();
void stopTransmitting();
// handlers
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
  
  Rf::init();
  
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
