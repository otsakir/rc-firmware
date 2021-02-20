/*
 * Build test with: 
 * 
 * .../grasscutter/test$ g++ -g -o0 -I.   -I../receiver/  -L/home/nando/src/CPUnit_0.95/lib -L../receiver/  -o test_receiver test_receiver.cpp ../receiver/receiver.cpp ../receiver/utils.cpp Arduino.cpp mocks.cpp -I/home/nando/src/CPUnit_0.95/src/ -lCPUnit
 * 
 */

#include <cpunit>

#include <Arduino.h>
#include <mocks.h>

#include <receiver.h>

// mock callbacks

void onPacketDropped(Packet& droppedPacket) {
	//mySerial.println("Dropped!");
}

void onPacketReceived(Packet& packet) {
  analogWrite(MOTOR1_PIN, packet.motor1);
  analogWrite(MOTOR2_PIN, packet.motor2);
  digitalWrite(MOTOR1DIR_PIN, bitRead(packet.bits, packetbit_MOTOR1) );
  digitalWrite(MOTOR2DIR_PIN, bitRead(packet.bits, packetbit_MOTOR2) );
}

#include <receiver.h>


#include <queue>
#include <iostream>
  
using namespace cpunit;


//receiverContext = ReceiverContext();

void initContext() {
	receiverContext = ReceiverContext();
}

// Test reading sensors, parsing values and mapping into acceptable limits (still in progress)
CPUNIT_GTEST(read_sensors) {
	
    mock_resetPin(MOTOR1_PIN);
    mock_resetPin(MOTOR2_PIN);
    mock_resetPin(MOTOR1DIR_PIN);
    mock_resetPin(MOTOR2DIR_PIN);
    
    std::cout << Serial.available() << std::endl;
    
    Packet packet1(50,60,4,15,0); // 4, stands for packetbit_MOTOR1 bit set
    packet1.crc = CRC8((byte*) &packet1, sizeof(Packet)) + 1;
    // simulate receiving of bytes
    Serial._buffer().write((char*) &packet1, sizeof(Packet));
    
    std::cout << Serial.available() << std::endl;
    
    //std::cout << Serial.available() << std::endl;
    // notify that some data have arrived
    serialEvent();
    
    std::cout << Serial.available() << std::endl;
    
    // process packet and push data to I/O ports
    checkReceived(receiverContext);
    
    assert_equals("motor1", 50, ArduinoPins[MOTOR1_PIN].front());
    assert_equals("motor1", 60, ArduinoPins[MOTOR2_PIN].front());
    assert_true("motor1 dir", ArduinoPins[MOTOR1DIR_PIN].front());
    assert_false("motor2 dir", ArduinoPins[MOTOR2DIR_PIN].front());
        
    //AnalogValues A0_values;
    //AnalogValues A1_values;
    //mock_set(A0, A0_values);
    //mock_set(A1, A1_values);
    //SensorData sensorData;

    // test zero position
/*    A0_values.push(512);
    A1_values.push(512);
    readSensors(sensorData);
    assert_equals("sensorData.fbNormalized", 0, int(sensorData.fbNormalized));
    assert_equals("sensorData.lrNormalized", 0, int(sensorData.lrNormalized));
    assert_equals("sensorData.bits",0, int(sensorData.bits & 0b1100));

    // test small movement forward-right
    A0_values.push(512 +80);
    A1_values.push(512 + 50);
    readSensors(sensorData);
    assert_equals("sensorData.fbNormalized", 39, int(sensorData.fbNormalized));
    assert_equals("sensorData.lrNormalized", 24, int(sensorData.lrNormalized));
    assert_equals("sensorData.bits",0, int(sensorData.bits & 0b1100)); 

    // test small movement backward-left. Same throttle as previously but oposite direction
    A0_values.push(512 -80);
    A1_values.push(512 -50);
    readSensors(sensorData);
    assert_equals("sensorData.fbNormalized", 39, int(sensorData.fbNormalized));
    assert_equals("sensorData.lrNormalized", 24, int(sensorData.lrNormalized));
    assert_equals("sensorData.bits",12, int(sensorData.bits & 0b1100)); 

    // test full throttle forward
    A0_values.push(senderContext.FB_MAX);
    A1_values.push(senderContext.LR_ZERO);
    readSensors(sensorData);
    assert_equals("sensorData.fbNormalized", 255, int(sensorData.fbNormalized));
    assert_equals("sensorData.lrNormalized", 0, int(sensorData.lrNormalized));
    assert_equals("sensorData.bits",0, int(sensorData.bits & 0b1100));
    */
}

