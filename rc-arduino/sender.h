#ifndef _SENDER_H_
#define _SENDER_H_

// sender.h is about remote-control

#define FRONTBACK_PIN A0
#define LEFTRIGHT_PIN A1
#define CALIBRATION_PIN 3
#define ZERO_THRESHOLD 10 // fb/lr threshold value under which it's considered zero. It's 10 from 255.

// forward declaration for remote-control arduino application 

void buttonCalibrateHandler(ButtonEvent event, Button& button);
void transmitTaskHandler(int dt);

#endif
