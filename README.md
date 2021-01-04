# grasscutter

Ηλεκτροκίνητο χορτοκοπτικό με αρκετό Γρανάζη μέσα :-). Το project αυτό αφορά κυρίως το λογισμικό.

### Κωδικοποίηση πορείας

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

Η πορεία του οχήματος αρχικά καθορίζεται από 4 παραμέτρους. Τις τιμές (0-255) fbNormalized, lrNormalized που παριστάνουν τον _μέτρο_ της ταχύτητας που θέλουμε το κινηθούμε ή να στρίψουμε καθώς και δύο boolean τιμές `sensorbit_BACKWARD`, `sensorbit_LEFT` που ορίζουν την κατεύθινση της πορείας ή στροφής. Όταν το `sensorbit_BACKWARD` είναι false κινούμαστε εμπρός (FORWARD). Διαφορετικά πίσω. Αντίστοιχα όταν το `sensorbit_LEFT` είναι false στρίβουμε δεξιά. Διαφερετικά αριστερά. 

Η παραπάνω κωδικοποίηση στη συνέχεια μετατρέπεται σε τιμές throttle που θα τροφοδοτήσουν τα δύο μοτέρ. To Packet.motor1 αποτελεί το αριστερό μοτέρ όταν το όχημα κινείται προς τα εμπρός. Μία στροφή δεξιά λοιπόν θα μεταφραστεί με μεγαλύτερη τιμή για το motor1 και μικρότερη για το motor2. Τα Packet.bits έχουν και εδώ την ιδιότητα της αντιστροφής. Αν το `packetbit_MOTOR1` είναι false τότε έχουμε κίνηση προς τα εμπρός. Διαφορετικά προς τα πίσω. Ομοίως για το `packetbit_MOTOR2`.












