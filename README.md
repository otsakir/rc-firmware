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

#### Normalization 

Κρατάμε το joystick έτσι ώστε η ωμική αντίσταση να αυξάνει έαν το σπρώξουμε εμπρός ή αριστερά. Στην ελεύθερη θέση, η ωμική αντίσταση είναι περίπου το μισό της μέγιστης και στα δύο ποτενσιόμετρα (~512). Οι τιμές αυτές θα αναχθούν αρχικά σε ένα πιο βολικό εύρος από 0-255 με το 0 και στους δύο άξονες να σημαίνει ελεύθερη θέση με δύο πρόσθετα bits να δηλώνουν την κατεύθυνση. Η αναγωγή αυτή ονομάζεται normalization.

Η πορεία του οχήματος τώρα καθορίζεται από 4 παραμέτρους. Τις τιμές (0-255) fbNormalized, lrNormalized που παριστάνουν τον _μέτρο_ της ταχύτητας που θέλουμε το κινηθούμε ή να στρίψουμε καθώς και δύο boolean τιμές sender.h/`sensorbit_BACKWARD`, sender.h/`sensorbit_LEFT` που ορίζουν την κατεύθινση της πορείας ή στροφής. Όταν το `sensorbit_BACKWARD` είναι false κινούμαστε εμπρός (FORWARD). Διαφορετικά πίσω. Αντίστοιχα όταν το `sensorbit_LEFT` είναι false στρίβουμε δεξιά. Διαφερετικά αριστερά. 

#### Zero tolerance

Στη συνέχεια, πετάμε τις πολύ μικρές τιμές για τα fbNormalized, lrNormalized. Οτιδήποτε είναι μικρότερο από το sender.h/`ZERO_TOLERANCE` γίνεται 0. Η τρέχουσα οριακή τιμή είναι 20. Μαζί με τις τιμές, τα αντίστοιχα direction bits `sensorbit_LEFT/RIGHT`,  γίνονται 0. Αυτό κάνει τον αλγόριθμο πιο καθαρό ενώ διόρθωσε και ένα περίεργο bug στη πορεία.

#### Throttle

Πριν φύγουν για τον δέκτη, τα fbNormalized, lrNormalized θα μετατραπούν σε τιμές throttle που θα τροφοδοτήσουν τα δύο μοτέρ. Το γκάζι για το αριστερό μοτέρ (κινούμενοι προς τα εμπρός) είναι το comm.h/`Packet.motor1`. Για το δεξί το comm.h/`Packet.motor2`. Μία δεξιά στροφή λοιπόν θα μεταφραστεί με μεγαλύτερη τιμή για το motor1 και μικρότερη για το motor2. Η _φορά_ κίνησης των μοτέρ ορίζεται στα comm.h/`Packet.bits`. Ισχύει και εδώ η ιδιότητα της αντιστροφής. Αν το `packetbit_MOTOR1` είναι false τότε έχουμε κίνηση προς τα εμπρός. Διαφορετικά προς τα πίσω. Ομοίως για το `packetbit_MOTOR2`.

### Sender serial monitor

Για να παρακολουθείται η λειτουργία του sender προστέθηκε νέα software serial θύρα στο #14. Είναι στα pins D2,D4. Η κύρια σειριακή θύρα, - pins TX,RX - έχει δεσμευτεί για την επικοινωνία με τον receiver. Για να κάνεις monitor:

1. Δώσε ρεύμα στο κύριο κύκλωμα του sender.
2. Σύνδεσε την εξωτερική  USB-to-serial κάρτα στον Η/Υ. Θα δμιουργηθεί ένα νέο usb device. Στην περίπτωση μας το `/dev/ttyUSB1`.
3. Ρύθμισε το νέο device.

    $ stty -F /dev/ttyUSB1 cs8 38400 -ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts

4. Άνοιξε ένα τερματικό να ακούει το device.  

    $ cat < /dev/ttyUSB1

Ό,τι μήνυμα γραφτεί στο mySerial stream του arduino πρέπει τώρα να εμφανιστεί στη οθόνη.


## Troubleshooting

Για να φορτώσεις νέο πρόγραμμα στις σε sender ή receiver πρέπει να αποσυνδέσεις τα καλώδια TX/RX με οποία επικοινωνούν οι πλακέτες μεταξύ τους (εάν χρησιμοποιείς σύνδεση μέσω καλωδίου και όχι RF τουλάχιστον). 














