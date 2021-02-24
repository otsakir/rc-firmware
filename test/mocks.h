#ifndef _MOCKS_H_
#define _MOCKS_H_

//#include <Arduino.h>


//std::queue<int>* A0_values;
//ArduinoQueues Arduino_Queues;
//ArduinoPinQueue ArduinoPins[15];


void mock_set(int analog_port, std::queue<int>& newAX);

void mock_resetPin(int pin);
	
void error(const char* msg);

void trace(const char* msg);

void warning(const char* msg, int intvalue);

void serialEvent(); // typically defined by official arduino lib


SerialClass mySerial(true);

// newAX stands for newA0, new A1 etc. for analog port value queues
void mock_set(int analog_port, std::queue<int>& newAX) {
    Arduino_Queues[analog_port] = & newAX;
}


void mock_resetPin(int pin) {
	while (!ArduinoPins[pin].empty()) {
		ArduinoPins[pin].pop();
	}
}

void error(const char* msg) {
  mySerial.print("[ERROR] "); mySerial.println(msg);
}

void trace(const char* msg) {
  mySerial.print("[TRACE] "); mySerial.println(msg);
}

void warning(const char* msg, int intvalue = -32768) { // display if other than -32768
  mySerial.print("[WARNING] "); mySerial.print(msg);
  if (intvalue != -32768) {
  mySerial.print(" - "); mySerial.print(intvalue); 
  }
  mySerial.println();
}


#endif


