#include <tinyNeoPixel_Static.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_pinIO.h>
#include <avr/pgmspace.h>
#include <util/crc16.h>

hd44780_pinIO lcd(LCD_RS, LCD_RW, LCD_EN, LCD_DATA4, LCD_DATA5, LCD_DATA6, LCD_DATA7);



#include <EEPROM.h>
#include "LightCtrl_RevF.h"
#include "Colors.h"
#include "Modes.h"
#include "HWSpecs.h"


#define MODE_DRIFT2 9






volatile byte lastEncPins             = 0;
volatile byte currentSettingLeft      = 0;
volatile byte currentSettingRight     = 0;
volatile byte currentValueLeft[10]    = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
volatile byte currentValueRight[10]   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
volatile byte UIChanged               = 7;
volatile unsigned long lastRFUpdateAt = 0;

mode_t * CurrentMode=&modes[0];
colorset_t * CurrentColors            = getColorSetPtr();

volatile unsigned long lastUserAction = 0;

//animation related globals
#define LENGTH 500
uint16_t         frameDelay           = 30;
uint32_t         lastFrameAt              ;
uint8_t  pixels  [LENGTH * 3]             ;
uint8_t scratch  [LENGTH * 3]             ;
scratchsector_t* [LENGTH / 50]            ;

uint32_t        frameNumber           = 0 ;

tinyNeoPixel leds = tinyNeoPixel(LENGTH, LEDPIN, NEO_GRB, pixels);



void setup() {
  //_PROTECTED_WRITE(CLKCTRL_OSCHFTUNE,CLKCTRL_OSCHFTUNE);
  setupPins();
  setupPCINT();
  //setupRF();
  Serial.swap(1);
  Serial.begin(115200);
  Serial.println("Hi, I started!");
  digitalWrite(LCD_BL_R, HIGH);
  digitalWrite(LCD_BL_G, HIGH);
  digitalWrite(LCD_BL_B, HIGH);
  pinMode(PIN_PD6, OUTPUT);
  digitalWrite(PIN_PD6, HIGH);
  delay(2000);
  lcd.clear();
  loadMode();
}

void loop() {

  static byte updated = 0;
  byte rlen = 0; // handleReceive();
  /*
    if (rlen) {
    processRFPacket(rlen);
    } else if ((!RFRX_NOW)  && (millis() - lastRFMsgAt > 100)) {

    }
  */
  //digitalWriteFast(INDICATE2, CHANGE);
  handleUI();
  handleLCD();
  if (!updated) {
    digitalWriteFast(INDICATE1, HIGH);
    updatePattern();
    digitalWriteFast(INDICATE1, LOW);
    updated = 1;
  } else if (millis() - lastFrameAt > getFrameDelay()) {
    lastFrameAt = millis();
    digitalWriteFast(INDICATE0, HIGH);
    updated = 0;
    leds.show();
    digitalWriteFast(INDICATE0, LOW);
  }
}

byte getFrameDelay() {
  if (currentMode == 1 || currentMode == 0) {
    return 80;
  } else if (currentMode == 3) {
    return 30 + 10 * currentMode->Adjust_ - currentValueRight[0];
  }
  return 30 + 20 * (&maxValueRight[currentMode][0]) - currentValueRight[0];

}

void processRFPacket(byte rlen) {
/*
  //+-byte vers = (rlen & 196) >> 6;
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
    delay(1000);
    UIChanged = 7;
  }
  */
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
  memset(scratch, 0, 3 * LENGTH);
  memset(pixels, 0, 3 * LENGTH);
  for (byte i = 0; i < 10; i++) { //set the current setting values to defaults
    if (defaultValueLeft[currentMode][i] == 255) {
      currentValueLeft[i] = random(maxValueLeft[currentMode][i]);
    } else {
      currentValueLeft[i] = Mode.mode->adjust_default[i];
    }
    if (defaultValueRight[currentMode][i] == 255) {
      currentValueRight[i] = random(maxValueRight[currentMode][i]);
    } else {
      currentValueRight[i] = currentMode][i];
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
  int len = (CurrentColors->colorcount * (getDwellFrames() + getTransitionFrames()));
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
  for (byte i = 0; i < 10; i++) {
    EEPROM.write(0x10 + i, currentValueLeft[i]);
    EEPROM.write(0x20 + i, currentValueRight[i]);
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
  currentSettingLeft = 0;
  currentSettingRight = 0;
}

void loadMode() {
  Serial.println("LoadMode Called");
  if (EEPROM.read(0x0F) != 255) {
    Serial.printHex(EEPROM.read(0x0F));
    currentMode = EEPROM.read(0x0F);
    for (byte i = 0; i < 8; i++) {
      currentValueLeft[i] = EEPROM.read(0x10 + i);
      currentValueRight[i] = EEPROM.read(0x18 + i);
    }
  }
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
    getModeColors(&r, &g, &b, random(0, CurrentColors->colorcount * (getTransitionFrames() + getDwellFrames())));
  } else {
    r = 0; g = 0; b = 0;
  }
  pushPixel(r, g, b, currentValueRight[2]);
}
// Supposed to be much faster.
// I am doubtful that this is correctly implementing the xorshift though.

/*
uint16_t rng(uint16_t seed) {
  static uint16_t y = 0;
  if (seed != 0) y += (seed && 0x1FFF); // seeded with a different number
  y ^= y << 2;
  y ^= y >> 7;
  y ^= y << 7;
  return (y);
}
*/

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
    pixels[i] = r;
    pixels[i + 1] = g;
    pixels[i + 2] = b;
  }
}

void pushPixel(byte r, byte g, byte b, bool dir) {
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
  if (currentMode == 4 || currentMode == 8) {
    return 2;
  } else {
    return 2 + (4 * currentValueRight[2]);
  }
  return 0; //TO DO
}
unsigned int getTransitionFrames() {
  if (currentMode == 4) {
    return 9 + (6 * currentValueRight[1]);
  } else if (currentMode == 8) {
    return 255;
  } else {
    return 3 * currentValueRight[1];
  }
}
byte getPalleteNumber() {
  if (currentMode == 4) {
    return 0;
  } else {
    return currentValueLeft[0];
  }
}
colorset_t * getColorSetPtr() {
  if (CurrentMode == 4) {
  return &ColorTable[0];
  }
  return &ColorTable[currentValueLeft[0]];
}
/*
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

*/
void updatePatternDrift2() {
  byte driftchance = 16 + currentValueRight[0] * 10;
  byte randinc = 255 - driftchance;
  byte randdec = driftchance;
  uint16_t len = (CurrentColors->colorcount) * (getDwellFrames() + getTransitionFrames());
  for (uint16_t i = 0; i < LENGTH; i += 2) {
    uint16_t rand = random(65535);
    uint16_t f1 = scratch[(i * 3) >> 1] + ((scratch[(i >> 1) * 3 + 2] & 0x0F) << 8);
    uint16_t f2 = scratch[1 + ((i * 3) >> 1)] + ((scratch[(i >> 1) * 3 + 2] & 0xF0) << 4);
    if ((byte)rand > randinc) {
      f1++;
      if (f1 > len) {
        f1 = 0;
      }
    } else if ((byte)rand < randdec) {
      f1--;
      if (f1 > len) {
        f1 = len;
      }
    }
    rand = rand >> 8;
    if (rand > randinc) {
      f2++;
      if (f2 > len) {
        f2 = 0;
      }
    } else if (rand < randdec) {
      f2--;
      if (f2 > len) {
        f2 = len;
      }
    }
    scratch[(i * 3) >> 1] = f1 & 0xFF;
    scratch[1 + ((i * 3) >> 1)] = f2 & 0xFF;
    scratch[(i >> 1) * 3 + 2] = (f1 >> 8) & ((f2 >> 4) & 0xF0);
    getDrift2Colors(&pixels[i * 3], &pixels[i * 3] + 1, &pixels[i * 3] + 2, f1);
    getDrift2Colors(&pixels[i * 3] + 3, &pixels[i * 3] + 4, &pixels[i * 3] + 5, f2);
  }
}

void getModeColors(byte * r, byte * g, byte * b, unsigned long fnumber) {
  unsigned int dwellFrames = getDwellFrames();
  unsigned int transitionFrames = getTransitionFrames();
  unsigned long tem = ((currentValueRight[currentMode == 4 ? 2 : 3] ? 0 : LENGTH) + fnumber) % (CurrentColors->colorcount * (dwellFrames + transitionFrames));
  unsigned int cyclepos = tem % (dwellFrames + transitionFrames);
  byte cyclenum = tem / (dwellFrames + transitionFrames);
  if (cyclepos < dwellFrames) {
    *r = CurrentColors->colors[cyclenum][0];
    *g = CurrentColors->colors[cyclenum][1];
    *b = CurrentColors->colors[cyclenum][2];
    return;
  } else {
    cyclepos -= dwellFrames;
    byte m = ((cyclenum + 1) >= CurrentColors->colorcount) ? 0 : cyclenum + 1;
    float ratio = ((float)cyclepos) / transitionFrames;
    if (ratio > 1.001 || ratio < 0.0) {
      Serial.print(F("ERROR: ratio out of range"));
      Serial.println(ratio);
      Serial.flush();
    }
    *r = 0.5 + (CurrentColors->colors[m][0] * ratio) + (CurrentColors->colors[cyclenum][0] * (1 - ratio));
    *g = 0.5 + (CurrentColors->colors[m][1] * ratio) + (CurrentColors->colors[cyclenum][1] * (1 - ratio));
    *b = 0.5 + (CurrentColors->colors[m][2] * ratio) + (CurrentColors->colors[cyclenum][2] * (1 - ratio));
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
  unsigned long tem = ((currentValueRight[currentMode == 4 ? 2 : 3] ? 0 : LENGTH) + fnumber) % (CurrentColors->colorcount * (dwellFrames + transitionFrames));
  unsigned int cyclepos = tem % (dwellFrames + transitionFrames);
  byte cyclenum = tem / (dwellFrames + transitionFrames);
  if (cyclepos < dwellFrames) {
    *r = CurrentColors->colors[cyclenum][0];
    *g = CurrentColors->colors[cyclenum][1];
    *b = CurrentColors->colors[cyclenum][2];
    return;
  } else {
    cyclepos -= dwellFrames;
    byte m = ((cyclenum + 1) >= CurrentColors->colorcount) ? 0 : cyclenum + 1;
    unsigned int start = (LENGTH * 3 + 1) >> 1 ;
    byte ratio = scratch[start + cyclepos];
    *g = (((unsigned int)(CurrentColors->colors[m][1] * ratio)) + (CurrentColors->colors[cyclenum][1] * (255 - ratio))) >> 8;
    *b = (((unsigned int)(CurrentColors->colors[m][2] * ratio)) + (CurrentColors->colors[cyclenum][2] * (255 - ratio))) >> 8;
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
