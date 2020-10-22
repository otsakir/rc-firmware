#include "buttons.h"
#include "periodictask.h"

#define FRONTBACK_PIN A0
#define LEFTRIGHT_PIN A1
#define CALIBRATION_PIN 3

// flags
bool CALIBRATING = false; // read-only value. should not be edited directly. Only through start/stopCalibration.
bool TRANSMITTING = false;

void readSensors();

void taskHandler(int dt) {
  Serial.println("task executed!");
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

// BUTTON_PRESS means fires when button is actually released
Button buttonCalibrate(3, BUTTON_PRESS, buttonCalibrateHandler);
PeriodicTask simpleTask(1000, taskHandler);

// all settings below are in terms of actual values read from the analog port
int FB_ZERO = 1024/2; // sensible defaults
int LR_ZERO = 1024/2; // sensible defaults
int FB_MAX,FB_MIN = FB_ZERO; 
int LR_MAX, LR_MIN = LR_ZERO;
int FB = FB_ZERO;
int LR = LR_ZERO;



void setup() {
  Serial.begin(9600);
  pinMode(CALIBRATION_PIN, INPUT);
  // initialize zero position in both axis
  FB_ZERO = analogRead(FRONTBACK_PIN);
  LR_ZERO = analogRead(LEFTRIGHT_PIN);  
  
}

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
}

void startTransmitting() {
  
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

void loop() {
  // put your main code here, to run repeatedly:
  readSensors();
  //Serial.print("frontback: "); Serial.print(FB); 
  //Serial.print("leftright: "); Serial.print(LR);
  //Serial.println();

  buttonCalibrate.update();
  simpleTask.check(millis());
  if ( CALIBRATING )
    calibrate();
}

void readSensors() {
  FB = analogRead(FRONTBACK_PIN);
  LR = analogRead(LEFTRIGHT_PIN);  
}
