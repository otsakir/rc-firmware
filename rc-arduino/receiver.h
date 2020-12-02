#ifndef _RECEIVER_H_
#define _RECEIVER_H_

// receiver.h is the receiver or vehicle program

#define MOTOR1_PIN 9
#define MOTOR2_PIN 10
#define MOTOR1DIR_PIN 7
#define MOTOR2DIR_PIN 8

// run this inside arduino's setup()
void receiverSetup() {
  pinMode(MOTOR1_PIN, OUTPUT);
  pinMode(MOTOR2_PIN, OUTPUT);
  pinMode(MOTOR1DIR_PIN,OUTPUT);
  pinMode(MOTOR2DIR_PIN, OUTPUT);
}





#endif
