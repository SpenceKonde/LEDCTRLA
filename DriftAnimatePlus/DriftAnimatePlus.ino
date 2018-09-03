#include <Adafruit_NeoPixel_Static.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <avr/pgmspace.h>
#include <util/crc16.h>
#include <EEPROM.h>

// UI + encoder involved globals
//LiquidCrystal_I2C lcd(0x3F, 16, 2);
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define ENC1_PINA 14
#define ENC1_PINB 15
#define ENC2_PINA 16
#define ENC2_PINB 17
#define ENC1_BTN 3
#define ENC2_BTN 4
#define MODE_BTN 2
#define LDR_PIN A6


#define MODE_DRIFT2 9

#define FLASH(flashptr) (reinterpret_cast<const __FlashStringHelper *>(pgm_read_word_near(&flashptr)))

// Names of mode settings - these get stuffed into modesL and modesR below.
const char mode0L0[] PROGMEM = "  RED  ";
const char mode0L1[] PROGMEM = " GREEN ";
const char mode0L2[] PROGMEM = "  BLUE ";
const char mode1L0[] PROGMEM = "MIN RED";
const char mode1L1[] PROGMEM = "MAX RED";
const char mode1L2[] PROGMEM = "MIN GRN";
const char mode1L3[] PROGMEM = "MAX GRN";
const char mode1L4[] PROGMEM = "MIN BLU";
const char mode1L5[] PROGMEM = "MAX BLU";
const char mode0R0[] PROGMEM = "       ";
const char mode1R0[] PROGMEM = " SPEED ";
const char mode1R1[] PROGMEM = " NUMBER";
const char mode2R2[] PROGMEM = " LENGTH";
const char mode4R2[] PROGMEM = " DIRECT";
const char mode5R1[] PROGMEM = "DENSITY";
const char mode7R2[] PROGMEM = " DWELL ";
const char mode0Name[] PROGMEM = " SOLID  ";
const char mode1Name[] PROGMEM = "DRIFTING";
const char mode2Name[] PROGMEM = " COMETS ";
const char mode3Name[] PROGMEM = " PULSE  ";
const char mode4Name[] PROGMEM = " RAINBOW";
const char mode5Name[] PROGMEM = "  DOTS  ";
const char mode6Name[] PROGMEM = "  FADE  ";
const char mode7Name[] PROGMEM = "  WAVE  ";
const char mode8Name[] PROGMEM = " DOTS2  ";
const char mode9Name[] PROGMEM = " FADE2  ";
const char mode10Name[] PROGMEM = " DRIFT2 ";


//Names of settings by mode
const char * const modesL[][8] PROGMEM = {
  {mode0L0, mode0L1, mode0L2},
  {mode1L0, mode1L1, mode1L2, mode1L3, mode1L4, mode1L5},
  {mode1L0, mode1L1, mode1L2, mode1L3, mode1L4, mode1L5},
  {mode1L0, mode1L1, mode1L2, mode1L3, mode1L4, mode1L5},
  {mode0R0}, //rainbow
  {mode1L0, mode1L1, mode1L2, mode1L3, mode1L4, mode1L5},
  {mode1L0, mode1L2, mode1L4, mode1L1, mode1L3, mode1L5}, //different order!
  {mode0R0}, //wave
  {mode0R0}, //dots with pallete
  {mode0R0}, //fade with pallete
  {mode0R0} //drift with pallete

};

const char * const modesR[][8] PROGMEM = {
  {mode0R0}, //solid
  {mode1R0, mode1R1}, //drift
  {mode1R0, mode1R1, mode2R2}, //comet
  {mode1R0, mode5R1}, //pulse
  {mode1R0, mode2R2, mode4R2}, //rainbow
  {mode1R0, mode5R1, mode4R2}, //dots
  {mode1R0}, //fade
  {mode1R0, mode2R2, mode7R2, mode4R2}, //wave
  {mode1R0, mode5R1, mode4R2}, //dots with pallete
  {mode1R0, mode2R2, mode7R2, mode4R2}, //fade with pallete
  {mode1R0, mode2R2, mode7R2} //drift with pallete

};

// names of modes
const char * const modeNames[] PROGMEM = {mode0Name, mode1Name, mode2Name, mode3Name, mode4Name, mode5Name, mode6Name, mode7Name, mode8Name, mode9Name, mode10Name};

#define COLORTABLEMAX 31

//max and default settings controlled by left knob. 26 is special, it indicates to use the leftValues array
const byte maxValueLeft[][8] PROGMEM = {
  {COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX},
  {COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX},
  {COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX},
  {COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX},
  {0},
  {COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX},
  {COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX},
  {3},
  {3},
  {3},
  {3}
};
const byte defaultValueLeft[][8] PROGMEM = { //255 is special - indicates to pick random value.
  {255, 255, 255},
  {0, COLORTABLEMAX, 0, COLORTABLEMAX, 0, COLORTABLEMAX},
  {0, COLORTABLEMAX, 0, COLORTABLEMAX, 0, COLORTABLEMAX},
  {0, COLORTABLEMAX, 0, COLORTABLEMAX, 0, COLORTABLEMAX},
  {0},
  {0, COLORTABLEMAX, 0, COLORTABLEMAX, 0, COLORTABLEMAX},
  {255, 255, 255, 255, 255, 255},
  {0},
  {0},
  {0},
  {0}
};

//if above max is COLORTABLEMAX, use this value - otherwise use raw value.
const byte leftValues[COLORTABLEMAX + 1] PROGMEM = {0, 1, 2, 3, 4, 6, 8, 11, 14, 18, 22, 27, 33, 39, 46, 54, 63, 73, 84, 95, 106, 117, 128, 139, 151, 163, 176, 189, 204, 220, 237, 255};

const byte maxValueRight[][8] PROGMEM = {
  {0},
  {10},
  {10, 10, 10},
  {10, 20},
  {10, 10, 1},
  {10, 12, 1},
  {10},
  {10, 20, 20, 1},
  {10, 12, 1},
  {10, 20, 20, 1},
  {10, 20, 20}
};
const byte defaultValueRight[][8] PROGMEM = {
  {0},
  {5},
  {10, 10, 5},
  {5, 5},
  {5, 10, 0},
  {5, 10, 0},
  {5},
  {5, 10, 2, 0},
  {5, 10, 0},
  {5, 10, 2,0},
  {5, 10, 2}
};
const byte maxSetting[][2] PROGMEM = {
  {2, 0}, //solid
  {5, 0}, //drift
  {5, 2}, //comets
  {5, 1}, //pulse
  {255, 2}, //rainbow
  {5, 2}, //dots
  {5, 0}, //fade
  {0, 3}, //wave
  {0, 2}, //dots2
  {0, 2}, //fade2
  {0, 2} //drift2
};

const byte maxMode = 10;

const byte pulseBrightnessTable[] PROGMEM = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 31, 34, 37, 40, 43, 46, 49, 52, 55, 59, 63, 67, 71, 75, 79, 83, 87, 92, 97, 102, 107, 112, 117, 122, 127, 133, 139, 145, 151, 157, 163, 169, 175, 182, 189, 196, 203, 210, 217, 224, 231, 239, 247, 255};

volatile byte lastEncPins = 0;
volatile byte currentSettingLeft = 0;
volatile byte currentSettingRight = 0;
volatile byte currentValueLeft[] = {0, 0, 0, 0, 0, 0, 0, 0};
volatile byte currentValueRight[] = {0, 0, 0, 0, 0, 0, 0, 0};
volatile byte UIChanged = 7;
volatile unsigned long lastRFUpdateAt = 0;

byte currentMode = 0;

const byte colorPallete[][8][3] PROGMEM = {
  {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
  {{255, 196, 64}, {255, 100, 40}, {196, 160, 140}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
  {{255, 0, 64}, {0, 64, 255}, {0, 160, 160}, {160, 0, 160}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
  {{255, 0, 0}, {128, 128, 128}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
};
const byte colorCount[] PROGMEM = {3, 3, 4, 3};

const char pallete0[] PROGMEM = "RAINBOW";
const char pallete1[] PROGMEM = "  WARM ";
const char pallete2[] PROGMEM = " SPOOKY";
const char pallete3[] PROGMEM = "  USA  ";

const char * const palleteNames[] PROGMEM = {pallete0, pallete1, pallete2, pallete3};


volatile unsigned long lastUserAction = 0;

//animation related globals
#define LENGTH 200
#define LEDPIN 10
unsigned int frameDelay = 30;
unsigned long lastFrameAt;
byte pixels[LENGTH * 3];
byte scratch[LENGTH * 3];
unsigned long frameNumber = 0;
Adafruit_NeoPixel leds = Adafruit_NeoPixel(LENGTH, LEDPIN, NEO_GRB + NEO_KHZ800, pixels);

//RF related globals
const byte MyAddress = 0;
volatile byte receiving = 0;
volatile byte bitnum = 0; //current bit received

volatile byte gotMessage = 0;
volatile byte dataIn = 0;
volatile byte pktLength = 31;
volatile unsigned long lastRFMsgAt = 0;
volatile byte rxBuffer[32];
byte recvMessage[32];
#define RX_PIN_STATE (PINB&1) //RX on pin 8 for input capture. 

unsigned long lastPacketTime = 0;
unsigned long lastPacketSig = 0;

// Version 2.2/2.3
#if(F_CPU==8000000)
#define TIME_MULT * 1
#elif(F_CPU==16000000)
#define TIME_MULT * 2
#endif

const unsigned int rxSyncMin  = 1750 TIME_MULT;
const unsigned int rxSyncMax  = 2250 TIME_MULT;
const unsigned int rxZeroMin  = 100 TIME_MULT;
const unsigned int rxZeroMax  = 390 TIME_MULT;
const unsigned int rxOneMin  = 410 TIME_MULT;
const unsigned int rxOneMax  = 700 TIME_MULT;
const unsigned int rxLowMax  = 600 TIME_MULT;
const int commandForgetTime = 5000;

void setup() {
  Wire.begin();
  lcd.begin();
  setupPins();
  setupPCINT();
  setupRF();
  Serial.begin(115200);
  lcd.backlight();
  lcd.print(F(" Hello - Let's"));
  lcd.setCursor(2, 1);
  lcd.print(F("party down!"));
  delay(2000);
  lcd.clear();
  loadMode();
}

void loop() {
  byte rlen = handleReceive();
  if (rlen) {
    processRFPacket(rlen);
  } else if ((!receiving)  && (millis() - lastRFMsgAt > 100)) { //will be if not receiving, but we don't know where we're saving the RXing status.
    handleUI();
    handleLCD();
    if (millis() - lastFrameAt > getFrameDelay()) {
      lastFrameAt = millis();
      updatePattern();
      leds.show();
    }
  }
}

byte getFrameDelay() {
  if (currentMode == 1 || currentMode == 0) {
    return 80;
  } else if (currentMode == 3) {
    return 30 + 10 * (pgm_read_byte_near(&maxValueRight[currentMode][0]) - currentValueRight[0]);
  }
  return 30 + 20 * (pgm_read_byte_near(&maxValueRight[currentMode][0]) - currentValueRight[0]);

}

void processRFPacket(byte rlen) {

  byte vers = (rlen & 196) >> 6;
  rlen &= 0x3F;
  if (recvMessage[1] == 0x54) {
    if (recvMessage[2] > maxMode) {
      return;
    }
    setMode(recvMessage[2]);
    currentValueLeft[0] = recvMessage[3];
    currentValueLeft[1] = recvMessage[4];
    currentValueLeft[2] = recvMessage[5];
    currentValueLeft[3] = recvMessage[6];
    currentValueLeft[4] = recvMessage[7];
    currentValueLeft[5] = recvMessage[8];
    currentValueRight[0] = recvMessage[9];
    currentValueRight[1] = recvMessage[10];
    currentValueRight[2] = recvMessage[11];
    currentValueRight[3] = recvMessage[12];
    currentValueRight[4] = recvMessage[13];
    currentValueRight[5] = recvMessage[14];
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("REMOTE OVERRIDE"));
    UIChanged = 7;
  }
}


void advanceMode() {
  if (currentMode >= maxMode) {
    setMode(0);
  } else {
    if (currentMode == 1) currentMode++;
    setMode(currentMode + 1);
  }
}

void setMode(byte mode) {
  currentMode = mode;
  memset(scratch, 0, 600);
  memset(pixels, 0, 600);
  for (byte i = 0; i < 8; i++) { //set the current setting values to defaults
    if (pgm_read_byte_near(&defaultValueLeft[currentMode][i]) == 255) {
      currentValueLeft[i] = random(pgm_read_byte_near(&maxValueLeft[currentMode][i]));
    } else {
      currentValueLeft[i] = pgm_read_byte_near(&defaultValueLeft[currentMode][i]);
    }
    if (pgm_read_byte_near(&defaultValueLeft[currentMode][i]) == 255) {
      currentValueRight[i] = random(pgm_read_byte_near(&maxValueRight[currentMode][i]));
    } else {
      currentValueRight[i] = pgm_read_byte_near(&defaultValueRight[currentMode][i]);
    }
  }
  if (currentMode == 10) {
    initLookupDrift2();
    initColorsDrift2();
  }
  // start with the first setting selected, in case we had a setting now out of index.
  frameNumber = 0;
  currentSettingLeft = 0;
  currentSettingRight = 0;
}

void initColorsDrift2() {
  int len = (pgm_read_byte_near(&colorCount[getPalleteNumber()]) * (getDwellFrames() + getTransitionFrames()));
  for (int j = 0; j < LENGTH; j++) {
    unsigned int r = random(0, len);
    scratch[((j * 3) / 2)] = r & 0xFF;
    if (j & 1) {
      scratch[(j / 2) * 3 + 2] &= ((r >> 4) & 0xF0);
    } else {
      scratch[(j / 2) * 3 + 2] = r >> 8;
    }
  }
}

void initLookupDrift2() {
  unsigned int start = (LENGTH * 3 + 1) / 2;
  for (unsigned int i = 0; i < getTransitionFrames(); i++) {
    scratch[start + i] = getModeRatio(i);
  }
}


void saveMode() {
  EEPROM.write(0x0F, currentMode);
  for (byte i = 0; i < 8; i++) {
    EEPROM.write(0x10 + i, currentValueLeft[i]);
    EEPROM.write(0x18 + i, currentValueRight[i]);
  }

}

void clearMode() {
  for (byte i = 0x0F; i < 0x20; i++) {
    EEPROM.write(i, 255);
  }
  currentMode = 0;
  for (byte i = 0; i < 8; i++) {
    currentValueLeft[i] = 0;
    currentValueRight[i] = 0;
  }

}

void loadMode() {
  if (EEPROM.read(0x0F) != 255) {
    currentMode = EEPROM.read(0x0F);
    for (byte i = 0; i < 8; i++) {
      currentValueLeft[i] = EEPROM.read(0x10 + i);
      currentValueRight[i] = EEPROM.read(0x18 + i);
    }
  }
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
      if (btnRead < lastBtnState) {
        if (!lastPressAt) {
          lastPressAt = millis();
        }
        //do nothing - was button being pressed
      } else {
        if (((btnRead & 1)) && !(lastBtnState & 1)) {
          Serial.println(millis() - lastPressAt);
          if (millis() - lastPressAt > 10000) {
            lcd.clear();
            clearMode();
            lcd.setCursor(3, 0);
            lcd.print(F("Saved mode"));
            lcd.setCursor(5, 1);
            lcd.print(F("cleared"));
            delay(1000);
            UIChanged |= 4;
          } else if (millis() - lastPressAt > 3000) {
            saveMode();
            lcd.clear();
            lcd.setCursor(3, 0);
            lcd.print(F("Mode Saved"));
            delay(1000);
          } else {
            advanceMode();
            UIChanged |= 4;
          }
          lastPressAt = 0;
        }
        if (((btnRead & 2)) && !(lastBtnState & 2)) {
          if (currentSettingLeft >= pgm_read_byte_near(&maxSetting[currentMode][0])) {
            currentSettingLeft = 0;
          } else {
            currentSettingLeft++;
          }
          UIChanged |= 2;
        }
        if (((btnRead & 4)) && !(lastBtnState & 4)) {
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
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print(F("PLAY WITH ME"));
        lcd.setCursor(0, 1);
        byte r = random(0, 3);
        if (r == 0) {
          lcd.print(F("USE KNOBS&BUTTON"));
        } else if (r == 1) {
          lcd.print(F("TURN MY KNOBS ;)"));
        } else if (r == 2) {
          lcd.print(F("ADJUST LIGHTING"));
        } else {
          lcd.print(F(" LIGHTS ARE FUN!"));
        }
      }
    }
    return;
  }
  if (attractmode) {
    lcd.clear();
  }
  if (uichg & 1 && currentMode == 10) {
    if (getPalleteNumber != drift2_colors) {
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


void updatePattern() {
  if (currentMode == 0) {
    for (unsigned int i = 0; i < LENGTH * 3; i++) {
      pixels[i] = getLeftVal(currentValueLeft[i % 3]);
    }
  } else if (currentMode == 1) {
    updatePatternDrift();
    //} else if (currentMode == 2) {
    //updatePatternComets();
  } else if (currentMode == 3) {
    updatePatternPulse();
  } else if (currentMode == 4) {
    updatePatternWave(); //rainbow is just a subset of wave
  } else if (currentMode == 5) {
    updatePatternDots();
  } else if (currentMode == 6) {
    updatePatternFade();
    
  } else if (currentMode == 7) {
   updatePatternWave();
  } else if (currentMode == 8) {
    updatePatternDots2();
  } else if (currentMode == 9) {
    updatePatternFade2();
  } else if (currentMode == 10) {
    updatePatternDrift2();
  } else {
    setMode(0);
  }
  frameNumber++;
}

void updatePatternDots() {
  byte r, g, b;
  if (!(frameNumber % (13 - currentValueRight[1]))) {
    r = random(getLeftVal(currentValueLeft[0]), getLeftVal(currentValueLeft[1]));
    g = random(getLeftVal(currentValueLeft[2]), getLeftVal(currentValueLeft[3]));
    b = random(getLeftVal(currentValueLeft[4]), getLeftVal(currentValueLeft[5]));
  } else {
    r = 0; g = 0; b = 0;
  }
  pushPixel(r, g, b, currentValueRight[2]);
}

void updatePatternDots2() {
  static byte r, g, b;
  if (!(frameNumber % (13 - currentValueRight[1]))) {
    getModeColors(&r, &g, &b, random(0, pgm_read_byte_near(&colorCount[getPalleteNumber()]) * (getTransitionFrames() + getDwellFrames())));
  } else {
    r = 0; g = 0; b = 0;
  }
  pushPixel(r, g, b, currentValueRight[2]);
}

void updatePatternFade() {
  static byte bright = 0;
  if (bright & 128) {
    if (bright & 63) {
      bright--;
    } else {
      bright = 0;
    }
  } else {
    if (bright >= 63) {
      bright = 0xBF;
    } else {
      bright++;
    }
  }
  byte nbright = pgm_read_byte_near(&pulseBrightnessTable[63 & bright]);
  byte r = map(nbright, 0, 255, getLeftVal(currentValueLeft[0]), getLeftVal(currentValueLeft[3]));
  byte g = map(nbright, 0, 255, getLeftVal(currentValueLeft[1]), getLeftVal(currentValueLeft[4]));
  byte b = map(nbright, 0, 255, getLeftVal(currentValueLeft[2]), getLeftVal(currentValueLeft[5]));
  setAll(r, g, b);
}

void updatePatternFade2() {
  byte r, g, b;
  getModeColors(&r, &g, &b, frameNumber);
  setAll(r, g, b);
}


void updatePatternWave() {
  byte r, g, b;
  getModeColors(&r, &g, &b, frameNumber);
  pushPixel(r, g, b, currentValueRight[currentMode == 4 ? 2 : 3]);
}


void setAll(byte r, byte g, byte b) {
  for (unsigned int i = 0; i < ((LENGTH) * 3); i += 3) {
    pixels [i] = r;
    pixels[i + 1] = g;
    pixels[i + 2] = b;
  }
}

void pushPixel(byte r, byte g, byte b, byte dir) {
  if (dir) { //reverse
    for (unsigned int i = 0; i < ((LENGTH - 1) * 3); i++) {
      pixels [i] = pixels[i + 3];
    }
    pixels[(LENGTH * 3) - 3] = r;
    pixels[(LENGTH * 3) - 2] = g;
    pixels[(LENGTH * 3) - 1] = b;
  } else {//forward
    for (unsigned int i = ((LENGTH) * 3) - 1; i > 2; i--) {
      pixels [i] = pixels[i - 3];
    }
    pixels[0] = r;
    pixels[1] = g;
    pixels[2] = b;
  }
}

unsigned int getDwellFrames() {
  if (currentMode == 4) {
    return 2;
  } else {
    return 2 + (4 * currentValueRight[2]);
  }
  return 0; //TO DO
}
unsigned int getTransitionFrames() {
  if (currentMode == 4) {
    return 9 + (6 * currentValueRight[1]);
  } else {
    return 6 * currentValueRight[1];
  }
}
byte getPalleteNumber() {
  if (currentMode == 4) {
    return 0;
  } else {
    return currentValueLeft[0];
  }
}
/*
 * void initColorsDrift2() {
  int len = (pgm_read_byte_near(&colorCount[getPalleteNumber()]) * (getDwellFrames() + getTransitionFrames()));
  for (int j = 0; j < LENGTH; j++) {
    unsigned int r = random(0, len);
    scratch[((j * 3) / 2)] = r & 0xFF;
    if (j & 1) {
      scratch[(j / 2) * 3 + 2] &= ((r >> 4) & 0xF0);
    } else {
      scratch[(j / 2) * 3 + 2] = r >> 8;
    }
  }
}
 * 
 */
void updatePatternDrift2() {
  byte driftchance = 16 + currentValueRight[0] * 10;
  byte randinc = 255 - driftchance;
  byte randdec = driftchance;
  int len = (pgm_read_byte_near(&colorCount[getPalleteNumber()]) * (getDwellFrames() + getTransitionFrames()));
  for (byte i = 0; i < LENGTH; i+=2) {
    unsigned int rand = random(65535);
    unsigned int f1=scratch[(i*3)>>1]+((scratch[(i>>1)*3+2]&0x0F)<<8);
    unsigned int f2=scratch[1+((i*3)>>1)]+((scratch[(i>>1)*3+2]&0xF0)<<4);
    if ((byte)rand > randinc) {
      f1++;
      if (f1>len){f1=0;}
    } else if ((byte)rand < randdec) {
      f1--;
      if (f1>len){f1=len;}
    }
    rand=rand>>8;
    if (rand > randinc) {
      f2++;
      if (f2>len){f2=0;}
    } else if (rand < randdec) {
      f2--;
      if (f2>len){f2=len;}
    }
    scratch[(i*3)>>1]=f1&0xFF;
    scratch[1+((i*3)>>1)]=f2&0xFF;
    scratch[(i>>1)*3+2]=(f1>>8)&((f2>>4)&0xF0);
    getDrift2Colors(&pixels[i*3], &pixels[i*3]+1, &pixels[i*3]+2, f1);
    getDrift2Colors(&pixels[i*3]+3, &pixels[i*3]+4, &pixels[i*3]+5, f2);
  }
}

void getModeColors(byte * r, byte * g, byte * b, unsigned long fnumber) {
  unsigned int dwellFrames = getDwellFrames();
  unsigned int transitionFrames = getTransitionFrames();
  byte colors = getPalleteNumber();
  unsigned long tem = ((currentValueRight[currentMode == 4 ? 2 : 3] ? 0 : LENGTH) + fnumber) % (pgm_read_byte_near(&colorCount[colors]) * (dwellFrames + transitionFrames));
  unsigned int cyclepos = tem % (dwellFrames + transitionFrames);
  byte cyclenum = tem / (dwellFrames + transitionFrames);
  if (cyclepos < dwellFrames) {
    *r = pgm_read_byte_near(&colorPallete[colors][cyclenum][0]);
    *g = pgm_read_byte_near(&colorPallete[colors][cyclenum][1]);
    *b = pgm_read_byte_near(&colorPallete[colors][cyclenum][2]);
    return;
  } else {
    cyclepos -= dwellFrames;
    byte m = ((cyclenum + 1) >= pgm_read_byte_near(&colorCount[colors])) ? 0 : cyclenum + 1;
    float ratio = ((float)cyclepos) / transitionFrames;
    if (ratio > 1.001 || ratio < 0.0) {
      Serial.print(F("ERROR: ratio out of range"));
      Serial.println(ratio);
      Serial.flush();
    }
    *r = 0.5 + (pgm_read_byte_near(&colorPallete[colors][m][0]) * ratio) + (pgm_read_byte_near(&colorPallete[colors][cyclenum][0]) * (1 - ratio));
    *g = 0.5 + (pgm_read_byte_near(&colorPallete[colors][m][1]) * ratio) + (pgm_read_byte_near(&colorPallete[colors][cyclenum][1]) * (1 - ratio));
    *b = 0.5 + (pgm_read_byte_near(&colorPallete[colors][m][2]) * ratio) + (pgm_read_byte_near(&colorPallete[colors][cyclenum][2]) * (1 - ratio));
  }
}

byte getModeRatio(unsigned int fnumber) {
  {
    float ratio = ((float)fnumber) / getTransitionFrames();
    if (ratio > 1.001 || ratio < 0.0) {
      Serial.print(F("ERROR: ratio out of range"));
      Serial.println(ratio);
      Serial.flush();
    }

    return (byte)(ratio * 255.5);
  }
}

void getDrift2Colors(byte * r, byte * g, byte * b, unsigned long fnumber) {
  unsigned int dwellFrames = getDwellFrames();
  unsigned int transitionFrames = getTransitionFrames();
  byte colors = getPalleteNumber();
  unsigned long tem = ((currentValueRight[currentMode == 4 ? 2 : 3] ? 0 : LENGTH) + fnumber) % (pgm_read_byte_near(&colorCount[colors]) * (dwellFrames + transitionFrames));
  unsigned int cyclepos = tem % (dwellFrames + transitionFrames);
  byte cyclenum = tem / (dwellFrames + transitionFrames);
  if (cyclepos < dwellFrames) {
    *r = pgm_read_byte_near(&colorPallete[colors][cyclenum][0]);
    *g = pgm_read_byte_near(&colorPallete[colors][cyclenum][1]);
    *b = pgm_read_byte_near(&colorPallete[colors][cyclenum][2]);
    return;
  } else {
    cyclepos -= dwellFrames;
    byte m = ((cyclenum + 1) >= pgm_read_byte_near(&colorCount[colors])) ? 0 : cyclenum + 1;
    unsigned int start = (LENGTH * 3 + 1) >>1 ;
    byte ratio=scratch[start+cyclepos];
    *r = (((unsigned int)(pgm_read_byte_near(&colorPallete[colors][m][0]) * ratio)) + (pgm_read_byte_near(&colorPallete[colors][cyclenum][0]) * (255 - ratio)))>>8;
    *g = (((unsigned int)(pgm_read_byte_near(&colorPallete[colors][m][1]) * ratio)) + (pgm_read_byte_near(&colorPallete[colors][cyclenum][1]) * (255 - ratio)))>>8;
    *b = (((unsigned int)(pgm_read_byte_near(&colorPallete[colors][m][2]) * ratio)) + (pgm_read_byte_near(&colorPallete[colors][cyclenum][2]) * (255 - ratio)))>>8;
  }
}

/*
   Below this are pattern handlers that have not been updated since getModeColors()
*/
void updatePatternDrift() {
  byte driftchance = 16 + currentValueRight[0] * 10;
  byte randinc = 255 - driftchance;
  byte randdec = driftchance;
  for (unsigned int i = 0; i < (LENGTH * 3); i++) {
    byte tem = i % 3;
    tem *= 2;
    byte rand = random(255);
    if (rand > (pixels[i] > 32 ? randinc : (randinc + driftchance / 2)) && (pixels[i] < getLeftVal(currentValueLeft[tem + 1]))) {
      if (pixels[i] > 128 && pixels[i] < 254) {
        pixels[i] += 2;
      } else {
        pixels[i]++;
      }
    } else if (rand < (pixels[i] > 32 ? randdec : (randdec - driftchance / 2)) && (pixels[i] > getLeftVal(currentValueLeft[tem]))) {
      if (pixels[i] > 128) {
        pixels[i] -= 2;
      } else {
        pixels[i]--;
      }
    }
  }
}

void updatePatternPulse() {
  for (int  i = 0; i < (LENGTH * 3) - 2; i += 3) {
    byte max_r = (scratch[i] & 0x7C) >> 2;
    byte max_g = (((scratch[i] & 0x02) << 3) | ((scratch[i + 1] >> 5))); //dont need to mask the low 5 bits here because we're just pushing them off the edge
    byte max_b = (scratch[i + 1]) & 0x1F;
    byte speed = 1 + (scratch[i + 2] >> 6);
    byte bright = scratch[i + 2] & 0x3F;
    byte dir = (scratch[i] >> 7);
    if (!(max_r + max_b + max_g)) { // need to consider generating new target
      if (random(0, (currentValueRight[0] + 2) * 4 * pgm_read_byte_near(&maxValueRight[currentMode][1])) < currentValueRight[1]) {
        max_r = random(currentValueLeft[0], currentValueLeft[1]);
        max_g = random(currentValueLeft[2], currentValueLeft[3]);
        max_b = random(currentValueLeft[4], currentValueLeft[5]);
        speed = random(0, 3);
        bright = 0;
        dir = 0;
      }
      pixels[i] = 0;
      pixels[i + 1] = 0;
      pixels[i + 2] = 0;
    } else {
      if (!(frameNumber % (speed))) {

        if (dir && !bright) {
          dir = 0;
          pixels[i] = 0;
          pixels[i + 1] = 0;
          pixels[i + 2] = 0;
          max_r = 0;
          max_g = 0;
          max_b = 0;
        } else {
          byte nbright = pgm_read_byte_near(&pulseBrightnessTable[bright]);
          //byte nbright = bright << 2;
          pixels[i] = map(nbright, 0, 255, 0, getLeftVal(max_r));
          pixels[i + 1] = map(nbright, 0, 255, 0, getLeftVal(max_g));
          pixels[i + 2] = map(nbright, 0, 255, 0, getLeftVal(max_b));
          if (dir) {
            bright--;
          } else {
            if (bright == 63) {
              bright--;
              dir = 1;
            } else {
              bright++;
            }
          }
        }
      }
    }
    scratch[i] = (dir << 7) | (max_r << 2) | (max_g >> 3);
    scratch[i + 1] = (max_g << 5) | max_b;
    scratch[i + 2] = ((speed - 1) << 6) | bright;
  }
}

void updatePatternChase() {
  static byte nextColorAt = 0;
  static byte r;
  static byte g;
  static byte b;
  if (!nextColorAt) {
    r = random(getLeftVal(currentValueLeft[0]), getLeftVal(currentValueLeft[3]));
    g = random(getLeftVal(currentValueLeft[1]), getLeftVal(currentValueLeft[4]));
    b = random(getLeftVal(currentValueLeft[2]), getLeftVal(currentValueLeft[5]));
    nextColorAt = random(5, 5 + (currentValueRight[1] * 2));
  } else {
    nextColorAt--;
  }
  if (currentValueRight[2]) { //reverse
    for (unsigned int i = 0; i < ((LENGTH - 1) * 3); i++) {
      pixels [i] = pixels[i + 3];
    }
    pixels[(LENGTH * 3) - 3] = r;
    pixels[(LENGTH * 3) - 2] = g;
    pixels[(LENGTH * 3) - 1] = b;
  } else {//forward
    for (unsigned int i = ((LENGTH) * 3) - 1; i > 2; i--) {
      pixels [i] = pixels[i - 3];
    }
    pixels[0] = r;
    pixels[1] = g;
    pixels[2] = b;
  }
}


void setupPins() {
  pinMode(LEDPIN, OUTPUT);
  pinMode(ENC1_PINA, INPUT_PULLUP);
  pinMode(ENC1_PINB, INPUT_PULLUP);
  pinMode(ENC1_BTN, INPUT_PULLUP);
  pinMode(ENC2_PINA, INPUT_PULLUP);
  pinMode(ENC2_PINB, INPUT_PULLUP);
  pinMode(ENC2_BTN, INPUT_PULLUP);
  pinMode(MODE_BTN, INPUT_PULLUP);
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
}

void setupPCINT() {
  PCMSK1 = 0x0F;
  PCICR = 2;
}

// ISR based on: https://www.circuitsathome.com/mcu/rotary-encoder-interrupt-service-routine-for-avr-micros/
// by Oleg Mazurov
ISR(PCINT1_vect)
{
  static uint8_t old_ABl = 3;  //lookup table index
  static int8_t enclval = 0;   //encoder value
  static uint8_t old_ABr = 3;  //lookup table index
  static int8_t encrval = 0;   //encoder value
  //static const int8_t enc_states [] PROGMEM = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};  //encoder lookup table
  static const int8_t enc_states [] PROGMEM = {0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0}; // reversed encoder table
  old_ABl <<= 2; //remember previous state
  old_ABr <<= 2; //remember previous state
  old_ABl |= ( PINC & 0x03 );
  old_ABr |= (( PINC & 0x0C ) >> 2);
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
  TIFR1 = bit (ICF1) | bit (TOV1);  // clear flags so we don't get a bogus interrupt
  TCNT1 = 0;          // Counter to zero
  TIMSK1 = 1 << ICIE1; // interrupt on Timer 1 input capture
  // start Timer 1, prescalar of 8, edge select on falling edge
  TCCR1B =  ((F_CPU == 1000000L) ? (1 << CS10) : (1 << CS11)) | 1 << ICNC1; //prescalar 8 except at 1mhz, where we use prescalar of 1, noise cancler active
  //ready to rock and roll
}

byte handleReceive() {
  if (gotMessage) {
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

    memcpy(recvMessage, rxBuffer, 32);
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
  memset(rxBuffer, 0, 32);
  gotMessage = 0;
  TIMSK1 = 1 << ICIE1;
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

ISR (TIMER1_CAPT_vect)
{
  static unsigned long lasttime = 0;
  unsigned int newTime = ICR1; //immediately get the ICR value
  byte state = (RX_PIN_STATE);
  TCCR1B = state ? (1 << CS11 | 1 << ICNC1) : (1 << CS11 | 1 << ICNC1 | 1 << ICES1); //and set edge
  unsigned int duration = newTime - lasttime;
  lasttime = newTime;
  if (state) {
    if (receiving) {
      if (duration > rxLowMax) {
        receiving = 0;
        bitnum = 0; // reset to bit zero
        memset(rxBuffer, 0, 32); //clear buffer
      }
    } else {
      if (duration > rxSyncMin && duration < rxSyncMax) {
        receiving = 1;
      }
    }
  } else {
    if (receiving) {
      if (duration > rxZeroMin && duration < rxZeroMax) {
        dataIn = dataIn << 1;
      } else if (duration > rxOneMin && duration < rxOneMax) {
        dataIn = (dataIn << 1) + 1;
      } else {
        receiving = 0;
        bitnum = 0; // reset to bit zero
        memset(rxBuffer, 0, 32); //clear buffer
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
        receiving = 0;
        gotMessage = 1;
        TIMSK1 = 0; //turn off input capture;
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
