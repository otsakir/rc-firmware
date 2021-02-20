#ifndef Arduino_h
#define Arduino_h

#include <iostream>
#include <sstream>
#include <cstring>

#define A0 13 // index in ArduinoPins array
#define A1 14 // this one too

#define byte unsigned char

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


extern SerialClass Serial;





#endif
