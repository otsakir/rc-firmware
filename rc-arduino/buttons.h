#ifndef _BUTTON_H_
#define _BUTTON_H_

enum ButtonType { BUTTON_PRESS, BUTTON_RELEASE, BUTTON_HOLD };
enum ButtonEvent { BUTTON_EVENT_PRESSED };

/*
 * High level class for handling button presses, releases, holds etc.
 * 
 * Normally, a button is open (port LOW) and when pressed the circuit is closed (port HIGH)
 */
struct Button {

  // define the 'Handler' callback type
  typedef void (*Handler)(ButtonEvent event, Button& button);

  Handler handler;
  unsigned char port;
  ButtonType type;
  bool status; // last value returned by the port 


  Button(unsigned char pport, ButtonType ptype, Handler phandler ) {
    port = pport;
    handler = phandler;
    type = ptype;
  }

  void init() { 
    pinMode(port, INPUT);
    status = digitalRead(port);   
    Serial.print("initializing button at port ");
    Serial.print(port);
    Serial.print(" - ");
    Serial.println(status);
  }

  void update() {
    unsigned char newStatus = digitalRead(port);  
    if ( type == BUTTON_PRESS) { 
      if (status == HIGH && newStatus == LOW) { 
        handler(BUTTON_EVENT_PRESSED, *this);
      }
    }
    // TODO BUTTON_RELEASE and BUTTON_HOLD
    status = newStatus;
  }

};


#endif
