


void handleUI() {
  static byte lastBtnState = 7;
  static byte lastBtnBounceState = 7;
  static unsigned long lastBtnAt = 0;
  static unsigned long lastPressAt = 0;
  byte btnRead = BTN_PORT_READ();
  if (!(btnRead == lastBtnBounceState)) { //debounce all buttons at once.
    lastBtnBounceState = btnRead;
    lastBtnAt = millis();
  } else {
    if (millis() - lastBtnAt > 50) { //has been stable for 50ms
      if (btnRead < lastBtnState ) {
        if (!lastPressAt && !(btnRead & 1)) {
          lastPressAt = millis();
        }
        //do nothing - was button being pressed
      } else {
        if (((btnRead & MODE_BTN_bm)) && !(lastBtnState & MODE_BTN_bm)) {
          if (lastPressAt && millis() - lastPressAt > 10000) {

            lcd.clear();
            clearMode();
            lcd.setCursor(3, 0);
            lcd.print(F("Saved mode"));
            lcd.setCursor(5, 1);
            lcd.print(F("cleared"));
            delay(1000);
            UIChanged = 7;
          } else if (lastPressAt && millis() - lastPressAt > 3000) {
            saveMode();
            lcd.clear();
            lcd.setCursor(3, 0);
            lcd.print(F("Mode Saved"));
            delay(1000);
            UIChanged = 7;
          } else {
            advanceMode();
            UIChanged |= 4;
          }
          lastPressAt = 0;
        }
        if (((btnRead & ENC1_BTN_bm)) && !(lastBtnState & ENC1_BTN_bm)) {
          if (currentSettingLeft >= pgm_read_byte_near(&maxSetting[currentMode][0])) {
            currentSettingLeft = 0;
          } else {
            currentSettingLeft++;
          }
          UIChanged |= 2;
        }
        if (((btnRead & ENC2_BTN_bm)) && !(lastBtnState & ENC2_BTN_bm)) {
          if (currentSettingRight >= pgm_read_byte_near(&maxSetting[currentMode][1])) {
            currentSettingRight = 0;
          } else {
            currentSettingRight++;
          }
          UIChanged |= 2;
        }
      }
      lastBtnState = btnRead;
    }
  }
}

byte getLeftVal(byte t) {
  if (pgm_read_byte_near(&maxValueLeft[currentMode][currentSettingLeft]) == COLORTABLEMAX) {
    if (t > COLORTABLEMAX) t = COLORTABLEMAX;
    return pgm_read_byte_near(&leftValues[t]);
  }
  return t;
}

void handleLCD() {
  static byte drift2_colors = 255;
  static unsigned long lastInputAt;
  static byte attractmode = 0;
  byte uichg = 0;
  if (millis() - lastRFUpdateAt < 5000 && lastRFUpdateAt) {
    return;
  }
  cli();
  uichg = UIChanged;
  UIChanged = 0;
  sei();
  if (uichg == 0) {
    if (millis() - lastInputAt > 60000) {
      if (!attractmode || (millis() - lastInputAt > 120000)) {
        attractmode = 1;
        lastInputAt = millis() - 60000;
        doAttractLCD();
      }
    }
    return;
  }
  if (attractmode) {
    lcd.clear();
  }
  if (uichg & 1 && currentMode == 10) {
    if (getPalleteNumber() != drift2_colors) {
      initColorsDrift2();
    }
    initLookupDrift2();
  }
  if (currentMode == 10) {
    if (getPalleteNumber() != drift2_colors) {
      drift2_colors = getPalleteNumber();
    }
  }
  lastInputAt = millis();
  if ((uichg & 6) || attractmode ) { //if setting or mode has changed, redraw settings
    lcd.setCursor(0, 0);
    if (currentMode < 7 ) {
      lcd.print(FLASH(modesL[currentMode][currentSettingLeft]));
    } else {
      lcd.print(CurrentColors->colorname);
    }
    lcd.print(' ');
    lcd.print(FLASH(modesR[currentMode][currentSettingRight]));
  }
  if ((uichg & 7) || attractmode) { //if mode, setting, or value has changed, redraw second line
    byte tval;
    if (currentMode > 6) {
      lcd.setCursor(0, 0);
      lcd.print(CurrentColors->colorname);
      lcd.setCursor(0, 1);
      lcd.print(F("    "));
    } else if (pgm_read_byte_near(&maxSetting[currentMode][0]) != 255) {
      lcd.setCursor(0, 1);
      tval = getLeftVal(currentValueLeft[currentSettingLeft]);
      lcd.print(tval);
      lcd.print(' ');
      if (tval < 10) lcd.print(' ');
    } else {
      lcd.setCursor(0, 1);
      lcd.print(F("    "));
    }
    lcd.setCursor(4, 1);
    lcd.print(FLASH(modeNames[currentMode]));
    lcd.setCursor(13, 1);
    if (pgm_read_byte_near(&maxValueRight[currentMode][currentSettingRight]) == 1) { //if max is 1, that means it's forward/reverse
      if (currentValueRight[currentSettingRight]) {
        lcd.print(F("REV"));
      } else {
        lcd.print(F("FWD"));
      }
    } else {
      tval = currentValueRight[currentSettingRight];
      if (tval < 100) lcd.print(' ');
      if (tval < 10) lcd.print(' ');
      if (pgm_read_byte_near(&maxValueRight[currentMode][currentSettingRight])) { //if max is 0, then this is blank
        lcd.print(tval);
      } else {
        lcd.print(' ');
      }
    }
  }
  attractmode = 0;
}
void doAttractLCD() {
  lcd.clear();
  byte s = random(0, 3);
  if (!s) {
    lcd.setCursor(0, 0);
    lcd.print(F("Light Test v2.1"));
    lcd.setCursor(0, 1);
    lcd.print(F("AVR128DB48"));
  } else if (s == 1) {
    lcd.setCursor(0, 0);
    lcd.print(F("This is not a"));
    lcd.setCursor(0, 1);
    lcd.print(F("real party"));
    lcd.write(0x7F);
  } else {
    lcd.setCursor(2, 0);
    lcd.print(F("PLAY WITH ME"));
    lcd.setCursor(0, 1);
    byte r = random(0, 2);
    if (r == 0) {
      lcd.print(F("USE KNOBS&BUTTON"));
    } else if (r == 1) {
      lcd.print(F("TURN MY KNOBS ;)"));
    } else {
      lcd.print(F("ADJUST LIGHTING"));
    }
  }
}

const int8_t enc_states [] PROGMEM_MAPPED = {0, -1,  1, 0,  1, 0, 0, -1, -1, 0, 0,  1, 0,  1, -1, 0}; //encoder lookup table
const int8_t enc_states_rev [] PROGMEM_MAPPED = {0, 1, -1, 0, -1, 0, 0,  1, 1, 0, 0, -1, 0, -1,  1, 0}; // reversed encoder table

ISR(PORTC_PORT_vect) {
  uint8_t enc_pinstate = ENC_PORT_READ();
  VPORTC.INTFLAGS = VPORTC.INTFLAGS;
  static uint8_t EncL_Prev = 3;  //lookup table index
  static int8_t EncL_Val = 0;   //encoder value
  static uint8_t EncR_Prev = 3;  //lookup table index
  static int8_t EncR_Val = 0;   //encoder value
  EncL_Prev <<= 2; //remember previous state
  EncR_Prev <<= 2; //remember previous state

  EncL_Prev |= (enc_pinstate & 0x0C) >> 2;
  EncR_Prev |= (enc_pinstate & 0x03);

  EncL_Val += enc_states[(EncL_Prev & 0x0F)];
  EncR_Val += enc_states[(EncR_Prev & 0x0F)];

  /* post "Navigation forward/reverse" event */
  if ( EncL_Val > 3 ) { //four steps forward
    if (currentValueLeft[currentSettingLeft] < (maxValueLeft[currentMode][currentSettingLeft])) currentValueLeft[currentSettingLeft]++;
    //hackjob to handle min exceeding max or vice versa.
    if ((currentMode == 1 || currentMode == 2 || currentMode == 3 || currentMode == 4 || currentMode == 5) && currentSettingLeft < 6) {
      if (!(currentSettingLeft & 1)) {
        if (currentValueLeft[currentSettingLeft] > currentValueLeft[currentSettingLeft + 1]) {
          currentValueLeft[currentSettingLeft + 1] = currentValueLeft[currentSettingLeft];
        }
      }
    }
    UIChanged |= 1;
    EncL_Val = 0;
  }
  else if ( EncL_Val < -3 ) { //four steps backwards
    if (currentValueLeft[currentSettingLeft])currentValueLeft[currentSettingLeft]--;
    //hackjob to handle min exceeding max or vice versa.
    if ((currentMode == 1 || currentMode == 2) && currentSettingLeft < 6) {
      if (currentSettingLeft & 1) {
        if (currentValueLeft[currentSettingLeft] < currentValueLeft[currentSettingLeft - 1]) {
          currentValueLeft[currentSettingLeft - 1] = currentValueLeft[currentSettingLeft];
        }
      }
    }
    UIChanged |= 1;
    EncL_Val = 0;
  }
  if ( EncR_Val > 3 ) { //four steps forward
    if (currentValueRight[currentSettingRight] < pgm_read_byte_near(&maxValueRight[currentMode][currentSettingRight])) {
      currentValueRight[currentSettingRight]++;
    }
    UIChanged |= 1;
    EncR_Val = 0;
  }
  else if ( EncR_Val < -3 ) { //four steps backwards
    if (currentValueRight[currentSettingRight]) currentValueRight[currentSettingRight]--;
    UIChanged |= 1;
    EncR_Val = 0;
  }
}
