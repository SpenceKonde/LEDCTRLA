// NeoPixel simple sketch (c) 2013 Shae Erisson, adapted to tinyNeoPixel library by Spence Konde 2019.
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library

#include <tinyNeoPixel_Static.h>


// Which pin on the Arduino is connected to the NeoPixels?
// This example uses pin 3, which is on PORTA on all megaTinyCore boards, so default menu option will work.
#define PIN            0

// How many NeoPixels are attached to the Arduino?
#define OUTERLEDS 18
#define OUTERCHAN 4
#define MIDLEDS 12
#define MIDCHAN 3
#define INNERLEDS 7
#define INNERCHAN 4
#define BUFFUSED ((OUTERLEDS*OUTERCHAN)+(INNERLEDS*INNERCHAN)+(MIDLEDS*MIDCHAN))
#define NUMPIXELS  ((BUFFUSED/3)+((BUFFUSED%3)?1:0)+1)

//index of pixels[] with first pixel of outer ring
#define OUTERSTART 0
#define OUTEREND ((OUTERLEDS*OUTERCHAN)-1)
#define MIDSTART OUTEREND+1
#define MIDEND (MIDSTART+(MIDLEDS*MIDCHAN)-1)
#define INNERSTART MIDEND+1
#define INNEREND BUFFUSED-1

// Since this is for the static version of the library, we need to supply the pixel array
// This saves space by eliminating use of malloc() and free(), and makes the RAM used for
// the frame buffer show up when the sketch is compiled.

byte pixels[NUMPIXELS * 3];

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.

tinyNeoPixel leds = tinyNeoPixel(NUMPIXELS, PIN, NEO_GRB, pixels);

int delayval = 250; // delay for half a second

void setup() {
  pinMode(PIN, OUTPUT);
  /*
  Serial.begin(9600);
  delay(100);
  
  Serial.println(NUMPIXELS);
  Serial.println(BUFFUSED);
  Serial.println(OUTERSTART);
  Serial.println(OUTEREND);
  Serial.println(MIDSTART);
  Serial.println(MIDEND);
  Serial.println(INNERSTART);
  Serial.println(INNEREND);
  */
  //with tinyNeoPixel_Static, you need to set pinMode yourself. This means you can eliminate pinMode()
  //and replace with direct port writes to save a couple hundred bytes in sketch size (note that this
  //savings is only present when you eliminate *all* references to pinMode).
  //leds.begin() not needed on tinyNeoPixel
}

void setOuterAll(byte r, byte g, byte b, byte w = 0) {
  int i = OUTERSTART;
  while (i < OUTEREND) {
    pixels[i++] = g;
    pixels[i++] = r;
    pixels[i++] = b;
    if (OUTERCHAN == 4) {
      pixels[i++] = w;
    }
  }
}
void setInnerAll(byte r, byte g, byte b, byte w = 0) {
  
  for (byte i = INNERSTART;i < INNEREND;i+=INNERCHAN) {
    pixels[i] = g;
    pixels[i+1] = r;
    pixels[i+2] = b;
    if (INNERCHAN == 4) {
      pixels[i+3] = w;
    }
  }
}
void setMidAll(byte r, byte g, byte b, byte w = 0) {
  byte i = MIDSTART;
  while (i < MIDEND) {
    pixels[i++] = g;
    pixels[i++] = r;
    pixels[i++] = b;
    if (MIDCHAN == 4) {
      pixels[i++] = w;
    }
  }
}

void loop() {
  
  setMidAll(0,0,0,0);
  setOuterAll(255,0,0,0);
  setInnerAll(255,0,0,0);
  leds.show();
  delay(2000);
  setMidAll(0,0,0,0);
  setOuterAll(0,255,0,0);
  setInnerAll(0,255,0,0);
  leds.show();
  delay(2000);
  setMidAll(0,0,0,0);
  setOuterAll(0,0,255,0);
  setInnerAll(0,0,255,0);
  leds.show();
  delay(2000);
  setMidAll(0,0,0,0);
  setOuterAll(0,0,0,0);
  setInnerAll(0,0,0,0);
  leds.show();
  delay(2000);
}
