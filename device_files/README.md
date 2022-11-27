## Vehicle detection device files:
**MAIN.ino**: Main program where the presence of a vehicle is determined, the message is sent through the LoRaWAN network and it is checked if any message has been received.

**FUNCTIONS.h**: auxiliary functions for vehicle detection and behavior modification of the device according to the received message.

**VECTOR.h**: functions for the comparison of different readings of the earth's magnetic field vector.

**ARDUINO_SECRETS.h**: keys to the app on The Things Network server (The Things Stack).
