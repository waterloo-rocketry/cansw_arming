
#include <SoftwareSerial.h>

SoftwareSerial mySerial(12,13);

int altitude = 0;

void setup() {
  mySerial.begin(9600);

}

void loop() {
  altitude = millis() % 10000; // cycle from 0 to 10,000ft, at 1000 ft/sec
  String alt_str = String(altitude);

  mySerial.println(alt_str); //will block untill space to send

}
