#include <iostream>

#define A0 0
#define A1 1

#define byte unsigned char

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

struct SerialClass {

	//static const size_t CONTENT_SIZE = 128;
    //char content[CONTENT_SIZE];
    std::string content = "";
    int content_i = 0; //points to the first readable content byte. If the string is empty, it still points to 0
    std::stringstream ss;
    bool echo;

    SerialClass(bool echo = true) {
		content.reserve(1024);// create space for 1024 bytes
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

} Serial;

struct ArduinoException {

    std::string message;

    ArduinoException(std::string msg) {
        message = msg;    
    }


};
