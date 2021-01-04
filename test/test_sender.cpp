#include <cpunit>

// app specific stuff
#include <arduino_types.h>
#include <mocks.h>
#include <comm.h>
#include <sender.h>

#include <queue>
#include <iostream>
  
using namespace cpunit;


void initContext() {
    SenderContext& ctx = senderContext;
    ctx.CALIBRATING = false;
    ctx.TRANSMITTING = false;
    // ctx.sensorData // is automatically initialized as an object
    ctx.FB_ZERO = 512;
    ctx.FB_MAX = 1023;
    ctx.FB_MIN = 1;
    ctx.LR_ZERO = 512;
    ctx.LR_MIN = 1;
    ctx.LR_MAX = 1023;

}

// Test reading sensors, parsing values and mapping into acceptable limits (still in progress)
CPUNIT_GTEST(read_sensors) {

    initContext();
    AnalogValues A0_values;
    AnalogValues A1_values;
    mock_set(A0, A0_values);
    mock_set(A1, A1_values);
    SensorData sensorData;

    // test zero position
    A0_values.push(512);
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
}

CPUNIT_GTEST(zero_tolerance) {

    initContext();
    AnalogValues A0_values;
    AnalogValues A1_values;
    mock_set(A0, A0_values); // overriding queue object. I hope this destroys the previous
    mock_set(A1, A1_values);
    SensorData sensorData;

    // if ZERO_THRESHOLD is not overcome we should still get 0 throttle after applying zero tolerance
    A0_values.push(512 + 20 ); // 20, when projected to 0-255 should be less than ZERO_THRESHOLD (10)
    A1_values.push(512 - 20 );
    readSensors(sensorData);
    applyZeroTolerance(sensorData);
    assert_equals("sensorData.fbNormalized", 0, int(sensorData.fbNormalized)); // 9 was returned and truncated to 0
    assert_equals("sensorData.lrNormalized", 0, int(sensorData.lrNormalized)); // same here
    assert_equals("sensorData.bits",8, int(sensorData.bits & 0b1100)); // LR direction should still be affected. We have small oposite reading but still oposite

    // ZERO_THRESHOLD value exceeded
    A0_values.push(512 + 30 ); 
    A1_values.push(512 - 30 );
    readSensors(sensorData);
    applyZeroTolerance(sensorData);
    assert_equals("sensorData.fbNormalized", 14, int(sensorData.fbNormalized));
    assert_equals("sensorData.lrNormalized", 14, int(sensorData.lrNormalized));
    assert_equals("sensorData.bits",8, int(sensorData.bits & 0b1100)); // FB true, LR false
}

// test the generation of the outgoing packet
CPUNIT_GTEST(build_packet) {
    initContext();
    AnalogValues A0_values;
    AnalogValues A1_values;
    mock_set(A0, A0_values); 
    mock_set(A1, A1_values);
    SensorData sensorData;
    Packet packet;

    // full throttle forward
    sensorData.fbNormalized = 100;
    sensorData.lrNormalized = 0;
    sensorData.bits = 0b1100; // FW/RIGHT direction
    buildPacket(sensorData, packet);
    assert_equals("packet.motor1", 100, int(packet.motor1));
    assert_equals("packet.motor2", 100, int(packet.motor2));

    // forward and a little to the right. The left motor (motor1) should get more power
    sensorData.fbNormalized = 100;
    sensorData.lrNormalized = 25;
    sensorData.bits = 0b0000; // FW/RIGHT direction
    buildPacket(sensorData, packet);
    assert_equals("packet.motor1", 125, int(packet.motor1));
    assert_equals("packet.motor2", 75, int(packet.motor2));

    // forward and a lot to the right. The left motor (motor1) should get lot of power (max out at 255) and right motor should reverse a little
    sensorData.fbNormalized = 150;
    sensorData.lrNormalized = 200;
    sensorData.bits = 0b0000; // FW/RIGHT direction
    buildPacket(sensorData, packet);
    assert_equals("packet.motor1", 255, int(packet.motor1));
    assert_equals("packet.motor2", 50, int(packet.motor2));
    assert_equals("packet.bits", 0b1000, int(packet.bits)); // motor1 FW, motor2 reverse
    
    // a little to the left only. The motors should run counter-wise. Motor 2 forward, motor1 backward
    sensorData.fbNormalized = 0;
    sensorData.lrNormalized = 25;
    sensorData.bits = 0b1000; // left direction
    buildPacket(sensorData, packet);
    assert_equals("packet.motor1", 25, int(packet.motor1));
    assert_equals("packet.motor2", 25, int(packet.motor2));
    assert_equals("packet.bits", 0b0100, int(packet.bits)); // motor1 BW (1), motor2 FW (0)
}


CPUNIT_GTEST(integrated_parseSensors_to_Packet) {

    initContext();
    AnalogValues A0_values;
    AnalogValues A1_values;
    mock_set(A0, A0_values); // overriding queue object. I hope this destroys the previous
    mock_set(A1, A1_values);
    SensorData sensorData;
    Packet packet;

    // Moving forward and a little to the right. Both motors should move forward. Left motor (1) should run higher.
    A0_values.push(512 + 100 ); 
    A1_values.push(512 + 40 );
    readSensors(sensorData);
    applyZeroTolerance(sensorData);
    buildPacket(sensorData, packet);
    assert_equals("packet.motor1", 68, int(packet.motor1));
    assert_equals("packet.motor2", 30, int(packet.motor2));
    assert_equals("packet.bits", 0b0000, int(packet.bits));

}
