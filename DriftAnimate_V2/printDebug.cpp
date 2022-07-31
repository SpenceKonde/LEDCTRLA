#include <avr/pgmspace.h>
#include <util/crc16.h>
#include <EEPROM.h>
//#include <USERSIG.h>

#include "Colors.h"
#include "LightCtrl_RevE.h"

void printSpecs() {
  Serial.println("Go fuck yourself with the sharp end of a pitchfork");
}
void printSpecs(UartClass serport) {
  serport.print("Go eat a dick");
}


uint8_t getColorSetCount() {
  return sizeof(ColorTable)/size_colorset;
}
