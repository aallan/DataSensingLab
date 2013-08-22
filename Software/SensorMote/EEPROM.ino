#include <EEPROM.h>

// retrieve a number from EEPROM
unsigned long getNumber() {
  unsigned long ctr;
  
  //initial setting of number
  if (EEPROM.read(5) != 1) { 
    
    // if number set status is false
    Serial.println("Initializing number in EEPROM");
    EEPROM.write(1,0); // write LSB zero
    EEPROM.write(2,0); // write 2ndB zero
    EEPROM.write(3,0); // write 3rdB zero
    EEPROM.write(4,0); // write MSB zero
    EEPROM.write(5,1); // counter set status is true
  }
  
  //get the number - add Bytes for 32-bit number
  ctr = (EEPROM.read(4) << 24) + (EEPROM.read(3) << 16) + (EEPROM.read(2) << 8) + (EEPROM.read(1)); 

  Serial.print("Getting number from EEPROM = ");
  Serial.println( ctr );
  return ctr;
}

// write a number to EEPROM
void setNumber(unsigned long ctr) {
  
  Serial.print("Setting number in EEPROM to = ");
  Serial.println( ctr );
  EEPROM.write(4,(ctr & 0xFFFFFFFF) >> 24); // write the MSB
  EEPROM.write(3,(ctr & 0xFFFFFF) >> 16); // write the 3rdB
  EEPROM.write(2,(ctr & 0xFFFF) >> 8); // write the 2ndB
  EEPROM.write(1,ctr & 0xFF); // write the LSB
}

