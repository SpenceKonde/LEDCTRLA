#include <Adafruit_NeoPixel_Static.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// UI + encoder involved globals
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define ENC1_PINA 14
#define ENC1_PINB 15
#define ENC2_PINA 16
#define ENC2_PINB 17
#define ENC1_BTN 3
#define ENC2_BTN 4
#define MODE_BTN 2
#define LDR_PIN A6


volatile byte lastEncPins = 0;
volatile byte currentSettingLeft = 0;
volatile byte currentSettingRight = 0;

byte currentMode = 0;
byte currentSetting = 0;



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
  setupPins();
  setupPCINT();
  //Serial.begin(115200);
  target[random(0,599)]=20;
  param[random(0,199)]=5;
 delay(target[random(0,599)]);
  delay(param[random(0,199)]);
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
}

void processRFPacket(byte rlen) {

}

void handleUI() {

}

void handleLCD() {

}

void updatePattern() {
  
}


void setupPins() {
  pinMode(LEDPIN,OUTPUT);
  pinMode(ENC1_PINA,INPUT_PULLUP);
  pinMode(ENC1_PINB,INPUT_PULLUP);
  pinMode(ENC1_BTN,INPUT_PULLUP);
  pinMode(ENC2_PINA,INPUT_PULLUP);
  pinMode(ENC2_PINB,INPUT_PULLUP);
  pinMode(ENC2_BTN,INPUT_PULLUP);
}

void setupPCINT(){
  PCMSK1=0x0F;
  PCICR=2;
}
ISR(PCINT1_vect) {
  byte newread=PINC&0x0F;
  
}

