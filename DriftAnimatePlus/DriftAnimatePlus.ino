#include <Adafruit_NeoPixel_Static.h>
#include <Wire.h>

// UI + encoder involved globals 
#define ENC1_PINA 
#define ENC1_PINB 
#define ENC2_PINA 
#define ENC2_PINB 
#define ENC1_BTN
#define ENC2_BTN
#define MODE_BTN


volatile byte lastEncPins=0;
volatile byte currentSettingLeft=0;
volatile byte currentSettingRight=0;

byte currentMode=0;
byte currentSetting=0;

volatile unsigned long lastUserAction=0;


//animation related globals
#define LENGTH 200;

unsigned long lastFrameAt
byte pixels[LENGTH*3];
byte target[LENGTH*3];
byte param[LENGTH];

//RF related globals
volatile byte rxBuffer[32];
byte recvMessage[32];


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

}

void loop() {
  byte rlen = handleReceive();
  if (rlen) {
    outputPacket(rlen);
  } else if (!rxing) {
    handleUI();
    handleLCD();
    if (millis() - lastFrameAt > frameDelay) {
      lastFrameAt=millis();
      updatePattern();
      leds.show();
  }
}
