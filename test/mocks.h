#ifndef _MOCKS_H_
#define _MOCKS_H_

#include <Arduino.h>
#include <queue>

typedef std::queue<int> AnalogValues; // a series of analog values. It represents the values returned by a single arduino analog port

typedef AnalogValues* ArduinoQueues[7]; // an array representing the values returned by all arduino analog ports. There are 7 of them.

typedef std::queue<int> ArduinoPinQueue;

extern std::queue<int>* A0_values;
extern ArduinoQueues Arduino_Queues;
extern ArduinoPinQueue ArduinoPins[15];


void mock_set(int analog_port, std::queue<int>& newAX);

void mock_resetPin(int pin);
	
void error(const char* msg);

void trace(const char* msg);

void warning(const char* msg, int intvalue);

void serialEvent(); // typically defined by official arduino lib


#endif


