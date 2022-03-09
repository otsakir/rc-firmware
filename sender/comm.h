#ifndef _RCPROTOCOL_H_
#define _RCPROTOCOL_H_

/*
 * Communication stuff
 * 
 * Functionality placed here will be used by both the sender and receiver
 * 
 */

// Error messages are defined here. In case we run out of memory we can use smaller error descriptions like "E_31" etc.
#define ERROR_LR_TOO_LOW "found LR value out of LR_MIN limit. Pushing limit down"
#define ERROR_LR_TOO_HIGH "found LR value out of LR_MAX limit. Pushing limit up"
#define ERROR_FB_TOO_LOW "found FB value out of FB_MIN limit. Pushing limit down"
#define ERROR_FB_TOO_HIGH "found FB value out of FB_MAX limit. Pushing limit up"
#define ERROR_PACKET_BUFFER_FULL "packet buffer full"
#define ERROR_FEWER_BYTES_IN_BUFFER "too few bytes in buffer then expected"
#define STRING_PACKET_CRC_CHECK_FAILED "Packet CRC check failed. Dropping packet"

#define TRACE_RECEIVED_BYTE_FROM_SERIAL "received byte form serial"


// indexes of bit information in Packet.buttons 0-7
#define packetbit_HORN 0
#define packetbit_BREAKS 1
#define packetbit_MOTOR1 2  // left one .Reverse motor 1 direction.
#define packetbit_MOTOR2 3  // right one


struct Packet {
  unsigned char motor1; // left motor (as we move forward)
  unsigned char motor2; // right motor
  unsigned char bits;
  unsigned char index;  // increased by one for each different packet sent.
  byte crc;

  Packet() : Packet(0,0,0,0,0) {
  }
  
  Packet(unsigned char m1, unsigned char m2, unsigned char b, unsigned char i, byte c) {
	  motor1 = m1;
	  motor2 = m2;
	  bits = b;
	  index = i;
	  crc = c;
  }

  void reset() {
    motor1 = 0;
    motor2 = 0;
    bits = 0;
    index = 0;
    crc = 0;    
  }
  
};

byte CRC8(const byte *data, byte len);

namespace Rf {

    RF24 radio(CE_PIN, CSN_PIN);
    bool senderRole = IS_RC;  // RC starts as a sender. Can be changed later

    // NOTE: Serial should be initialized
    void init() {
    
        while (!Serial) {} // some boards need to wait to ensure access to serial over USB

        // initialize the transceiver on the SPI bus
        if (!radio.begin()) {
            Serial.println(F("radio hardware is not responding!!"));
            while (1) {} // hold in infinite loop
        }

        if (radio.isChipConnected()) {
            Serial.println("chip is CONNECTED");
        } else {
            Serial.println("chip is NOT CONNECTED!");
        }

        radio.setPALevel(RF_LEVEL);
        radio.setPayloadSize(sizeof(Packet));
        radio.setAutoAck(false);

        radio.openWritingPipe(sendAddress);
        radio.openReadingPipe(1, receiveAddress);

        if (senderRole) {
            radio.stopListening();  // put radio in TX mode
        } else {
            radio.startListening(); // put radio in RX mode
        }

        // For debugging info
        printf_begin();             // needed only once for printing details
        //radio.printDetails();       // (smaller) function that prints raw register values
        radio.printPrettyDetails(); // (larger) function that prints human readable data
    }
    
    // send over air
    bool send(const Packet& packet) {
        //unsigned long start_timer = micros();                    // start the timer
        bool report = radio.write(&packet, sizeof(packet));      // transmit & save the report
        //unsigned long end_timer = micros();                      // end the timer

        if (report) {
          //Serial.print(F("Transmission successful! "));          // payload was delivered
          //Serial.print(F("Time to transmit = "));
          //Serial.print(end_timer - start_timer);                 // print the timer result
          //Serial.print(F(" us. Sent: "));
          //payload.println();
          //payload.next();    
          return true;                                   // increment float payload
        } else {
          //radio.printPrettyDetails();
          //radio.printDetails();
          return false;
        }

    }
    
    // receive from air. Returns number of packets (1) or -1 in case of error.
    int recv(Packet& packet) {
        uint8_t pipe;
        
        if (radio.available(&pipe)) {             // is there a payload? get the pipe number that recieved it
          uint8_t bytes = radio.getPayloadSize(); // get the size of the payload
          if ( bytes != sizeof(Packet) ) {
              Serial.print("invalid incoming size packet"); Serial.println(bytes);
              return -1;
          }
          radio.read(&packet, bytes);            // fetch payload from FIFO
          /*Serial.print(F("Received "));
          Serial.print(bytes);                    // print the size of the payload
          Serial.print(F(" bytes on pipe "));
          Serial.print(pipe);                     // print the pipe number
          Serial.print(F(": "));
          payload.println();*/
          
          return 1;
        } else {
            return 0;
        }
    } 
    

}



#endif
