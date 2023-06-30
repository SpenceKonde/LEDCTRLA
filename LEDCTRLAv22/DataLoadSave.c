#include <USERSIG.h>


void loadUSERROW() {
  uint16_t crc = *(uint16_t*)(uint16_t)0x1102;
  // Seed it with the device ID
  uint8_t *Sigdata = (uint8_t*)(uint16_t)0x1110;
  for (byte i = 0; i < 12; i++) {
    crc = _crc16_update(crc, *(Sigdata++));
  }
  uint8_t datastate = 0x00;
  Hardware.SigCRC = crc;
  uint16_t userRowMagicNumber = (*(uint16_t*)(uint16_t)0x1400)
  if (MagicNumber != crc) {
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
    Sigdata = (uint8_t*)(uint16_t)0x1402;
    for (uint8_t i = 16; i; i--) {
      crc = _crc16_update(crc, *(Sigdata++));
    }
    byteWord config_crc;
    config_crc.b[0] = *(Sigdata++);
    config_crc.b[1] = *(Sigdata++);
    if (config_crc.w != crc) {
      Serial.println(F("Error loadng Userrow config, invalid CRC. Please reinitialize by sending 'i'"));
      Serial.printHex(temp);
      Serial.print(" != ");
      Serial.println(crc);
      datastate = offerReinit();
    } else {
      Serial.println("Found valid configuration")
      datastate = 1;
    }
  }
  if (datastate == 1) {
    Serial.print("Initializing...");
    Sigdata = (uint8_t*)(uint16_t)0x1402;
    Hardware.HW_Version         = *(Sigdata++); //  starts at 0x1402 in the unified address space. PCB rev.
    Hardware.Seq_Number         = *(Sigdata++); //  Running tally of Dx-based boards produced.
    Hardware.LCD_flags          = *(Sigdata++); //
    Hardware.RF_Flags           = *(Sigdata++); //
    Hardware.Basic_Features     = *(Sigdata++); //  circit protection and connections present
    Hardware.UI_connections     = *(Sigdata++); //  (invert encoders and such)
    Hardware.data_config        = *(Sigdata++); //
    Hardware.RF_config          = *(Sigdata++); //
    Hardware.extensionconfig[0] = *(Sigdata++); //
    Hardware.extensionconfig[1] = *(Sigdata++); //
    Hardware.extensionconfig[2] = *(Sigdata++); //
    Hardware.extensionconfig[3] = *(Sigdata++); //
    Hardware.extensionconfig[4] = *(Sigdata++); //
    Hardware.extensionconfig[5] = *(Sigdata++); //
    Hardware.extensionconfig[6] = *(Sigdata++); //
    Hardware.extensionconfig[7] = *(Sigdata++); //
    Hardware.ConfigCRC          = crc;          //
    Serial.print("Initialization complete")
  }
}
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
  Serial.print(F_CPU/1000000UL)
  Serial.println(" MHz");
#if COLCK_SOURCE != 0
  Serial.println("Using internal osc");
#elif CLOCK_SOURCE = 1
  Serial.println("Using external HF XTal");
#else
  Serial.println("Using external clock");
#endif
}
