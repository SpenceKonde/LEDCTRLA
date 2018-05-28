#include <Adafruit_NeoPixel_Static.h>
#include <avr/pgmspace.h>

// UI + encoder involved globals
//LiquidCrystal_I2C lcd(0x27, 16, 2);


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
const char mode0Name[] PROGMEM = " SOLID  ";
const char mode1Name[] PROGMEM = "DRIFTING";
const char mode2Name[] PROGMEM = " COMETS ";
const char mode3Name[] PROGMEM = " PULSE  ";
const char mode4Name[] PROGMEM = " RAINBOW";
const char mode5Name[] PROGMEM = "  DOTS  ";
const char mode6Name[] PROGMEM = "  FADE  ";
const char mode7Name[] PROGMEM = "  WAVE  ";
const char mode8Name[] PROGMEM = " CHASE  ";


//Names of settings by mode
const char * const modesL[][8] PROGMEM = {
  {mode0L0, mode0L1, mode0L2},
  {mode1L0, mode1L1, mode1L2, mode1L3, mode1L4, mode1L5},
  {mode1L0, mode1L1, mode1L2, mode1L3, mode1L4, mode1L5},
  {mode1L0, mode1L1, mode1L2, mode1L3, mode1L4, mode1L5},
  {mode1L0, mode1L1, mode1L2, mode1L3, mode1L4, mode1L5},
  {mode1L0, mode1L1, mode1L2, mode1L3, mode1L4, mode1L5},
  {mode1L0, mode1L2, mode1L4, mode1L1, mode1L3, mode1L5}, //different order!
  {mode1L0, mode1L2, mode1L4, mode1L1, mode1L3, mode1L5}, //different order!
  {mode1L0, mode1L1, mode1L2, mode1L3, mode1L4, mode1L5}

};

const char * const modesR[][8] PROGMEM = {
  {mode0R0},
  {mode1R0, mode1R1},
  {mode1R0, mode1R1, mode2R2},
  {mode1R0, mode5R1},
  {mode1R0, mode2R2, mode4R2},
  {mode1R0, mode5R1, mode4R2},
  {mode1R0},
  {mode1R0, mode2R2, mode4R2},
  {mode1R0, mode2R2, mode4R2}

};

// names of modes
const char * const modeNames[] PROGMEM = {mode0Name, mode1Name, mode2Name, mode3Name, mode4Name, mode5Name,mode6Name,mode7Name,mode8Name};

#define COLORTABLEMAX 31

//max and default settings controlled by left knob. 26 is special, it indicates to use the leftValues array
const byte maxValueLeft[][8] PROGMEM = {
  {COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX},
  {COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX},
  {COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX},
  {COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX},
  {COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX},
  {COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX},
  {COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX},
  {COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX},
  {COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX}
};
const byte defaultValueLeft[][8] PROGMEM = { //255 is special - indicates to pick random value.
  {255, 255, 255},
  {0, COLORTABLEMAX, 0, COLORTABLEMAX, 0, COLORTABLEMAX},
  {0, COLORTABLEMAX, 0, COLORTABLEMAX, 0, COLORTABLEMAX},
  {0, COLORTABLEMAX, 0, COLORTABLEMAX, 0, COLORTABLEMAX},
  {0, COLORTABLEMAX, 0, COLORTABLEMAX, 0, COLORTABLEMAX},
  {0, COLORTABLEMAX, 0, COLORTABLEMAX, 0, COLORTABLEMAX},
  {255,255, 255, 255, 255, 255},
  {255,255, 255, 255, 255, 255},
  {0, COLORTABLEMAX, 0, COLORTABLEMAX, 0, COLORTABLEMAX}
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
  {10, 7, 1},
  {10, 20, 1}
};
const byte defaultValueRight[][8] PROGMEM = {
  {0},
  {5},
  {10, 10, 5},
  {5, 5},
  {5, 10, 0},
  {5, 10, 0},
  {5},
  {5, 4, 0},
  {5, 10, 0}
};
const byte maxSetting[][2] PROGMEM = {
  {2, 0}, //solid
  {5, 0}, //drift
  {5, 2}, //comets
  {5, 1}, //pulse
  {5, 2}, //rainbow
  {5, 2}, //dots
  {5, 0}, //fade
  {5, 2}, //wave
  {5, 2} //chase
};

const byte maxMode = 8;

const byte pulseBrightnessTable[] PROGMEM = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 31, 34, 37, 40, 43, 46, 49, 52, 55, 59, 63, 67, 71, 75, 79, 83, 87, 92, 97, 102, 107, 112, 117, 122, 127, 133, 139, 145, 151, 157, 163, 169, 175, 182, 189, 196, 203, 210, 217, 224, 231, 239, 247, 255};

volatile byte lastEncPins = 0;
volatile byte currentSettingLeft = 0;
volatile byte currentSettingRight = 0;
volatile byte currentValueLeft[] = {0, 0, 0, 0, 0, 0, 0, 0};
volatile byte currentValueRight[] = {0, 0, 0, 0, 0, 0, 0, 0};
volatile byte UIChanged = 7;
volatile unsigned long lastRFUpdateAt=0;

byte currentMode = 0;

volatile unsigned long lastUserAction = 0;

//animation related globals
#define LENGTH 11
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
  setupPins();
  setupRF();
  Serial.begin(115200);
  delay(2000);
}

void loop() {
  byte rlen = handleReceive();
  if (rlen) {
    processRFPacket(rlen);
  } else if ((!receiving)  && (millis() - lastRFMsgAt > 100)) { //will be if not receiving, but we don't know where we're saving the RXing status.
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
  } else if (currentMode==3) {
    return 30 + 10 * (pgm_read_byte_near(&maxValueRight[currentMode][0]) - currentValueRight[0]);
  }
  return 30 + 20 * (pgm_read_byte_near(&maxValueRight[currentMode][0]) - currentValueRight[0]);

}

byte getLeftVal(byte t) {
  if (pgm_read_byte_near(&maxValueLeft[currentMode][currentSettingLeft]) == COLORTABLEMAX) {
    if (t>COLORTABLEMAX) t=COLORTABLEMAX;
    return pgm_read_byte_near(&leftValues[t]);
  }
  return t;
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
    UIChanged = 7;
  }
}


void advanceMode() {
  if (currentMode >= maxMode) {
    setMode(0);
  } else {
    if (currentMode==1) currentMode++;
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
  // start with the first setting selected, in case we had a setting now out of index.
  frameNumber = 0;
  currentSettingLeft = 0;
  currentSettingRight = 0;
}


void updatePattern() {
  if (currentMode == 0) {
    for (unsigned int i = 0; i < LENGTH * 3; i++) {
      pixels[i] = getLeftVal(currentValueLeft[i % 3]);
    }
    //pixels[0]=255;
    pixels[1]=0;
    pixels[2]=0;
    pixels[3]=0;
    //pixels[4]=255;
    pixels[5]=0;
    pixels[6]=0;
    //pixels[7]=255;
    pixels[8]=0;
    pixels[24]=255;
    pixels[25]=255;
    pixels[26]=255;
    pixels[29]=255;
    
  } else if (currentMode == 1) {
    updatePatternDrift();
  } else if (currentMode == 3) {
    updatePatternPulse();
  } else if (currentMode == 4) {
    updatePatternRainbow();
  } else if (currentMode == 5) {
    updatePatternDots();
  } else if (currentMode == 6) {
    updatePatternFade();
  } else if (currentMode == 7) {
    updatePatternWave();
  } else if (currentMode == 8) {
    updatePatternChase();
  } else {
    setMode(0);
  }
  frameNumber++;
}

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

void updatePatternDots() {
  if (currentValueRight[2]) {//reverse
    for (unsigned int i = 0; i < ((LENGTH - 1) * 3); i++) {
      pixels [i] = pixels[i + 3];
    }
    if (!(frameNumber % (13 - currentValueRight[1]))) {
      pixels[(3 * LENGTH) - 3] = random(getLeftVal(currentValueLeft[0]), getLeftVal(currentValueLeft[1]));
      pixels[(3 * LENGTH) - 2] = random(getLeftVal(currentValueLeft[2]), getLeftVal(currentValueLeft[3]));
      pixels[(3 * LENGTH) - 1] = random(getLeftVal(currentValueLeft[4]), getLeftVal(currentValueLeft[5]));
    } else {
      pixels[LENGTH - 3] = 0;
      pixels[LENGTH - 2] = 0;
      pixels[LENGTH - 1] = 0;
    }
  } else { //forward
    for (unsigned int i = (LENGTH - 1) * 3; i > 2; i--) {
      pixels [i] = pixels[i - 3];
    }
    if (!(frameNumber % (13 - currentValueRight[1]))) {
      pixels[0] = random(getLeftVal(currentValueLeft[0]), getLeftVal(currentValueLeft[1]));
      pixels[1] = random(getLeftVal(currentValueLeft[2]), getLeftVal(currentValueLeft[3]));
      pixels[2] = random(getLeftVal(currentValueLeft[4]), getLeftVal(currentValueLeft[5]));
    } else {
      pixels[0] = 0;
      pixels[1] = 0;
      pixels[2] = 0;
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
    scratch[i + 2] = ((speed-1) << 6) | bright;
  }
}

void updatePatternFade() {
  static byte bright=0;
  if (bright&128) {
    if (bright&63) {
      bright--;
    } else {
      bright=0;
    }
  } else {
    if (bright>=63) {
      bright=0xBF;
    } else {
      bright++;
    }
  }
  byte nbright = pgm_read_byte_near(&pulseBrightnessTable[63&bright]);
  byte r = map(nbright, 0, 255, getLeftVal(currentValueLeft[0]), getLeftVal(currentValueLeft[3]));
  byte g = map(nbright, 0, 255, getLeftVal(currentValueLeft[1]), getLeftVal(currentValueLeft[4]));
  byte b = map(nbright, 0, 255, getLeftVal(currentValueLeft[2]), getLeftVal(currentValueLeft[5]));
  for (unsigned int i=0;i<(LENGTH*3);i+=3) {
    pixels[i]=r;
    pixels[i+1]=g;
    pixels[i+2]=b;
  }   
}

void updatePatternWave() {
  static byte bright=0;
    for (byte i=0;i<(1+pgm_read_byte_near(&maxValueRight[currentMode][1])-currentValueRight[1]);i++) { 
  if (bright&128) {
    if (bright&63) {
      bright--;
    } else {
      bright=0;
    }
  } else {
    if (bright>=63) {
      bright=0xBF;
    } else {
      bright++;
    }
  }
  }
  byte nbright = pgm_read_byte_near(&pulseBrightnessTable[63&bright]);
  byte r = map(nbright, 0, 255, getLeftVal(currentValueLeft[0]), getLeftVal(currentValueLeft[3]));
  byte g = map(nbright, 0, 255, getLeftVal(currentValueLeft[1]), getLeftVal(currentValueLeft[4]));
  byte b = map(nbright, 0, 255, getLeftVal(currentValueLeft[2]), getLeftVal(currentValueLeft[5]));
  if (currentValueRight[2]) { //reverse
    for (unsigned int i = 0; i < ((LENGTH - 1) * 3); i++) {
      pixels [i] = pixels[i + 3];
    }
    pixels[(LENGTH * 3) - 3] = r;
    pixels[(LENGTH * 3) - 2] = g;
    pixels[(LENGTH * 3) - 1] = b;
  } else {//forward
    for (unsigned int i = ((LENGTH) * 3)-1; i > 2; i--) {
      pixels [i] = pixels[i - 3];
    }
    pixels[0] = r;
    pixels[1] = g;
    pixels[2] = b;
  }
}

void updatePatternChase() {
  static byte nextColorAt=0;
  static byte r;
  static byte g;
  static byte b;
  if (!nextColorAt){
   r = random(getLeftVal(currentValueLeft[0]), getLeftVal(currentValueLeft[3]));
   g = random(getLeftVal(currentValueLeft[1]), getLeftVal(currentValueLeft[4]));
   b = random(getLeftVal(currentValueLeft[2]), getLeftVal(currentValueLeft[5]));
   nextColorAt=random(5,5+(currentValueRight[1]*2)); 
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
    for (unsigned int i = ((LENGTH) * 3)-1; i > 2; i--) {
      pixels [i] = pixels[i - 3];
    }
    pixels[0] = r;
    pixels[1] = g;
    pixels[2] = b;
  }
}

void updatePatternRainbow() {
  byte maxVal = COLORTABLEMAX;
  byte l = 9 + (6 * currentValueRight[1]);
  byte f = ((currentValueRight[2] ? 0 : LENGTH) + frameNumber) % (3 * l); //if in forward direction, add 200, otherwise don't - this keeps the color from skipping when reversing the direction.
  byte r = 0;
  byte g = 0;
  byte b = 0;
  byte fal = 0.0;
  byte rise = 0.0;
  if (f % l) {

    float tem = f % l;
    tem /= l;
    float temr = (tem * (getLeftVal(currentValueLeft[1]) - getLeftVal(currentValueLeft[0])) + getLeftVal(currentValueLeft[0]));
    float temg = (tem * (getLeftVal(currentValueLeft[3]) - getLeftVal(currentValueLeft[2])) + getLeftVal(currentValueLeft[2]));
    float temb = (tem * (getLeftVal(currentValueLeft[5]) - getLeftVal(currentValueLeft[4])) + getLeftVal(currentValueLeft[4]));
    if (f < l) { // sector 1 - green rising red falling
      g = temg + 0.5;
      temr = 255 - temr;
      r = temr + 0.5;
      b = getLeftVal(currentValueLeft[4]);
    } else if (f < 2 * l) { // sector 2 - blue rising green falling
      temg = 255 - temg;
      g = temg + 0.5;
      b = temb + 0.5;
      r = getLeftVal(currentValueLeft[0]);
    } else { // sector 3 - red rising blue falling
      temb = 255 - temb;
      b = temb + 0.5;
      r = temr + 0.5;
      g = getLeftVal(currentValueLeft[2]);
    }
  } else {
    if (f == 0) {
      r = getLeftVal(currentValueLeft[1]);
      g = getLeftVal(currentValueLeft[2]);
      b = getLeftVal(currentValueLeft[4]);
    } else if (f == l) {
      r = getLeftVal(currentValueLeft[0]);
      g = getLeftVal(currentValueLeft[3]);
      b = getLeftVal(currentValueLeft[4]);
    } else {
      r = getLeftVal(currentValueLeft[0]);
      g = getLeftVal(currentValueLeft[2]);
      b = getLeftVal(currentValueLeft[5]);
    }
  }
  if (currentValueRight[2]) { //reverse
    for (unsigned int i = 0; i < ((LENGTH - 1) * 3); i++) {
      pixels [i] = pixels[i + 3];
    }
    pixels[(LENGTH * 3) - 3] = r;
    pixels[(LENGTH * 3) - 2] = g;
    pixels[(LENGTH * 3) - 1] = b;
  } else {//forward
    for (unsigned int i = ((LENGTH) * 3)-1; i > 2; i--) {
      pixels [i] = pixels[i - 3];
    }
    pixels[0] = r;
    pixels[1] = g;
    pixels[2] = b;
  }
}

void setupPins() {
  pinMode(LEDPIN, OUTPUT);
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
  if (bitnum > 4) {lastRFMsgAt=millis();}
  bitnum = 0;
  memset(rxBuffer, 0, 32);
  gotMessage = 0;
  TIMSK1 = 1 << ICIE1;
  return;
}

byte checkCSC() {
  byte rxchecksum = 0;
  byte rxchecksum2 = 0;
  byte rxc2;
  for (byte i = 0; i < pktLength >> 3; i++) {
    rxchecksum = rxchecksum ^ rxBuffer[i];
    rxc2 = rxchecksum2 & 128 ? 1 : 0;
    rxchecksum2 = (rxchecksum2 << 1 + rxc2)^rxBuffer[i];
  }
  if (pktLength >> 3 == 3) {
    rxchecksum = (rxchecksum & 0x0F) ^ (rxchecksum >> 4) ^ ((rxBuffer[3] & 0xF0) >> 4);
    rxchecksum2 = (rxchecksum2 & 0x0F) ^ (rxchecksum2 >> 4) ^ ((rxBuffer[3] & 0xF0) >> 4);
    if (rxchecksum == rxchecksum2)rxchecksum2++;
    return (rxBuffer[3] & 0x0F) == rxchecksum ? 1 : ((rxBuffer[3] & 0x0F) == rxchecksum2 ) ? 2 : 0;
  } else {
    if (rxchecksum == rxchecksum2)rxchecksum2++;
    return ((rxBuffer[pktLength >> 3] == rxchecksum) ? 1 : ((rxBuffer[bitnum >> 3] == rxchecksum2 ) ? 2 : 0));
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

