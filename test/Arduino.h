#ifndef Arduino_h
#define Arduino_h

#include <iostream>
#include <sstream>
#include <cstring>

#define A0 13 // index in ArduinoPins array
#define A1 14 // this one too

#define byte unsigned char

#include <queue>

typedef std::queue<int> AnalogValues; // a series of analog values. It represents the values returned by a single arduino analog port
typedef AnalogValues* ArduinoQueues[7]; // an array representing the values returned by all arduino analog ports. There are 7 of them.
typedef std::queue<int> ArduinoPinQueue;

ArduinoQueues Arduino_Queues = {0,0,0,0,0,0,0};
ArduinoPinQueue ArduinoPins[15];
std::queue<int>* A0_values = 0;


void bitSet(unsigned char& bits, byte index);

void bitClear(unsigned char& bits, byte index);

int bitRead(unsigned char& bits, byte index);

void bitWrite(unsigned char& bits, byte index, bool value);

int analogRead(int port);

void analogWrite(int port, unsigned char v);

void digitalWrite(int port, unsigned char v);

struct SerialClass {

	//static const size_t CONTENT_SIZE = 128;
    //char content[CONTENT_SIZE];
    std::string content = "";
    std::stringstream ss;
    bool echo;

    SerialClass(bool echo = true) {
		this->echo = echo;
    }
   
    void print(const char* s) {
        if (echo) std::cout << s;
        ss << s;
    }

    void print(int n) {
        if (echo) std::cout << n;
        ss << n;
    }

    void println(const char* s) {
        if (echo) std::cout << s << std::endl;
        ss << s << std::endl;
    }

    void println() {
        if (echo) std::cout << std::endl;
        ss << std::endl;
    }
    
    size_t write(const char* buf, int len) {
		ss.write(buf, len); 
		return len; // assumes that everything was written ok
	}
    	    	
	// return all content concatenated in stringstream (ss).
	std::string _str() {
		return ss.str();
	}
    
    std::stringstream& _buffer() {
		return ss;
	}
    
    // returns the number of available characters to read from stream (ss)
    int available() {
		std::streampos oldp = ss.tellg();
		ss.seekg(0, ss.end);
		int avail = ss.tellg() - oldp;
		ss.seekg(oldp);
		return avail;
	}
	
	int read() {
		return ss.get();
	}

};


struct ArduinoException {

    std::string message;

    ArduinoException(std::string msg) {
        message = msg;    
    }


};




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

extern SerialClass Serial;





#endif
