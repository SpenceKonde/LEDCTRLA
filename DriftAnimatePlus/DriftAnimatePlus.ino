#include <Adafruit_NeoPixel_Static.h>
#include <Wire.h>

// UI + encoder involved globals 
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
