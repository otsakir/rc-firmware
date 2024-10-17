# RC firmware

Arduino based firmware for remote controlled vehicles. This repository includes two different arduino based projects for transmitter and receiver respectively. Communication is one-way.

## Transmitter

The transmitter consists of an Arduino nano micro controller, a joystick and an RF24 communication board. All sorts of driving commands like foward, backward, left and right and throttle are encoded in two sets of values for the left and right motors respectively and transmitted as a packet over RF. Transmitter reads analog values process them and sends to the receiver (the vehicle).

Besides the driving commands, the communication protocol supports more operational options. Such slots could be used for controlling the vehicles lights, play a sound or perform other actions depending on the actual capabilities of the vehicle.


The transmitter features:

* **Analog input calibarion**. Different analog input controllers (joystics) can be used. An input device is calibrared the first time its used or when the operator triggers the `calibrate` button.
* **Normalization and tolerance**. Driving input values are normalized within limits and a tolerance threshold can be supplied to filter out noise
* **Configuration persistance**. All configuration settings are peristed on the transmitter board's flash memory.
* Easy debugging through TX/RX arduino pins.

## Receiver

The receiver reads the incoming packet and drives the motors accordingly by encoding these values over PWM to *an external driver circuit*. Other information is also processed at this stage and feeds the board's pins.

The receiver features:

* **CRC based packet verification**. The incoming packet is validated using a lightweight CRC algorithm.
* **Low latency**. The system can transmit and process without issues over 50 packets per second. 
* **Automatic halt**. In case communication is lost or several packets are lost the vehicle is brought to halt.
* Easy debugging through TX/RX arduino pins.

## Calibration

When the system is first used it passes through a calibration stage (arduino PIN 3). From the operator point of view this involves letting the joystick settle and press calibration button. Then move the joystick in all possible extends. Once all extremes are covered, press the calibration button again. That's it. All center and extreme values are now stored in flash memory.

You can later perform the calibation again by pressing the calibrate button again.

## Build and deploy

Both transmitter (sender) and receiver firmware is implemented as an arduino project. **arduino-mk** makefiles are also available for fast and automated iterations and are the recommended way to deploy on the actual boards and further develop the system.

To install arduino-mk on ubuntu:

    $ sudo apt install arduino-mk

and from `sender`/`receiver` directory execute:    

    $ MONITOR_PORT=/dev/ttyUSB0 BOARD_SUB=atmega328old BOARD_TAG=nano make && MONITOR_PORT=/dev/ttyUSB0 BOARD_SUB=atmega328old BOARD_TAG=nano make upload && MONITOR_PORT=/dev/ttyUSB0 BOARD_SUB=atmega328old BOARD_TAG=nano make monitor

This one-liner will build, upload and and start monitoring the device in the current terminal window. You can stop the monitor by pressing Ctrl+A+K (it runs `screen` underneath).
 
Of cource, such monitoring terminals can run for both the sender and receiver simultaneously to give a better overview of the comminication from both ends. 
 
### Environment

Some additional environment variables may need some tweaking in case arduino-mk builds don't work right away. To get an idea, here is what works in my system. 

    cat ~/.bashrc
    ...
    export ARDUINO_DIR=/home/nando/bin/arduino-1.8.13
    export ARDMK_DIR=/usr/share/arduino
    export AVR_TOOLS_DIR=/
    
You can find more on arduino make here [https://github.com/sudar/Arduino-Makefile](https://github.com/sudar/Arduino-Makefile)


## Internals

To get an idea of the protocol and the driving algorithm, here is a code snippet taken from `comm.h` and `sender.h.

```
#define sensorbit_BACKWARD 2 // zero is forward, 1 is backward
#define sensorbit_LEFT 3 // zero is right, 1 is left. These bits work as a reverse operation. Positive is FW/RIGHT...
...

struct SensorData {
  unsigned short fbNormalized;
  unsigned short lrNormalized;
  unsigned char bits;
  ...

#define packetbit_MOTOR1 2  // left one .Reverse motor 1 direction.
#define packetbit_MOTOR2 4  // right one
...

struct Packet {
  unsigned char motor1; // left motor (as we move forward)
  unsigned char motor2; // right motor
  unsigned char bits;
  byte crc;
  ...
```


#### Normalization  phase

Let's assume that the joystick is settling free in the center. At this position, the resistance of both axis will be almost half in both potensiometers (~512). This values will be convertes to more convenient range within 0-255. In both axis, 0  means that the joystick is resting whereas there are two additional bits conveying the direction of the movement. This conversion is called **normalization**.

The movement of the vehicle is now determined from 4 parameters in pairs. So, for forward/backward movements we have `fbNormalized`in the range 0-255 that is the magnitude of the speed and a boolean values representing the direction for such movement. Forward or backward. Likewise, `lrNormalized`is the magnitude of turning left or right and is complemented by a boolean value representing whether it's left or right. This is part of the `SensorData` data structure. 

#### Zero tolerance

When reading from an analog input device there will probably be some "noise" like reading that should be ingored. This is achieved by using a tolerance factor. This is a below threashold below which `fb/lrNormalized` will be considered 0. By default it's set to 20 through ZERO_THRESHOLD define. 

#### Throttle

Before being transmitted `fb/lrNormalized`/direction value pairs will be processed and encoded as values for the `motor1`, and `motor2` variables of the `Packet` protocol structure and direction bits. For the left motor comm.h/`Packet.motor1`. For the right, it's comm.h/`Packet.motor2`. Thus, a right turn will be translated to a bigger value for motor1 and a smaller for motor2. The direction of the motors is in comm.h/`Packet.bits`, If `packetbit_MOTOR1` is false the we are moving forward. Otherwise backward. Same goes for `packetbit_MOTOR2`.


### Sender serial monitor (n/a)

In the route of the development of this system, default TX/RX pins were reserved for the communication protocol. For monitoring and debugging a software serial implementation on pins D2, D4 was used. Though this is not the case any more, i'm taking this down as a reference.

To monitor this software serial port:

* Supply power to the main board of the sender.
* Connect the external USB-to-serial board (yes, you'll need a couple of those) to the development laptop. A new usb device will appear. In our case is was `/dev/ttyUSB1`.
* Configure the new device

    `$ stty -F /dev/ttyUSB1 cs8 38400 -ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts`

* Start a terminal that listens to the device.

	 `$ cat < /dev/ttyUSB1`

Whatever is written to mySerial stream of the arduino board should now appean on the screen.


















