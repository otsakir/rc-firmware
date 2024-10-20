#include "buttons.h"
#include "periodictask.h"

#include <SPI.h>
#include "printf.h"
#include "RF24.h"
#include <EEPROM.h>

/*  Ports used 
 *  
 *  D3 - calibration button
 *  A0 - joystick front/back input
 *  A1 - joystick left/right input
 * 
 *  D13 - SCK (RF24/6)
 *  D12 - MISO (RF24/5)
 *  D11 - MOSI (RF24/3)
 *  D8 - CSN (RF24/2)
 *  D7 - CE (RF24/7)
 *  D3 - Calibration button
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
#define PAIRBUTTON_1A_PIN 2
#define PAIRBUTTON_1B_PIN 4
#define PAIRBUTTON_2A_PIN 5
#define PAIRBUTTON_2B_PIN 6
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
void calibrate();
void startTransmitting();
void stopTransmitting();
// handlers
void buttonCalibrateHandler(ButtonEvent event, Button& button);
void transmitTaskHandler(int dt);



// BUTTON_PRESS - fire when button is actually released
Button buttonCalibrate(3, BUTTON_PRESS, buttonCalibrateHandler);
// transmit every 1000 msec
PeriodicTask transmitTask(100, transmitTaskHandler);


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
    //senderContext.calInfo.dump(Serial);
  }
}

void updatePairbuttons(SenderContext& senderContext) {
  
  // only check 1A if the other (1B) is not pressed
  
  int status1A = digitalRead(PAIRBUTTON_1A_PIN);
  int status1B = digitalRead(PAIRBUTTON_1B_PIN);
  
  if ( status1A && !senderContext.pairbutton1B) { 
    senderContext.pairbutton1A = status1A;
  } else
    senderContext.pairbutton1A = LOW;

  if (status1B && !senderContext.pairbutton1A) {
    senderContext.pairbutton1B = status1B;
  } else
    senderContext.pairbutton1B = LOW;
  
  Serial.print("1A: "); Serial.print(senderContext.pairbutton1A);
  Serial.print("  1B: "); Serial.print(senderContext.pairbutton1B);
  
  
  int status2A = digitalRead(PAIRBUTTON_2A_PIN);
  int status2B = digitalRead(PAIRBUTTON_2B_PIN);
  
  if ( status2A && !senderContext.pairbutton2B) { 
    senderContext.pairbutton2A = status2A;
  } else
    senderContext.pairbutton2A = LOW;

  if (status2B && !senderContext.pairbutton2A) {
    senderContext.pairbutton2B = status2B;
  } else
    senderContext.pairbutton2B = LOW;
  
  Serial.print(" 2A: "); Serial.print(senderContext.pairbutton2A);
  Serial.print("  2B: "); Serial.print(senderContext.pairbutton2B);
  Serial.println();   
  
}

EepromTool eepromTool;


/* ---  Arduino-specific stuff --- */
void setup() {
  
  Serial.begin(57600); // used for serial monitoring
  Serial.println("sender: hello");
  
  // pairbuttons
  pinMode(PAIRBUTTON_1A_PIN, INPUT);
  pinMode(PAIRBUTTON_1B_PIN, INPUT);
  pinMode(PAIRBUTTON_2A_PIN, INPUT);
  pinMode(PAIRBUTTON_2B_PIN, INPUT);
  
  // calibration and EEPROM
  pinMode(CALIBRATION_PIN, INPUT);
  eepromTool.init();
  bool calibrated = eepromTool.isCalibrated();
  Serial.print("isCalibrated: "); Serial.println(calibrated);
  if (calibrated) {
  	eepromTool.eepromContent.calibrationInfo.dump(Serial);
  	senderContext.calInfo = eepromTool.eepromContent.calibrationInfo;
  	senderContext.CALIBRATING = false;
  	startTransmitting();
  } else {
	  // initialize zero position in both axis
	  senderContext.calInfo.FB_ZERO = analogRead(FRONTBACK_PIN);
	  senderContext.calInfo.LR_ZERO = analogRead(LEFTRIGHT_PIN); 
	  senderContext.CALIBRATING = true;
  }
  senderContext.senderPacketIndex = 0; 
  
  // RF24
  Rf::init();
  
}

void loop() {
  // check if buttonCalibrate is pressed and trigger its handler if it is
  buttonCalibrate.update();
  updatePairbuttons(senderContext);
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

  senderContext.calInfo.FB_ZERO = analogRead(FRONTBACK_PIN);
  senderContext.calInfo.LR_ZERO = analogRead(LEFTRIGHT_PIN); 
  senderContext.calInfo.FB_MIN = senderContext.calInfo.FB_ZERO;
  senderContext.calInfo.FB_MAX = senderContext.calInfo.FB_ZERO;
  senderContext.calInfo.LR_MIN = senderContext.calInfo.LR_ZERO;
  senderContext.calInfo.LR_MAX = senderContext.calInfo.LR_ZERO;
}

void stopCalibration() {
  senderContext.CALIBRATING = false;
  Serial.println("stopped calibration");
  eepromTool.saveCalibration(senderContext.calInfo);
  senderContext.calInfo.dump(Serial);
  // TODO maybe we should have an explicit command of the user to start transmission
  startTransmitting();
}

void startTransmitting() {
  senderContext.TRANSMITTING = true;
}

void stopTransmitting() {
  senderContext.TRANSMITTING = false;
}

// setup *_MAX,*_MIN calibration limits. Keep calling this while moving the stick to possible positions.
void calibrate() {
  int fb = analogRead(FRONTBACK_PIN);
  int lr = analogRead(LEFTRIGHT_PIN);  
  senderContext.calInfo.FB_MAX = fb > senderContext.calInfo.FB_MAX ? fb : senderContext.calInfo.FB_MAX;
  senderContext.calInfo.FB_MIN = fb < senderContext.calInfo.FB_MIN ? fb : senderContext.calInfo.FB_MIN;
  senderContext.calInfo.LR_MAX = lr > senderContext.calInfo.LR_MAX ? lr : senderContext.calInfo.LR_MAX;
  senderContext.calInfo.LR_MIN = lr < senderContext.calInfo.LR_MIN ? lr : senderContext.calInfo.LR_MIN;
}
