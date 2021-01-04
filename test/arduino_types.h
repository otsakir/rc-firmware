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

    SerialClass() {

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

} Serial;

struct ArduinoException {

    std::string message;

    ArduinoException(std::string msg) {
        message = msg;    
    }


};
