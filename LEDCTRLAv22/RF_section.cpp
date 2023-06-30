

void setupRF() {
  TCB3.CTRLA = 0x02; //disable, CKPER/2 clock source.
  TCB3.CTRLB = 0x03; //Input Capture Frequency Measurement mode
  TCB3.INTFLAGS = 3; //clear flags
  TCB3.CNT = 0; //reset count 0
  TCB3.INTCTRL = 0x01;
  EVSYS.CHANNEL4 = EVSYS_CHANNEL4_PORTF_PIN3_gc; //On PF3.
  EVSYS_USERTCB3CAPT = 0x05;
  TCB3.EVCTRL = 0x51; //filter, falling edge, ICIE=1
  TCB3.CTRLA = 0x03; //enable
}

byte handleReceive() {
  if (GOT_MESSAGE) {
    byte vers = checkCSC(); //checkCSC() gives 0 on failed CSC, 1 on v1 structure (ACD...), 2 on v2 structure (DSCD...)
    if (!vers) { //if vers=0, unknown format ot bad CSC
      resetReceive();
      return 0;
    }
    if (!isForMe()) { //matches on MyAddress==0, destination address==0, destination address==MyAddress.
      resetReceive();
      return 0;
    }
    if (lastPacketSig == getPacketSig() && lastPacketTime) {

      lastPacketTime = millis();
      resetReceive();
      return 0;
    }
    lastPacketSig = getPacketSig();
    lastPacketTime = millis();
    byte rlen = ((pktLength >> 3) + 1) | ((vers - 1) << 6);

    memcpy((void*)recvMessage, (const void*)rxBuffer, 32); //copy received message - safe because we haven't called resetReceive, so IC int is off.
    resetReceive();
    return rlen;
  } else {
    unsigned long t = (millis() - lastPacketTime);
    if (lastPacketTime && (t > commandForgetTime)) {
      lastPacketTime = 0;
      lastPacketSig = 0;
    }
    return 0;
  }
}

void resetReceive() {
  if (bitnum > 4) {
    lastRFMsgAt = millis();
  }
  bitnum = 0;
  memset((void*)rxBuffer, 0, 32); //clear buffer - safe because haven't reenabled IC interrupt yet.
  CLR_MESSAGE;
  TCB3.INTCTRL = 0x01;
  return;
}

byte checkCSC() {
  byte rxchecksum = 0;
  byte rxchecksum2 = 0;
  for (byte i = 0; i < pktLength >> 3; i++) {
    rxchecksum = rxchecksum ^ rxBuffer[i];
    rxchecksum2 = _crc8_ccitt_update(rxchecksum2, rxBuffer[i]);
  }
  if (pktLength >> 3 == 3) {
    rxchecksum = (rxchecksum & 0x0F) ^ (rxchecksum >> 4) ^ ((rxBuffer[3] & 0xF0) >> 4);
    rxchecksum2 = (rxchecksum2 & 0x0F) ^ (rxchecksum2 >> 4) ^ ((rxBuffer[3] & 0xF0) >> 4);
    if (rxchecksum == rxchecksum2)rxchecksum2++;
    return (rxBuffer[3] & 0x0F) == rxchecksum ? 1 : ((rxBuffer[3] & 0x0F) == rxchecksum2 ) ? 2 : 0;
  } else {
    if (rxchecksum == rxchecksum2)rxchecksum2++;
    return ((rxBuffer[pktLength >> 3] == rxchecksum) ? 1 : ((rxBuffer[pktLength >> 3] == rxchecksum2 ) ? 2 : 0));
  }
}

byte isForMe() {
  if ((rxBuffer[0] & 0x3F) == MyAddress || MyAddress == 0 || (rxBuffer[0] & 0x3F) == 0) {
    return 1;
  }
  return 0;
}

unsigned long getPacketSig() {
  byte len = pktLength >> 3;
  unsigned long lastpacketsig = 0;
  for (byte i = (len == 3 ? 0 : 1); i < (len == 3 ? 3 : 4); i++) {
    lastpacketsig += rxBuffer[i];
    lastpacketsig = lastpacketsig << 8;
  }
  lastpacketsig += rxBuffer[len];
  return lastpacketsig;
}

ISR (TCB3_INT_vect) {
  static unsigned long lasttime = 0;
  unsigned int newTime = TCB3.CCMP; //immediately get the ICR value
  uint8_t state = digitalReadFast(RF_PIN);
  TCB3.EVCTRL = state ? 0x51 : 0x41; //trigger on falling edge if pin is high, otherwise rising edge
  unsigned int duration = newTime - lasttime;
  lasttime = newTime;
  if (state) {
    if (RFRX_NOW) {
      if (duration > rxLowMax) {
        DONE_RX;
        bitnum = 0; // reset to bit zero
        memset((void*)rxBuffer, 0, 32); //clear buffer - memset on rxBuffer is safe because interrupts disabled in ISR
      }
    } else {
      if (duration > rxSyncMin && duration < rxSyncMax) {
        START_RX;
      }
    }
  } else {
    if (RFRX_NOW) {
      if (duration > rxZeroMin && duration < rxZeroMax) {
        dataIn = dataIn << 1;
      } else if (duration > rxOneMin && duration < rxOneMax) {
        dataIn = (dataIn << 1) + 1;
      } else {
        DONE_RX;
        bitnum = 0; // reset to bit zero
        memset((void*)rxBuffer, 0, 32); //clear buffer - memset on rxBuffer is safe because interrupts disabled in ISR
        return;
      }
      if ((bitnum & 7) == 7) {
        rxBuffer[bitnum >> 3] = dataIn;
        if (bitnum == 7) {
          byte t = dataIn >> 6;
          pktLength = t ? (t == 1 ? 63 : (t == 2 ? 127 : 255)) : 31;
        }
        dataIn = 0;
      }
      if (bitnum >= pktLength) {
        bitnum = 0;
        DONE_RX;
        SET_MESSAGE;
        TCB3.INTCTRL = 0; //turn off input capture;
      } else {
        bitnum++;
      }
    }
  }
}

/* Doesn't work
  void updatePatternComets() {
  static byte nextCometIn = 0;
  memset(pixels, 0, 600);
  for (byte i = 0; (i + 1) * 20 <= (LENGTH); i++) {
    if (i == 0) {
      Serial.println(scratch[i * 60]);
    }
    if (scratch[i * 60]) {

      if (!frameNumber % (scratch[i * 60 + 3] + 1)) {
        scratch[i * 60 + 1] += 1;
      }
      if ((scratch[i * 60 + 1] + scratch[i * 60 + 2]) > LENGTH) {
        removeComet(i);
      } else {
        for (byte j = 0; (j < (scratch[i * 60 + 2] * 3)); j++) {
          if (j <= (scratch[i * 60 + 1] * 3)) {
            pixels[(scratch[i * 60 + 1] * 3) - j] = scratch[i * 60 + 12 + j];
          }
        }

      }
    }
  }
  if (nextCometIn == 0) {
    nextCometIn = random(30 - 2 * currentValueRight[2], 200 - 10 * currentValueRight[2]);
  } else {

    nextCometIn--;
  }
  }

  //comet data:
  // last 48 bytes are the comet
  // Byte 0: active - 0 = inactive 1 = active
  // byte 1: starts at position
  // byte 2: length
  // byte 3: speed


  byte createComet() {
  for (byte i = 0; (i + 1) * 20 <= (LENGTH); i++) {
    if (scratch[i * 60] == 0) {
      unsigned int index = i * 60;
      scratch[index] = 1;
      scratch[index + 1] = 0; //start at position 0.
      scratch[index + 2] = random(3, currentValueRight[1] + 6);
      scratch[index + 3] = random(0, 3);
      unsigned int l = 3 * scratch[index + 2];
      byte r = random(currentValueLeft[0], currentValueLeft[1]);
      byte g = random(currentValueLeft[2], currentValueLeft[3]);
      byte b = random(currentValueLeft[4], currentValueLeft[5]);
      for (byte j = 0; j < l; j += 3) {
        if (!j || random(0, 1)) {
          scratch[i * 60 + 12 + j] = r;
          scratch[i * 60 + 13 + j] = g;
          scratch[i * 60 + 14 + j] = b;
        } else {
          scratch[i * 60 + 12 + j] = 0;
          scratch[i * 60 + 13 + j] = 0;
          scratch[i * 60 + 14 + j] = 0;
        }
      }
      return 1;
    }
  }
  return 0;
  }
  void removeComet(byte index) {
  if ((index + 1) * 20 <= (LENGTH)) {
    for (byte i = 0; i < 60; i++) {
      scratch[index * 60 + i] = 0;
    }
  }
  }
*/
