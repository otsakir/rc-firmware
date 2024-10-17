## nRF24 
[Datasheet](https://www.sparkfun.com/datasheets/Wireless/Nordic/nRF24L01P_Product_Specification_1_0.pdf)

### Power analysis

Στο UNO

pin 3.3V

    reset -> 0.0mA
    Receive mode - 21 mA.
    Transmit - 0.0mA                                                                                                   
    
pin D8

	  0.0mA σε όλη τη διάρκεια.
	
pin D7

		receiver - 1.2mA
		transmitter - 0.0mA
		
pin D13

	  0.0mA σε όλη τη διάρκεια.

	
pin D12 (miso)

	  0.0mA σε όλη τη διάρκεια.
	
pin D11 (mosi)

	  0.0mA σε όλη τη διάρκεια.

pin GND

  receiver - 22.6mA
  transmitter - 0.01-0.4mA. Κάνει μια μικρή κορυφή ανά δευτερόλεπτο (όταν πράγματι στέλνει).
	
	
