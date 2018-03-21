#include <Adafruit_NeoPixel_Static.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <avr/pgmspace.h>

// UI + encoder involved globals
LiquidCrystal_I2C lcd(0x3F, 16, 2);
#define ENC1_PINA 14
#define ENC1_PINB 15
#define ENC2_PINA 16
#define ENC2_PINB 17
#define ENC1_BTN 3
#define ENC2_BTN 4
#define MODE_BTN 2
#define LDR_PIN A6

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
const char mode5R1[] PROGMEM = "DENSITY";
const char mode0Name[] PROGMEM = "  SOLID ";
const char mode1Name[] PROGMEM = "DRIFTING";
const char mode2Name[] PROGMEM = " COMETS ";
const char mode3Name[] PROGMEM = "  PULSE ";
const char mode4Name[] PROGMEM = " RAINBOW";
const char mode5Name[] PROGMEM = "   DOTS ";


//Names of settings by mode
const char * const modesL[][8] PROGMEM = {
  {mode0L0, mode0L1, mode0L2},
  {mode1L0, mode1L1, mode1L2, mode1L3, mode1L4, mode1L5},
  {mode1L0, mode1L1, mode1L2, mode1L3, mode1L4, mode1L5},
  {mode1L0, mode1L1, mode1L2, mode1L3, mode1L4, mode1L5},
  {mode1L0, mode1L1, mode1L2, mode1L3, mode1L4, mode1L5},
  {mode1L0, mode1L1, mode1L2, mode1L3, mode1L4, mode1L5}

};

const char * const modesR[][8] PROGMEM = {
  {mode0R0},
  {mode1R0, mode1R1},
  {mode1R0, mode1R1, mode2R2},
  {mode1R0, mode1R1, mode2R2},
  {mode1R0, mode1R1},
  {mode1R0, mode5R1}

};

// names of modes
const char * const modeNames[] PROGMEM = {mode0Name, mode1Name, mode2Name, mode3Name, mode4Name, mode5Name};

//max and default settings controlled by left knob. 26 is special, it indicates to use the leftValues array
const byte maxValueLeft[][8] PROGMEM = {
  {26, 26, 26},
  {26, 26, 26, 26, 26, 26},
  {26, 26, 26, 26, 26, 26},
  {26, 26, 26, 26, 26, 26},
  {26, 26, 26, 26, 26, 26},
  {26, 26, 26, 26, 26, 26}
};
const byte defaultValueLeft[][8] PROGMEM = { //255 is special - indicates to pick random value.
  {255, 255, 255},
  {0, 26, 0, 26, 0, 26},
  {0, 26, 0, 26, 0, 26},
  {0, 26, 0, 26, 0, 26},
  {0, 26, 0, 26, 0, 26},
  {0, 26, 0, 26, 0, 26}
};

//if above max is 26, use this value - otherwise use raw value.
const byte leftValues[] PROGMEM = {0, 1, 2, 3, 4, 6, 8, 11, 14, 18, 23, 29, 36, 44, 53, 63, 74, 86, 99, 113, 128, 144, 162, 182, 205, 229, 255};

const byte maxValueRight[][8] PROGMEM = {
  {0},
  {10, 10},
  {10, 10, 10}
};
const byte defaultValueRight[][8] PROGMEM = {
  {0},
  {5},
  {5, 10, 5},
  {5, 10, 5},
  {5, 10},
  {5, 10}
};
const byte maxSetting[][2] PROGMEM = {
  {2, 0},
  {5, 0},
  {5, 2},
  {5, 2},
  {5, 1},
  {5, 1}
};

const byte maxMode = 5;

volatile byte lastEncPins = 0;
volatile byte currentSettingLeft = 0;
volatile byte currentSettingRight = 0;
volatile byte currentValueLeft[] = {0, 0, 0, 0, 0, 0, 0, 0};
volatile byte currentValueRight[] = {0, 0, 0, 0, 0, 0, 0, 0};
volatile byte UIChanged = 7;

byte currentMode = 0;

volatile unsigned long lastUserAction = 0;


//animation related globals
#define LENGTH 200
#define LEDPIN 10
unsigned int frameDelay = 30;
unsigned long lastFrameAt;
byte pixels[LENGTH * 3];
byte scratch[LENGTH * 3];
unsigned long frameNumber=0;
Adafruit_NeoPixel leds = Adafruit_NeoPixel(LENGTH, LEDPIN, NEO_GRB + NEO_KHZ800, pixels);

//RF related globals
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
  Serial.begin(9600);
  lcd.backlight();
  lcd.print("test");
  delay(1000);
}

void loop() {
  //byte rlen = handleReceive();
  byte rlen = 0; //debug
  if (rlen) {
    //processRFPacket(rlen);
  } else if (true) { //will be if not receiving, but we don't know where we're saving the RXing status.
    handleUI();
    handleLCD();
    if (millis() - lastFrameAt > frameDelay) {
      lastFrameAt = millis();
      updatePattern();
      leds.show();
    }
  }
  delay(1);
}

void processRFPacket(byte rlen) {

}



void advanceMode() {
  if (currentMode >= maxMode) {
    currentMode = 0;
  } else {
    currentMode++;
  }
  memset(scratch, 0, 600);
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
  currentSettingLeft = 0;
  currentSettingRight = 0;
}




void handleUI() {
  static byte lastBtnState = 7;
  static byte lastBtnBounceState = 7;
  static unsigned long lastBtnAt = 0;
  byte btnRead = (PIND & 0x1C) >> 2;
  if (!(btnRead == lastBtnBounceState)) { //debounce all buttons at once.
    lastBtnBounceState = btnRead;
    lastBtnAt = millis();
  } else {
    if (millis() - lastBtnAt > 50) { //has been stable for 50ms
      if (btnRead > lastBtnState) {
        //do nothing - was button being released
      } else {
        if ((!(btnRead & 1)) && (lastBtnState & 1)) {
          advanceMode();
          UIChanged |= 4;
        }
        if ((!(btnRead & 2)) && (lastBtnState & 2)) {
          if (currentSettingLeft >= pgm_read_byte_near(&maxSetting[currentMode][0])) {
            currentSettingLeft = 0;

          } else {
            currentSettingLeft++;

          }


          UIChanged |= 2;
        }
        if ((!(btnRead & 4)) && (lastBtnState & 4)) {
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
  if pgm_read_byte_near(&maxValueLeft[currentMode][currentSettingLeft] == 26) {
    return pgm_read_byte_near(&leftValues[t]);
  }
  return t;
}

void handleLCD() {
  byte uichg = 0;
  cli();
  if (!UIChanged) {
    sei();
    return;
  } else {
    uichg = UIChanged;
    UIChanged = 0;
    sei();
  }
  if (uichg & 6) { //if setting or mode has changed, redraw settings
    lcd.setCursor(0, 0);
    lcd.print(FLASH(modesL[currentMode][currentSettingLeft]));
    lcd.print(' ');
    lcd.print(FLASH(modesR[currentMode][currentSettingRight]));
  }
  if (uichg & 7) { //if mode, setting, or value has changed, redraw second line
    lcd.setCursor(0, 1);
    byte tval = getLeftVal(currentValueLeft[currentSettingLeft]);
    lcd.print(tval);
    lcd.print(' ');
    if (tval < 10) lcd.print(' ');
    lcd.setCursor(4, 1);
    lcd.print(FLASH(modeNames[currentMode]));
    lcd.setCursor(13, 1);
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


void updatePattern() {
  if (currentMode == 0) {
    for (unsigned int i = 0; i < LENGTH * 3; i++) {
      pixels[i] = getLeftVal(currentValueLeft[i % 3]);
    }
  } else if (currentMode == 1) {
    updatePatternDrift();
  } else if (currentMode == 4) {
    updatePatternRainbow();
  } else if (currentMode == 5) {
    updatePatternDots();
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
  
  for (unsigned int i=3; i<LENGTH*3;i++) {
    pixels [i]=pixels[i-3];
  }
  if (!(frameNumber%(currentSettingRight[1]+2))) {
    pixels[0]=random(getLeftVal(currentSettingLeft[0]),getLeftVal(currentSettingLeft[1)]);
    pixels[1]=random(getLeftVal(currentSettingLeft[2]),getLeftVal(currentSettingLeft[3)]);
    pixels[2]=random(getLeftVal(currentSettingLeft[4]),getLeftVal(currentSettingLeft[5)]);
  }
}



void updatePatternRainbow() {
  for (unsigned int i=3; i<LENGTH*3;i++) {
    pixels [i]=pixels[i-3];
  }
  pixels[0]=0;
  pixels[1]=0;
  pixels[2]=0;
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
  /**/
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
    if ((currentMode == 1 || currentMode == 2) && currentSettingLeft < 6) {
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
