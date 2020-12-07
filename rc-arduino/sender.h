#ifndef _SENDER_H_
#define _SENDER_H_

// sender.h is about remote-control

#define FRONTBACK_PIN A0
#define LEFTRIGHT_PIN A1
#define CALIBRATION_PIN 3
#define ZERO_THRESHOLD 10 // fb/lr threshold value under which it's considered zero. It's 10 from 255.
#define TURN_FACTOR 1 // that's a factor affecting how quickly to turn

// indexes of bit information in SensorData 
#define sensorbit_HORN 0
#define sensorbit_BREAKS 1
#define sensorbit_FORWARD 2 // going forward or backward
#define sensorbit_RIGHT 4 // turning left or right


struct SensorData {
  unsigned short fbNormalized;
  unsigned short lrNormalized;
  unsigned char bits;

  SensorData() {
    fbNormalized = 0;
    lrNormalized = 0;
    bits = 0;
  }  
};


// forward declaration for remote-control arduino application 

void buttonCalibrateHandler(ButtonEvent event, Button& button);
void transmitTaskHandler(int dt);



#endif
