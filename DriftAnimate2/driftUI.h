#include <avr/pgmspace.h>

#define FLASH(flashptr) (reinterpret_cast<const __FlashStringHelper *>(pgm_read_word_near(&flashptr)))

void printWelcome() {
  lcd.print(F("Hello Cabin!!"));
  lcd.setCursor(0, 1);
  lcd.print(F("Lets party down!"));
}

void printOverride() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("REMOTE OVERRIDE"));
}

void printMaxCurrent() {
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print(F("Maximum power"));
  lcd.setCursor(3, 1);
  lcd.print(F("exceeded"));
}

void doAttractLCD() {
  lcd.clear();
  byte s = random(0, 3);
  if (!s) {
    lcd.setCursor(0, 0);
    lcd.print(F("TRICK OR TREAT!"));
    lcd.setCursor(0, 1);
    lcd.print(F("Let's get weird!"));
  } else if (s==1) {
    lcd.setCursor(0, 0);
    lcd.print(F("Cabin Weekend VI"));
    lcd.setCursor(0, 1);
    lcd.print(F(" ~HALLOWEEKEND"));
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
      lcd.print(FLASH(palleteNames[currentValueLeft[0]]));
    }
    lcd.print(' ');
    lcd.print(FLASH(modesR[currentMode][currentSettingRight]));
  }
  if ((uichg & 7) || attractmode) { //if mode, setting, or value has changed, redraw second line
    byte tval;
    if (currentMode > 6) {
      lcd.setCursor(0, 0);
      lcd.print(FLASH(palleteNames[currentValueLeft[0]]));
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

void handleUI() {
  static byte lastBtnState = 7;
  static byte lastBtnBounceState = 7;
  static unsigned long lastBtnAt = 0;
  static unsigned long lastPressAt = 0;
  byte btnRead = (PIND & 0x1C) >> 2;
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
        if (((btnRead & 1)) && !(lastBtnState & 1)) {
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
        #ifdef __AVR_ATmega328P__
        if (((btnRead & 2)) && !(lastBtnState & 2)) { //Rev - boards based on pro minis
        #else
        if (((btnRead & 4)) && !(lastBtnState & 4)) { //Rev B 328pb/1284p based boards
        #endif
          if (currentSettingLeft >= pgm_read_byte_near(&maxSetting[currentMode][0])) {
            currentSettingLeft = 0;
          } else {
            currentSettingLeft++;
          }
          UIChanged |= 2;
        }
        #ifdef __AVR_ATmega328P__
        if (((btnRead & 4)) && !(lastBtnState & 4)) { //Rev - boards based on pro minis
        #else
        if (((btnRead & 2)) && !(lastBtnState & 2)) { //Rev B 328pb/1284p based boards
        #endif
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

// ISR based on: https://www.circuitsathome.com/mcu/rotary-encoder-interrupt-service-routine-for-avr-micros/
// by Oleg Mazurov
#ifndef __AVR_ATmega1284P__
ISR(PCINT1_vect)
#else
ISR(PCINT2_vect)
#endif
{
  static uint8_t old_ABl = 3;  //lookup table index
  static int8_t enclval = 0;   //encoder value
  static uint8_t old_ABr = 3;  //lookup table index
  static int8_t encrval = 0;   //encoder value
  //static const int8_t enc_states [] PROGMEM = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};  //encoder lookup table
  static const int8_t enc_states [] PROGMEM = {0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0}; // reversed encoder table
  old_ABl <<= 2; //remember previous state
  old_ABr <<= 2; //remember previous state
  #ifdef __AVR_ATmega328P__
  old_ABl |= ( PINC & 0x03 );
  old_ABr |= (( PINC & 0x0C ) >> 2);
  #else
  old_ABr |= ( PINC & 0x03 );
  old_ABl |= (( PINC & 0x0C ) >> 2);
  #endif
  enclval += pgm_read_byte(&(enc_states[( old_ABl & 0x0f )]));
  encrval += pgm_read_byte(&(enc_states[( old_ABr & 0x0f )]));
  /* post "Navigation forward/reverse" event */
  if ( enclval > 3 ) { //four steps forward
    if (currentValueLeft[currentSettingLeft] < pgm_read_byte_near(&maxValueLeft[currentMode][currentSettingLeft]))currentValueLeft[currentSettingLeft]++;
    //hackjob to handle min exceeding max or vice versa.
    if ((currentMode == 1 || currentMode == 2 || currentMode == 3 || currentMode == 4 || currentMode == 5) && currentSettingLeft < 6) {
      if (!(currentSettingLeft & 1)) {
        if (currentValueLeft[currentSettingLeft] > currentValueLeft[currentSettingLeft + 1]) {
          currentValueLeft[currentSettingLeft + 1] = currentValueLeft[currentSettingLeft];
        }
      }
    }
    UIChanged |= 1;
    enclval = 0;
  }
  else if ( enclval < -3 ) { //four steps backwards
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
    enclval = 0;
  }
  if ( encrval > 3 ) { //four steps forward
    if (currentValueRight[currentSettingRight] < pgm_read_byte_near(&maxValueRight[currentMode][currentSettingRight]))currentValueRight[currentSettingRight]++;
    UIChanged |= 1;
    encrval = 0;
  }
  else if ( encrval < -3 ) { //four steps backwards
    if (currentValueRight[currentSettingRight])currentValueRight[currentSettingRight]--;
    UIChanged |= 1;
    encrval = 0;
  }
}

void setupRF() {
  TCCR1A = 0;
  TCCR1B = 0;
  TIFR1 = (1<<ICF1) | (1<<TOV1);  // clear flags so we don't get a bogus interrupt
  TCNT1 = 0;          // Counter to zero
  TIMSK1 = 1 << ICIE1; // interrupt on Timer 1 input capture
  // start Timer 1, prescalar of 8, edge select on falling edge
  TCCR1B =  ((F_CPU == 1000000L) ? (1 << CS10) : (1 << CS11)) | 1 << ICNC1; //prescalar 8 except at 1mhz, where we use prescalar of 1, noise cancler active
  //ready to rock and roll
}
