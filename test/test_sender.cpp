#include <cpunit>

// app specific stuff
#include <arduino_types.h>
#include <mocks.h>
#include <rcprotocol.h>
#include <sender.h>
#include <sender_core.h>

#include <queue>
#include <iostream>
  
using namespace cpunit;

/*
CPUNIT_GTEST(example_test) {

    AnalogValues A0_values;
    A0_values.push(5);
    A0_values.push(6);

    try {
        mock_set(A0, A0_values);
        assert_equals("should be equal", analogRead(0), 5);
        assert_equals("should be equal", analogRead(0), 6);

        unsigned char bb = 0b101;
        bitSet(bb, 1);

        assert_equals("should be equal", 7, int(bb));
        
        bb = 0b111;
        bitClear(bb,1);
        assert_equals("should be equal", 5, int(bb));

        bb = 0b0101100;
        assert_equals("should be equal", 1, bitRead(bb, 2) );
        assert_equals("should be equal", 0, bitRead(bb, 4) );

    } catch (const ArduinoException& e){
        std::cout << "error:" << e.message << std::endl;
    }
}
*/

CPUNIT_GTEST(the_test) {
    


    AnalogValues A0_values;
    A0_values.push(5);
    A0_values.push(6);
    mock_set(A0, A0_values);
    AnalogValues A1_values;
    A1_values.push(7);
    A1_values.push(8);
    mock_set(A1, A1_values);

    SensorData sensorData;
    
    readSensors(sensorData);

    std::cout << sensorData.fbNormalized << std::endl;

    assert_equals("should be equal", 7, int(sensorData.fbNormalized));

}

