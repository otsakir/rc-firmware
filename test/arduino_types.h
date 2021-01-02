
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

struct SerialClass {

    SerialClass() {

    }

    void print(const char* s) {

    }

    void print(int) {

    }

    void println(const char* s) {
    }

    void println() {
    }

} Serial;

struct ArduinoException {

    std::string message;

    ArduinoException(std::string msg) {
        message = msg;    
    }


};
