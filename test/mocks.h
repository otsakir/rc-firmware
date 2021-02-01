#include <queue>

typedef std::queue<int> AnalogValues; // a series of analog values. It represents the values returns by a single arduino analog port

typedef AnalogValues* ArduinoQueues[7]; // an array representing the values returned by all arduino analog ports. There are 7 of then.


ArduinoQueues Arduino_Queues = {0,0,0,0,0,0,0};

SerialClass mySerial;

std::queue<int>* A0_values = 0;

// newAX stands for newA0, new A1 etc. for analog port value queues
void mock_set(int analog_port, std::queue<int>& newAX) {
    Arduino_Queues[analog_port] = & newAX;
}

int analogRead(int port) {
    if (!Arduino_Queues[port] || Arduino_Queues[port]->empty())
        throw ArduinoException(std::string("Arduino analog queue empty or not initialized: A") + std::to_string(port));

    int v = Arduino_Queues[port]->front();
    Arduino_Queues[port]->pop();
    return v;
}
