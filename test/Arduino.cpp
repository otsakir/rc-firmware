#include "Arduino.h"

#include "mocks.h"


void bitSet(unsigned char& bits, byte index) {
    bits = bits | (0b1 << index);
}

void bitClear(unsigned char& bits, byte index) {
    bits = bits & ~(0b1 << index);
}


int bitRead(unsigned char& bits, byte index) {
    return (bits >> index) & 1;
}

void bitWrite(unsigned char& bits, byte index, bool value) {
    if (value)
        bitSet(bits, index);
    else
        bitClear(bits, index);
}

int analogRead(int port) {
    if (!Arduino_Queues[port] || Arduino_Queues[port]->empty())
        throw ArduinoException(std::string("Arduino analog queue empty or not initialized: A") + std::to_string(port));

    int v = Arduino_Queues[port]->front();
    Arduino_Queues[port]->pop();
    return v;
}

void analogWrite(int port, unsigned char v) {
	//if (!Arduino_Queues[port])
    //    throw ArduinoException(std::string("Arduino analog queue not initialized: A") + std::to_string(port));
    //Arduino_Queues[port]->push(v);
    ArduinoPins[port].push(v);
}

void digitalWrite(int port, unsigned char v) {
	//if (!Arduino_Queues[port])
      //  throw ArduinoException(std::string("Arduino analog queue not initialized: A") + std::to_string(port));
    ArduinoPins[port].push(v);
    //Arduino_Queues[port]->push(v);	
}


SerialClass Serial;
