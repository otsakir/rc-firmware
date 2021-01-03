#include <cpunit>

// app specific stuff
#include <arduino_types.h>
#include <mocks.h>
#include <rcprotocol.h>
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
    assert_equals("sensorData.bits",15, int(sensorData.bits)); // horn and breaks are set to '1' explicitly for now

    // test small movement forward-right
    A0_values.push(512 +80);
    A1_values.push(512 + 50);
    readSensors(sensorData);
    assert_equals("sensorData.fbNormalized", 39, int(sensorData.fbNormalized));
    assert_equals("sensorData.lrNormalized", 24, int(sensorData.lrNormalized));
    assert_equals("sensorData.bits",15, int(sensorData.bits)); // horn and breaks are set to '1' explicitly for now

    // test small movement backward-left. Same throttle as previously but oposite direction
    A0_values.push(512 -80);
    A1_values.push(512 -50);
    readSensors(sensorData);
    assert_equals("sensorData.fbNormalized", 39, int(sensorData.fbNormalized));
    assert_equals("sensorData.lrNormalized", 24, int(sensorData.lrNormalized));
    assert_equals("sensorData.bits",3, int(sensorData.bits)); // horn and breaks are set to '1' explicitly for now

    // test full throttle forward
    A0_values.push(senderContext.FB_MAX);
    A1_values.push(senderContext.LR_ZERO);
    readSensors(sensorData);
    assert_equals("sensorData.fbNormalized", 255, int(sensorData.fbNormalized));
    assert_equals("sensorData.lrNormalized", 0, int(sensorData.lrNormalized));
    assert_equals("sensorData.bits",15, int(sensorData.bits)); // horn and breaks are set to '1' explicitly for now
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
    assert_equals("sensorData.bits",7, int(sensorData.bits)); // LR direction should still be affected. We have small oposite reading but still oposite

    // ZERO_THRESHOLD value exceeded
    A0_values.push(512 + 30 ); 
    A1_values.push(512 - 30 );
    readSensors(sensorData);
    applyZeroTolerance(sensorData);
    assert_equals("sensorData.fbNormalized", 14, int(sensorData.fbNormalized));
    assert_equals("sensorData.lrNormalized", 14, int(sensorData.lrNormalized));
    assert_equals("sensorData.bits",7, int(sensorData.bits)); // FB true, LR false
}

// test the generation of the outgoing packet
// WIP
/*
CPUNIT_GTEST(build_packet) {

    initContext();
    AnalogValues A0_values;
    AnalogValues A1_values;
    mock_set(A0, A0_values); // overriding queue object. I hope this destroys the previous
    mock_set(A1, A1_values);
    SensorData sensorData;

*/
