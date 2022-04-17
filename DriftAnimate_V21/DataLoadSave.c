#include <USERSIG.h>

void initializeSetttings() {
  #if !defined PRODUCTION
    void printChipInfo();
  #endif
  loadUSERROW();
}

void printChipInfo() {
  Serial.println("Hi, I started! Serial Number: ");
  Serial.printHex((uint8_t*)(uint16_t)(0x1110), 0x10; ':');
  Serial.println();
  Serial.print("Die Rev: ");
  Serial.print(SYSCFG.REVID);
  Serial.print("CLK_PER: ");
  Serial.println("MHz");
#if COLCK_SOURCE != 0
  Serial.println("Uaing internal osc");
#elif CLOCK_SOURCE = 1
  Serial.println("Uaing external HF XTal");
#else
  Serial.println("Using external clock");
#endif
}
void loadUSERROW() {
  uint16_t crc = *(uint16_t*)(uint16_t)0x1102;
  // Seed it with the device ID
  uint8_t *Sigdata = (uint8_t*)(uint16_t)0x1110;
  for (byte i = 0; i < 16; i++) {
    crc = _crc16_update(crc, *(Sigdata++));
  }
  uint8_t datastate = 0x00;

  uint16_t MagicNumber = crc;
  Sigdata = (uint8_t*)(uint16_t)0x1402;
  uint16_t userRowMagicNumber = (*(uint16_t*)(uint16_t)0x1400)
  if (MagicNumber != userRowMagicNumber) {
  datastate = -1;
  Serial.print("Expected: ");
    Serial.printHex(MagicNumber);
    Serial.print(" Got: ");
    Serial.printHexln(userRowMagicNumber);
    if (userRowMagicNumber == 0xFFFF) {
      Serial.println(F("USERROW not initialized - you can do that by sending an 'i' over serial w/in 5 seconds"));
    } else {
      Serial.println(F("Other data is in the USERROW. Initialize it to tell this device about itself by sending 'i' within 5 secconds"));
    }
    datastate = offerReinit();
  } else {
    crc = _crc16_update(crc, *(Sigdata++));
    crc = _crc16_update(crc, *(Sigdata++));
    Sigdata+=14;
    userRowMagicNumber = (*(uint16_t*)(uint16_t)Sigdata++); // read the bytes 19 and 20l;
    if (temp == crc) {
      datastate = 1;
    } else {
      Serial.println(F("Error loadng Userrow config, invalid CEC. Please reinitialize by sending 'i'"));
      Serial.printHex(temp);
      Serial.print(" != ");
      Serial.println(crc);
      datastate = offerReinit();
    }
  }
  if (datastate == 1) {
    Serial.print("Initializing...");
    Sigdata = (uint8_t*)(uint16_t)0x1402;;

  }
}
