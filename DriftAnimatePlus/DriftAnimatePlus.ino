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

const char mode0L0[] PROGMEM="  RED  ";
const char mode0L1[] PROGMEM=" GREEN ";
const char mode0L2[] PROGMEM="  BLUE ";
const char mode0R0[] PROGMEM=" SPEED ";
const char mode0R1[] PROGMEM="BRIGHT ";
const char mode1R2[] PROGMEM="NUMBER ";
const char mode0Name[] PROGMEM="DRIFTING";
const char mode1Name[] PROGMEM=" COMETS ";


const char * const modesL[][8] PROGMEM ={
  {mode0L0,mode0L1,mode0L2},
  {mode0L0,mode0L1,mode0L2}
  
};

const char * const modesR[][8] PROGMEM ={
  {mode0R0,mode0R1},
  {mode0R0,mode0R1,mode1R2}
  
};

const char * const modeNames[] PROGMEM={mode0Name,mode1Name};

const byte maxValueLeft[][8] PROGMEM = {
  {10,10,10},
  {10,10,10}
};

const byte maxValueRight[][8] PROGMEM = {
  {10,10,10},
  {10,10,10}
};
const byte maxSetting[][2] PROGMEM = {
  {2,1},
  {2,2}
}

const byte maxMode=1;

volatile byte lastEncPins = 0;
volatile byte currentSettingLeft = 0;
volatile byte currentSettingRight = 0;
volatile byte currentValueLeft = 0;
volatile byte currentValueRight = 0;
volatile byte UIChanged=1;

byte currentMode = 0;

volatile unsigned long lastUserAction = 0;


//animation related globals
#define LENGTH 200
#define LEDPIN 10
unsigned int frameDelay = 30;
unsigned long lastFrameAt;
byte pixels[LENGTH * 3];
byte target[LENGTH * 3];
byte param[LENGTH];
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
  Serial.println("test");
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

void handleUI() {
  static byte lastBtnState=7;
  static byte lastBtnBounceState=7;
  static unsigned long lastBtnAt=0;
  byte btnRead=(PIND&0x1C)>>2;
  if (!(btnRead==lastBtnBounceState)) {
    lastBtnBounceState=btnRead;
    lastBtnAt=millis();
  } else {
    if (millis()-lastBtnAt > 50) {
      if (btn
    }
}

void handleLCD() {
  if (!UIChanged){return;}
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(FLASH(modesL[currentMode][currentSettingLeft]));
  lcd.print(' ');
  lcd.print(FLASH(modesR[currentMode][currentSettingRight]));
  //lcd.setCursor(currentValueLeft<10?2:(currentValueLeft>99?0:1),1);
  lcd.setCursor(0,1);
  lcd.print(currentValueLeft);
  lcd.setCursor(4,1);
  lcd.print(FLASH(modeNames[currentMode]));
  lcd.setCursor(currentValueRight<10?15:(currentValueLeft>99?13:14),1);
  lcd.print(currentValueRight);
  UIChanged=0;
}

void updatePattern() {

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
  static const int8_t enc_states [] PROGMEM = 
  {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};  //encoder lookup table
  /**/
  old_ABl <<=2;  //remember previous state
  old_ABr <<=2;  //remember previous state
  old_ABl |= ( PINC & 0x03 );
  old_ABr |= (( PINC & 0x0C )>>2);
  enclval += pgm_read_byte(&(enc_states[( old_ABl & 0x0f )]));
  encrval += pgm_read_byte(&(enc_states[( old_ABr & 0x0f )]));
  /* post "Navigation forward/reverse" event */
  if( enclval > 3 ) {  //four steps forward
    currentValueLeft++;
    if(currentValueLeft>pgm_read_byte_near(&maxValueLeft[currentMode][currentSettingLeft]))currentValueLeft=0;
    UIChanged=1;
    enclval = 0;
  }
  else if( enclval < -3 ) {  //four steps backwards
    currentValueLeft--;
    if(currentValueLeft==255)currentValueLeft=pgm_read_byte_near(&maxValueLeft[currentMode][currentSettingLeft]);
    UIChanged=1;
    enclval = 0;
  }
    if( encrval > 3 ) {  //four steps forward
    currentValueRight++;
    if(currentValueRight>pgm_read_byte_near(&maxValueRight[currentMode][currentSettingRight]))currentValueRight=0;
    UIChanged=1;
    encrval = 0;
  }
  else if( encrval < -3 ) {  //four steps backwards
    currentValueRight--;
    if(currentValueRight==255)currentValueRight=pgm_read_byte_near(&maxValueRight[currentMode][currentSettingRight]);
    UIChanged=1;
    encrval = 0;
  }
}
