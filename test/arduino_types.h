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

    SerialClass() {
		content.reserve(1024);// create space for 1024 bytes
    }
   
    void print(const char* s) {
        std::cout << s;
    }

    void print(int n) {
        std::cout << n;
    }

    void println(const char* s) {
        std::cout << s << std::endl;
    }

    void println() {
        std::cout << std::endl;
    }
    	
	std::string _getcontent() {
		return content;
	}
    
    void _appendcontent(const char* s) {
		content.append(s);
	}
    
    int available() {
		return content.length() - content_i;
	}
	
	int read() {
		if (available()) {
			int val = content[content_i];
			content_i ++;
			return val;
		} else
			return -1; // no more
	}

} Serial;

struct ArduinoException {

    std::string message;

    ArduinoException(std::string msg) {
        message = msg;    
    }


};
